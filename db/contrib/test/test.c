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
#include "utils/lsyscache.h"
#include "utils/syscache.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(test);

#define		  RELKIND_RELATION		  'r'	/* ordinary table */
#define		  RELKIND_INDEX			  'i'	/* secondary index */
#define		  RELKIND_SEQUENCE		  'S'	/* sequence object */
#define		  RELKIND_TOASTVALUE	  't'	/* for out-of-line values */
#define		  RELKIND_VIEW			  'v'	/* view */
#define		  RELKIND_MATVIEW		  'm'	/* materialized view */
#define		  RELKIND_COMPOSITE_TYPE  'c'	/* composite type */
#define		  RELKIND_FOREIGN_TABLE   'f'	/* foreign table */
#define		  RELKIND_PARTITIONED_TABLE 'p' /* partitioned table */
#define		  RELKIND_PARTITIONED_INDEX 'I' /* partitioned index */

#define		  RELPERSISTENCE_PERMANENT	'p' /* regular table */
#define		  RELPERSISTENCE_UNLOGGED	'u' /* unlogged permanent table */
#define		  RELPERSISTENCE_TEMP		't' /* temporary table */

/* default selection for replica identity (primary key or nothing) */
#define		  REPLICA_IDENTITY_DEFAULT	'd'
/* no replica identity is logged for this relation */
#define		  REPLICA_IDENTITY_NOTHING	'n'
/* all columns are logged as replica identity */
#define		  REPLICA_IDENTITY_FULL		'f'
/*
 * an explicitly chosen candidate key's columns are used as replica identity.
 * Note this will still be set if the index has been dropped; in that case it
 * has the same meaning as 'd'.
 */
#define		  REPLICA_IDENTITY_INDEX	'i'


Oid
get_namespaceid(const char *nspname)
{
	return GetSysCacheOid(NAMESPACENAME,
				CStringGetDatum(nspname),
				0, 0, 0);
}

Datum
test(PG_FUNCTION_ARGS)
{
	char *tabName;
	char *nameSpace;
	char relkind;
	char relpersistence;
	bool result = true;
	Oid relnamespace;
	RelationLocInfo *locInfo;
	Oid relid;

	tabName =  PG_GETARG_CSTRING(0);
	nameSpace = get_current_schema();
	relnamespace = get_namespaceid(nameSpace);

	relid = get_relname_relid(tabName, relnamespace);
	
	rel = heap_open(relid, NoLock);
	
	locInfo = GetRelationLocInfo(relid);
	
	relkind = rel->rd_rel->relkind;
	relpersistence = rel->rd_rel->relpersistence;
	
	
	heap_close(rel, NoLock);

	PG_RETURN_BOOL(result);
}

