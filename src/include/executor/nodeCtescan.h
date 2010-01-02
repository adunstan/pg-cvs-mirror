/*-------------------------------------------------------------------------
 *
 * nodeCtescan.h
 *
 *
 *
 * Portions Copyright (c) 1996-2010, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/executor/nodeCtescan.h,v 1.3 2009/09/27 21:10:53 tgl Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef NODECTESCAN_H
#define NODECTESCAN_H

#include "nodes/execnodes.h"

extern CteScanState *ExecInitCteScan(CteScan *node, EState *estate, int eflags);
extern TupleTableSlot *ExecCteScan(CteScanState *node);
extern void ExecEndCteScan(CteScanState *node);
extern void ExecCteScanReScan(CteScanState *node, ExprContext *exprCtxt);

#endif   /* NODECTESCAN_H */
