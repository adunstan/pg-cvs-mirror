/*-------------------------------------------------------------------------
 *
 * nodeResult.h--
 *
 *
 *
 * Copyright (c) 1994, Regents of the University of California
 *
 * $Id: nodeResult.h,v 1.2 1997/09/07 04:57:58 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef NODERESULT_H
#define NODERESULT_H

extern TupleTableSlot *ExecResult(Result * node);
extern bool		ExecInitResult(Result * node, EState * estate, Plan * parent);
extern int		ExecCountSlotsResult(Result * node);
extern void		ExecEndResult(Result * node);

#endif							/* NODERESULT_H */
