/*-------------------------------------------------------------------------
 *
 * nodeSubplan.h
 *
 *
 *
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/executor/nodeSubplan.h,v 1.27 2008/01/01 19:45:57 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef NODESUBPLAN_H
#define NODESUBPLAN_H

#include "nodes/execnodes.h"

extern SubPlanState *ExecInitSubPlan(SubPlan *subplan, PlanState *parent);

extern AlternativeSubPlanState *ExecInitAlternativeSubPlan(AlternativeSubPlan *asplan, PlanState *parent);

extern void ExecReScanSetParamPlan(SubPlanState *node, PlanState *parent);

extern void ExecSetParamPlan(SubPlanState *node, ExprContext *econtext);

#endif   /* NODESUBPLAN_H */
