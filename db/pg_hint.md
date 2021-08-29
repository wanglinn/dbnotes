postgres 上使用hint
# 1 源码下载
postgres 13.1
https://www.postgresql.org/ftp/source/

pg_hint_plan
https://github.com/ossc-db/pg_hint_plan

# 2 源码安装

## 2.1 postgres
./configure --prefix=/home/wln/install/pg131 --enable-debug  
make  
make install  

**配置环境变量**
#postgres  
export PGPORT=5432  
export PGDATABASE=db1  
export PGHOME=/home/$USER/install/pg131  
export PGDATA=/home/$USER/pgdata/pg131  
export PATH=$PGHOME/bin:$PATH  
export LD_LIBRARY_PATH=$PGHOME/lib:$LD_LIBRARY_PATH  

## 2.2 pg_hint_plan
编译pg_hint_plan时会自动读取设置的环境变量PGHOME的信息  
make  
make install  

## 2.3 配置插件
postgresql.conf 配置：  
shared_preload_libraries = 'pg_hint_plan'  
pg_hint_plan.enable_hint = on  
pg_hint_plan.debug_print = on  
pg_hint_plan.message_level = debug  


重启数据库;  
创建插件 create extension pg_hint_plan;  

测试：  
drop table t1 cascade;  
create table t1(id11 int, id12 int);  
insert into t1 select generate_series(1,3), generate_series(1,3);  
create index i_t1_id11 on t1(id11);  
create index i_t1_id12 on t1(id12);  
```
db1=# explain select id11 from t1 where id11 = 1;  
                    QUERY PLAN                    
--------------------------------------------------
 Seq Scan on t1  (cost=0.00..1.04 rows=1 width=4)
   Filter: (id11 = 1)
(2 rows)
```
使用hint  
```
db1=# explain select /*+  indexscan(t1) */id11 from t1 where id11 = 1;  
                             QUERY PLAN                             
--------------------------------------------------------------------
 Index Scan using i_t1_id11 on t1  (cost=0.13..8.15 rows=1 width=4)
   Index Cond: (id11 = 1)
(2 rows)

```

# 3 hint支持的类型


|  hint format    | 描述     |
|    :-          |    :-   |
| 扫描    |      |
| SeqScan(table)  | seq scan  |
| indexScan(table)  | index scan  |
| indexOnlyScan(table)  | index only scan  |
| noSeqScan(table)  | no seq scan  |
| noindexScan(table)  | no index scan  |
| noSeqScan(table)  | no seq scan  |
| 表连接| |
| NestLoop(table table[ table...]) | nest loop |
|noNestLoop(table table[ table...])| no nest join|
|HashJoin(table table[ table...])| hash join|
|noHashJoin(table table[ table...])| no hash join|
|mergeJoin(table table[ table...])| merge join|
|noMergeJoin(table table[ table...])| no merge join|
|表连接顺序||
|Leading(table table[ table...])|指定表连接顺序作为外表|

