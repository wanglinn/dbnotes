
1. 问题描述：select 一个表时，如果不指定limit 则限制输出10行。(postgres 10.6 version)

修改：
在函数
static Query *
transformSelectStmt(ParseState *pstate, SelectStmt *stmt)
中 L1301行添加：
if (!qry->limitCount)
{    
		A_Const *n = makeNode(A_Const);
		n->val.type = T_Integer;
		n->val.val.ival = 10;
		n->location = 0; 
		qry->limitCount = transformLimitClause(pstate, (Node *)n, EXPR_KIND_LIMIT, "LIMIT");
} 

这样修改带来的问题就是：
create table t1(id int);
insert into t1 select generate_series(1,100); -- 实际只插入10行


        
