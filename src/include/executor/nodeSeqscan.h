/*-------------------------------------------------------------------------
 *
 * nodeSeqscan.h
 *
 *
 *
 * Portions Copyright (c) 1996-2002, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $Id: nodeSeqscan.h,v 1.17 2003/08/04 00:43:31 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef NODESEQSCAN_H
#define NODESEQSCAN_H

#include "nodes/execnodes.h"

extern int	ExecCountSlotsSeqScan(SeqScan *node);
extern SeqScanState *ExecInitSeqScan(SeqScan *node, EState *estate);
extern TupleTableSlot *ExecSeqScan(SeqScanState * node);
extern void ExecEndSeqScan(SeqScanState * node);
extern void ExecSeqMarkPos(SeqScanState * node);
extern void ExecSeqRestrPos(SeqScanState * node);
extern void ExecSeqReScan(SeqScanState * node, ExprContext *exprCtxt);

#endif   /* NODESEQSCAN_H */
