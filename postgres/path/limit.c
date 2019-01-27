
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

---
测试：
postgres=# explain verbose select * from t1;
                            QUERY PLAN                             
-------------------------------------------------------------------
 Limit  (cost=0.00..0.14 rows=10 width=4)
   Output: id
   ->  Seq Scan on public.t1  (cost=0.00..35.50 rows=2550 width=4)
         Output: id
(4 rows)
		 
这样修改带来的问题就是：
create table t1(id int);
insert into t1 select generate_series(1,100); -- 实际只插入10行

postgres=# explain verbose insert into t1 select generate_series(1,100); 
                         QUERY PLAN                          
-------------------------------------------------------------
 Insert on public.t1  (cost=0.00..0.15 rows=10 width=4)
   ->  Limit  (cost=0.00..0.05 rows=10 width=4)
         Output: (generate_series(1, 100))
         ->  ProjectSet  (cost=0.00..5.02 rows=1000 width=4)
               Output: generate_series(1, 100)
               ->  Result  (cost=0.00..0.01 rows=1 width=0)
(6 rows)

        
