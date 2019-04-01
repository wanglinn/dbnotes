#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(ora_nlssort);

Datum
ora_nlssort(PG_FUNCTION_ARGS)
{
	text *locale;
	text *result;
	char *localeStr;
	char *ora_pinyin = "schinese_pinyin_m";
	char *ora_zh = "zh_CN";

	if (PG_ARGISNULL(0))
		PG_RETURN_NULL();

	if (PG_ARGISNULL(1))
	{
		locale = PG_GETARG_TEXT_PP(1);
	}
	else
	{
		localeStr = text_to_cstring(PG_GETARG_TEXT_PP(1));

		if (strcasestr(localeStr, ora_pinyin) || strcasestr(localeStr, ora_zh))
			locale = cstring_to_text(ora_zh);
		else
			ereport(ERROR, (errcode(ERRCODE_UNDEFINED_OBJECT), errmsg("not support \"%s\" now", localeStr)));
	}

	result = PG_GETARG_TEXT_PP(0);

	if (!result)
		PG_RETURN_NULL();

	PG_RETURN_TEXT_P(result);
}