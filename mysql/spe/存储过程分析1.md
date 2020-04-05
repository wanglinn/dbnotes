## 1. 存储过程用例##
```
delimiter ;;

create procedure testCur()
begin
declare id1 int;
end
;;

delimiter ;

```
## 1.1 查看系统表 mysql.proc##

```
mysql> select * from mysql.proc  where name = 'testCur' limit 1  \G;
*************************** 1. row ***************************
                  db: db1
                name: testCur
                type: PROCEDURE
       specific_name: testCur
            language: SQL
     sql_data_access: CONTAINS_SQL
    is_deterministic: NO
       security_type: DEFINER
          param_list: 
             returns: 
                body: begin
declare id1 int;
end
             definer: skip-grants user@skip-grants host
             created: 2020-04-01 22:58:35
            modified: 2020-04-01 22:58:35
            sql_mode: ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION
             comment: 
character_set_client: utf8mb4
collation_connection: utf8mb4_general_ci
        db_collation: utf8mb4_general_ci
           body_utf8: begin
declare id1 int;
end
1 row in set (0.01 sec)

ERROR: 
No query specified
```


# 2 调式

## 2.1 sp_start_parsing
```

#0  sp_start_parsing (thd=0x7f3118000970, sp_type=SP_TYPE_PROCEDURE, sp_name=0x7f3118006dc8) at /home/wl/software/mysql-5.7.26/sql/sp.cc:2378
#1  0x000000000169d29c in MYSQLparse (YYTHD=0x7f3118000970)
    at /export/home2/pb2/build/sb_2-33647514-1555162505.73/mysql-5.7.26-release-export-14634697_gpl/sql/sql_yacc.yy:15361
#2  0x0000000001544ede in parse_sql (thd=0x7f3118000970, parser_state=0x7f313e14fdf0, creation_ctx=0x7f31180062d8)
    at /home/wl/software/mysql-5.7.26/sql/sql_parse.cc:7116
#3  0x000000000147a9d9 in sp_compile (thd=0x7f3118000970, defstr=0x7f313e150010, sql_mode=1436549152, creation_ctx=0x7f31180062d8)
    at /home/wl/software/mysql-5.7.26/sql/sp.cc:787
#4  0x000000000147ae70 in db_load_routine (thd=0x7f3118000970, type=SP_TYPE_PROCEDURE, name=0x7f313e151090, sphp=0x7f313e1512e0, sql_mode=1436549152, 
    params=0x1f56760 "", returns=0x1f56760 "", body=0x7f3118006248 "begin\ndeclare id1 int;\nend", chistics=..., 
    definer=0x7f3118006268 "skip-grants user@skip-grants host", created=20200401225835, modified=20200401225835, creation_ctx=0x7f31180062d8)
    at /home/wl/software/mysql-5.7.26/sql/sp.cc:920
#5  0x000000000147a771 in db_find_routine (thd=0x7f3118000970, type=SP_TYPE_PROCEDURE, name=0x7f313e151090, sphp=0x7f313e1512e0)
    at /home/wl/software/mysql-5.7.26/sql/sp.cc:710
#6  0x000000000147e619 in sp_cache_routine (thd=0x7f3118000970, type=SP_TYPE_PROCEDURE, name=0x7f313e151090, lookup_only=false, sp=0x7f313e1512e0)
    at /home/wl/software/mysql-5.7.26/sql/sp.cc:2154
#7  0x000000000147e4aa in sp_cache_routine (thd=0x7f3118000970, rt=0x7f3118006050, lookup_only=false, sp=0x7f313e1512e0)
    at /home/wl/software/mysql-5.7.26/sql/sp.cc:2106
#8  0x00000000014b980b in open_and_process_routine (thd=0x7f3118000970, prelocking_ctx=0x7f3118002c98, rt=0x7f3118006050, prelocking_strategy=0x7f313e1514b0, 
    has_prelocking_list=false, ot_ctx=0x7f313e151370, need_prelocking=0x7f313e1513b6, routine_modifies_data=0x7f313e1513b7)
    at /home/wl/software/mysql-5.7.26/sql/sql_base.cc:4956
#9  0x00000000014bafe7 in open_tables (thd=0x7f3118000970, start=0x7f313e151430, counter=0x7f313e151474, flags=0, prelocking_strategy=0x7f313e1514b0)
    at /home/wl/software/mysql-5.7.26/sql/sql_base.cc:5876
#10 0x00000000014bbf5f in open_and_lock_tables (thd=0x7f3118000970, tables=0x0, flags=0, prelocking_strategy=0x7f313e1514b0)
    at /home/wl/software/mysql-5.7.26/sql/sql_base.cc:6504
#11 0x0000000000ed83ff in open_and_lock_tables (thd=0x7f3118000970, tables=0x0, flags=0) at /home/wl/software/mysql-5.7.26/sql/sql_base.h:479
#12 0x000000000153f20e in mysql_execute_command (thd=0x7f3118000970, first_level=true) at /home/wl/software/mysql-5.7.26/sql/sql_parse.cc:4455
#13 0x0000000001541e89 in mysql_parse (thd=0x7f3118000970, parser_state=0x7f313e152690) at /home/wl/software/mysql-5.7.26/sql/sql_parse.cc:5570
#14 0x0000000001537918 in dispatch_command (thd=0x7f3118000970, com_data=0x7f313e152df0, command=COM_QUERY) at /home/wl/software/mysql-5.7.26/sql/sql_parse.cc:1484
#15 0x000000000153684c in do_command (thd=0x7f3118000970) at /home/wl/software/mysql-5.7.26/sql/sql_parse.cc:1025
#16 0x0000000001666e08 in handle_connection (arg=0x4f08ed0) at /home/wl/software/mysql-5.7.26/sql/conn_handler/connection_handler_per_thread.cc:306
#17 0x0000000001cf14d4 in pfs_spawn_thread (arg=0x4f0c890) at /home/wl/software/mysql-5.7.26/storage/perfschema/pfs.cc:2190
#18 0x00007f3141894dd5 in start_thread () from /lib64/libpthread.so.0
#19 0x00007f314055a02d in clone () from /lib64/libc.so.6

```

存储过程第一次执行时 call sp_name ，会将sphead 保存到hash表缓存中，当下次调用就不用再次  
解析（call sp_name, 根据sp_name 找到 存储过程mysql.proc中对应行信息，然后进行相应解析操作，  
变为存储过程调度处理程序可以识别的内容）

## 2.2 查看sp->m_root_parsing_ctx

```
> p *sp->m_root_parsing_ctx
$15 = {
  <Sql_alloc> = {<No data fields>}, 
  members of sp_pcontext: 
  m_level = 0, 
  m_max_var_index = 0, 
  m_max_cursor_index = 0, 
  m_parent = 0x0, 
  m_var_offset = 0, 
  m_cursor_offset = 0, 
  m_pboundary = 0, 
  m_num_case_exprs = 0, 
  m_vars = {
    <Mem_root_array_YY<sp_variable*, true>> = {
      m_root = 0x7f3110002d28, 
      m_array = 0x0, 
      m_size = 0, 
      m_capacity = 0
    }, <No data fields>}, 
  m_case_expr_ids = {
    <Mem_root_array_YY<int, true>> = {
      m_root = 0x7f3110002d28, 
      m_array = 0x0, 
      m_size = 0, 
      m_capacity = 0
    }, <No data fields>}, 
  m_conditions = {
    <Mem_root_array_YY<sp_condition*, true>> = {
      m_root = 0x7f3110002d28, 
      m_array = 0x0, 
      m_size = 0, 
      m_capacity = 0
    }, <No data fields>}, 
  m_cursors = {
    <Mem_root_array_YY<st_mysql_lex_string, true>> = {
      m_root = 0x7f3110002d28, 
      m_array = 0x0, 
      m_size = 0, 
      m_capacity = 0
    }, <No data fields>}, 
  m_handlers = {
    <Mem_root_array_YY<sp_handler*, true>> = {
      m_root = 0x7f3110002d28, 
      m_array = 0x0, 
      m_size = 0, 
      m_capacity = 0
    }, <No data fields>}, 
      m_labels = {
    <base_list> = {
      <Sql_alloc> = {<No data fields>}, 
      members of base_list: 
      first = 0x2d1a530 <end_of_list>, 
      last = 0x7f3110002ff8, 
      elements = 0
    }, <No data fields>}, 
  m_children = {
    <Mem_root_array_YY<sp_pcontext*, true>> = {
      m_root = 0x7f3110002d28, 
      m_array = 0x0, 
      m_size = 0, 
      m_capacity = 0
    }, <No data fields>}, 
  m_scope = sp_pcontext::REGULAR_SCOPE
}

```

## 2.3 查看 sphead
```
> p *sp

$61 = {
  <Query_arena> = {
    _vptr.Query_arena = 0x2c24a30 <vtable for sp_head+16>, 
    free_list = 0x0, 
    mem_root = 0x7f3110002d28, 
    is_backup_arena = false, 
    is_reprepared = false, 
    state = Query_arena::STMT_INITIALIZED_FOR_SP
  }, 
  members of sp_head: 
  m_type = SP_TYPE_PROCEDURE, 
  m_flags = 0, 
  m_sp_share = 0x0, 
  m_return_field_def = {
    <Sql_alloc> = {<No data fields>}, 
    members of Create_field: 
    field_name = 0x8f8f8f8f8f8f8f8f <Address 0x8f8f8f8f8f8f8f8f out of bounds>, 
    change = 0x8f8f8f8f8f8f8f8f <Address 0x8f8f8f8f8f8f8f8f out of bounds>, 
    after = 0x0, 
    comment = {
      str = 0x8f8f8f8f8f8f8f8f <Address 0x8f8f8f8f8f8f8f8f out of bounds>, 
      length = 10344644713448996865
    }, 
    def = 0x8f8f8f8f00008f00, 
    sql_type = 2408550287, 
    length = 10344644715844964239, 
    char_length = 10344644715844964239, 
    decimals = 2408550287, 
    flags = 2408550287, 
    pack_length = 10344644713436479488, 
    key_length = 10344644713436450560, 
    unireg_check = 2408550287, 
    interval = 0x8f8f8f8f8f8f8f8f, 
    save_interval = 0x8f8f8f8f8f8f8f8f, 
    interval_list = {
      <base_list> = {
        <Sql_alloc> = {<No data fields>}, 
        members of base_list: 
        first = 0x2d1a530 <end_of_list>, 
        last = 0x7f3110002888, 
        elements = 0
      }, <No data fields>}, 
    charset = 0x0, 
    geom_type = 2408550287, 
    field = 0x8f8f8f8f8f8f8f8f, 
    row = 143 '\217', 
    col = 143 '\217', 
    sc_length = 143 '\217', 
    interval_id = 143 '\217', 
    offset = 2408550287, 
    pack_flag = 65536, 
    gcol_info = 0x8f8f8f8f00008f00, 
    stored_in_db = 143
  }, 
  m_parser_data = {
    m_expr_start_ptr = 0x0, 
    m_current_stmt_start_ptr = 0x0, 
    m_option_start_ptr = 0x0, 
    m_lex_stack = {
      <base_list> = {
        <Sql_alloc> = {<No data fields>}, 
        members of base_list: 
        first = 0x2d1a530 <end_of_list>, 
        last = 0x7f31100028f0, 
        elements = 0
      }, <No data fields>}, 
    m_param_start_ptr = 0x7f311000a411 ")\nbegin\ndeclare id1 int;\nend", 
    m_param_end_ptr = 0x7f311000a411 ")\nbegin\ndeclare id1 int;\nend", 
    m_body_start_ptr = 0x7f311000a413 "begin\ndeclare id1 int;\nend", 
    m_backpatch = {
      <base_list> = {
        <Sql_alloc> = {<No data fields>}, 
        members of base_list: 
        first = 0x2d1a530 <end_of_list>, 
        last = 0x7f3110002920, 
        elements = 0
      }, <No data fields>}, 
    m_cont_backpatch = {
      <base_list> = {
        <Sql_alloc> = {<No data fields>}, 
        members of base_list: 
        first = 0x2d1a530 <end_of_list>, 
        last = 0x7f3110002938, 
        elements = 0
      }, <No data fields>}, 
    m_cont_level = 0, 
    m_saved_memroot = 0x0, 
    m_saved_free_list = 0x0
  }, 
  m_chistics = 0x7f31100044a0, 
  m_sql_mode = 1436549152, 
  m_qname = {
          str = 0x7f3110003048 "db1.testCur", 
    length = 11
  }, 
  m_explicit_name = false, 
  m_db = {
    str = 0x7f3110003038 "db1", 
    length = 3
  }, 
  m_name = {
    str = 0x7f3110003040 "testCur", 
    length = 7
  }, 
  m_params = {
    str = 0x7f31100043c0 "", 
    length = 0
  }, 
  m_body = {
    str = 0x7f31100043c8 "begin\ndeclare id1 int;\nend", 
    length = 26
  }, 
  m_body_utf8 = {
    str = 0x7f31100043e8 "begin\ndeclare id1 int;\nend", 
    length = 26
  }, 
  m_defstr = {
    str = 0x7f3110004408 "CREATE DEFINER=`skip-grants user`@`skip-grants host` PROCEDURE `testCur`()\nbegin\ndeclare id1 int;\nend", 
    length = 101
  }, 
  m_definer_user = {
    str = 0x7f3110004470 "skip-grants user", 
    length = 16
  }, 
  m_definer_host = {
    str = 0x7f3110004488 "skip-grants host", 
    length = 16
  }, 
  m_created = 20200401225835, 
  m_modified = 20200401225835, 
  m_recursion_level = 0, 
  m_next_cached_sp = 0x0, 
  m_first_instance = 0x7f31100027e0, 
  m_first_free_instance = 0x7f31100027e0, 
  m_last_cached_sp = 0x7f31100027e0, 
  m_sroutines = {
    key_offset = 0, 
    key_length = 0, 
    blength = 1, 
    records = 0, 
    flags = 0, 
    array = {
      buffer = 0x7f311002faf0 "\377", 
      elements = 0, 
      max_element = 511, 
      alloc_increment = 511, 
      size_of_element = 16, 
      m_psi_key = 0
    }, 
    get_key = 0x147dfbc <sp_sroutine_key(uchar const*, size_t*, my_bool)>, 
    free = 0x0, 
    charset = 0x2d04a40 <my_charset_utf8_general_ci>, 
    hash_function = 0x1852cd6 <cset_hash_sort_adapter>, 
    m_psi_key = 0
  }, 
  m_security_ctx = {
    m_user = {
      m_ptr = 0x0, 
      m_length = 0, 
      m_charset = 0x2d04a40 <my_charset_utf8_general_ci>, 
      m_alloced_length = 0, 
      m_is_alloced = false
    }, 
    m_host = {
      m_ptr = 0x1e226fd "", 
      m_length = 0, 
      m_charset = 0x2d04a40 <my_charset_utf8_general_ci>, 
      m_alloced_length = 0, 
      m_is_alloced = false
    }, 
    m_ip = {
      m_ptr = 0x1e226fd "", 
      m_length = 0, 
      m_charset = 0x2d04a40 <my_charset_utf8_general_ci>, 
      m_alloced_length = 0, 
      m_is_alloced = false
    }, 
    m_host_or_ip = {
      m_ptr = 0x1e22883 "connecting host", 
      m_length = 15, 
      m_charset = 0x2d04a40 <my_charset_utf8_general_ci>, 
      m_alloced_length = 0, 
      m_is_alloced = false
    }, 
    m_external_user = {
      m_ptr = 0x1e226fd "", 
      m_length = 0, 
      m_charset = 0x2d04a40 <my_charset_utf8_general_ci>, 
      m_alloced_length = 0, 
      m_is_alloced = false
    }, 
    m_priv_user = "\000UBSTRIN\001\000`\000quot\004d\000\000able, -90)), v_quoted_table), ' has \004\000\f\000wron\b \000\000finition.'); SIGNAL SQLSTATE", 
    m_priv_user_length = 0, 
    m_proxy_user = "\000SET MESSAGE_TEXT = v_error_msg; END IF; END IF; END IF;   IF (in_views IS NULL OR in_views = '') THEN SET in_views = 'with_runtimes_in_95th_percentile,analysis,w", 
    m_proxy_user_length = 0, 
    m_priv_host = "\000arnings,with_full_table_scans,with_sorting,with_temp_tables'", 
    m_priv_host_length = 0, 
    m_master_access = 0, 
    m_db_access = 1073741824, 
    m_password_expired = false
  }, 
  m_list_of_trig_fields_item_lists = {
    <Sql_alloc> = {<No data fields>}, 
    members of SQL_I_List<SQL_I_List<Item_trigger_field> >: 
    elements = 0, 
    first = 0x0, 
    next = 0x7f3110002cd8
  }, 
  m_cur_instr_trig_field_items = {
    <Sql_alloc> = {<No data fields>}, 
    members of SQL_I_List<Item_trigger_field>: 
    elements = 0, 
    first = 0x0, 
    next = 0x7f3110002cf0
  }, 
  m_trg_chistics = {
    action_time = (TRG_ACTION_AFTER | unknown: 1937011560), 
    event = 1226849065, 
    ordering_clause = TRG_ORDER_NONE, 
    anchor_trigger_name = {
      str = 0x0, 
      length = 0
    }
  }, 
  m_trg_list = 0x0, 
  main_mem_root = {
    free = 0x7f31100027d0, 
    used = 0x0, 
    pre_alloc = 0x7f31100027d0, 
    min_malloc = 32, 
    block_size = 8160, 
    block_num = 5, 
    first_block_usage = 1, 
    max_capacity = 18446744073709551615, 
    allocated_size = 16368, 
    error_for_capacity_exceeded = 1 '\001', 
    error_handler = 0x14b0295 <sql_alloc_error_handler()>, 
    m_psi_key = 0
  }, 
  m_root_parsing_ctx = 0x7f3110002f30, 
  m_instructions = {
    <Mem_root_array_YY<sp_instr*, true>> = {
      m_root = 0x7f3110002d28, 
      m_array = 0x7f3110002e30, 
      m_size = 1, 
      m_capacity = 32
    }, <No data fields>}, 
  m_sptabs = {
    key_offset = 0, 
    key_length = 0, 
    blength = 1, 
    records = 0, 
    flags = 0, 
    array = {
      buffer = 0x7f3110004810 "_SELECT_FULL_RANGE_JOIN`, 0) AS ''SUM_SELECT_FULL_RANGE_JOIN'', `d_end`.`SUM_SELECT_RANGE`-IFNULL(`d_start`.`SUM_SELECT_RANGE`, 0) AS ''SUM_SELECT_RANGE'', `d_end`.`SUM_SELECT_RANGE_CHECK`-IFNULL(`d_s"..., 
      elements = 0, 
      max_element = 511, 
      alloc_increment = 511, 
      size_of_element = 16, 
      m_psi_key = 0
    }, 
    get_key = 0x148130b <sp_table_key(unsigned char const*, unsigned long*, char)>, 
    free = 0x0, 
    charset = 0x2d04a40 <my_charset_utf8_general_ci>, 
    hash_function = 0x1852cd6 <cset_hash_sort_adapter>, 
    m_psi_key = 0
  }, 
  m_sp_cache_version = 0, 
  m_creation_ctx = 0x7f31100044c0, 
  unsafe_flags = 0
}
```

## 2.3 execute

```
#0  sp_head::execute (this=0x7f31100027e0, thd=0x7f31100124e0, merge_da_on_success=true) at /home/wl/software/mysql-5.7.26/sql/sp_head.cc:542
#1  0x0000000001484df6 in sp_head::execute_procedure (this=0x7f31100027e0, thd=0x7f31100124e0, args=0x7f3110014bb8)
    at /home/wl/software/mysql-5.7.26/sql/sp_head.cc:1522
#2  0x000000000153f489 in mysql_execute_command (thd=0x7f31100124e0, first_level=true) at /home/wl/software/mysql-5.7.26/sql/sql_parse.cc:4534
#3  0x0000000001541e89 in mysql_parse (thd=0x7f31100124e0, parser_state=0x7f313e194690) at /home/wl/software/mysql-5.7.26/sql/sql_parse.cc:5570
#4  0x0000000001537918 in dispatch_command (thd=0x7f31100124e0, com_data=0x7f313e194df0, command=COM_QUERY) at /home/wl/software/mysql-5.7.26/sql/sql_parse.cc:1484
#5  0x000000000153684c in do_command (thd=0x7f31100124e0) at /home/wl/software/mysql-5.7.26/sql/sql_parse.cc:1025
#6  0x0000000001666e08 in handle_connection (arg=0x4f34b40) at /home/wl/software/mysql-5.7.26/sql/conn_handler/connection_handler_per_thread.cc:306
#7  0x0000000001cf14d4 in pfs_spawn_thread (arg=0x4f0c890) at /home/wl/software/mysql-5.7.26/storage/perfschema/pfs.cc:2190
#8  0x00007f3141894dd5 in start_thread () from /lib64/libpthread.so.0
#9  0x00007f314055a02d in clone () from /lib64/libc.so.6
(gdb) 
```
