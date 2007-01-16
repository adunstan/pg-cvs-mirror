/*-------------------------------------------------------------------------
 *
 * procarray.h
 *	  POSTGRES process array definitions.
 *
 *
 * Portions Copyright (c) 1996-2007, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/storage/procarray.h,v 1.11 2007/01/05 22:19:58 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef PROCARRAY_H
#define PROCARRAY_H

#include "storage/lock.h"


extern Size ProcArrayShmemSize(void);
extern void CreateSharedProcArray(void);
extern void ProcArrayAdd(PGPROC *proc);
extern void ProcArrayRemove(PGPROC *proc);

extern bool TransactionIdIsInProgress(TransactionId xid);
extern bool TransactionIdIsActive(TransactionId xid);
extern TransactionId GetOldestXmin(bool allDbs, bool ignoreVacuum);

extern PGPROC *BackendPidGetProc(int pid);
extern int	BackendXidGetPid(TransactionId xid);
extern bool IsBackendPid(int pid);
extern bool DatabaseCancelAutovacuumActivity(Oid databaseId, bool ignoreMyself);

extern int	CountActiveBackends(void);
extern int	CountDBBackends(Oid databaseid);
extern int	CountUserBackends(Oid roleid);

extern void XidCacheRemoveRunningXids(TransactionId xid,
						  int nxids, TransactionId *xids);

#endif   /* PROCARRAY_H */
