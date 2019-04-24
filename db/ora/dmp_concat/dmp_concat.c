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

typedef struct
{
	StringInfo state;
	StringInfo stateRecord;
	HTAB *hashtab;
} StringHashTabData;

typedef struct
{
	long int address;
} ConcatElemData;

static int
ora_cmp_string(const void *a, const void *b, Size keysize)
{
	long int adda = *(long int*)a;
	long int addb = *(long int*)b;

	const char *sa = (const char *)adda;
	const char *sb = (const char *)addb;

	return strcmp(sa, sb);
}

static uint32
ora_string_hash(const void *key, Size keysize)
{
	/*
	 * If the string exceeds keysize-1 bytes, we want to hash only that many,
	 * because when it is copied into the hash table it will be truncated at
	 * that length.
	 */
	long int address = *(long int*)key;
	Size s_len = strlen((const char *) address);

	return DatumGetUInt32(hash_any((const unsigned char *) address,
								   (int) s_len));
}

static HTAB *
init_concat_hashtable(void)
{
	HTAB *hashtab = NULL;
	HASHCTL	hash_ctl;
	int num_elem = 2056;

	MemSet(&hash_ctl, 0, sizeof(hash_ctl));
	hash_ctl.keysize = sizeof(long int);
	hash_ctl.entrysize = sizeof(ConcatElemData);
	hash_ctl.match = ora_cmp_string;
	hash_ctl.hash = ora_string_hash;
	hash_ctl.hcxt = CurrentMemoryContext;
	hashtab = hash_create("dmp_concat table",
						num_elem,
						&hash_ctl,
						HASH_ELEM |HASH_FUNCTION | HASH_COMPARE |HASH_CONTEXT);
	if (!hashtab)
			elog(ERROR, "create hash table fail");

	return hashtab;
}

static void
dmp_concat_distinct_showdown(Datum arg)
{
	StringHashTabData *stringHashTab;

	stringHashTab = (StringHashTabData *) DatumGetPointer(arg);

	if (stringHashTab != NULL)
	{
		hash_destroy(stringHashTab->hashtab);
		stringHashTab->hashtab = NULL;

		if (stringHashTab->stateRecord)
		{
			pfree(stringHashTab->stateRecord);
			stringHashTab->stateRecord = NULL;
		}

		if (stringHashTab->state)
		{
			pfree(stringHashTab->state);
			stringHashTab->state = NULL;
		}

		pfree(stringHashTab);
		stringHashTab = NULL;
	}
}

static StringInfo
makeStringAggState(FunctionCallInfo fcinfo)
{
	StringInfo	state;
	MemoryContext aggcontext;
	MemoryContext oldcontext;

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
	MemoryContextSwitchTo(oldcontext);

	return state;
}

static StringHashTabData *
makeAggState(FunctionCallInfo fcinfo)
{
	StringHashTabData *stringHashTab;
	MemoryContext aggcontext;
	MemoryContext oldcontext;

	if (!AggCheckCallContext(fcinfo, &aggcontext))
	{
		/* cannot be called directly because of internal-type argument */
		elog(ERROR, "ora_distinct_string_agg_transfn called in non-aggregate context");
	}

	/*
	 * Create state in aggregate context.  It'll stay there across subsequent
	 * calls.
	 */
	oldcontext = MemoryContextSwitchTo(aggcontext);
	stringHashTab = (StringHashTabData *)palloc(sizeof(StringHashTabData));
	stringHashTab->state = NULL;
	stringHashTab->hashtab = NULL;
	stringHashTab->state = makeStringInfo();
	stringHashTab->stateRecord = makeStringInfo();
	stringHashTab->hashtab = init_concat_hashtable();
	MemoryContextSwitchTo(oldcontext);

	if (fcinfo->context && IsA(fcinfo->context, AggState))
	{
		AggState   *aggstate = (AggState *) fcinfo->context;
		ExprContext *cxt = aggstate->curaggcontext;

		RegisterExprContextCallback(cxt, dmp_concat_distinct_showdown, PointerGetDatum(stringHashTab));
	}

	return stringHashTab;
}

static void
appendStringInfoText(StringInfo str, const text *t)
{
	appendBinaryStringInfo(str, VARDATA_ANY(t), VARSIZE_ANY_EXHDR(t));
}

static bool
ora_find_hashtab_match_str(HTAB *hashtab, long int *address)
{
	bool find = false;

	if (!hashtab)
		return false;

	hash_search(hashtab
					, address
					, HASH_FIND
					, &find);

	return find;
}

Datum
ora_string_agg_transfn(PG_FUNCTION_ARGS)
{
	StringInfo state;
	char *delimiterStr = ",";

	state = PG_ARGISNULL(0) ? NULL : (StringInfo) PG_GETARG_POINTER(0);

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
	StringHashTabData *stringHashTab;
	char *str;
	char *delimiterStr = ",";
	char *point;
	bool find = false;
	int cursor = 0;
	long int address = 0;

	stringHashTab = PG_ARGISNULL(0) ? NULL : (StringHashTabData *) PG_GETARG_POINTER(0);

	/* Append the value unless null. */
	if (!PG_ARGISNULL(1))
	{
		str = text_to_cstring(PG_GETARG_TEXT_PP(1));

		/* On the first time through, we ignore the delimiter. */
		if (stringHashTab == NULL)
			stringHashTab = makeAggState(fcinfo);
		else
		{
			address = (long int)&str[0];
			find = ora_find_hashtab_match_str(stringHashTab->hashtab, &address);
			if (!find)
				appendStringInfoText(stringHashTab->state, cstring_to_text(delimiterStr));	/* delimiter */
		}

		if (!find)
		{
			ConcatElemData *element;
			appendStringInfoText(stringHashTab->state, PG_GETARG_TEXT_PP(1));	/* value */
			cursor = stringHashTab->stateRecord->len;
			point = &(stringHashTab->stateRecord->data[cursor]);
			appendStringInfoString(stringHashTab->stateRecord, str);
			appendStringInfoCharMacro(stringHashTab->stateRecord, '\0');

			address = (long int)&point[0];
			element = (ConcatElemData *) hash_search(stringHashTab->hashtab,
							 &address,
							HASH_ENTER,
							NULL);
		}
	}

	/*
	 * The transition type for string_agg() is declared to be "internal",
	 * which is a pass-by-value type the same size as a pointer.
	 */
	PG_RETURN_POINTER(stringHashTab);
}

Datum
ora_string_agg_finalfn(PG_FUNCTION_ARGS)
{
	StringInfo	state;

	/* cannot be called directly because of internal-type argument */
	Assert(AggCheckCallContext(fcinfo, NULL));

	state = PG_ARGISNULL(0) ? NULL : (StringInfo) PG_GETARG_POINTER(0);

	if (state != NULL)
		PG_RETURN_TEXT_P(cstring_to_text_with_len(state->data, state->len));
	else
		PG_RETURN_NULL();
}

Datum
ora_distinct_string_agg_finalfn(PG_FUNCTION_ARGS)
{
	StringHashTabData *stringHashTab;

	/* cannot be called directly because of internal-type argument */
	Assert(AggCheckCallContext(fcinfo, NULL));

	stringHashTab = PG_ARGISNULL(0) ? NULL : (StringHashTabData *) PG_GETARG_POINTER(0);

	if (stringHashTab != NULL)
		PG_RETURN_TEXT_P(cstring_to_text_with_len(stringHashTab->state->data, stringHashTab->state->len));
	else
		PG_RETURN_NULL();
}
