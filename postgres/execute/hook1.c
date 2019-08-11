文件 src/backend/executor/execMain.c

在执行器里，可以使用postgres提供的标准函数，也可以使用用户自定义钩子hook函数进行处理。

void ExecutorStart(QueryDesc *queryDesc, int eflags) 函数：
	(*ExecutorStart_hook) (queryDesc, eflags)；
    standard_ExecutorStart(queryDesc, eflags)；
     其中ExecutorStart_hook 为指针函数。

void ExecutorRun(QueryDesc *queryDesc,
				 ScanDirection direction, uint64 count,
				 bool execute_once) 函数：

	(*ExecutorRun_hook)(queryDesc, direction, count, execute_once)
	standard_ExecutorRun(queryDesc, direction, count, execute_once);
	其中ExecutorRun_hook为指针函数。

void ExecutorFinish(QueryDesc *queryDesc) 函数：
	(*ExecutorFinish_hook) (queryDesc);
	standard_ExecutorFinish(queryDesc);
	其中ExecutorFinish_hook 为指针函数。

void ExecutorEnd(QueryDesc *queryDesc) 函数：
	(*ExecutorEnd_hook) (queryDesc);
	standard_ExecutorEnd(queryDesc);
	其中 ExecutorEnd_hook 为指针函数。

上述ExecutorStart, ExecutorRun, ExecutorFinish, ExecutorEnd 都属于查询执行范围的函数。那么都是什么调用这些函数的呢，
具体见《PostgreSQL数据库内核分析》第六章p282。查询执行器中将查询分为以下两种：处理类似DDL操作的 ProcessUtility 模块 及 
处理DML等的Executor模块。常见的portal系列函数：
void PortalStart(Portal portal, ParamListInfo params,
			int eflags, Snapshot snapshot);

bool PortalRun(Portal portal, long count, bool isTopLevel, bool run_once,
		  DestReceiver *dest, DestReceiver *altdest,
		  char *completionTag);

void PortalDrop(Portal portal, bool isTopCommit);

portal执行过程见 该书 p288 "6.1.4 Portal执行过程"， 图6-6描述了portal执行时的函数调用关系，其中executorStart等executor
系列函数就穿插在其中。p291 图6-8 描述了表创建过程的函数流程，ProcessUtility函数的调用位于函数PortalRun函数内。

主要执行函数如下：
PortalRun --> PortalRunSelect --> ExecutorRun
PortalRun -->PortalRunMulti --> PortalRunUtility --> ProcessUtility



/* ----------------------------------------------------------------
 *		ExecutorStart
 *
 *		This routine must be called at the beginning of any execution of any
 *		query plan
 *
 * Takes a QueryDesc previously created by CreateQueryDesc (which is separate
 * only because some places use QueryDescs for utility commands).  The tupDesc
 * field of the QueryDesc is filled in to describe the tuples that will be
 * returned, and the internal fields (estate and planstate) are set up.
 *
 * eflags contains flag bits as described in executor.h.
 *
 * NB: the CurrentMemoryContext when this is called will become the parent
 * of the per-query context used for this Executor invocation.
 *
 * We provide a function hook variable that lets loadable plugins
 * get control when ExecutorStart is called.  Such a plugin would
 * normally call standard_ExecutorStart().
 *
 * ----------------------------------------------------------------
 */
void
ExecutorStart(QueryDesc *queryDesc, int eflags)
{
	if (ExecutorStart_hook)
		(*ExecutorStart_hook) (queryDesc, eflags);
	else
		standard_ExecutorStart(queryDesc, eflags);
}

/* ----------------------------------------------------------------
 *		ExecutorRun
 *
 *		This is the main routine of the executor module. It accepts
 *		the query descriptor from the traffic cop and executes the
 *		query plan.
 *
 *		ExecutorStart must have been called already.
 *
 *		If direction is NoMovementScanDirection then nothing is done
 *		except to start up/shut down the destination.  Otherwise,
 *		we retrieve up to 'count' tuples in the specified direction.
 *
 *		Note: count = 0 is interpreted as no portal limit, i.e., run to
 *		completion.  Also note that the count limit is only applied to
 *		retrieved tuples, not for instance to those inserted/updated/deleted
 *		by a ModifyTable plan node.
 *
 *		There is no return value, but output tuples (if any) are sent to
 *		the destination receiver specified in the QueryDesc; and the number
 *		of tuples processed at the top level can be found in
 *		estate->es_processed.
 *
 *		We provide a function hook variable that lets loadable plugins
 *		get control when ExecutorRun is called.  Such a plugin would
 *		normally call standard_ExecutorRun().
 *
 * ----------------------------------------------------------------
 */
void ExecutorRun(QueryDesc *queryDesc,
				 ScanDirection direction, uint64 count,
				 bool execute_once)
{
	if (ExecutorRun_hook)
		(*ExecutorRun_hook)(queryDesc, direction, count, execute_once);
	else
		standard_ExecutorRun(queryDesc, direction, count, execute_once);
}

/* ----------------------------------------------------------------
 *		ExecutorFinish
 *
 *		This routine must be called after the last ExecutorRun call.
 *		It performs cleanup such as firing AFTER triggers.  It is
 *		separate from ExecutorEnd because EXPLAIN ANALYZE needs to
 *		include these actions in the total runtime.
 *
 *		We provide a function hook variable that lets loadable plugins
 *		get control when ExecutorFinish is called.  Such a plugin would
 *		normally call standard_ExecutorFinish().
 *
 * ----------------------------------------------------------------
 */
void ExecutorFinish(QueryDesc *queryDesc)
{
	if (ExecutorFinish_hook)
		(*ExecutorFinish_hook)(queryDesc);
	else
		standard_ExecutorFinish(queryDesc);
}

/* ----------------------------------------------------------------
 *		ExecutorEnd
 *
 *		This routine must be called at the end of execution of any
 *		query plan
 *
 *		We provide a function hook variable that lets loadable plugins
 *		get control when ExecutorEnd is called.  Such a plugin would
 *		normally call standard_ExecutorEnd().
 *
 * ----------------------------------------------------------------
 */
void ExecutorEnd(QueryDesc *queryDesc)
{
	if (ExecutorEnd_hook)
		(*ExecutorEnd_hook)(queryDesc);
	else
		standard_ExecutorEnd(queryDesc);
}


总结
hook 以指针函数形式半路拦截程序原有运行模式，引入到用户自定义函数中进行相应任务的处理。

针对auto_explain， 本意在于对执行的SQL 后台多做了一步操作--生成执行计划打印到日志中去，而原来要执行的内容仍需要按照原有的
规划去执行。因此针对auto_explain插件代码中，在处理的各种阶段：
ExecutorStart, ExecutorRun, ExecutorFinish, ExecutorEnd函数中 均需要再次回到标准执行函数 standard_ExecutorXX 中去。


参考
hook in postgresql
https://wiki.postgresql.org/images/e/e3/Hooks_in_postgresql.pdf

《PostgreSQL数据库内核分析》 彭智勇等著

auto_explain postgres手册章节内容
http://www.postgres.cn/docs/9.5/auto-explain.html


