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
	bool result = true;
	Oid relnamespace;
	RelationLocInfo *locInfo;
	Oid relid;

	tabName =  PG_GETARG_CSTRING(0);
	nameSpace = get_current_schema();
	relnamespace = get_namespaceid(nameSpace);

	relid = get_relname_relid(tabName, relnamespace);
	locInfo = GetRelationLocInfo(relid);
	
	PG_RETURN_BOOL(result);
}

