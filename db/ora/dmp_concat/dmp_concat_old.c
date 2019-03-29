#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(ora_string_agg_transfn);
PG_FUNCTION_INFO_V1(ora_distinct_string_agg_transfn);
PG_FUNCTION_INFO_V1(ora_string_agg_finalfn);

static StringInfo
makeStringAggState(FunctionCallInfo fcinfo)
{
	StringInfo state;
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

static void
appendStringInfoText(StringInfo str, const text *t)
{
	appendBinaryStringInfo(str, VARDATA_ANY(t), VARSIZE_ANY_EXHDR(t));
}

static bool
ora_check_string_contain_element(StringInfo infosendmsg, char *element, char *delimiter)
{
	char *splitStr;
	char *elemStr = NULL;
	char *strRemain = NULL;
	bool find = false;

	if (!infosendmsg || infosendmsg->len < 1)
		return find;

	Assert(infosendmsg);
	Assert(element);
	Assert(delimiter);

	splitStr = (char *)palloc0(infosendmsg->len + 1);
	memcpy(splitStr, infosendmsg->data, infosendmsg->len + 1);
	elemStr = strtok_r(splitStr, delimiter, &strRemain);
	while (elemStr != NULL)
	{
		elemStr = strtok_r(NULL, delimiter, &strRemain);
		if (elemStr && strcmp(elemStr, element) == 0)
		{
			find = true;
			break;
		}
	}
	pfree(splitStr);

	return find;
}

Datum
	ora_string_agg_transfn(PG_FUNCTION_ARGS)
{
	StringInfo state;
	char *delimiterStr = ",";

	state = PG_ARGISNULL(0) ? NULL : (StringInfo)PG_GETARG_POINTER(0);

	/* Append the value unless null. */
	if (!PG_ARGISNULL(1))
	{
		/* On the first time through, we ignore the delimiter. */
		if (state == NULL)
			state = makeStringAggState(fcinfo);
		else
			appendStringInfoText(state, cstring_to_text(delimiterStr)); /* delimiter */

		appendStringInfoText(state, PG_GETARG_TEXT_PP(1)); /* value */
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
	StringInfo state;
	char *str;
	char *delimiterStr = ",";
	bool find = false;

	state = PG_ARGISNULL(0) ? NULL : (StringInfo)PG_GETARG_POINTER(0);

	/* Append the value unless null. */
	if (!PG_ARGISNULL(1))
	{
		str = text_to_cstring(PG_GETARG_TEXT_PP(1));
		find = ora_check_string_contain_element(state, str, delimiterStr);
		/* On the first time through, we ignore the delimiter. */
		if (state == NULL)
			state = makeStringAggState(fcinfo);
		else
		{
			if (!find)
				appendStringInfoText(state, cstring_to_text(delimiterStr)); /* delimiter */
		}

		if (!find)
			appendStringInfoText(state, PG_GETARG_TEXT_PP(1)); /* value */
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
	StringInfo state;

	/* cannot be called directly because of internal-type argument */
	Assert(AggCheckCallContext(fcinfo, NULL));

	state = PG_ARGISNULL(0) ? NULL : (StringInfo)PG_GETARG_POINTER(0);

	if (state != NULL)
		PG_RETURN_TEXT_P(cstring_to_text_with_len(state->data, state->len));
	else
		PG_RETURN_NULL();
}
