/*-------------------------------------------------------------------------
 *
 * planner.h
 *	  prototypes for planner.c.
 *
 *
 * Portions Copyright (c) 1996-2003, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql-server/src/include/optimizer/planner.h,v 1.28 2003/11/29 22:41:07 pgsql Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef PLANNER_H
#define PLANNER_H

#include "nodes/params.h"
#include "nodes/parsenodes.h"
#include "nodes/plannodes.h"


extern ParamListInfo PlannerBoundParamList;	/* current boundParams */

extern Plan *planner(Query *parse, bool isCursor, int cursorOptions,
					 ParamListInfo boundParams);
extern Plan *subquery_planner(Query *parse, double tuple_fraction);

#endif   /* PLANNER_H */
