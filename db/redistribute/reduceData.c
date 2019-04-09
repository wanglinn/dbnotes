
typedef struct TableInfoData
{
	char *relname;
	Oid relnamespace;
	Oid relpersistence;
} TableInfoData;

/*
 * create shadow table
 *
 */
static void
distrib_create_shadow(RedistribState *distribState, RedistribCommand *command)
{
	StringInfoData msg;
	List *remoteList = NIL;
	List *nodeOids = NIL;
	Relation rel;
	Oid relid;
	Oid OIDNewHeap;
	char relpersistence = RELPERSISTENCE_TEMP;
	char *relname;


		Assert(command->type == DISTRIB_COPY_TO);

	relid = distribState->relid;
	relname = get_rel_name(relid);
	relid = get_relname_relid(relname, relnamespace);
	rel = relation_open(relid, NoLock);
	reltablespace = rel->reltablespace;
	relpersistence = rel->relpersistence;
	relation_close(rel, NoLock);

	initStringInfo(&msg);

	nodeOids = adb_redist_get_nodeOids(distribState, DISTRIB_COPY_FROM);
	ClusterTocSetCustomFun(&msg, ClusterCreateShadowTable);

	begin_mem_toc_insert(&msg, REMOTE_KEY_CREATE_SHADOW_TABLE);
	save_node_string(&msg, relname);
	appendBinaryStringInfo(&msg, (char *)&reltablespace, sizeof(reltablespace));
	appendBinaryStringInfo(&msg, (char *)&relpersistence, sizeof(relpersistence));
	end_mem_toc_insert(&msg, REMOTE_KEY_CREATE_SHADOW_TABLE);

	remoteList = ExecClusterCustomFunction(nodeOids, &msg, 0);

}



static List *
adb_redist_get_nodeOids(RedistribState *distribState, RedistribOperation type)
{
	ListCell *item;

	if (distribState->commands = NIL)
		return NIL;

	foreach (item, distribState->commands)
	{
		RedistribCommand  *command = (RedistribCommand *)lfirst(item);
		if (command->type == type)
			return command->execNodes->nodeids;
	}

	return NIL;
}

static void
ClusterCreateShadowTable(StringInfo mem_toc)
{
	Oid relid;
	Oid relnamespace ;
	char *relname;
	StringInfoData buf;

	buf.data = mem_toc_lookup(mem_toc, REMOTE_KEY_CREATE_SHADOW_TABLE, &buf.maxlen);
	if (buf.data == NULL)
	{
		ereport(ERROR,
				(errmsg("Can not found shadowTableInfo in cluster message"),
				 errcode(ERRCODE_PROTOCOL_VIOLATION)));
	}
	buf.len = buf.maxlen;
	buf.cursor = 0;

	relname = buf.data;

	relid = get_relname_relid(relname, relnamespace);
	rel = relation_open(relid, NoLock);
	tableSpace = rel->reltablespace;
	relpersistence = rel->relpersistence;

	relation_close(rel, NoLock);

	OIDNewHeap = make_new_heap(relid, tableSpace, relpersistence,
							   ExclusiveLock);
	distribState->shadowRelid = OIDNewHeap;
}