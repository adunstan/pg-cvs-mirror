/*
 * dblink.c
 *
 * Functions returning results from a remote database
 *
 * Joe Conway <mail@joeconway.com>
 * And contributors:
 * Darko Prenosil <Darko.Prenosil@finteh.hr>
 * Shridhar Daithankar <shridhar_daithankar@persistent.co.in>
 *
 * Copyright (c) 2001-2006, PostgreSQL Global Development Group
 * ALL RIGHTS RESERVED;
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without a written agreement
 * is hereby granted, provided that the above copyright notice and this
 * paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL THE AUTHOR OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
 * LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF THE AUTHOR OR DISTRIBUTORS HAVE BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * THE AUTHOR AND DISTRIBUTORS SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE AUTHOR AND DISTRIBUTORS HAS NO OBLIGATIONS TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 */
#include "postgres.h"

#include "libpq-fe.h"
#include "fmgr.h"
#include "funcapi.h"
#include "miscadmin.h"
#include "access/tupdesc.h"
#include "access/heapam.h"
#include "catalog/catname.h"
#include "catalog/namespace.h"
#include "catalog/pg_index.h"
#include "catalog/pg_type.h"
#include "executor/executor.h"
#include "executor/spi.h"
#include "lib/stringinfo.h"
#include "nodes/nodes.h"
#include "nodes/execnodes.h"
#include "nodes/pg_list.h"
#include "parser/parse_type.h"
#include "parser/scansup.h"
#include "tcop/tcopprot.h"
#include "utils/builtins.h"
#include "utils/fmgroids.h"
#include "utils/acl.h"
#include "utils/array.h"
#include "utils/lsyscache.h"
#include "utils/syscache.h"
#include "utils/dynahash.h"
#include "utils/hsearch.h"
#include "utils/memutils.h"

#include "dblink.h"

typedef struct remoteConn
{
	PGconn	   *con;			/* Hold the remote connection */
	bool		remoteTrFlag;	/* Indicates whether or not a transaction
								 * on remote database is in progress */
}	remoteConn;

/*
 * Internal declarations
 */
static remoteConn *getConnectionByName(const char *name);
static HTAB *createConnHash(void);
static void createNewConnection(const char *name, remoteConn * con);
static void deleteConnection(const char *name);
static char **get_pkey_attnames(Relation rel, int16 *numatts);
static char *get_sql_insert(Relation rel, int *pkattnums, int pknumatts, char **src_pkattvals, char **tgt_pkattvals);
static char *get_sql_delete(Relation rel, int *pkattnums, int pknumatts, char **tgt_pkattvals);
static char *get_sql_update(Relation rel, int *pkattnums, int pknumatts, char **src_pkattvals, char **tgt_pkattvals);
static char *quote_literal_cstr(char *rawstr);
static char *quote_ident_cstr(char *rawstr);
static int	get_attnum_pk_pos(int *pkattnums, int pknumatts, int key);
static HeapTuple get_tuple_of_interest(Relation rel, int *pkattnums, int pknumatts, char **src_pkattvals);
static Relation get_rel_from_relname(text *relname_text, LOCKMODE lockmode, AclMode aclmode);
static char *generate_relation_name(Relation rel);
static char *connstr_strip_password(const char *connstr);
static void dblink_security_check(PGconn *conn, remoteConn *rcon, const char *connstr);
static void validate_pkattnums(Relation rel,
				   int16 *pkattnums_arg, int32 pknumatts_arg,
				   int **pkattnums, int *pknumatts);

/* Global */
List	   *res_id = NIL;
int			res_id_index = 0;
PGconn	   *persistent_conn = NULL;
static HTAB *remoteConnHash = NULL;

/*
Following is list that holds multiple remote connections.
Calling convention of each dblink function changes to accept
connection name as the first parameter. The connection list is
much like ecpg e.g. a mapping between a name and a PGconn object.
*/

typedef struct remoteConnHashEnt
{
	char		name[NAMEDATALEN];
	remoteConn *rcon;
}	remoteConnHashEnt;

/* initial number of connection hashes */
#define NUMCONN 16

/* general utility */
#define GET_TEXT(cstrp) DatumGetTextP(DirectFunctionCall1(textin, CStringGetDatum(cstrp)))
#define GET_STR(textp) DatumGetCString(DirectFunctionCall1(textout, PointerGetDatum(textp)))
#define xpfree(var_) \
	do { \
		if (var_ != NULL) \
		{ \
			pfree(var_); \
			var_ = NULL; \
		} \
	} while (0)
#define DBLINK_RES_INTERNALERROR(p2) \
	do { \
			msg = pstrdup(PQerrorMessage(conn)); \
			if (res) \
				PQclear(res); \
			elog(ERROR, "%s: %s", p2, msg); \
	} while (0)
#define DBLINK_RES_ERROR(p2) \
	do { \
			msg = pstrdup(PQerrorMessage(conn)); \
			if (res) \
				PQclear(res); \
			ereport(ERROR, \
					(errcode(ERRCODE_SYNTAX_ERROR), \
					 errmsg("%s", p2), \
					 errdetail("%s", msg))); \
	} while (0)
#define DBLINK_RES_ERROR_AS_NOTICE(p2) \
	do { \
			msg = pstrdup(PQerrorMessage(conn)); \
			if (res) \
				PQclear(res); \
			ereport(NOTICE, \
					(errcode(ERRCODE_SYNTAX_ERROR), \
					 errmsg("%s", p2), \
					 errdetail("%s", msg))); \
	} while (0)
#define DBLINK_CONN_NOT_AVAIL \
	do { \
		if(conname) \
			ereport(ERROR, \
					(errcode(ERRCODE_CONNECTION_DOES_NOT_EXIST), \
					 errmsg("connection \"%s\" not available", conname))); \
		else \
			ereport(ERROR, \
					(errcode(ERRCODE_CONNECTION_DOES_NOT_EXIST), \
					 errmsg("connection not available"))); \
	} while (0)
#define DBLINK_GET_CONN \
	do { \
			char *conname_or_str = GET_STR(PG_GETARG_TEXT_P(0)); \
			rcon = getConnectionByName(conname_or_str); \
			if(rcon) \
			{ \
				conn = rcon->con; \
			} \
			else \
			{ \
				connstr = conname_or_str; \
				dblink_security_check(conn, rcon, connstr); \
				conn = PQconnectdb(connstr); \
				if (PQstatus(conn) == CONNECTION_BAD) \
				{ \
					msg = pstrdup(PQerrorMessage(conn)); \
					PQfinish(conn); \
					ereport(ERROR, \
							(errcode(ERRCODE_SQLCLIENT_UNABLE_TO_ESTABLISH_SQLCONNECTION), \
							 errmsg("could not establish connection"), \
							 errdetail("%s", msg))); \
				} \
				freeconn = true; \
			} \
	} while (0)


/*
 * Create a persistent connection to another database
 */
PG_FUNCTION_INFO_V1(dblink_connect);
Datum
dblink_connect(PG_FUNCTION_ARGS)
{
	char	   *connstr = NULL;
	char	   *connname = NULL;
	char	   *msg;
	MemoryContext oldcontext;
	PGconn	   *conn = NULL;
	remoteConn *rcon = NULL;

	if (PG_NARGS() == 2)
	{
		connstr = GET_STR(PG_GETARG_TEXT_P(1));
		connname = GET_STR(PG_GETARG_TEXT_P(0));
	}
	else if (PG_NARGS() == 1)
		connstr = GET_STR(PG_GETARG_TEXT_P(0));

	oldcontext = MemoryContextSwitchTo(TopMemoryContext);

	if (connname)
		rcon = (remoteConn *) palloc(sizeof(remoteConn));

	/* check password used if not superuser */
	dblink_security_check(conn, rcon, connstr);
	conn = PQconnectdb(connstr);

	MemoryContextSwitchTo(oldcontext);

	if (PQstatus(conn) == CONNECTION_BAD)
	{
		msg = pstrdup(PQerrorMessage(conn));
		PQfinish(conn);
		if (rcon)
			pfree(rcon);

		ereport(ERROR,
		   (errcode(ERRCODE_SQLCLIENT_UNABLE_TO_ESTABLISH_SQLCONNECTION),
			errmsg("could not establish connection"),
			errdetail("%s", msg)));
	}

	if (connname)
	{
		rcon->con = conn;
		createNewConnection(connname, rcon);
	}
	else
		persistent_conn = conn;

	PG_RETURN_TEXT_P(GET_TEXT("OK"));
}

/*
 * Clear a persistent connection to another database
 */
PG_FUNCTION_INFO_V1(dblink_disconnect);
Datum
dblink_disconnect(PG_FUNCTION_ARGS)
{
	char	   *conname = NULL;
	remoteConn *rcon = NULL;
	PGconn	   *conn = NULL;

	if (PG_NARGS() == 1)
	{
		conname = GET_STR(PG_GETARG_TEXT_P(0));
		rcon = getConnectionByName(conname);
		if (rcon)
			conn = rcon->con;
	}
	else
		conn = persistent_conn;

	if (!conn)
		DBLINK_CONN_NOT_AVAIL;

	PQfinish(conn);
	if (rcon)
	{
		deleteConnection(conname);
		pfree(rcon);
	}
	else
		persistent_conn = NULL;

	PG_RETURN_TEXT_P(GET_TEXT("OK"));
}

/*
 * opens a cursor using a persistent connection
 */
PG_FUNCTION_INFO_V1(dblink_open);
Datum
dblink_open(PG_FUNCTION_ARGS)
{
	char	   *msg;
	PGresult   *res = NULL;
	PGconn	   *conn = NULL;
	char	   *curname = NULL;
	char	   *sql = NULL;
	char	   *conname = NULL;
	StringInfo	str = makeStringInfo();
	remoteConn *rcon = NULL;
	bool		fail = true;	/* default to backward compatible behavior */

	if (PG_NARGS() == 2)
	{
		/* text,text */
		curname = GET_STR(PG_GETARG_TEXT_P(0));
		sql = GET_STR(PG_GETARG_TEXT_P(1));
		conn = persistent_conn;
	}
	else if (PG_NARGS() == 3)
	{
		/* might be text,text,text or text,text,bool */
		if (get_fn_expr_argtype(fcinfo->flinfo, 2) == BOOLOID)
		{
			curname = GET_STR(PG_GETARG_TEXT_P(0));
			sql = GET_STR(PG_GETARG_TEXT_P(1));
			fail = PG_GETARG_BOOL(2);
			conn = persistent_conn;
		}
		else
		{
			conname = GET_STR(PG_GETARG_TEXT_P(0));
			curname = GET_STR(PG_GETARG_TEXT_P(1));
			sql = GET_STR(PG_GETARG_TEXT_P(2));
			rcon = getConnectionByName(conname);
			if (rcon)
				conn = rcon->con;
		}
	}
	else if (PG_NARGS() == 4)
	{
		/* text,text,text,bool */
		conname = GET_STR(PG_GETARG_TEXT_P(0));
		curname = GET_STR(PG_GETARG_TEXT_P(1));
		sql = GET_STR(PG_GETARG_TEXT_P(2));
		fail = PG_GETARG_BOOL(3);
		rcon = getConnectionByName(conname);
		if (rcon)
			conn = rcon->con;
	}

	if (!conn)
		DBLINK_CONN_NOT_AVAIL;

	res = PQexec(conn, "BEGIN");
	if (PQresultStatus(res) != PGRES_COMMAND_OK)
		DBLINK_RES_INTERNALERROR("begin error");

	PQclear(res);

	appendStringInfo(str, "DECLARE %s CURSOR FOR %s", curname, sql);
	res = PQexec(conn, str->data);
	if (!res || PQresultStatus(res) != PGRES_COMMAND_OK)
	{
		if (fail)
			DBLINK_RES_ERROR("sql error");
		else
		{
			DBLINK_RES_ERROR_AS_NOTICE("sql error");
			PG_RETURN_TEXT_P(GET_TEXT("ERROR"));
		}
	}

	PQclear(res);
	PG_RETURN_TEXT_P(GET_TEXT("OK"));
}

/*
 * closes a cursor
 */
PG_FUNCTION_INFO_V1(dblink_close);
Datum
dblink_close(PG_FUNCTION_ARGS)
{
	PGconn	   *conn = NULL;
	PGresult   *res = NULL;
	char	   *curname = NULL;
	char	   *conname = NULL;
	StringInfo	str = makeStringInfo();
	char	   *msg;
	remoteConn *rcon = NULL;
	bool		fail = true;	/* default to backward compatible behavior */

	if (PG_NARGS() == 1)
	{
		/* text */
		curname = GET_STR(PG_GETARG_TEXT_P(0));
		conn = persistent_conn;
	}
	else if (PG_NARGS() == 2)
	{
		/* might be text,text or text,bool */
		if (get_fn_expr_argtype(fcinfo->flinfo, 1) == BOOLOID)
		{
			curname = GET_STR(PG_GETARG_TEXT_P(0));
			fail = PG_GETARG_BOOL(1);
			conn = persistent_conn;
		}
		else
		{
			conname = GET_STR(PG_GETARG_TEXT_P(0));
			curname = GET_STR(PG_GETARG_TEXT_P(1));
			rcon = getConnectionByName(conname);
			if (rcon)
				conn = rcon->con;
		}
	}
	if (PG_NARGS() == 3)
	{
		/* text,text,bool */
		conname = GET_STR(PG_GETARG_TEXT_P(0));
		curname = GET_STR(PG_GETARG_TEXT_P(1));
		fail = PG_GETARG_BOOL(2);
		rcon = getConnectionByName(conname);
		if (rcon)
			conn = rcon->con;
	}

	if (!conn)
		DBLINK_CONN_NOT_AVAIL;

	appendStringInfo(str, "CLOSE %s", curname);

	/* close the cursor */
	res = PQexec(conn, str->data);
	if (!res || PQresultStatus(res) != PGRES_COMMAND_OK)
	{
		if (fail)
			DBLINK_RES_ERROR("sql error");
		else
		{
			DBLINK_RES_ERROR_AS_NOTICE("sql error");
			PG_RETURN_TEXT_P(GET_TEXT("ERROR"));
		}
	}

	PQclear(res);

	/* commit the transaction */
	res = PQexec(conn, "COMMIT");
	if (PQresultStatus(res) != PGRES_COMMAND_OK)
		DBLINK_RES_INTERNALERROR("commit error");

	PQclear(res);

	PG_RETURN_TEXT_P(GET_TEXT("OK"));
}

/*
 * Fetch results from an open cursor
 */
PG_FUNCTION_INFO_V1(dblink_fetch);
Datum
dblink_fetch(PG_FUNCTION_ARGS)
{
	FuncCallContext *funcctx;
	TupleDesc	tupdesc = NULL;
	int			call_cntr;
	int			max_calls;
	AttInMetadata *attinmeta;
	char	   *msg;
	PGresult   *res = NULL;
	MemoryContext oldcontext;
	char	   *conname = NULL;
	remoteConn *rcon = NULL;

	/* stuff done only on the first call of the function */
	if (SRF_IS_FIRSTCALL())
	{
		Oid			functypeid;
		char		functyptype;
		Oid			funcid = fcinfo->flinfo->fn_oid;
		PGconn	   *conn = NULL;
		StringInfo	str = makeStringInfo();
		char	   *curname = NULL;
		int			howmany = 0;
		ReturnSetInfo *rsinfo = (ReturnSetInfo *) fcinfo->resultinfo;
		bool		fail = true;	/* default to backward compatible */

		if (PG_NARGS() == 4)
		{
			/* text,text,int,bool */
			conname = GET_STR(PG_GETARG_TEXT_P(0));
			curname = GET_STR(PG_GETARG_TEXT_P(1));
			howmany = PG_GETARG_INT32(2);
			fail = PG_GETARG_BOOL(3);

			rcon = getConnectionByName(conname);
			if (rcon)
				conn = rcon->con;
		}
		else if (PG_NARGS() == 3)
		{
			/* text,text,int or text,int,bool */
			if (get_fn_expr_argtype(fcinfo->flinfo, 2) == BOOLOID)
			{
				curname = GET_STR(PG_GETARG_TEXT_P(0));
				howmany = PG_GETARG_INT32(1);
				fail = PG_GETARG_BOOL(2);
				conn = persistent_conn;
			}
			else
			{
				conname = GET_STR(PG_GETARG_TEXT_P(0));
				curname = GET_STR(PG_GETARG_TEXT_P(1));
				howmany = PG_GETARG_INT32(2);

				rcon = getConnectionByName(conname);
				if (rcon)
					conn = rcon->con;
			}
		}
		else if (PG_NARGS() == 2)
		{
			/* text,int */
			curname = GET_STR(PG_GETARG_TEXT_P(0));
			howmany = PG_GETARG_INT32(1);
			conn = persistent_conn;
		}

		if (!conn)
			DBLINK_CONN_NOT_AVAIL;

		/* create a function context for cross-call persistence */
		funcctx = SRF_FIRSTCALL_INIT();

		/*
		 * switch to memory context appropriate for multiple function
		 * calls
		 */
		oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

		appendStringInfo(str, "FETCH %d FROM %s", howmany, curname);

		res = PQexec(conn, str->data);
		if (!res ||
			(PQresultStatus(res) != PGRES_COMMAND_OK &&
			 PQresultStatus(res) != PGRES_TUPLES_OK))
		{
			if (fail)
				DBLINK_RES_ERROR("sql error");
			else
			{
				DBLINK_RES_ERROR_AS_NOTICE("sql error");
				SRF_RETURN_DONE(funcctx);
			}
		}
		else if (PQresultStatus(res) == PGRES_COMMAND_OK)
		{
			/* cursor does not exist - closed already or bad name */
			PQclear(res);
			ereport(ERROR,
					(errcode(ERRCODE_INVALID_CURSOR_NAME),
					 errmsg("cursor \"%s\" does not exist", curname)));
		}

		funcctx->max_calls = PQntuples(res);

		/* got results, keep track of them */
		funcctx->user_fctx = res;

		/* check typtype to see if we have a predetermined return type */
		functypeid = get_func_rettype(funcid);
		functyptype = get_typtype(functypeid);

		if (functyptype == 'c')
			tupdesc = TypeGetTupleDesc(functypeid, NIL);
		else if (functypeid == RECORDOID)
		{
			if (!rsinfo || !IsA(rsinfo, ReturnSetInfo) ||
				rsinfo->expectedDesc == NULL)
				ereport(ERROR,
						(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
					errmsg("function returning record called in context "
						   "that cannot accept type record")));

			/* get the requested return tuple description */
			tupdesc = CreateTupleDescCopy(rsinfo->expectedDesc);
		}
		else
			/* shouldn't happen */
			elog(ERROR, "return type must be a row type");

		/* check result and tuple descriptor have the same number of columns */
		if (PQnfields(res) != tupdesc->natts)
			ereport(ERROR,
					(errcode(ERRCODE_DATATYPE_MISMATCH),
				errmsg("remote query result rowtype does not match "
						"the specified FROM clause rowtype")));

		/* fast track when no results */
		if (funcctx->max_calls < 1)
		{
			if (res)
				PQclear(res);
			SRF_RETURN_DONE(funcctx);
		}

		/* store needed metadata for subsequent calls */
		attinmeta = TupleDescGetAttInMetadata(tupdesc);
		funcctx->attinmeta = attinmeta;

		MemoryContextSwitchTo(oldcontext);
	}

	/* stuff done on every call of the function */
	funcctx = SRF_PERCALL_SETUP();

	/*
	 * initialize per-call variables
	 */
	call_cntr = funcctx->call_cntr;
	max_calls = funcctx->max_calls;

	res = (PGresult *) funcctx->user_fctx;
	attinmeta = funcctx->attinmeta;
	tupdesc = attinmeta->tupdesc;

	if (call_cntr < max_calls)	/* do when there is more left to send */
	{
		char	  **values;
		HeapTuple	tuple;
		Datum		result;
		int			i;
		int			nfields = PQnfields(res);

		values = (char **) palloc(nfields * sizeof(char *));
		for (i = 0; i < nfields; i++)
		{
			if (PQgetisnull(res, call_cntr, i) == 0)
				values[i] = PQgetvalue(res, call_cntr, i);
			else
				values[i] = NULL;
		}

		/* build the tuple */
		tuple = BuildTupleFromCStrings(attinmeta, values);

		/* make the tuple into a datum */
		result = HeapTupleGetDatum(tuple);

		SRF_RETURN_NEXT(funcctx, result);
	}
	else
	{
		/* do when there is no more left */
		PQclear(res);
		SRF_RETURN_DONE(funcctx);
	}
}

/*
 * Note: this is the new preferred version of dblink
 */
PG_FUNCTION_INFO_V1(dblink_record);
Datum
dblink_record(PG_FUNCTION_ARGS)
{
	FuncCallContext *funcctx;
	TupleDesc	tupdesc = NULL;
	int			call_cntr;
	int			max_calls;
	AttInMetadata *attinmeta;
	char	   *msg;
	PGresult   *res = NULL;
	bool		is_sql_cmd = false;
	char	   *sql_cmd_status = NULL;
	MemoryContext oldcontext;
	bool		freeconn = false;

	/* stuff done only on the first call of the function */
	if (SRF_IS_FIRSTCALL())
	{
		Oid			functypeid;
		char		functyptype;
		Oid			funcid = fcinfo->flinfo->fn_oid;
		PGconn	   *conn = NULL;
		char	   *connstr = NULL;
		char	   *sql = NULL;
		char	   *conname = NULL;
		remoteConn *rcon = NULL;
		ReturnSetInfo *rsinfo = (ReturnSetInfo *) fcinfo->resultinfo;
		bool		fail = true;	/* default to backward compatible */

		/* create a function context for cross-call persistence */
		funcctx = SRF_FIRSTCALL_INIT();

		/*
		 * switch to memory context appropriate for multiple function
		 * calls
		 */
		oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

		if (PG_NARGS() == 3)
		{
			/* text,text,bool */
			DBLINK_GET_CONN;
			sql = GET_STR(PG_GETARG_TEXT_P(1));
			fail = PG_GETARG_BOOL(2);
		}
		else if (PG_NARGS() == 2)
		{
			/* text,text or text,bool */
			if (get_fn_expr_argtype(fcinfo->flinfo, 1) == BOOLOID)
			{
				conn = persistent_conn;
				sql = GET_STR(PG_GETARG_TEXT_P(0));
				fail = PG_GETARG_BOOL(1);
			}
			else
			{
				DBLINK_GET_CONN;
				sql = GET_STR(PG_GETARG_TEXT_P(1));
			}
		}
		else if (PG_NARGS() == 1)
		{
			/* text */
			conn = persistent_conn;
			sql = GET_STR(PG_GETARG_TEXT_P(0));
		}
		else
			/* shouldn't happen */
			elog(ERROR, "wrong number of arguments");

		if (!conn)
			DBLINK_CONN_NOT_AVAIL;

		res = PQexec(conn, sql);
		if (!res ||
			(PQresultStatus(res) != PGRES_COMMAND_OK &&
			 PQresultStatus(res) != PGRES_TUPLES_OK))
		{
			if (fail)
				DBLINK_RES_ERROR("sql error");
			else
			{
				DBLINK_RES_ERROR_AS_NOTICE("sql error");
				if (freeconn)
					PQfinish(conn);
				SRF_RETURN_DONE(funcctx);
			}
		}

		if (PQresultStatus(res) == PGRES_COMMAND_OK)
		{
			is_sql_cmd = true;

			/* need a tuple descriptor representing one TEXT column */
			tupdesc = CreateTemplateTupleDesc(1, false);
			TupleDescInitEntry(tupdesc, (AttrNumber) 1, "status",
							   TEXTOID, -1, 0);

			/*
			 * and save a copy of the command status string to return as
			 * our result tuple
			 */
			sql_cmd_status = PQcmdStatus(res);
			funcctx->max_calls = 1;
		}
		else
			funcctx->max_calls = PQntuples(res);

		/* got results, keep track of them */
		funcctx->user_fctx = res;

		/* if needed, close the connection to the database and cleanup */
		if (freeconn)
			PQfinish(conn);

		/* check typtype to see if we have a predetermined return type */
		functypeid = get_func_rettype(funcid);
		functyptype = get_typtype(functypeid);

		if (!is_sql_cmd)
		{
			if (functyptype == 'c')
				tupdesc = TypeGetTupleDesc(functypeid, NIL);
			else if (functypeid == RECORDOID)
			{
				if (!rsinfo || !IsA(rsinfo, ReturnSetInfo) ||
					rsinfo->expectedDesc == NULL)
					ereport(ERROR,
							(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
					errmsg("function returning record called in context "
						   "that cannot accept type record")));

				/* get the requested return tuple description */
				tupdesc = CreateTupleDescCopy(rsinfo->expectedDesc);
			}
			else
				/* shouldn't happen */
				elog(ERROR, "return type must be a row type");
		}

		/* check result and tuple descriptor have the same number of columns */
		if (PQnfields(res) != tupdesc->natts)
			ereport(ERROR,
					(errcode(ERRCODE_DATATYPE_MISMATCH),
				errmsg("remote query result rowtype does not match "
						"the specified FROM clause rowtype")));

		/* fast track when no results */
		if (funcctx->max_calls < 1)
		{
			if (res)
				PQclear(res);
			SRF_RETURN_DONE(funcctx);
		}

		/* store needed metadata for subsequent calls */
		attinmeta = TupleDescGetAttInMetadata(tupdesc);
		funcctx->attinmeta = attinmeta;

		MemoryContextSwitchTo(oldcontext);
	}

	/* stuff done on every call of the function */
	funcctx = SRF_PERCALL_SETUP();

	/*
	 * initialize per-call variables
	 */
	call_cntr = funcctx->call_cntr;
	max_calls = funcctx->max_calls;

	res = (PGresult *) funcctx->user_fctx;
	attinmeta = funcctx->attinmeta;
	tupdesc = attinmeta->tupdesc;

	if (call_cntr < max_calls)	/* do when there is more left to send */
	{
		char	  **values;
		HeapTuple	tuple;
		Datum		result;

		if (!is_sql_cmd)
		{
			int			i;
			int			nfields = PQnfields(res);

			values = (char **) palloc(nfields * sizeof(char *));
			for (i = 0; i < nfields; i++)
			{
				if (PQgetisnull(res, call_cntr, i) == 0)
					values[i] = PQgetvalue(res, call_cntr, i);
				else
					values[i] = NULL;
			}
		}
		else
		{
			values = (char **) palloc(1 * sizeof(char *));
			values[0] = sql_cmd_status;
		}

		/* build the tuple */
		tuple = BuildTupleFromCStrings(attinmeta, values);

		/* make the tuple into a datum */
		result = HeapTupleGetDatum(tuple);

		SRF_RETURN_NEXT(funcctx, result);
	}
	else
	{
		/* do when there is no more left */
		PQclear(res);
		SRF_RETURN_DONE(funcctx);
	}
}

/*
 * Execute an SQL non-SELECT command
 */
PG_FUNCTION_INFO_V1(dblink_exec);
Datum
dblink_exec(PG_FUNCTION_ARGS)
{
	char	   *msg;
	PGresult   *res = NULL;
	text	   *sql_cmd_status = NULL;
	TupleDesc	tupdesc = NULL;
	PGconn	   *conn = NULL;
	char	   *connstr = NULL;
	char	   *sql = NULL;
	char	   *conname = NULL;
	remoteConn *rcon = NULL;
	bool		freeconn = false;
	bool		fail = true;	/* default to backward compatible behavior */

	if (PG_NARGS() == 3)
	{
		/* must be text,text,bool */
		DBLINK_GET_CONN;
		sql = GET_STR(PG_GETARG_TEXT_P(1));
		fail = PG_GETARG_BOOL(2);
	}
	else if (PG_NARGS() == 2)
	{
		/* might be text,text or text,bool */
		if (get_fn_expr_argtype(fcinfo->flinfo, 1) == BOOLOID)
		{
			conn = persistent_conn;
			sql = GET_STR(PG_GETARG_TEXT_P(0));
			fail = PG_GETARG_BOOL(1);
		}
		else
		{
			DBLINK_GET_CONN;
			sql = GET_STR(PG_GETARG_TEXT_P(1));
		}
	}
	else if (PG_NARGS() == 1)
	{
		/* must be single text argument */
		conn = persistent_conn;
		sql = GET_STR(PG_GETARG_TEXT_P(0));
	}
	else
		/* shouldn't happen */
		elog(ERROR, "wrong number of arguments");

	if (!conn)
		DBLINK_CONN_NOT_AVAIL;

	res = PQexec(conn, sql);
	if (!res ||
		(PQresultStatus(res) != PGRES_COMMAND_OK &&
		 PQresultStatus(res) != PGRES_TUPLES_OK))
	{
		if (fail)
			DBLINK_RES_ERROR("sql error");
		else
			DBLINK_RES_ERROR_AS_NOTICE("sql error");

		/* need a tuple descriptor representing one TEXT column */
		tupdesc = CreateTemplateTupleDesc(1, false);
		TupleDescInitEntry(tupdesc, (AttrNumber) 1, "status",
						   TEXTOID, -1, 0);

		/*
		 * and save a copy of the command status string to return as our
		 * result tuple
		 */
		sql_cmd_status = GET_TEXT("ERROR");

	}
	else if (PQresultStatus(res) == PGRES_COMMAND_OK)
	{
		/* need a tuple descriptor representing one TEXT column */
		tupdesc = CreateTemplateTupleDesc(1, false);
		TupleDescInitEntry(tupdesc, (AttrNumber) 1, "status",
						   TEXTOID, -1, 0);

		/*
		 * and save a copy of the command status string to return as our
		 * result tuple
		 */
		sql_cmd_status = GET_TEXT(PQcmdStatus(res));
		PQclear(res);
	}
	else
	{
		PQclear(res);
		ereport(ERROR,
			  (errcode(ERRCODE_S_R_E_PROHIBITED_SQL_STATEMENT_ATTEMPTED),
			   errmsg("statement returning results not allowed")));
	}

	/* if needed, close the connection to the database and cleanup */
	if (freeconn)
		PQfinish(conn);

	PG_RETURN_TEXT_P(sql_cmd_status);
}


/*
 * dblink_get_pkey
 *
 * Return list of primary key fields for the supplied relation,
 * or NULL if none exists.
 */
PG_FUNCTION_INFO_V1(dblink_get_pkey);
Datum
dblink_get_pkey(PG_FUNCTION_ARGS)
{
	int16		numatts;
	char	  **results;
	FuncCallContext *funcctx;
	int32		call_cntr;
	int32		max_calls;
	AttInMetadata *attinmeta;
	MemoryContext oldcontext;

	/* stuff done only on the first call of the function */
	if (SRF_IS_FIRSTCALL())
	{
		Relation	rel;
		TupleDesc	tupdesc;

		/* create a function context for cross-call persistence */
		funcctx = SRF_FIRSTCALL_INIT();

		/*
		 * switch to memory context appropriate for multiple function
		 * calls
		 */
		oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

		/* open target relation */
		rel = get_rel_from_relname(PG_GETARG_TEXT_P(0), AccessShareLock, ACL_SELECT);

		/* get the array of attnums */
		results = get_pkey_attnames(rel, &numatts);

		relation_close(rel, AccessShareLock);

		/*
		 * need a tuple descriptor representing one INT and one TEXT
		 * column
		 */
		tupdesc = CreateTemplateTupleDesc(2, false);
		TupleDescInitEntry(tupdesc, (AttrNumber) 1, "position",
						   INT4OID, -1, 0);
		TupleDescInitEntry(tupdesc, (AttrNumber) 2, "colname",
						   TEXTOID, -1, 0);

		/*
		 * Generate attribute metadata needed later to produce tuples from
		 * raw C strings
		 */
		attinmeta = TupleDescGetAttInMetadata(tupdesc);
		funcctx->attinmeta = attinmeta;

		if ((results != NULL) && (numatts > 0))
		{
			funcctx->max_calls = numatts;

			/* got results, keep track of them */
			funcctx->user_fctx = results;
		}
		else
			/* fast track when no results */
			SRF_RETURN_DONE(funcctx);

		MemoryContextSwitchTo(oldcontext);
	}

	/* stuff done on every call of the function */
	funcctx = SRF_PERCALL_SETUP();

	/*
	 * initialize per-call variables
	 */
	call_cntr = funcctx->call_cntr;
	max_calls = funcctx->max_calls;

	results = (char **) funcctx->user_fctx;
	attinmeta = funcctx->attinmeta;

	if (call_cntr < max_calls)	/* do when there is more left to send */
	{
		char	  **values;
		HeapTuple	tuple;
		Datum		result;

		values = (char **) palloc(2 * sizeof(char *));
		values[0] = (char *) palloc(12);		/* sign, 10 digits, '\0' */

		sprintf(values[0], "%d", call_cntr + 1);

		values[1] = results[call_cntr];

		/* build the tuple */
		tuple = BuildTupleFromCStrings(attinmeta, values);

		/* make the tuple into a datum */
		result = HeapTupleGetDatum(tuple);

		SRF_RETURN_NEXT(funcctx, result);
	}
	else
	{
		/* do when there is no more left */
		SRF_RETURN_DONE(funcctx);
	}
}


/*
 * dblink_build_sql_insert
 *
 * Used to generate an SQL insert statement
 * based on an existing tuple in a local relation.
 * This is useful for selectively replicating data
 * to another server via dblink.
 *
 * API:
 * <relname> - name of local table of interest
 * <pkattnums> - an int2vector of attnums which will be used
 * to identify the local tuple of interest
 * <pknumatts> - number of attnums in pkattnums
 * <src_pkattvals_arry> - text array of key values which will be used
 * to identify the local tuple of interest
 * <tgt_pkattvals_arry> - text array of key values which will be used
 * to build the string for execution remotely. These are substituted
 * for their counterparts in src_pkattvals_arry
 */
PG_FUNCTION_INFO_V1(dblink_build_sql_insert);
Datum
dblink_build_sql_insert(PG_FUNCTION_ARGS)
{
	Relation	rel;
	text	   *relname_text;
	int16	   *pkattnums_arg;
	int32		pknumatts_arg;
	int		   *pkattnums;
	int			pknumatts;
	char	  **src_pkattvals;
	char	  **tgt_pkattvals;
	ArrayType  *src_pkattvals_arry;
	ArrayType  *tgt_pkattvals_arry;
	int			src_ndim;
	int		   *src_dim;
	int			src_nitems;
	int			tgt_ndim;
	int		   *tgt_dim;
	int			tgt_nitems;
	int			i;
	char	   *ptr;
	char	   *sql;
	int16		typlen;
	bool		typbyval;
	char		typalign;

	relname_text = PG_GETARG_TEXT_P(0);

	/*
	 * Open target relation.
	 */
	rel = get_rel_from_relname(relname_text, AccessShareLock, ACL_SELECT);

	/*
	 * Process pkattnums argument.
	 */
	pkattnums_arg = (int16 *) PG_GETARG_POINTER(1);
	pknumatts_arg = PG_GETARG_INT32(2);
	validate_pkattnums(rel, pkattnums_arg, pknumatts_arg,
					   &pkattnums, &pknumatts);

	src_pkattvals_arry = PG_GETARG_ARRAYTYPE_P(3);
	tgt_pkattvals_arry = PG_GETARG_ARRAYTYPE_P(4);

	/*
	 * Source array is made up of key values that will be used to locate
	 * the tuple of interest from the local system.
	 */
	src_ndim = ARR_NDIM(src_pkattvals_arry);
	src_dim = ARR_DIMS(src_pkattvals_arry);
	src_nitems = ArrayGetNItems(src_ndim, src_dim);

	/*
	 * There should be one source array key value for each key attnum
	 */
	if (src_nitems != pknumatts)
		ereport(ERROR,
				(errcode(ERRCODE_ARRAY_SUBSCRIPT_ERROR),
			 errmsg("source key array length must match number of key " \
					"attributes")));

	/*
	 * get array of pointers to c-strings from the input source array
	 */
	Assert(ARR_ELEMTYPE(src_pkattvals_arry) == TEXTOID);
	get_typlenbyvalalign(ARR_ELEMTYPE(src_pkattvals_arry),
						 &typlen, &typbyval, &typalign);

	src_pkattvals = (char **) palloc(src_nitems * sizeof(char *));
	ptr = ARR_DATA_PTR(src_pkattvals_arry);
	for (i = 0; i < src_nitems; i++)
	{
		src_pkattvals[i] = DatumGetCString(DirectFunctionCall1(textout, PointerGetDatum(ptr)));
		ptr = att_addlength(ptr, typlen, PointerGetDatum(ptr));
		ptr = (char *) att_align(ptr, typalign);
	}

	/*
	 * Target array is made up of key values that will be used to build
	 * the SQL string for use on the remote system.
	 */
	tgt_ndim = ARR_NDIM(tgt_pkattvals_arry);
	tgt_dim = ARR_DIMS(tgt_pkattvals_arry);
	tgt_nitems = ArrayGetNItems(tgt_ndim, tgt_dim);

	/*
	 * There should be one target array key value for each key attnum
	 */
	if (tgt_nitems != pknumatts)
		ereport(ERROR,
				(errcode(ERRCODE_ARRAY_SUBSCRIPT_ERROR),
			 errmsg("target key array length must match number of key " \
					"attributes")));

	/*
	 * get array of pointers to c-strings from the input target array
	 */
	Assert(ARR_ELEMTYPE(tgt_pkattvals_arry) == TEXTOID);
	get_typlenbyvalalign(ARR_ELEMTYPE(tgt_pkattvals_arry),
						 &typlen, &typbyval, &typalign);

	tgt_pkattvals = (char **) palloc(tgt_nitems * sizeof(char *));
	ptr = ARR_DATA_PTR(tgt_pkattvals_arry);
	for (i = 0; i < tgt_nitems; i++)
	{
		tgt_pkattvals[i] = DatumGetCString(DirectFunctionCall1(textout, PointerGetDatum(ptr)));
		ptr = att_addlength(ptr, typlen, PointerGetDatum(ptr));
		ptr = (char *) att_align(ptr, typalign);
	}

	/*
	 * Prep work is finally done. Go get the SQL string.
	 */
	sql = get_sql_insert(rel, pkattnums, pknumatts, src_pkattvals, tgt_pkattvals);

	/*
	 * Now we can close the relation.
	 */
	relation_close(rel, AccessShareLock);

	/*
	 * And send it
	 */
	PG_RETURN_TEXT_P(GET_TEXT(sql));
}


/*
 * dblink_build_sql_delete
 *
 * Used to generate an SQL delete statement.
 * This is useful for selectively replicating a
 * delete to another server via dblink.
 *
 * API:
 * <relname> - name of remote table of interest
 * <pkattnums> - an int2vector of attnums which will be used
 * to identify the remote tuple of interest
 * <pknumatts> - number of attnums in pkattnums
 * <tgt_pkattvals_arry> - text array of key values which will be used
 * to build the string for execution remotely.
 */
PG_FUNCTION_INFO_V1(dblink_build_sql_delete);
Datum
dblink_build_sql_delete(PG_FUNCTION_ARGS)
{
	Relation	rel;
	text	   *relname_text;
	int16	   *pkattnums_arg;
	int32		pknumatts_arg;
	int		   *pkattnums;
	int			pknumatts;
	char	  **tgt_pkattvals;
	ArrayType  *tgt_pkattvals_arry;
	int			tgt_ndim;
	int		   *tgt_dim;
	int			tgt_nitems;
	int			i;
	char	   *ptr;
	char	   *sql;
	int16		typlen;
	bool		typbyval;
	char		typalign;

	relname_text = PG_GETARG_TEXT_P(0);

	/*
	 * Open target relation.
	 */
	rel = get_rel_from_relname(relname_text, AccessShareLock, ACL_SELECT);

	/*
	 * Process pkattnums argument.
	 */
	pkattnums_arg = (int16 *) PG_GETARG_POINTER(1);
	pknumatts_arg = PG_GETARG_INT32(2);
	validate_pkattnums(rel, pkattnums_arg, pknumatts_arg,
					   &pkattnums, &pknumatts);

	tgt_pkattvals_arry = PG_GETARG_ARRAYTYPE_P(3);

	/*
	 * Target array is made up of key values that will be used to build
	 * the SQL string for use on the remote system.
	 */
	tgt_ndim = ARR_NDIM(tgt_pkattvals_arry);
	tgt_dim = ARR_DIMS(tgt_pkattvals_arry);
	tgt_nitems = ArrayGetNItems(tgt_ndim, tgt_dim);

	/*
	 * There should be one target array key value for each key attnum
	 */
	if (tgt_nitems != pknumatts)
		ereport(ERROR,
				(errcode(ERRCODE_ARRAY_SUBSCRIPT_ERROR),
			 errmsg("target key array length must match number of key " \
					"attributes")));

	/*
	 * get array of pointers to c-strings from the input target array
	 */
	Assert(ARR_ELEMTYPE(tgt_pkattvals_arry) == TEXTOID);
	get_typlenbyvalalign(ARR_ELEMTYPE(tgt_pkattvals_arry),
						 &typlen, &typbyval, &typalign);

	tgt_pkattvals = (char **) palloc(tgt_nitems * sizeof(char *));
	ptr = ARR_DATA_PTR(tgt_pkattvals_arry);
	for (i = 0; i < tgt_nitems; i++)
	{
		tgt_pkattvals[i] = DatumGetCString(DirectFunctionCall1(textout, PointerGetDatum(ptr)));
		ptr = att_addlength(ptr, typlen, PointerGetDatum(ptr));
		ptr = (char *) att_align(ptr, typalign);
	}

	/*
	 * Prep work is finally done. Go get the SQL string.
	 */
	sql = get_sql_delete(rel, pkattnums, pknumatts, tgt_pkattvals);

	/*
	 * Now we can close the relation.
	 */
	relation_close(rel, AccessShareLock);

	/*
	 * And send it
	 */
	PG_RETURN_TEXT_P(GET_TEXT(sql));
}


/*
 * dblink_build_sql_update
 *
 * Used to generate an SQL update statement
 * based on an existing tuple in a local relation.
 * This is useful for selectively replicating data
 * to another server via dblink.
 *
 * API:
 * <relname> - name of local table of interest
 * <pkattnums> - an int2vector of attnums which will be used
 * to identify the local tuple of interest
 * <pknumatts> - number of attnums in pkattnums
 * <src_pkattvals_arry> - text array of key values which will be used
 * to identify the local tuple of interest
 * <tgt_pkattvals_arry> - text array of key values which will be used
 * to build the string for execution remotely. These are substituted
 * for their counterparts in src_pkattvals_arry
 */
PG_FUNCTION_INFO_V1(dblink_build_sql_update);
Datum
dblink_build_sql_update(PG_FUNCTION_ARGS)
{
	Relation	rel;
	text	   *relname_text;
	int16	   *pkattnums_arg;
	int32		pknumatts_arg;
	int		   *pkattnums;
	int			pknumatts;
	char	  **src_pkattvals;
	char	  **tgt_pkattvals;
	ArrayType  *src_pkattvals_arry;
	ArrayType  *tgt_pkattvals_arry;
	int			src_ndim;
	int		   *src_dim;
	int			src_nitems;
	int			tgt_ndim;
	int		   *tgt_dim;
	int			tgt_nitems;
	int			i;
	char	   *ptr;
	char	   *sql;
	int16		typlen;
	bool		typbyval;
	char		typalign;

	relname_text = PG_GETARG_TEXT_P(0);

	/*
	 * Open target relation.
	 */
	rel = get_rel_from_relname(relname_text, AccessShareLock, ACL_SELECT);

	/*
	 * Process pkattnums argument.
	 */
	pkattnums_arg = (int16 *) PG_GETARG_POINTER(1);
	pknumatts_arg = PG_GETARG_INT32(2);
	validate_pkattnums(rel, pkattnums_arg, pknumatts_arg,
					   &pkattnums, &pknumatts);

	src_pkattvals_arry = PG_GETARG_ARRAYTYPE_P(3);
	tgt_pkattvals_arry = PG_GETARG_ARRAYTYPE_P(4);

	/*
	 * Source array is made up of key values that will be used to locate
	 * the tuple of interest from the local system.
	 */
	src_ndim = ARR_NDIM(src_pkattvals_arry);
	src_dim = ARR_DIMS(src_pkattvals_arry);
	src_nitems = ArrayGetNItems(src_ndim, src_dim);

	/*
	 * There should be one source array key value for each key attnum
	 */
	if (src_nitems != pknumatts)
		ereport(ERROR,
				(errcode(ERRCODE_ARRAY_SUBSCRIPT_ERROR),
			 errmsg("source key array length must match number of key " \
					"attributes")));

	/*
	 * get array of pointers to c-strings from the input source array
	 */
	Assert(ARR_ELEMTYPE(src_pkattvals_arry) == TEXTOID);
	get_typlenbyvalalign(ARR_ELEMTYPE(src_pkattvals_arry),
						 &typlen, &typbyval, &typalign);

	src_pkattvals = (char **) palloc(src_nitems * sizeof(char *));
	ptr = ARR_DATA_PTR(src_pkattvals_arry);
	for (i = 0; i < src_nitems; i++)
	{
		src_pkattvals[i] = DatumGetCString(DirectFunctionCall1(textout, PointerGetDatum(ptr)));
		ptr = att_addlength(ptr, typlen, PointerGetDatum(ptr));
		ptr = (char *) att_align(ptr, typalign);
	}

	/*
	 * Target array is made up of key values that will be used to build
	 * the SQL string for use on the remote system.
	 */
	tgt_ndim = ARR_NDIM(tgt_pkattvals_arry);
	tgt_dim = ARR_DIMS(tgt_pkattvals_arry);
	tgt_nitems = ArrayGetNItems(tgt_ndim, tgt_dim);

	/*
	 * There should be one target array key value for each key attnum
	 */
	if (tgt_nitems != pknumatts)
		ereport(ERROR,
				(errcode(ERRCODE_ARRAY_SUBSCRIPT_ERROR),
			 errmsg("target key array length must match number of key " \
					"attributes")));

	/*
	 * get array of pointers to c-strings from the input target array
	 */
	Assert(ARR_ELEMTYPE(tgt_pkattvals_arry) == TEXTOID);
	get_typlenbyvalalign(ARR_ELEMTYPE(tgt_pkattvals_arry),
						 &typlen, &typbyval, &typalign);

	tgt_pkattvals = (char **) palloc(tgt_nitems * sizeof(char *));
	ptr = ARR_DATA_PTR(tgt_pkattvals_arry);
	for (i = 0; i < tgt_nitems; i++)
	{
		tgt_pkattvals[i] = DatumGetCString(DirectFunctionCall1(textout, PointerGetDatum(ptr)));
		ptr = att_addlength(ptr, typlen, PointerGetDatum(ptr));
		ptr = (char *) att_align(ptr, typalign);
	}

	/*
	 * Prep work is finally done. Go get the SQL string.
	 */
	sql = get_sql_update(rel, pkattnums, pknumatts, src_pkattvals, tgt_pkattvals);

	/*
	 * Now we can close the relation.
	 */
	relation_close(rel, AccessShareLock);

	/*
	 * And send it
	 */
	PG_RETURN_TEXT_P(GET_TEXT(sql));
}

/*
 * dblink_current_query
 * return the current query string
 * to allow its use in (among other things)
 * rewrite rules
 */
PG_FUNCTION_INFO_V1(dblink_current_query);
Datum
dblink_current_query(PG_FUNCTION_ARGS)
{
	PG_RETURN_TEXT_P(GET_TEXT(debug_query_string));
}


/*************************************************************
 * internal functions
 */


/*
 * get_pkey_attnames
 *
 * Get the primary key attnames for the given relation.
 * Return NULL, and set numatts = 0, if no primary key exists.
 */
static char **
get_pkey_attnames(Relation rel, int16 *numatts)
{
	Relation	indexRelation;
	ScanKeyData entry;
	HeapScanDesc scan;
	HeapTuple	indexTuple;
	int			i;
	char	  **result = NULL;
	TupleDesc	tupdesc;

	/* initialize numatts to 0 in case no primary key exists */
	*numatts = 0;

	tupdesc = rel->rd_att;

	/* use relid to get all related indexes */
	indexRelation = heap_openr(IndexRelationName, AccessShareLock);
	ScanKeyInit(&entry,
				Anum_pg_index_indrelid,
				BTEqualStrategyNumber, F_OIDEQ,
				ObjectIdGetDatum(RelationGetRelid(rel)));
	scan = heap_beginscan(indexRelation, SnapshotNow, 1, &entry);

	while ((indexTuple = heap_getnext(scan, ForwardScanDirection)) != NULL)
	{
		Form_pg_index index = (Form_pg_index) GETSTRUCT(indexTuple);

		/* we're only interested if it is the primary key */
		if (index->indisprimary == TRUE)
		{
			i = 0;
			while (index->indkey[i++] != 0)
				(*numatts)++;

			if (*numatts > 0)
			{
				result = (char **) palloc(*numatts * sizeof(char *));

				for (i = 0; i < *numatts; i++)
					result[i] = SPI_fname(tupdesc, index->indkey[i]);
			}
			break;
		}
	}
	heap_endscan(scan);
	heap_close(indexRelation, AccessShareLock);

	return result;
}

static char *
get_sql_insert(Relation rel, int *pkattnums, int pknumatts, char **src_pkattvals, char **tgt_pkattvals)
{
	char	   *relname;
	HeapTuple	tuple;
	TupleDesc	tupdesc;
	int			natts;
	StringInfo	str = makeStringInfo();
	char	   *sql;
	char	   *val;
	int			key;
	int			i;
	bool		needComma;

	/* get relation name including any needed schema prefix and quoting */
	relname = generate_relation_name(rel);

	tupdesc = rel->rd_att;
	natts = tupdesc->natts;

	tuple = get_tuple_of_interest(rel, pkattnums, pknumatts, src_pkattvals);
	if (!tuple)
		ereport(ERROR,
				(errcode(ERRCODE_CARDINALITY_VIOLATION),
				 errmsg("source row not found")));

	appendStringInfo(str, "INSERT INTO %s(", relname);

	needComma = false;
	for (i = 0; i < natts; i++)
	{
		if (tupdesc->attrs[i]->attisdropped)
			continue;

		if (needComma)
			appendStringInfo(str, ",");

		appendStringInfo(str, "%s",
				  quote_ident_cstr(NameStr(tupdesc->attrs[i]->attname)));
		needComma = true;
	}

	appendStringInfo(str, ") VALUES(");

	/*
	 * remember attvals are 1 based
	 */
	needComma = false;
	for (i = 0; i < natts; i++)
	{
		if (tupdesc->attrs[i]->attisdropped)
			continue;

		if (needComma)
			appendStringInfo(str, ",");

		if (tgt_pkattvals != NULL)
			key = get_attnum_pk_pos(pkattnums, pknumatts, i);
		else
			key = -1;

		if (key > -1)
			val = pstrdup(tgt_pkattvals[key]);
		else
			val = SPI_getvalue(tuple, tupdesc, i + 1);

		if (val != NULL)
		{
			appendStringInfo(str, "%s", quote_literal_cstr(val));
			pfree(val);
		}
		else
			appendStringInfo(str, "NULL");
		needComma = true;
	}
	appendStringInfo(str, ")");

	sql = pstrdup(str->data);
	pfree(str->data);
	pfree(str);

	return (sql);
}

static char *
get_sql_delete(Relation rel, int *pkattnums, int pknumatts, char **tgt_pkattvals)
{
	char	   *relname;
	TupleDesc	tupdesc;
	int			natts;
	StringInfo	str = makeStringInfo();
	char	   *sql;
	char	   *val = NULL;
	int			i;

	/* get relation name including any needed schema prefix and quoting */
	relname = generate_relation_name(rel);

	tupdesc = rel->rd_att;
	natts = tupdesc->natts;

	appendStringInfo(str, "DELETE FROM %s WHERE ", relname);
	for (i = 0; i < pknumatts; i++)
	{
		int			pkattnum = pkattnums[i];

		if (i > 0)
			appendStringInfo(str, " AND ");

		appendStringInfo(str, "%s",
		quote_ident_cstr(NameStr(tupdesc->attrs[pkattnum]->attname)));

		if (tgt_pkattvals != NULL)
			val = pstrdup(tgt_pkattvals[i]);
		else
			/* internal error */
			elog(ERROR, "target key array must not be NULL");

		if (val != NULL)
		{
			appendStringInfo(str, " = %s", quote_literal_cstr(val));
			pfree(val);
		}
		else
			appendStringInfo(str, " IS NULL");
	}

	sql = pstrdup(str->data);
	pfree(str->data);
	pfree(str);

	return (sql);
}

static char *
get_sql_update(Relation rel, int *pkattnums, int pknumatts, char **src_pkattvals, char **tgt_pkattvals)
{
	char	   *relname;
	HeapTuple	tuple;
	TupleDesc	tupdesc;
	int			natts;
	StringInfo	str = makeStringInfo();
	char	   *sql;
	char	   *val;
	int			key;
	int			i;
	bool		needComma;

	/* get relation name including any needed schema prefix and quoting */
	relname = generate_relation_name(rel);

	tupdesc = rel->rd_att;
	natts = tupdesc->natts;

	tuple = get_tuple_of_interest(rel, pkattnums, pknumatts, src_pkattvals);
	if (!tuple)
		ereport(ERROR,
				(errcode(ERRCODE_CARDINALITY_VIOLATION),
				 errmsg("source row not found")));

	appendStringInfo(str, "UPDATE %s SET ", relname);

	needComma = false;
	for (i = 0; i < natts; i++)
	{
		if (tupdesc->attrs[i]->attisdropped)
			continue;

		if (needComma)
			appendStringInfo(str, ", ");

		appendStringInfo(str, "%s = ",
				  quote_ident_cstr(NameStr(tupdesc->attrs[i]->attname)));

		if (tgt_pkattvals != NULL)
			key = get_attnum_pk_pos(pkattnums, pknumatts, i);
		else
			key = -1;

		if (key > -1)
			val = pstrdup(tgt_pkattvals[key]);
		else
			val = SPI_getvalue(tuple, tupdesc, i + 1);

		if (val != NULL)
		{
			appendStringInfo(str, "%s", quote_literal_cstr(val));
			pfree(val);
		}
		else
			appendStringInfo(str, "NULL");
		needComma = true;
	}

	appendStringInfo(str, " WHERE ");

	for (i = 0; i < pknumatts; i++)
	{
		int			pkattnum = pkattnums[i];

		if (i > 0)
			appendStringInfo(str, " AND ");

		appendStringInfo(str, "%s",
		quote_ident_cstr(NameStr(tupdesc->attrs[pkattnum]->attname)));

		if (tgt_pkattvals != NULL)
			val = pstrdup(tgt_pkattvals[i]);
		else
			val = SPI_getvalue(tuple, tupdesc, pkattnum + 1);

		if (val != NULL)
		{
			appendStringInfo(str, " = %s", quote_literal_cstr(val));
			pfree(val);
		}
		else
			appendStringInfo(str, " IS NULL");
	}

	sql = pstrdup(str->data);
	pfree(str->data);
	pfree(str);

	return (sql);
}

/*
 * Return a properly quoted literal value.
 * Uses quote_literal in quote.c
 */
static char *
quote_literal_cstr(char *rawstr)
{
	text	   *rawstr_text;
	text	   *result_text;
	char	   *result;

	rawstr_text = DatumGetTextP(DirectFunctionCall1(textin, CStringGetDatum(rawstr)));
	result_text = DatumGetTextP(DirectFunctionCall1(quote_literal, PointerGetDatum(rawstr_text)));
	result = DatumGetCString(DirectFunctionCall1(textout, PointerGetDatum(result_text)));

	return result;
}

/*
 * Return a properly quoted identifier.
 * Uses quote_ident in quote.c
 */
static char *
quote_ident_cstr(char *rawstr)
{
	text	   *rawstr_text;
	text	   *result_text;
	char	   *result;

	rawstr_text = DatumGetTextP(DirectFunctionCall1(textin, CStringGetDatum(rawstr)));
	result_text = DatumGetTextP(DirectFunctionCall1(quote_ident, PointerGetDatum(rawstr_text)));
	result = DatumGetCString(DirectFunctionCall1(textout, PointerGetDatum(result_text)));

	return result;
}

static int
get_attnum_pk_pos(int *pkattnums, int pknumatts, int key)
{
	int			i;

	/*
	 * Not likely a long list anyway, so just scan for the value
	 */
	for (i = 0; i < pknumatts; i++)
		if (key == pkattnums[i])
			return i;

	return -1;
}

static HeapTuple
get_tuple_of_interest(Relation rel, int *pkattnums, int pknumatts, char **src_pkattvals)
{
	char	   *relname;
	TupleDesc	tupdesc;
	StringInfo	str = makeStringInfo();
	char	   *sql = NULL;
	int			ret;
	HeapTuple	tuple;
	int			i;
	char	   *val = NULL;

	/* get relation name including any needed schema prefix and quoting */
	relname = generate_relation_name(rel);

	tupdesc = rel->rd_att;

	/*
	 * Connect to SPI manager
	 */
	if ((ret = SPI_connect()) < 0)
		/* internal error */
		elog(ERROR, "SPI connect failure - returned %d", ret);

	/*
	 * Build sql statement to look up tuple of interest Use src_pkattvals
	 * as the criteria.
	 */
	appendStringInfo(str, "SELECT * FROM %s WHERE ", relname);

	for (i = 0; i < pknumatts; i++)
	{
		int			pkattnum = pkattnums[i];

		if (i > 0)
			appendStringInfo(str, " AND ");

		appendStringInfo(str, "%s",
		quote_ident_cstr(NameStr(tupdesc->attrs[pkattnum]->attname)));

		val = pstrdup(src_pkattvals[i]);
		if (val != NULL)
		{
			appendStringInfo(str, " = %s", quote_literal_cstr(val));
			pfree(val);
		}
		else
			appendStringInfo(str, " IS NULL");
	}

	sql = pstrdup(str->data);
	pfree(str->data);
	pfree(str);

	/*
	 * Retrieve the desired tuple
	 */
	ret = SPI_exec(sql, 0);
	pfree(sql);

	/*
	 * Only allow one qualifying tuple
	 */
	if ((ret == SPI_OK_SELECT) && (SPI_processed > 1))
		ereport(ERROR,
				(errcode(ERRCODE_CARDINALITY_VIOLATION),
				 errmsg("source criteria matched more than one record")));

	else if (ret == SPI_OK_SELECT && SPI_processed == 1)
	{
		SPITupleTable *tuptable = SPI_tuptable;

		tuple = SPI_copytuple(tuptable->vals[0]);
		SPI_finish();

		return tuple;
	}
	else
	{
		/*
		 * no qualifying tuples
		 */
		SPI_finish();

		return NULL;
	}

	/*
	 * never reached, but keep compiler quiet
	 */
	return NULL;
}

/*
 * Open the relation named by relname_text, acquire specified type of lock,
 * verify we have specified permissions.
 * Caller must close rel when done with it.
 */
static Relation
get_rel_from_relname(text *relname_text, LOCKMODE lockmode, AclMode aclmode)
{
	RangeVar   *relvar;
	Relation	rel;
	AclResult	aclresult;

	relvar = makeRangeVarFromNameList(textToQualifiedNameList(relname_text, "get_rel_from_relname"));
	rel = heap_openrv(relvar, lockmode);

	aclresult = pg_class_aclcheck(RelationGetRelid(rel), GetUserId(),
								  aclmode);
	if (aclresult != ACLCHECK_OK)
		aclcheck_error(aclresult, ACL_KIND_CLASS,
					   RelationGetRelationName(rel));

	return rel;
}

/*
 * generate_relation_name - copied from ruleutils.c
 *		Compute the name to display for a relation
 *
 * The result includes all necessary quoting and schema-prefixing.
 */
static char *
generate_relation_name(Relation rel)
{
	char	   *nspname;
	char	   *result;

	/* Qualify the name if not visible in search path */
	if (RelationIsVisible(RelationGetRelid(rel)))
		nspname = NULL;
	else
		nspname = get_namespace_name(rel->rd_rel->relnamespace);

	result = quote_qualified_identifier(nspname, RelationGetRelationName(rel));

	return result;
}


static remoteConn *
getConnectionByName(const char *name)
{
	remoteConnHashEnt *hentry;
	char	   *key;

	if (!remoteConnHash)
		remoteConnHash = createConnHash();

	key = pstrdup(name);
	truncate_identifier(key, strlen(key), true);
	hentry = (remoteConnHashEnt *) hash_search(remoteConnHash,
											   key, HASH_FIND, NULL);

	if (hentry)
		return (hentry->rcon);

	return (NULL);
}

static HTAB *
createConnHash(void)
{
	HASHCTL		ctl;

	ctl.keysize = NAMEDATALEN;
	ctl.entrysize = sizeof(remoteConnHashEnt);

	return hash_create("Remote Con hash", NUMCONN, &ctl, HASH_ELEM);
}

static void
createNewConnection(const char *name, remoteConn * con)
{
	remoteConnHashEnt *hentry;
	bool		found;
	char	   *key;

	if (!remoteConnHash)
		remoteConnHash = createConnHash();

	key = pstrdup(name);
	truncate_identifier(key, strlen(key), true);
	hentry = (remoteConnHashEnt *) hash_search(remoteConnHash, key,
											   HASH_ENTER, &found);

	if (!hentry)
		ereport(ERROR,
				(errcode(ERRCODE_OUT_OF_MEMORY),
				 errmsg("out of memory")));

	if (found)
	{
		PQfinish(con->con);
		pfree(con);

		ereport(ERROR,
				(errcode(ERRCODE_DUPLICATE_OBJECT),
				 errmsg("duplicate connection name")));
	}

	hentry->rcon = con;
	strncpy(hentry->name, name, NAMEDATALEN - 1);
}

static void
deleteConnection(const char *name)
{
	remoteConnHashEnt *hentry;
	bool		found;
	char	   *key;

	if (!remoteConnHash)
		remoteConnHash = createConnHash();

	key = pstrdup(name);
	truncate_identifier(key, strlen(key), true);
	hentry = (remoteConnHashEnt *) hash_search(remoteConnHash,
											   key, HASH_REMOVE, &found);

	if (!hentry)
		ereport(ERROR,
				(errcode(ERRCODE_UNDEFINED_OBJECT),
				 errmsg("undefined connection name")));

}

/*
 * Modified version of conninfo_parse() from fe-connect.c
 * Used to remove any password from the connection string
 * in order to test whether the server auth method will
 * require it.
 */
static char *
connstr_strip_password(const char *connstr)
{
	char		   *pname;
	char		   *pval;
	char		   *buf;
	char		   *cp;
	char		   *cp2;
	StringInfoData	result;

	/* initialize return value */
	initStringInfo(&result);

	/* Need a modifiable copy of the input string */
	buf = pstrdup(connstr);
	cp = buf;

	while (*cp)
	{
		/* Skip blanks before the parameter name */
		if (isspace((unsigned char) *cp))
		{
			cp++;
			continue;
		}

		/* Get the parameter name */
		pname = cp;
		while (*cp)
		{
			if (*cp == '=')
				break;
			if (isspace((unsigned char) *cp))
			{
				*cp++ = '\0';
				while (*cp)
				{
					if (!isspace((unsigned char) *cp))
						break;
					cp++;
				}
				break;
			}
			cp++;
		}

		/* Check that there is a following '=' */
		if (*cp != '=')
			ereport(ERROR,
					(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
					errmsg("missing \"=\" after \"%s\" in connection string", pname)));
		*cp++ = '\0';

		/* Skip blanks after the '=' */
		while (*cp)
		{
			if (!isspace((unsigned char) *cp))
				break;
			cp++;
		}

		/* Get the parameter value */
		pval = cp;

		if (*cp != '\'')
		{
			cp2 = pval;
			while (*cp)
			{
				if (isspace((unsigned char) *cp))
				{
					*cp++ = '\0';
					break;
				}
				if (*cp == '\\')
				{
					cp++;
					if (*cp != '\0')
						*cp2++ = *cp++;
				}
				else
					*cp2++ = *cp++;
			}
			*cp2 = '\0';
		}
		else
		{
			cp2 = pval;
			cp++;
			for (;;)
			{
				if (*cp == '\0')
					ereport(ERROR,
							(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
							errmsg("unterminated quoted string in connection string")));
				if (*cp == '\\')
				{
					cp++;
					if (*cp != '\0')
						*cp2++ = *cp++;
					continue;
				}
				if (*cp == '\'')
				{
					*cp2 = '\0';
					cp++;
					break;
				}
				*cp2++ = *cp++;
			}
		}

		/*
		 * Now we have the name and the value. If it is not a password,
		 * append to the return connstr.
		 */
		if (strcmp("password", pname) != 0)
			/* append the value */
			appendStringInfo(&result, " %s='%s'", pname, pval);
	}

	return result.data;
}

static void
dblink_security_check(PGconn *conn, remoteConn *rcon, const char *connstr)
{
	if (!superuser())
	{
		/* this attempt must fail */
		conn = PQconnectdb(connstr_strip_password(connstr));

		if (PQstatus(conn) == CONNECTION_OK)
		{
			PQfinish(conn);
			if (rcon)
				pfree(rcon);

			ereport(ERROR,
					(errcode(ERRCODE_S_R_E_PROHIBITED_SQL_STATEMENT_ATTEMPTED),
					 errmsg("password is required"),
					 errdetail("Non-superuser cannot connect if the server does not request a password."),
					 errhint("Target server's authentication method must be changed.")));
		}
		else
			PQfinish(conn);
	}
}

/*
 * Validate the PK-attnums argument for dblink_build_sql_insert() and related
 * functions, and translate to the internal representation.
 *
 * The user supplies an int2vector of 1-based physical attnums, plus a count
 * argument (the need for the separate count argument is historical, but we
 * still check it).  We check that each attnum corresponds to a valid,
 * non-dropped attribute of the rel.  We do *not* prevent attnums from being
 * listed twice, though the actual use-case for such things is dubious.
 *
 * The internal representation is a palloc'd int array of 0-based physical
 * attnums.
 */
static void
validate_pkattnums(Relation rel,
				   int16 *pkattnums_arg, int32 pknumatts_arg,
				   int **pkattnums, int *pknumatts)
{
	TupleDesc	tupdesc = rel->rd_att;
	int			natts = tupdesc->natts;
	int			i;

	/* Must have at least one pk attnum selected */
	if (pknumatts_arg <= 0)
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				 errmsg("number of key attributes must be > 0")));

	/* Allocate output array */
	*pkattnums = (int *) palloc(pknumatts_arg * sizeof(int));
	*pknumatts = pknumatts_arg;

	/* Validate attnums and convert to internal form */
	for (i = 0; i < pknumatts_arg; i++)
	{
		int		pkattnum = pkattnums_arg[i];

		if (pkattnum <= 0 || pkattnum > natts ||
			tupdesc->attrs[pkattnum - 1]->attisdropped)
			ereport(ERROR,
					(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
					 errmsg("invalid attribute number %d", pkattnum)));
		(*pkattnums)[i] = pkattnum - 1;
	}
}
