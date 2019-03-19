/*gcc -g -o $1 $2  -I ~/install/install_40/include/ -L ~/install/install_40/lib -lpq
 *
 *  begin; commit; 多次执行都不会报错，只是有个提示，对执行结果无影响
 */

#include <stdio.h>
#include <libpq-fe.h>
#include <stdlib.h>

void check(PGconn *conn)
{
	if(PQstatus(conn)==CONNECTION_BAD)
	{
		fprintf(stderr,"ERROR : %s\n", PQerrorMessage(conn));
		PQfinish(conn);
		exit(1);
	}
}

void execSql(PGconn *conn, char *sqlstr)
{
	PGresult *res;

	check(conn);

	res=PQexec(conn, sqlstr);

	if(PQresultStatus(res) == PGRES_COMMAND_OK)
	{
		fprintf(stderr,"ERROR : not use the right function for %s\n", sqlstr);
		PQfinish(conn);
		exit(1);
	}

	if(PQresultStatus(res) != PGRES_TUPLES_OK)
	{
		fprintf(stderr,"ERROR : %s\n", PQerrorMessage(conn));
		PQfinish(conn);
		exit(1);
	}
	else
		fprintf(stdout, "%s : ok\n", sqlstr);

	PQclear(res);
}

void execCommand(PGconn *conn, char *sqlstr)
{
	PGresult *res;

	check(conn);

	res=PQexec(conn, sqlstr);

	if(PQresultStatus(res) == PGRES_TUPLES_OK)
	{
		fprintf(stderr,"ERROR : not use the right function for %s\n", sqlstr);
		PQfinish(conn);
		exit(1);
	}

	if(PQresultStatus(res) != PGRES_COMMAND_OK)
	{
		fprintf(stderr,"ERROR : %s\n", PQerrorMessage(conn));
		PQfinish(conn);
		exit(1);
	}
	else
		fprintf(stdout, "%s : ok\n", sqlstr);

	PQclear(res);
}


int main()
{
	PGconn *conn;
	char *pghost="127.0.0.1";
	char *pgport="55100";
	char *pgoptions=NULL;
	char *pgtty=NULL;
	char *dbname="postgres";

	/*database name*/
	int i=0;
	int t=0;
	int s,k;

	conn=PQsetdb(pghost,pgport,pgoptions,pgtty,dbname);


	char *sqlstr1 = "select pg_pause_cluster();";
	char *sqlstr2 = "select pg_unpause_cluster();";
	char *sqlstr3 = "begin;";
	char *sqlstr4 = "commit;";
	char *sqlstr5 = "rollback;";

	char *sqlstr6 = "insert into t1 values(1);";
	char *sqlstr7 = "insert into t1 values(2);";
	char *sqlstr8 = "insert into t1 values(3);";

	//execCommand(conn, sqlstr3);
	// execSql(conn, sqlstr1);
	execCommand(conn, sqlstr3);
	execCommand(conn, sqlstr6);
	execCommand(conn, sqlstr3);
	execCommand(conn, sqlstr7);
	execCommand(conn, sqlstr4);

	getchar();
	execCommand(conn, sqlstr8);
	execCommand(conn, sqlstr4);
	execCommand(conn, sqlstr4);

	//getchar();
	//execSql(conn, sqlstr2);

	//getchar();
	//execCommand(conn, sqlstr4);

	//i=PQntuples(res);
	/*get the nummer of the record*/
	//t=PQnfields(res);
	/*get the fileds num*/
/* for(s=0;s<i;s++)
	{
	for(k=0;k<t;k++)
	{
		printf("%s",PQgetvalue(res,s,k));
		printf(" ");
	}
	printf("\n");
	}

	PQfinish(conn);
	PQclear(res);
*/

	return 0;
}
