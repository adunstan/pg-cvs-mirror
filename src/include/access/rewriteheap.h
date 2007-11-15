/*-------------------------------------------------------------------------
 *
 * rewriteheap.h
 *	  Declarations for heap rewrite support functions
 *
 * Portions Copyright (c) 1996-2007, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994-5, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/access/rewriteheap.h,v 1.2 2007/05/17 15:28:29 alvherre Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef REWRITE_HEAP_H
#define REWRITE_HEAP_H

#include "access/htup.h"
#include "utils/rel.h"

/* struct definition is private to rewriteheap.c */
typedef struct RewriteStateData *RewriteState;

extern RewriteState begin_heap_rewrite(Relation NewHeap,
				   TransactionId OldestXmin, TransactionId FreezeXid,
				   bool use_wal);
extern void end_heap_rewrite(RewriteState state);
extern void rewrite_heap_tuple(RewriteState state, HeapTuple oldTuple,
				   HeapTuple newTuple);
extern void rewrite_heap_dead_tuple(RewriteState state, HeapTuple oldTuple);

#endif   /* REWRITE_HEAP_H */
