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



# 1 add_path
```
#0  add_path (parent_rel=0x204b528, new_path=0x204c468) at pathnode.c:424
#1  0x0000000000781913 in set_plain_rel_pathlist (root=0x1f76cd8, rel=0x204b528, rte=0x1f765a8) at allpaths.c:778
#2  0x00000000007815c9 in set_rel_pathlist (root=0x1f76cd8, rel=0x204b528, rti=1, rte=0x1f765a8) at allpaths.c:500
#3  0x000000000078129e in set_base_rel_pathlists (root=0x1f76cd8) at allpaths.c:352
#4  0x000000000078108b in make_one_rel (root=0x1f76cd8, joinlist=0x204bf38) at allpaths.c:222
#5  0x00000000007b6e26 in query_planner (root=0x1f76cd8, qp_callback=0x7bd13c <standard_qp_callback>, qp_extra=0x7ffdd41f83f0) at planmain.c:269
#6  0x00000000007ba34f in grouping_planner (root=0x1f76cd8, inheritance_update=false, tuple_fraction=0) at planner.c:2058
#7  0x00000000007b88b0 in subquery_planner (glob=0x1f77178, parse=0x1f76498, parent_root=0x0, hasRecursion=false, tuple_fraction=0) at planner.c:1015
#8  0x00000000007b72c1 in standard_planner (parse=0x1f76498, query_string=0x1f75458 "select /*+ indexscan(t1) */ id11 from t1 where id11=  1;", cursorOptions=256, 
    boundParams=0x0) at planner.c:405
#9  0x00007f646244e3ed in pg_hint_plan_planner (parse=0x1f76498, query_string=0x1f75458 "select /*+ indexscan(t1) */ id11 from t1 where id11=  1;", cursorOptions=256, 
    boundParams=0x0) at pg_hint_plan.c:3176
#10 0x00000000007b7050 in planner (parse=0x1f76498, query_string=0x1f75458 "select /*+ indexscan(t1) */ id11 from t1 where id11=  1;", cursorOptions=256, 
    boundParams=0x0) at planner.c:273
#11 0x00000000008ba146 in pg_plan_query (querytree=0x1f76498, query_string=0x1f75458 "select /*+ indexscan(t1) */ id11 from t1 where id11=  1;", cursorOptions=256, 
    boundParams=0x0) at postgres.c:875
#12 0x00000000008ba271 in pg_plan_queries (querytrees=0x1f76c68, query_string=0x1f75458 "select /*+ indexscan(t1) */ id11 from t1 where id11=  1;", cursorOptions=256, 
    boundParams=0x0) at postgres.c:966
#13 0x00000000008ba5b4 in exec_simple_query (query_string=0x1f75458 "select /*+ indexscan(t1) */ id11 from t1 where id11=  1;") at postgres.c:1158
#14 0x00000000008be5c4 in PostgresMain (argc=1, argv=0x1f9d0b8, dbname=0x1f9cfb8 "db1", username=0x1f72018 "wln") at postgres.c:4315
#15 0x000000000081f4d1 in BackendRun (port=0x1f957a0) at postmaster.c:4536
#16 0x000000000081ecd8 in BackendStartup (port=0x1f957a0) at postmaster.c:4220
#17 0x000000000081b636 in ServerLoop () at postmaster.c:1739
#18 0x000000000081af12 in PostmasterMain (argc=1, argv=0x1f6ffd0) at postmaster.c:1412
#19 0x00000000007314cd in main (argc=1, argv=0x1f6ffd0) at main.c:210

```


```
#0  build_index_paths (root=0x1f76cd8, rel=0x204b528, index=0x204bb58, clauses=0x7ffdd41f7ff0, useful_predicate=false, scantype=ST_ANYSCAN, 
    skip_nonnative_saop=0x7ffdd41f7d57, skip_lower_saop=0x7ffdd41f7d56) at indxpath.c:917
#1  0x0000000000796ce5 in get_index_paths (root=0x1f76cd8, rel=0x204b528, index=0x204bb58, clauses=0x7ffdd41f7ff0, bitindexpaths=0x7ffdd41f8108) at indxpath.c:744
#2  0x0000000000795d53 in create_index_paths (root=0x1f76cd8, rel=0x204b528) at indxpath.c:276
#3  0x000000000078194c in set_plain_rel_pathlist (root=0x1f76cd8, rel=0x204b528, rte=0x1f765a8) at allpaths.c:785
#4  0x00000000007815c9 in set_rel_pathlist (root=0x1f76cd8, rel=0x204b528, rti=1, rte=0x1f765a8) at allpaths.c:500
#5  0x000000000078129e in set_base_rel_pathlists (root=0x1f76cd8) at allpaths.c:352
#6  0x000000000078108b in make_one_rel (root=0x1f76cd8, joinlist=0x204bf38) at allpaths.c:222
#7  0x00000000007b6e26 in query_planner (root=0x1f76cd8, qp_callback=0x7bd13c <standard_qp_callback>, qp_extra=0x7ffdd41f83f0) at planmain.c:269
#8  0x00000000007ba34f in grouping_planner (root=0x1f76cd8, inheritance_update=false, tuple_fraction=0) at planner.c:2058
#9  0x00000000007b88b0 in subquery_planner (glob=0x1f77178, parse=0x1f76498, parent_root=0x0, hasRecursion=false, tuple_fraction=0) at planner.c:1015
#10 0x00000000007b72c1 in standard_planner (parse=0x1f76498, query_string=0x1f75458 "select /*+ indexscan(t1) */ id11 from t1 where id11=  1;", cursorOptions=256, 
    boundParams=0x0) at planner.c:405
```

- **判断是否可以作为唯一索引**  
#0  check_index_only (rel=0x204b528, index=0x204bb58) at indxpath.c:1824   

```
#0  pg_hint_plan_set_rel_pathlist (root=0x1f76cd8, rel=0x204b528, rti=1, rte=0x1f765a8) at pg_hint_plan.c:4679
#1  0x000000000078167d in set_rel_pathlist (root=0x1f76cd8, rel=0x204b528, rti=1, rte=0x1f765a8) at allpaths.c:540
#2  0x000000000078129e in set_base_rel_pathlists (root=0x1f76cd8) at allpaths.c:352
#3  0x000000000078108b in make_one_rel (root=0x1f76cd8, joinlist=0x204bf38) at allpaths.c:222
#4  0x00000000007b6e26 in query_planner (root=0x1f76cd8, qp_callback=0x7bd13c <standard_qp_callback>, qp_extra=0x7ffdd41f83f0) at planmain.c:269
#5  0x00000000007ba34f in grouping_planner (root=0x1f76cd8, inheritance_update=false, tuple_fraction=0) at planner.c:2058
#6  0x00000000007b88b0 in subquery_planner (glob=0x1f77178, parse=0x1f76498, parent_root=0x0, hasRecursion=false, tuple_fraction=0) at planner.c:1015
#7  0x00000000007b72c1 in standard_planner (parse=0x1f76498, query_string=0x1f75458 "select /*+ indexscan(t1) */ id11 from t1 where id11=  1;", cursorOptions=256, 
    boundParams=0x0) at planner.c:405
#8  0x00007f646244e3ed in pg_hint_plan_planner (parse=0x1f76498, query_string=0x1f75458 "select /*+ indexscan(t1) */ id11 from t1 where id11=  1;", cursorOptions=256, 
    boundParams=0x0) at pg_hint_plan.c:3176
#9  0x00000000007b7050 in planner (parse=0x1f76498, query_string=0x1f75458 "select /*+ indexscan(t1) */ id11 from t1 where id11=  1;", cursorOptions=256, 
    boundParams=0x0) at planner.c:273
#10 0x00000000008ba146 in pg_plan_query (querytree=0x1f76498, query_string=0x1f75458 "select /*+ indexscan(t1) */ id11 from t1 where id11=  1;", cursorOptions=256,
```


This relation doesn't have a parent. Cancel current_hint_state.       

- **find_scan_hint**  

```
#0  find_scan_hint (root=0x1f76cd8, relid=1) at pg_hint_plan.c:3255
#1  0x00007f646244fc9d in setup_hint_enforcement (root=0x1f76cd8, rel=0x204b528, rshint=0x0, rphint=0x7ffdd41f81d8) at pg_hint_plan.c:3933
#2  0x00007f6462451a16 in pg_hint_plan_set_rel_pathlist (root=0x1f76cd8, rel=0x204b528, rti=1, rte=0x1f765a8) at pg_hint_plan.c:4758
#3  0x000000000078167d in set_rel_pathlist (root=0x1f76cd8, rel=0x204b528, rti=1, rte=0x1f765a8) at allpaths.c:540
#4  0x000000000078129e in set_base_rel_pathlists (root=0x1f76cd8) at allpaths.c:352
#5  0x000000000078108b in make_one_rel (root=0x1f76cd8, joinlist=0x204bf38) at allpaths.c:222
#6  0x00000000007b6e26 in query_planner (root=0x1f76cd8, qp_callback=0x7bd13c <standard_qp_callback>, qp_extra=0x7ffdd41f83f0) at planmain.c:269
#7  0x00000000007ba34f in grouping_planner (root=0x1f76cd8, inheritance_update=false, tuple_fraction=0) at planner.c:2058
#8  0x00000000007b88b0 in subquery_planner (glob=0x1f77178, parse=0x1f76498, parent_root=0x0, hasRecursion=false, tuple_fraction=0) at planner.c:1015
#9  0x00000000007b72c1 in standard_planner (parse=0x1f76498, query_string=0x1f75458 "select /*+ indexscan(t1) */ id11 from t1 where id11=  1;", cursorOptions=256, 
    boundParams=0x0) at planner.c:405
#10 0x00007f646244e3ed in pg_hint_plan_planner (parse=0x1f76498, query_string=0x1f75458 "select /*+ indexscan(t1) */ id11 from t1 where id11=  1;", cursorOptions=256, 
    boundParams=0x0) at pg_hint_plan.c:3176
```

find_scan_hint  
using_parent_hint  





