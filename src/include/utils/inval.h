/*-------------------------------------------------------------------------
 *
 * inval.h
 *	  POSTGRES cache invalidation dispatcher definitions.
 *
 *
 * Portions Copyright (c) 1996-2003, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql-server/src/include/utils/inval.h,v 1.29 2003/11/29 22:41:15 pgsql Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef INVAL_H
#define INVAL_H

#include "access/htup.h"


typedef void (*CacheCallbackFunction) (Datum arg, Oid relid);


extern void AcceptInvalidationMessages(void);

extern void AtEOXactInvalidationMessages(bool isCommit);

extern void CommandEndInvalidationMessages(bool isCommit);

extern void CacheInvalidateHeapTuple(Relation relation, HeapTuple tuple);

extern void CacheInvalidateRelcache(Relation relation);

extern void CacheInvalidateRelcacheByTuple(HeapTuple classTuple);

extern void CacheRegisterSyscacheCallback(int cacheid,
							  CacheCallbackFunction func,
							  Datum arg);

extern void CacheRegisterRelcacheCallback(CacheCallbackFunction func,
							  Datum arg);

#endif   /* INVAL_H */
