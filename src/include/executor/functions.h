/*-------------------------------------------------------------------------
 *
 * functions.h
 *		Declarations for execution of SQL-language functions.
 *
 *
 * Portions Copyright (c) 1996-2010, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/executor/functions.h,v 1.34 2009/12/14 02:15:54 tgl Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "nodes/execnodes.h"
#include "tcop/dest.h"


extern Datum fmgr_sql(PG_FUNCTION_ARGS);

extern bool check_sql_fn_retval(Oid func_id, Oid rettype,
					List *queryTreeList,
					bool *modifyTargetList,
					JunkFilter **junkFilter);

extern DestReceiver *CreateSQLFunctionDestReceiver(void);

#endif   /* FUNCTIONS_H */
