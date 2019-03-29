#include "postgres.h"
#include "fmgr.h"
#include "access/hash.h"
#include "access/htup.h"
#include "catalog/pg_type.h"
#include "executor/tuptable.h"
#include "funcapi.h"
#include "storage/buffile.h"
#include "utils/builtins.h"
#include "utils/hashstore.h"
#include "utils/hsearch.h"
#include "utils/memutils.h"
#include "utils/resowner.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(ora_string_agg_transfn);
PG_FUNCTION_INFO_V1(ora_string_agg_finalfn);
PG_FUNCTION_INFO_V1(ora_distinct_string_agg_transfn);
PG_FUNCTION_INFO_V1(ora_distinct_string_agg_finalfn);

#define NUM_FREELISTS			32
MemoryContext hashstore_context;
MemoryContext oldcontext;

typedef struct HashBucketHead
{
	BlockNumber	first;
	BlockNumber	last;
}HashBucketHead;

/* A hash bucket is a linked list of HASHELEMENTs */
typedef HASHELEMENT *HASHBUCKET;

/* A hash segment is an array of bucket headers */
typedef HASHBUCKET *HASHSEGMENT;

typedef struct HashStoreBufferDesc
{
	BlockNumber	tag;			/* ID of page contained in buffer */
	int			buf_id;			/* buffer's index number (from 0) */

	/* state of the tag, containing flags, refcount and usagecount */
	uint32		state;
}HashStoreBufferDesc;

typedef struct HashReader
{
	MinimalTuple	cache_tuple;
	BlockNumber		next_block;
	BlockNumber		cur_block;
	OffsetNumber	cur_offset;
	OffsetNumber	max_offset;
	bool			is_idle;
	uint32			hashvalue;
	StringInfoData	buf;
}HashReader;

typedef struct
{
	slock_t		mutex;			/* spinlock for this freelist */
	long		nentries;		/* number of entries in associated buckets */
	HASHELEMENT *freeList;		/* chain of free elements */
} FreeListData;

struct HASHHDR
{
	/*
	 * The freelist can become a point of contention in high-concurrency hash
	 * tables, so we use an array of freelists, each with its own mutex and
	 * nentries count, instead of just a single one.  Although the freelists
	 * normally operate independently, we will scavenge entries from freelists
	 * other than a hashcode's default freelist when necessary.
	 *
	 * If the hash table is not partitioned, only freeList[0] is used and its
	 * spinlock is not used at all; callers' locking is assumed sufficient.
	 */
	FreeListData freeList[NUM_FREELISTS];

	/* These fields can change, but not in a partitioned table */
	/* Also, dsize can't change in a shared table, even if unpartitioned */
	long		dsize;			/* directory size */
	long		nsegs;			/* number of allocated segments (<= dsize) */
	uint32		max_bucket;		/* ID of maximum bucket in use */
	uint32		high_mask;		/* mask to modulo into entire table */
	uint32		low_mask;		/* mask to modulo into lower half of table */

	/* These fields are fixed at hashtable creation */
	Size		keysize;		/* hash key length in bytes */
	Size		entrysize;		/* total user element size in bytes */
	long		num_partitions; /* # partitions (must be power of 2), or 0 */
	long		ffactor;		/* target fill factor */
	long		max_dsize;		/* 'dsize' limit if directory is fixed size */
	long		ssize;			/* segment size --- must be power of 2 */
	int			sshift;			/* segment shift = log2(ssize) */
	int			nelem_alloc;	/* number of entries to allocate at once */

#ifdef HASH_STATISTICS

	/*
	 * Count statistics here.  NB: stats code doesn't bother with mutex, so
	 * counts could be corrupted a bit in a partitioned table.
	 */
	long		accesses;
	long		collisions;
#endif
};

struct HTAB
{
	HASHHDR    *hctl;			/* => shared control information */
	HASHSEGMENT *dir;			/* directory of segment starts */
	HashValueFunc hash;			/* hash function */
	HashCompareFunc match;		/* key comparison function */
	HashCopyFunc keycopy;		/* key copying function */
	HashAllocFunc alloc;		/* memory allocator */
	MemoryContext hcxt;			/* memory context if default allocator used */
	char	   *tabname;		/* table name (for error messages) */
	bool		isshared;		/* true if table is in shared memory */
	bool		isfixed;		/* if true, don't enlarge */

	/* freezing a shared table isn't allowed, so we can keep state here */
	bool		frozen;			/* true = no more inserts allowed */

	/* We keep local copies of these fixed values to reduce contention */
	Size		keysize;		/* hash key length in bytes */
	long		ssize;			/* segment size --- must be power of 2 */
	int			sshift;			/* segment shift = log2(ssize) */
};

struct Hashstorestate
{
	BufFile				   *myfile;
	HashReader			   *reader;
	char				   *cache_page;
	HashStoreBufferDesc	   *cache_desc;
	HTAB				   *hash_page;		/* hash BlockNumber to buffer id */
	int						next_free_buf_id;
	BlockNumber				next_block;		/* next of file BlockNumber */
	int						nbucket;
	int						max_reader;
	int						max_cache;
	HashBucketHead			header[FLEXIBLE_ARRAY_MEMBER];	/* must at last */
};

static struct Hashstorestate *buffHash = NULL;

static StringInfo
makeStringAggState(FunctionCallInfo fcinfo)
{
	StringInfo	state;
	MemoryContext aggcontext;
	MemoryContext oldcontext;
	bool interXact = false;
	int nbuckets = 256;

	if (!AggCheckCallContext(fcinfo, &aggcontext))
	{
		/* cannot be called directly because of internal-type argument */
		elog(ERROR, "ora_string_agg_transfn called in non-aggregate context");
	}

	/*
	 * Create state in aggregate context.  It'll stay there across subsequent
	 * calls.
	 */
	oldcontext = MemoryContextSwitchTo(aggcontext);
	state = makeStringInfo();
	buffHash = hashstore_begin_heap(interXact, nbuckets);
	MemoryContextSwitchTo(oldcontext);

	return state;
}

static void
appendStringInfoText(StringInfo str, const text *t)
{
	appendBinaryStringInfo(str, VARDATA_ANY(t), VARSIZE_ANY_EXHDR(t));
}

static bool
ora_find_hashstore_match_value(Hashstorestate *buffHash, TupleTableSlot *checkSlot, uint32 hashValue, TupleDesc desc)
{
	bool find = false;
	int reader;
	TupleTableSlot *hashSlot;
	char *checkValueStr;
	char *valueStr;

	if (!buffHash)
		return false;

	reader = hashstore_begin_read(buffHash, hashValue);
	hashSlot = MakeSingleTupleTableSlot(desc);


	for (;;)
	{
		hashSlot = hashstore_next_slot(buffHash, hashSlot, reader, false);
		if (TupIsNull(hashSlot))
		{
			find = false;
			break;
		}
		valueStr = text_to_cstring((DatumGetTextP(hashSlot)));
		checkValueStr = text_to_cstring((DatumGetTextP(checkSlot)));
		if (strcmp(valueStr, checkValueStr) == 0)
		{
			find = true;
			break;
		}
	}

	hashstore_end_read(buffHash, reader);

	return find;
}

Datum
ora_string_agg_transfn(PG_FUNCTION_ARGS)
{
	StringInfo	state;
	char *delimiterStr = ",";

	state = PG_ARGISNULL(0) ? NULL : (StringInfo) PG_GETARG_POINTER(0);
	if (!state)
		buffHash = NULL;

	/* Append the value unless null. */
	if (!PG_ARGISNULL(1))
	{
		/* On the first time through, we ignore the delimiter. */
		if (state == NULL)
			state = makeStringAggState(fcinfo);
		else
			appendStringInfoText(state, cstring_to_text(delimiterStr));	/* delimiter */

		appendStringInfoText(state, PG_GETARG_TEXT_PP(1));	/* value */
	}

	/*
	 * The transition type for string_agg() is declared to be "internal",
	 * which is a pass-by-value type the same size as a pointer.
	 */
	PG_RETURN_POINTER(state);
}


Datum
ora_distinct_string_agg_transfn(PG_FUNCTION_ARGS)
{
	StringInfo	state;
	char *str;
	char *delimiterStr = ",";
	bool find = false;
	uint32 hashValue;
	TupleTableSlot *slot;
	TupleDesc desc = NULL;

	state = PG_ARGISNULL(0) ? NULL : (StringInfo) PG_GETARG_POINTER(0);
	if (!state)
		buffHash = NULL;

	/* Append the value unless null. */
	if (!PG_ARGISNULL(1))
	{
		str = text_to_cstring(PG_GETARG_TEXT_PP(1));

		desc = CreateTemplateTupleDesc(1, false);
		TupleDescInitEntry(desc, (AttrNumber) 1, "col", TEXTOID, -1, 0);
		BlessTupleDesc(desc);
		slot = MakeSingleTupleTableSlot(desc);
		slot->tts_values[0] = CStringGetTextDatum(str);
		slot->tts_isnull[0] = false;
		ExecStoreVirtualTuple(slot);

		hashValue = DatumGetUInt32(hash_any((unsigned char *) str, strlen(str)));
		find = ora_find_hashstore_match_value(buffHash, slot, hashValue, desc);

		/* On the first time through, we ignore the delimiter. */
		if (state == NULL)
			state = makeStringAggState(fcinfo);
		else
		{
			if (!find)
				appendStringInfoText(state, cstring_to_text(delimiterStr));	/* delimiter */
		}

		if (!find)
		{
			hashstore_put_tupleslot(buffHash, slot, hashValue);
			appendStringInfoText(state, PG_GETARG_TEXT_PP(1));	/* value */
		}

		ExecDropSingleTupleTableSlot(slot);
	}

	/*
	 * The transition type for string_agg() is declared to be "internal",
	 * which is a pass-by-value type the same size as a pointer.
	 */
	PG_RETURN_POINTER(state);
}

Datum
ora_string_agg_finalfn(PG_FUNCTION_ARGS)
{
	StringInfo	state;

	/* cannot be called directly because of internal-type argument */
	Assert(AggCheckCallContext(fcinfo, NULL));

	state = PG_ARGISNULL(0) ? NULL : (StringInfo) PG_GETARG_POINTER(0);

	if (state != NULL)
	{
		hashstore_end(buffHash);

		PG_RETURN_TEXT_P(cstring_to_text_with_len(state->data, state->len));
	}
	else
		PG_RETURN_NULL();
}

Datum
ora_distinct_string_agg_finalfn(PG_FUNCTION_ARGS)
{
	StringInfo	state;

	/* cannot be called directly because of internal-type argument */
	Assert(AggCheckCallContext(fcinfo, NULL));

	state = PG_ARGISNULL(0) ? NULL : (StringInfo) PG_GETARG_POINTER(0);

	if (state != NULL)
	{
		hashstore_end(buffHash);

		PG_RETURN_TEXT_P(cstring_to_text_with_len(state->data, state->len));
	}
	else
		PG_RETURN_NULL();
}
