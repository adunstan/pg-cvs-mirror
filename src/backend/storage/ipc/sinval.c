/*-------------------------------------------------------------------------
 *
 * sinval.c
 *	  POSTGRES shared cache invalidation communication code.
 *
 * Portions Copyright (c) 1996-2001, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  $Header: /home/cvsmirror/pg/pgsql/src/backend/storage/ipc/sinval.c,v 1.36 2001/07/12 04:11:13 tgl Exp $
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include <sys/types.h>

#include "storage/proc.h"
#include "storage/sinval.h"
#include "storage/sinvaladt.h"
#include "utils/tqual.h"

SPINLOCK	SInvalLock = (SPINLOCK) NULL;

/****************************************************************************/
/*	CreateSharedInvalidationState()		 Initialize SI buffer				*/
/*																			*/
/*	should be called only by the POSTMASTER									*/
/****************************************************************************/
void
CreateSharedInvalidationState(int maxBackends)
{
	/* SInvalLock must be initialized already, during spinlock init */
	SIBufferInit(maxBackends);
}

/*
 * InitBackendSharedInvalidationState
 *		Initialize new backend's state info in buffer segment.
 */
void
InitBackendSharedInvalidationState(void)
{
	int		flag;

	SpinAcquire(SInvalLock);
	flag = SIBackendInit(shmInvalBuffer);
	SpinRelease(SInvalLock);
	if (flag < 0)				/* unexpected problem */
		elog(FATAL, "Backend cache invalidation initialization failed");
	if (flag == 0)				/* expected problem: MaxBackends exceeded */
		elog(FATAL, "Sorry, too many clients already");
}

/*
 * SendSharedInvalidMessage
 *	Add a shared-cache-invalidation message to the global SI message queue.
 */
void
SendSharedInvalidMessage(SharedInvalidationMessage *msg)
{
	bool		insertOK;

	SpinAcquire(SInvalLock);
	insertOK = SIInsertDataEntry(shmInvalBuffer, msg);
	SpinRelease(SInvalLock);
	if (!insertOK)
		elog(DEBUG, "SendSharedInvalidMessage: SI buffer overflow");
}

/*
 * ReceiveSharedInvalidMessages
 *		Process shared-cache-invalidation messages waiting for this backend
 */
void
ReceiveSharedInvalidMessages(
	void (*invalFunction) (SharedInvalidationMessage *msg),
	void (*resetFunction) (void))
{
	SharedInvalidationMessage data;
	int			getResult;
	bool		gotMessage = false;

	for (;;)
	{
		SpinAcquire(SInvalLock);
		getResult = SIGetDataEntry(shmInvalBuffer, MyBackendId, &data);
		SpinRelease(SInvalLock);
		if (getResult == 0)
			break;				/* nothing more to do */
		if (getResult < 0)
		{
			/* got a reset message */
			elog(DEBUG, "ReceiveSharedInvalidMessages: cache state reset");
			resetFunction();
		}
		else
		{
			/* got a normal data message */
			invalFunction(&data);
		}
		gotMessage = true;
	}

	/* If we got any messages, try to release dead messages */
	if (gotMessage)
	{
		SpinAcquire(SInvalLock);
		SIDelExpiredDataEntries(shmInvalBuffer);
		SpinRelease(SInvalLock);
	}
}


/****************************************************************************/
/* Functions that need to scan the PROC structures of all running backends. */
/* It's a bit strange to keep these in sinval.c, since they don't have any	*/
/* direct relationship to shared-cache invalidation.  But the procState		*/
/* array in the SI segment is the only place in the system where we have	*/
/* an array of per-backend data, so it is the most convenient place to keep */
/* pointers to the backends' PROC structures.  We used to implement these	*/
/* functions with a slow, ugly search through the ShmemIndex hash table --- */
/* now they are simple loops over the SI ProcState array.					*/
/****************************************************************************/


/*
 * DatabaseHasActiveBackends -- are there any backends running in the given DB
 *
 * If 'ignoreMyself' is TRUE, ignore this particular backend while checking
 * for backends in the target database.
 *
 * This function is used to interlock DROP DATABASE against there being
 * any active backends in the target DB --- dropping the DB while active
 * backends remain would be a Bad Thing.  Note that we cannot detect here
 * the possibility of a newly-started backend that is trying to connect
 * to the doomed database, so additional interlocking is needed during
 * backend startup.
 */

bool
DatabaseHasActiveBackends(Oid databaseId, bool ignoreMyself)
{
	bool		result = false;
	SISeg	   *segP = shmInvalBuffer;
	ProcState  *stateP = segP->procState;
	int			index;

	SpinAcquire(SInvalLock);

	for (index = 0; index < segP->lastBackend; index++)
	{
		SHMEM_OFFSET pOffset = stateP[index].procStruct;

		if (pOffset != INVALID_OFFSET)
		{
			PROC	   *proc = (PROC *) MAKE_PTR(pOffset);

			if (proc->databaseId == databaseId)
			{
				if (ignoreMyself && proc == MyProc)
					continue;

				result = true;
				break;
			}
		}
	}

	SpinRelease(SInvalLock);

	return result;
}

/*
 * TransactionIdIsInProgress -- is given transaction running by some backend
 */
bool
TransactionIdIsInProgress(TransactionId xid)
{
	bool		result = false;
	SISeg	   *segP = shmInvalBuffer;
	ProcState  *stateP = segP->procState;
	int			index;

	SpinAcquire(SInvalLock);

	for (index = 0; index < segP->lastBackend; index++)
	{
		SHMEM_OFFSET pOffset = stateP[index].procStruct;

		if (pOffset != INVALID_OFFSET)
		{
			PROC	   *proc = (PROC *) MAKE_PTR(pOffset);

			if (TransactionIdEquals(proc->xid, xid))
			{
				result = true;
				break;
			}
		}
	}

	SpinRelease(SInvalLock);

	return result;
}

/*
 * GetXmaxRecent -- returns oldest transaction that was running
 *					when all current transaction were started.
 *					It's used by vacuum to decide what deleted
 *					tuples must be preserved in a table.
 *
 * Note: we include all currently running xids in the set of considered xids.
 * This ensures that if a just-started xact has not yet set its snapshot,
 * when it does set the snapshot it cannot set xmin less than what we compute.
 */
void
GetXmaxRecent(TransactionId *XmaxRecent)
{
	SISeg	   *segP = shmInvalBuffer;
	ProcState  *stateP = segP->procState;
	TransactionId result;
	int			index;

	result = GetCurrentTransactionId();

	SpinAcquire(SInvalLock);

	for (index = 0; index < segP->lastBackend; index++)
	{
		SHMEM_OFFSET pOffset = stateP[index].procStruct;

		if (pOffset != INVALID_OFFSET)
		{
			PROC	   *proc = (PROC *) MAKE_PTR(pOffset);
			TransactionId xid;

			xid = proc->xid;
			if (! TransactionIdIsSpecial(xid))
			{
				if (TransactionIdPrecedes(xid, result))
					result = xid;
				xid = proc->xmin;
				if (! TransactionIdIsSpecial(xid))
					if (TransactionIdPrecedes(xid, result))
						result = xid;
			}
		}
	}

	SpinRelease(SInvalLock);

	*XmaxRecent = result;
}

/*
 * GetSnapshotData -- returns information about running transactions.
 */
Snapshot
GetSnapshotData(bool serializable)
{
	Snapshot	snapshot = (Snapshot) malloc(sizeof(SnapshotData));
	SISeg	   *segP = shmInvalBuffer;
	ProcState  *stateP = segP->procState;
	int			index;
	int			count = 0;

	if (snapshot == NULL)
		elog(ERROR, "Memory exhausted in GetSnapshotData");

	snapshot->xmin = GetCurrentTransactionId();

	SpinAcquire(SInvalLock);

	/*
	 * There can be no more than lastBackend active transactions, so this
	 * is enough space:
	 */
	snapshot->xip = (TransactionId *)
		malloc(segP->lastBackend * sizeof(TransactionId));
	if (snapshot->xip == NULL)
	{
		SpinRelease(SInvalLock);
		elog(ERROR, "Memory exhausted in GetSnapshotData");
	}

	/*
	 * Unfortunately, we have to call ReadNewTransactionId() after
	 * acquiring SInvalLock above. It's not good because
	 * ReadNewTransactionId() does SpinAcquire(XidGenLockId) but
	 * _necessary_.
	 */
	ReadNewTransactionId(&(snapshot->xmax));

	for (index = 0; index < segP->lastBackend; index++)
	{
		SHMEM_OFFSET pOffset = stateP[index].procStruct;

		if (pOffset != INVALID_OFFSET)
		{
			PROC	   *proc = (PROC *) MAKE_PTR(pOffset);
			TransactionId xid = proc->xid;

			/*
			 * Ignore my own proc (dealt with my xid above), procs not
			 * running a transaction, and xacts started since we read
			 * the next transaction ID.  There's no need to store XIDs
			 * above what we got from ReadNewTransactionId, since we'll
			 * treat them as running anyway.
			 */
			if (proc == MyProc ||
				TransactionIdIsSpecial(xid) ||
				! TransactionIdPrecedes(xid, snapshot->xmax))
				continue;

			if (TransactionIdPrecedes(xid, snapshot->xmin))
				snapshot->xmin = xid;
			snapshot->xip[count] = xid;
			count++;
		}
	}

	if (serializable)
		MyProc->xmin = snapshot->xmin;
	/* Serializable snapshot must be computed before any other... */
	Assert(MyProc->xmin != InvalidTransactionId);

	SpinRelease(SInvalLock);

	snapshot->xcnt = count;
	return snapshot;
}

/*
 * CountActiveBackends --- count backends (other than myself) that are in
 *		active transactions.  This is used as a heuristic to decide if
 *		a pre-XLOG-flush delay is worthwhile during commit.
 *
 * An active transaction is something that has written at least one XLOG
 * record; read-only transactions don't count.  Also, do not count backends
 * that are blocked waiting for locks, since they are not going to get to
 * run until someone else commits.
 */
int
CountActiveBackends(void)
{
	SISeg	   *segP = shmInvalBuffer;
	ProcState  *stateP = segP->procState;
	int			count = 0;
	int			index;

	/*
	 * Note: for speed, we don't acquire SInvalLock.  This is a little bit
	 * bogus, but since we are only testing xrecoff for zero or nonzero,
	 * it should be OK.  The result is only used for heuristic purposes
	 * anyway...
	 */
	for (index = 0; index < segP->lastBackend; index++)
	{
		SHMEM_OFFSET pOffset = stateP[index].procStruct;

		if (pOffset != INVALID_OFFSET)
		{
			PROC	   *proc = (PROC *) MAKE_PTR(pOffset);

			if (proc == MyProc)
				continue;		/* do not count myself */
			if (proc->logRec.xrecoff == 0)
				continue;		/* do not count if not in a transaction */
			if (proc->waitLock != NULL)
				continue;		/* do not count if blocked on a lock */
			count++;
		}
	}

	return count;
}

/*
 * GetUndoRecPtr -- returns oldest PROC->logRec.
 */
XLogRecPtr
GetUndoRecPtr(void)
{
	SISeg	   *segP = shmInvalBuffer;
	ProcState  *stateP = segP->procState;
	XLogRecPtr	urec = {0, 0};
	XLogRecPtr	tempr;
	int			index;

	SpinAcquire(SInvalLock);

	for (index = 0; index < segP->lastBackend; index++)
	{
		SHMEM_OFFSET pOffset = stateP[index].procStruct;

		if (pOffset != INVALID_OFFSET)
		{
			PROC	   *proc = (PROC *) MAKE_PTR(pOffset);

			tempr = proc->logRec;
			if (tempr.xrecoff == 0)
				continue;
			if (urec.xrecoff != 0 && XLByteLT(urec, tempr))
				continue;
			urec = tempr;
		}
	}

	SpinRelease(SInvalLock);

	return (urec);
}

/*
 * BackendIdGetProc - given a BackendId, find its PROC structure
 *
 * This is a trivial lookup in the ProcState array.  We assume that the caller
 * knows that the backend isn't going to go away, so we do not bother with
 * locking.
 */
struct proc *
BackendIdGetProc(BackendId procId)
{
	SISeg	   *segP = shmInvalBuffer;

	if (procId > 0 && procId <= segP->lastBackend)
	{
		ProcState  *stateP = &segP->procState[procId - 1];
		SHMEM_OFFSET pOffset = stateP->procStruct;

		if (pOffset != INVALID_OFFSET)
		{
			PROC	   *proc = (PROC *) MAKE_PTR(pOffset);

			return proc;
		}
	}

	return NULL;
}
