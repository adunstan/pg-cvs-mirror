/*-------
 * Module:			odbcapi30.c
 *
 * Description:		This module contains routines related to ODBC 3.0
 *			most of their implementations are temporary
 *			and must be rewritten properly.
 *			2001/07/23	inoue
 *
 * Classes:			n/a
 *
 * API functions:	SQLAllocHandle, SQLBindParam, SQLCloseCursor,
			SQLColAttribute, SQLCopyDesc, SQLEndTran,
			SQLFetchScroll, SQLFreeHandle, SQLGetDescField,
			SQLGetDescRec, SQLGetDiagField, SQLGetDiagRec,
			SQLGetEnvAttr, SQLGetConnectAttr, SQLGetStmtAttr,
			SQLSetConnectAttr, SQLSetDescField, SQLSetDescRec,
			SQLSetEnvAttr, SQLSetStmtAttr, SQLBulkOperations
 *-------
 */

#ifndef ODBCVER
#define ODBCVER 0x0300
#endif
#include "psqlodbc.h"
#include <stdio.h>
#include <string.h>

#include "environ.h"
#include "connection.h"
#include "statement.h"
#include "pgapifunc.h"

/*	SQLAllocConnect/SQLAllocEnv/SQLAllocStmt -> SQLAllocHandle */
RETCODE		SQL_API
SQLAllocHandle(SQLSMALLINT HandleType,
			   SQLHANDLE InputHandle, SQLHANDLE * OutputHandle)
{
	mylog("[[SQLAllocHandle]]");
	switch (HandleType)
	{
		case SQL_HANDLE_ENV:
			return PGAPI_AllocEnv(OutputHandle);
		case SQL_HANDLE_DBC:
			return PGAPI_AllocConnect(InputHandle, OutputHandle);
		case SQL_HANDLE_STMT:
			return PGAPI_AllocStmt(InputHandle, OutputHandle);
		default:
			break;
	}
	return SQL_ERROR;
}

/*	SQLBindParameter/SQLSetParam -> SQLBindParam */
RETCODE		SQL_API
SQLBindParam(HSTMT StatementHandle,
			 SQLUSMALLINT ParameterNumber, SQLSMALLINT ValueType,
			 SQLSMALLINT ParameterType, SQLUINTEGER LengthPrecision,
			 SQLSMALLINT ParameterScale, PTR ParameterValue,
			 SQLINTEGER *StrLen_or_Ind)
{
	int			BufferLength = 512;		/* Is it OK ? */

	mylog("[[SQLBindParam]]");
	return PGAPI_BindParameter(StatementHandle, ParameterNumber, SQL_PARAM_INPUT, ValueType, ParameterType, LengthPrecision, ParameterScale, ParameterValue, BufferLength, StrLen_or_Ind);
}

/*	New function */
RETCODE		SQL_API
SQLCloseCursor(HSTMT StatementHandle)
{
	mylog("[[SQLCloseCursor]]");
	return PGAPI_FreeStmt(StatementHandle, SQL_CLOSE);
}

/*	SQLColAttributes -> SQLColAttribute */
RETCODE		SQL_API
SQLColAttribute(HSTMT StatementHandle,
				SQLUSMALLINT ColumnNumber, SQLUSMALLINT FieldIdentifier,
				PTR CharacterAttribute, SQLSMALLINT BufferLength,
				SQLSMALLINT *StringLength, PTR NumericAttribute)
{
	mylog("[[SQLColAttribute]]");
	return PGAPI_ColAttributes(StatementHandle, ColumnNumber,
					   FieldIdentifier, CharacterAttribute, BufferLength,
							   StringLength, NumericAttribute);
}

static HSTMT
descHandleFromStatementHandle(HSTMT StatementHandle, SQLINTEGER descType) 
{
	switch (descType)
	{
		case SQL_ATTR_APP_ROW_DESC:		/* 10010 */
			return StatementHandle;	/* this is bogus */
		case SQL_ATTR_APP_PARAM_DESC:	/* 10011 */
			return (HSTMT) ((SQLUINTEGER) StatementHandle + 1) ; /* this is bogus */
		case SQL_ATTR_IMP_ROW_DESC:		/* 10012 */
			return (HSTMT) ((SQLUINTEGER) StatementHandle + 2); /* this is bogus */
		case SQL_ATTR_IMP_PARAM_DESC:	/* 10013 */
			return (HSTMT) ((SQLUINTEGER) StatementHandle + 3); /* this is bogus */
	}
	return (HSTMT) 0;
}
static HSTMT
statementHandleFromDescHandle(HSTMT DescHandle, SQLINTEGER *descType) 
{
	SQLUINTEGER res = (SQLUINTEGER) DescHandle % 4;
	switch (res)
	{
		case 0: *descType = SQL_ATTR_APP_ROW_DESC; /* 10010 */
			break;
		case 1: *descType = SQL_ATTR_APP_PARAM_DESC; /* 10011 */
			break;
		case 2: *descType = SQL_ATTR_IMP_ROW_DESC; /* 10012 */
			break;
		case 3: *descType = SQL_ATTR_IMP_PARAM_DESC; /* 10013 */
			break;
	}
	return (HSTMT) ((SQLUINTEGER) DescHandle - res);
}

/*	new function */
RETCODE		SQL_API
SQLCopyDesc(SQLHDESC SourceDescHandle,
			SQLHDESC TargetDescHandle)
{
	mylog("[[SQLCopyDesc]]\n");
	mylog("Error not implemented\n");
	return SQL_ERROR;
}

/*	SQLTransact -> SQLEndTran */
RETCODE		SQL_API
SQLEndTran(SQLSMALLINT HandleType, SQLHANDLE Handle,
		   SQLSMALLINT CompletionType)
{
	mylog("[[SQLEndTran]]");
	switch (HandleType)
	{
		case SQL_HANDLE_ENV:
			return PGAPI_Transact(Handle, SQL_NULL_HDBC, CompletionType);
		case SQL_HANDLE_DBC:
			return PGAPI_Transact(SQL_NULL_HENV, Handle, CompletionType);
		default:
			break;
	}
	return SQL_ERROR;
}

/*	SQLExtendedFetch -> SQLFetchScroll */
RETCODE		SQL_API
SQLFetchScroll(HSTMT StatementHandle,
			   SQLSMALLINT FetchOrientation, SQLINTEGER FetchOffset)
{
	static char *func = "SQLFetchScroll";
	StatementClass *stmt = (StatementClass *) StatementHandle;
	RETCODE		ret;
	IRDFields	*irdopts = SC_get_IRD(stmt);
	SQLUSMALLINT *rowStatusArray = irdopts->rowStatusArray;
	SQLINTEGER *pcRow = irdopts->rowsFetched;

	mylog("[[%s]] %d,%d\n", func, FetchOrientation, FetchOffset);
	if (FetchOrientation == SQL_FETCH_BOOKMARK)
	{
		if (stmt->options.bookmark_ptr)
{
			FetchOffset += *((Int4 *) stmt->options.bookmark_ptr);
mylog("real FetchOffset = %d\n", FetchOffset);
}
		else
		{
			stmt->errornumber = STMT_SEQUENCE_ERROR;
			stmt->errormsg = "Bookmark isn't specifed yet";
			SC_log_error(func, "", stmt);
			return SQL_ERROR;
		}
	}
	ret = PGAPI_ExtendedFetch(StatementHandle, FetchOrientation, FetchOffset,
							  pcRow, rowStatusArray);
	if (ret != SQL_SUCCESS)
		mylog("%s return = %d\n", func, ret);
	return ret;
}

/*	SQLFree(Connect/Env/Stmt) -> SQLFreeHandle */
RETCODE		SQL_API
SQLFreeHandle(SQLSMALLINT HandleType, SQLHANDLE Handle)
{
	mylog("[[SQLFreeHandle]]");
	switch (HandleType)
	{
		case SQL_HANDLE_ENV:
			return PGAPI_FreeEnv(Handle);
		case SQL_HANDLE_DBC:
			return PGAPI_FreeConnect(Handle);
		case SQL_HANDLE_STMT:
			return PGAPI_FreeStmt(Handle, SQL_DROP);
		default:
			break;
	}
	return SQL_ERROR;
}

/*	new function */
RETCODE		SQL_API
SQLGetDescField(SQLHDESC DescriptorHandle,
				SQLSMALLINT RecNumber, SQLSMALLINT FieldIdentifier,
				PTR Value, SQLINTEGER BufferLength,
				SQLINTEGER *StringLength)
{
	mylog("[[SQLGetDescField]]\n");
	return PGAPI_GetDescField(DescriptorHandle, RecNumber, FieldIdentifier,
			Value, BufferLength, StringLength);
}

/*	new function */
RETCODE		SQL_API
SQLGetDescRec(SQLHDESC DescriptorHandle,
			  SQLSMALLINT RecNumber, SQLCHAR *Name,
			  SQLSMALLINT BufferLength, SQLSMALLINT *StringLength,
			  SQLSMALLINT *Type, SQLSMALLINT *SubType,
			  SQLINTEGER *Length, SQLSMALLINT *Precision,
			  SQLSMALLINT *Scale, SQLSMALLINT *Nullable)
{
	mylog("[[SQLGetDescRec]]\n");
	mylog("Error not implemented\n");
	return SQL_ERROR;
}

/*	new function */
RETCODE		SQL_API
SQLGetDiagField(SQLSMALLINT HandleType, SQLHANDLE Handle,
				SQLSMALLINT RecNumber, SQLSMALLINT DiagIdentifier,
				PTR DiagInfo, SQLSMALLINT BufferLength,
				SQLSMALLINT *StringLength)
{
	mylog("[[SQLGetDiagField]] Handle=(%u,%x) Rec=%d Id=%d\n", HandleType, Handle, RecNumber, DiagIdentifier);
	return SQL_ERROR;
}

/*	SQLError -> SQLDiagRec */
RETCODE		SQL_API
SQLGetDiagRec(SQLSMALLINT HandleType, SQLHANDLE Handle,
			  SQLSMALLINT RecNumber, SQLCHAR *Sqlstate,
			  SQLINTEGER *NativeError, SQLCHAR *MessageText,
			  SQLSMALLINT BufferLength, SQLSMALLINT *TextLength)
{
	mylog("[[SQLGetDiagRec]]\n");
	return PGAPI_GetDiagRec(HandleType, Handle, RecNumber, Sqlstate,
			NativeError, MessageText, BufferLength, TextLength);
}

/*	new function */
RETCODE		SQL_API
SQLGetEnvAttr(HENV EnvironmentHandle,
			  SQLINTEGER Attribute, PTR Value,
			  SQLINTEGER BufferLength, SQLINTEGER *StringLength)
{
	EnvironmentClass *env = (EnvironmentClass *) EnvironmentHandle;

	mylog("[[SQLGetEnvAttr]] %d\n", Attribute);
	switch (Attribute)
	{
		case SQL_ATTR_CONNECTION_POOLING:
			*((unsigned int *) Value) = SQL_CP_OFF;
			break;
		case SQL_ATTR_CP_MATCH:
			*((unsigned int *) Value) = SQL_CP_RELAXED_MATCH;
			break;
		case SQL_ATTR_ODBC_VERSION:
			*((unsigned int *) Value) = EN_is_odbc2(env) ? SQL_OV_ODBC2 : SQL_OV_ODBC3;
			break;
		case SQL_ATTR_OUTPUT_NTS:
			*((unsigned int *) Value) = SQL_TRUE;
			break;
		default:
			env->errornumber = CONN_INVALID_ARGUMENT_NO;
			return SQL_ERROR;
	}
	return SQL_SUCCESS;
}

/*	SQLGetConnectOption -> SQLGetconnectAttr */
RETCODE		SQL_API
SQLGetConnectAttr(HDBC ConnectionHandle,
				  SQLINTEGER Attribute, PTR Value,
				  SQLINTEGER BufferLength, SQLINTEGER *StringLength)
{
	mylog("[[SQLGetConnectAttr]] %d\n", Attribute);
	return PGAPI_GetConnectAttr(ConnectionHandle, Attribute,Value,
			BufferLength, StringLength);
}

/*	SQLGetStmtOption -> SQLGetStmtAttr */
RETCODE		SQL_API
SQLGetStmtAttr(HSTMT StatementHandle,
			   SQLINTEGER Attribute, PTR Value,
			   SQLINTEGER BufferLength, SQLINTEGER *StringLength)
{
	static char *func = "SQLGetStmtAttr";

	mylog("[[%s]] Handle=%u %d\n", func, StatementHandle, Attribute);
	return PGAPI_GetStmtAttr(StatementHandle, Attribute, Value,
			BufferLength, StringLength);
}

/*	SQLSetConnectOption -> SQLSetConnectAttr */
RETCODE		SQL_API
SQLSetConnectAttr(HDBC ConnectionHandle,
				  SQLINTEGER Attribute, PTR Value,
				  SQLINTEGER StringLength)
{
	ConnectionClass *conn = (ConnectionClass *) ConnectionHandle;

	mylog("[[SQLSetConnectAttr]] %d\n", Attribute);
	return PGAPI_SetConnectAttr(ConnectionHandle, Attribute, Value,
				  StringLength);
}

/*	new function */
RETCODE		SQL_API
SQLSetDescField(SQLHDESC DescriptorHandle,
				SQLSMALLINT RecNumber, SQLSMALLINT FieldIdentifier,
				PTR Value, SQLINTEGER BufferLength)
{
	RETCODE		ret;

	mylog("[[SQLSetDescField]] h=%u rec=%d field=%d val=%x\n", DescriptorHandle, RecNumber, FieldIdentifier, Value);
	ret = PGAPI_SetDescField(DescriptorHandle, RecNumber, FieldIdentifier,
				Value, BufferLength);
	return ret;
}

/*	new fucntion */
RETCODE		SQL_API
SQLSetDescRec(SQLHDESC DescriptorHandle,
			  SQLSMALLINT RecNumber, SQLSMALLINT Type,
			  SQLSMALLINT SubType, SQLINTEGER Length,
			  SQLSMALLINT Precision, SQLSMALLINT Scale,
			  PTR Data, SQLINTEGER *StringLength,
			  SQLINTEGER *Indicator)
{
	const char *func = "SQLSetDescRec";

	mylog("[[SQLSetDescRec]]\n");
	mylog("Error not implemented\n");
	return SQL_ERROR;
}

/*	new function */
RETCODE		SQL_API
SQLSetEnvAttr(HENV EnvironmentHandle,
			  SQLINTEGER Attribute, PTR Value,
			  SQLINTEGER StringLength)
{
	EnvironmentClass *env = (EnvironmentClass *) EnvironmentHandle;

	mylog("[[SQLSetEnvAttr]] att=%d,%u\n", Attribute, Value);
	switch (Attribute)
	{
		case SQL_ATTR_CONNECTION_POOLING:
			if ((SQLUINTEGER) Value == SQL_CP_OFF)
				return SQL_SUCCESS;
			break;
		case SQL_ATTR_CP_MATCH:
			/* *((unsigned int *) Value) = SQL_CP_RELAXED_MATCH; */
			return SQL_SUCCESS;
		case SQL_ATTR_ODBC_VERSION:
			if ((SQLUINTEGER) Value == SQL_OV_ODBC2)
				EN_set_odbc2(env);
			else
				EN_set_odbc3(env);
			return SQL_SUCCESS;
			break;
		case SQL_ATTR_OUTPUT_NTS:
			if ((SQLUINTEGER) Value == SQL_TRUE)
				return SQL_SUCCESS;
			break;
		default:
			env->errornumber = CONN_INVALID_ARGUMENT_NO;
			return SQL_ERROR;
	}
	env->errornumber = CONN_OPTION_VALUE_CHANGED;
	env->errormsg = "SetEnv changed to ";
	return SQL_SUCCESS_WITH_INFO;
}

/*	SQLSet(Param/Scroll/Stmt)Option -> SQLSetStmtAttr */
RETCODE		SQL_API
SQLSetStmtAttr(HSTMT StatementHandle,
			   SQLINTEGER Attribute, PTR Value,
			   SQLINTEGER StringLength)
{
	static char *func = "SQLSetStmtAttr";
	StatementClass *stmt = (StatementClass *) StatementHandle;

	mylog("[[%s]] Handle=%u %d,%u\n", func, StatementHandle, Attribute, Value);
	return PGAPI_SetStmtAttr(StatementHandle, Attribute, Value, StringLength);
}

#define SQL_FUNC_ESET(pfExists, uwAPI) \
		(*(((UWORD*) (pfExists)) + ((uwAPI) >> 4)) \
			|= (1 << ((uwAPI) & 0x000F)) \
				)
RETCODE		SQL_API
PGAPI_GetFunctions30(HDBC hdbc, UWORD fFunction, UWORD FAR * pfExists)
{
	ConnectionClass	*conn = (ConnectionClass *) hdbc;
	ConnInfo	*ci = &(conn->connInfo);

	if (fFunction != SQL_API_ODBC3_ALL_FUNCTIONS)
		return SQL_ERROR;
	memset(pfExists, 0, sizeof(UWORD) * SQL_API_ODBC3_ALL_FUNCTIONS_SIZE);

	/* SQL_FUNC_ESET(pfExists, SQL_API_SQLALLOCCONNECT); 1 deprecated */
	/* SQL_FUNC_ESET(pfExists, SQL_API_SQLALLOCENV); 2 deprecated */
	/* SQL_FUNC_ESET(pfExists, SQL_API_SQLALLOCSTMT); 3 deprecated */

	/*
	 * for (i = SQL_API_SQLBINDCOL; i <= 23; i++) SQL_FUNC_ESET(pfExists,
	 * i);
	 */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLBINDCOL);		/* 4 */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLCANCEL); /* 5 */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLCOLATTRIBUTE);	/* 6 */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLCONNECT);		/* 7 */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLDESCRIBECOL);	/* 8 */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLDISCONNECT);		/* 9 */
	/* SQL_FUNC_ESET(pfExists, SQL_API_SQLERROR);  10 deprecated */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLEXECDIRECT);		/* 11 */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLEXECUTE);		/* 12 */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLFETCH);	/* 13 */
	/* SQL_FUNC_ESET(pfExists, SQL_API_SQLFREECONNECT); 14 deprecated */
	/* SQL_FUNC_ESET(pfExists, SQL_API_SQLFREEENV); 15 deprecated */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLFREESTMT);		/* 16 */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLGETCURSORNAME);	/* 17 */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLNUMRESULTCOLS);	/* 18 */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLPREPARE);		/* 19 */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLROWCOUNT);		/* 20 */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLSETCURSORNAME);	/* 21 */
	/* SQL_FUNC_ESET(pfExists, SQL_API_SQLSETPARAM); 22 deprecated */
	/* SQL_FUNC_ESET(pfExists, SQL_API_SQLTRANSACT); 23 deprecated */

	/*
	 * for (i = 40; i < SQL_API_SQLEXTENDEDFETCH; i++)
	 * SQL_FUNC_ESET(pfExists, i);
	 */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLCOLUMNS);		/* 40 */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLDRIVERCONNECT);	/* 41 */
	/* SQL_FUNC_ESET(pfExists, SQL_API_SQLGETCONNECTOPTION); 42 deprecated */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLGETDATA);		/* 43 */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLGETFUNCTIONS);	/* 44 */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLGETINFO);		/* 45 */
	/* SQL_FUNC_ESET(pfExists, SQL_API_SQLGETSTMTOPTION); 46 deprecated */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLGETTYPEINFO);	/* 47 */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLPARAMDATA);		/* 48 */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLPUTDATA);		/* 49 */

	/* SQL_FUNC_ESET(pfExists, SQL_API_SQLSETCONNECTIONOPTION); 50 deprecated */
	/* SQL_FUNC_ESET(pfExists, SQL_API_SQLSETSTMTOPTION); 51 deprecated */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLSPECIALCOLUMNS);	/* 52 */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLSTATISTICS);		/* 53 */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLTABLES); /* 54 */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLBROWSECONNECT);	/* 55 */
	if (ci->drivers.lie)
		SQL_FUNC_ESET(pfExists, SQL_API_SQLCOLUMNPRIVILEGES); /* 56 not implemented yet */ 
	SQL_FUNC_ESET(pfExists, SQL_API_SQLDATASOURCES);	/* 57 */
	if (ci->drivers.lie)
		SQL_FUNC_ESET(pfExists, SQL_API_SQLDESCRIBEPARAM); /* 58 not properly implemented */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLEXTENDEDFETCH); /* 59 deprecated ? */

	/*
	 * for (++i; i < SQL_API_SQLBINDPARAMETER; i++)
	 * SQL_FUNC_ESET(pfExists, i);
	 */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLFOREIGNKEYS);	/* 60 */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLMORERESULTS);	/* 61 */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLNATIVESQL);		/* 62 */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLNUMPARAMS);		/* 63 */
	/* SQL_FUNC_ESET(pfExists, SQL_API_SQLPARAMOPTIONS); 64 deprecated */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLPRIMARYKEYS);	/* 65 */
	if (ci->drivers.lie)
		SQL_FUNC_ESET(pfExists, SQL_API_SQLPROCEDURECOLUMNS); /* 66 not implemeted yet */ 
	SQL_FUNC_ESET(pfExists, SQL_API_SQLPROCEDURES);		/* 67 */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLSETPOS);		/* 68 */
	/* SQL_FUNC_ESET(pfExists, SQL_API_SQLSETSCROLLOPTIONS); 69 deprecated */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLTABLEPRIVILEGES);		/* 70 */
	/* SQL_FUNC_ESET(pfExists, SQL_API_SQLDRIVERS); */	/* 71 */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLBINDPARAMETER);	/* 72 */

	SQL_FUNC_ESET(pfExists, SQL_API_SQLALLOCHANDLE);	/* 1001 */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLBINDPARAM);		/* 1002 */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLCLOSECURSOR);	/* 1003 */
	if (ci->drivers.lie)
		SQL_FUNC_ESET(pfExists, SQL_API_SQLCOPYDESC); /* 1004 not implemented yet */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLENDTRAN);		/* 1005 */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLFREEHANDLE);		/* 1006 */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLGETCONNECTATTR);	/* 1007 */
	if (ci->drivers.lie)
	{
		SQL_FUNC_ESET(pfExists, SQL_API_SQLGETDESCFIELD); /* 1008 not implemented yet */
		SQL_FUNC_ESET(pfExists, SQL_API_SQLGETDESCREC); /* 1009 not implemented yet */
		SQL_FUNC_ESET(pfExists, SQL_API_SQLGETDIAGFIELD); /* 1010 not implemented yet */
	}
	SQL_FUNC_ESET(pfExists, SQL_API_SQLGETDIAGREC);		/* 1011 */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLGETENVATTR);		/* 1012 */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLGETSTMTATTR);	/* 1014 */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLSETCONNECTATTR);	/* 1016 */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLSETDESCFIELD);	/* 1017 */
	if (ci->drivers.lie)
	{
		SQL_FUNC_ESET(pfExists, SQL_API_SQLSETDESCREC); /* 1018 not implemented yet */
	}
	SQL_FUNC_ESET(pfExists, SQL_API_SQLSETENVATTR);		/* 1019 */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLSETSTMTATTR);	/* 1020 */
	SQL_FUNC_ESET(pfExists, SQL_API_SQLFETCHSCROLL);	/* 1021 */
	if (ci->updatable_cursors)
		SQL_FUNC_ESET(pfExists, SQL_API_SQLBULKOPERATIONS);	/* 24 */

	return SQL_SUCCESS;
}

RETCODE	SQL_API
SQLBulkOperations(HSTMT hstmt, SQLSMALLINT operation)
{
	static char	*func = "SQLBulkOperations";
	StatementClass	*stmt = (StatementClass *) hstmt;
	ARDFields	*opts = SC_get_ARD(stmt);
	RETCODE		ret;
	UInt4		offset, bind_size = opts->bind_size, *bmark;
	int		i, processed;
	ConnectionClass	*conn;
	BOOL		auto_commit_needed = FALSE;

	mylog("[[%s]] operation = %d\n", func, operation);
	offset = opts->row_offset_ptr ? *opts->row_offset_ptr : 0;
	switch (operation)
	{
		case SQL_ADD:
			ret = PGAPI_SetPos(hstmt, 0, operation, SQL_LOCK_NO_CHANGE);
			break;
		default:
			if (SQL_FETCH_BY_BOOKMARK != operation)
			{
				conn = SC_get_conn(stmt);
				if (auto_commit_needed = CC_is_in_autocommit(conn), auto_commit_needed)
					PGAPI_SetConnectOption(conn, SQL_AUTOCOMMIT,
SQL_AUTOCOMMIT_OFF);
			}
			if (bmark = (UInt4 *) opts->bookmark->buffer, !bmark)
			{
				stmt->errormsg = "bookmark isn't specified";
				return SQL_ERROR;
			}
			bmark += (offset >> 4);
			for (i = 0, processed = 0; i < opts->rowset_size; i++)
			{
				if (!opts->row_operation_ptr || SQL_ROW_PROCEED == opts->row_operation_ptr[i])
				{
					switch (operation)
					{
						case SQL_UPDATE_BY_BOOKMARK:
							ret = SC_pos_update(stmt, (UWORD) i, *bmark);
							break;
						case SQL_DELETE_BY_BOOKMARK:
							ret = SC_pos_delete(stmt, (UWORD) i, *bmark);
							break;
						case SQL_FETCH_BY_BOOKMARK:
							ret = SC_pos_refresh(stmt, (UWORD) i, *bmark);
							break;
					}
					processed++;
					if (SQL_ERROR == ret)
						break;
					if (bind_size > 0)
						bmark += (bind_size >> 2);
					else
						bmark++; 
				}
			}
			if (auto_commit_needed)
				PGAPI_SetConnectOption(conn, SQL_AUTOCOMMIT, SQL_AUTOCOMMIT_ON);
			if (SC_get_IRD(stmt)->rowsFetched)
				*SC_get_IRD(stmt)->rowsFetched = processed;
			break;
	}
	return ret;
}	
