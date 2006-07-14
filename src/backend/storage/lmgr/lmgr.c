/*-------------------------------------------------------------------------
 *
 * lmgr.c
 *	  POSTGRES lock manager code
 *
 * Portions Copyright (c) 1996-2006, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  $PostgreSQL: pgsql/src/backend/storage/lmgr/lmgr.c,v 1.83 2006/05/04 16:07:29 tgl Exp $
 *
 *-------------------------------------------------------------------------
 */

#include "postgres.h"

#include "access/subtrans.h"
#include "miscadmin.h"
#include "storage/lmgr.h"
#include "storage/procarray.h"
#include "utils/inval.h"


/*
 * RelationInitLockInfo
 *		Initializes the lock information in a relation descriptor.
 *
 *		relcache.c must call this during creation of any reldesc.
 */
void
RelationInitLockInfo(Relation relation)
{
	Assert(RelationIsValid(relation));
	Assert(OidIsValid(RelationGetRelid(relation)));

	relation->rd_lockInfo.lockRelId.relId = RelationGetRelid(relation);

	if (relation->rd_rel->relisshared)
		relation->rd_lockInfo.lockRelId.dbId = InvalidOid;
	else
		relation->rd_lockInfo.lockRelId.dbId = MyDatabaseId;
}

/*
 *		LockRelation
 */
void
LockRelation(Relation relation, LOCKMODE lockmode)
{
	LOCKTAG		tag;
	LockAcquireResult res;

	SET_LOCKTAG_RELATION(tag,
						 relation->rd_lockInfo.lockRelId.dbId,
						 relation->rd_lockInfo.lockRelId.relId);

	res = LockAcquire(&tag, relation->rd_istemp, lockmode, false, false);

	/*
	 * Check to see if the relcache entry has been invalidated while we were
	 * waiting to lock it.	If so, rebuild it, or ereport() trying. Increment
	 * the refcount to ensure that RelationFlushRelation will rebuild it and
	 * not just delete it.	We can skip this if the lock was already held,
	 * however.
	 */
	if (res != LOCKACQUIRE_ALREADY_HELD)
	{
		RelationIncrementReferenceCount(relation);
		AcceptInvalidationMessages();
		RelationDecrementReferenceCount(relation);
	}
}

/*
 *		ConditionalLockRelation
 *
 * As above, but only lock if we can get the lock without blocking.
 * Returns TRUE iff the lock was acquired.
 *
 * NOTE: we do not currently need conditional versions of all the
 * LockXXX routines in this file, but they could easily be added if needed.
 */
bool
ConditionalLockRelation(Relation relation, LOCKMODE lockmode)
{
	LOCKTAG		tag;
	LockAcquireResult res;

	SET_LOCKTAG_RELATION(tag,
						 relation->rd_lockInfo.lockRelId.dbId,
						 relation->rd_lockInfo.lockRelId.relId);

	res = LockAcquire(&tag, relation->rd_istemp, lockmode, false, true);

	if (res == LOCKACQUIRE_NOT_AVAIL)
		return false;

	/*
	 * Check to see if the relcache entry has been invalidated while we were
	 * waiting to lock it.	If so, rebuild it, or ereport() trying. Increment
	 * the refcount to ensure that RelationFlushRelation will rebuild it and
	 * not just delete it.	We can skip this if the lock was already held,
	 * however.
	 */
	if (res != LOCKACQUIRE_ALREADY_HELD)
	{
		RelationIncrementReferenceCount(relation);
		AcceptInvalidationMessages();
		RelationDecrementReferenceCount(relation);
	}

	return true;
}

/*
 *		UnlockRelation
 */
void
UnlockRelation(Relation relation, LOCKMODE lockmode)
{
	LOCKTAG		tag;

	SET_LOCKTAG_RELATION(tag,
						 relation->rd_lockInfo.lockRelId.dbId,
						 relation->rd_lockInfo.lockRelId.relId);

	LockRelease(&tag, lockmode, false);
}

/*
 *		LockRelationForSession
 *
 * This routine grabs a session-level lock on the target relation.	The
 * session lock persists across transaction boundaries.  It will be removed
 * when UnlockRelationForSession() is called, or if an ereport(ERROR) occurs,
 * or if the backend exits.
 *
 * Note that one should also grab a transaction-level lock on the rel
 * in any transaction that actually uses the rel, to ensure that the
 * relcache entry is up to date.
 */
void
LockRelationForSession(LockRelId *relid, bool istemprel, LOCKMODE lockmode)
{
	LOCKTAG		tag;

	SET_LOCKTAG_RELATION(tag, relid->dbId, relid->relId);

	(void) LockAcquire(&tag, istemprel, lockmode, true, false);
}

/*
 *		UnlockRelationForSession
 */
void
UnlockRelationForSession(LockRelId *relid, LOCKMODE lockmode)
{
	LOCKTAG		tag;

	SET_LOCKTAG_RELATION(tag, relid->dbId, relid->relId);

	LockRelease(&tag, lockmode, true);
}

/*
 *		LockRelationForExtension
 *
 * This lock tag is used to interlock addition of pages to relations.
 * We need such locking because bufmgr/smgr definition of P_NEW is not
 * race-condition-proof.
 *
 * We assume the caller is already holding some type of regular lock on
 * the relation, so no AcceptInvalidationMessages call is needed here.
 */
void
LockRelationForExtension(Relation relation, LOCKMODE lockmode)
{
	LOCKTAG		tag;

	SET_LOCKTAG_RELATION_EXTEND(tag,
								relation->rd_lockInfo.lockRelId.dbId,
								relation->rd_lockInfo.lockRelId.relId);

	(void) LockAcquire(&tag, relation->rd_istemp, lockmode, false, false);
}

/*
 *		UnlockRelationForExtension
 */
void
UnlockRelationForExtension(Relation relation, LOCKMODE lockmode)
{
	LOCKTAG		tag;

	SET_LOCKTAG_RELATION_EXTEND(tag,
								relation->rd_lockInfo.lockRelId.dbId,
								relation->rd_lockInfo.lockRelId.relId);

	LockRelease(&tag, lockmode, false);
}

/*
 *		LockPage
 *
 * Obtain a page-level lock.  This is currently used by some index access
 * methods to lock individual index pages.
 */
void
LockPage(Relation relation, BlockNumber blkno, LOCKMODE lockmode)
{
	LOCKTAG		tag;

	SET_LOCKTAG_PAGE(tag,
					 relation->rd_lockInfo.lockRelId.dbId,
					 relation->rd_lockInfo.lockRelId.relId,
					 blkno);

	(void) LockAcquire(&tag, relation->rd_istemp, lockmode, false, false);
}

/*
 *		ConditionalLockPage
 *
 * As above, but only lock if we can get the lock without blocking.
 * Returns TRUE iff the lock was acquired.
 */
bool
ConditionalLockPage(Relation relation, BlockNumber blkno, LOCKMODE lockmode)
{
	LOCKTAG		tag;

	SET_LOCKTAG_PAGE(tag,
					 relation->rd_lockInfo.lockRelId.dbId,
					 relation->rd_lockInfo.lockRelId.relId,
					 blkno);

	return (LockAcquire(&tag, relation->rd_istemp,
						lockmode, false, true) != LOCKACQUIRE_NOT_AVAIL);
}

/*
 *		UnlockPage
 */
void
UnlockPage(Relation relation, BlockNumber blkno, LOCKMODE lockmode)
{
	LOCKTAG		tag;

	SET_LOCKTAG_PAGE(tag,
					 relation->rd_lockInfo.lockRelId.dbId,
					 relation->rd_lockInfo.lockRelId.relId,
					 blkno);

	LockRelease(&tag, lockmode, false);
}

/*
 *		LockTuple
 *
 * Obtain a tuple-level lock.  This is used in a less-than-intuitive fashion
 * because we can't afford to keep a separate lock in shared memory for every
 * tuple.  See heap_lock_tuple before using this!
 */
void
LockTuple(Relation relation, ItemPointer tid, LOCKMODE lockmode)
{
	LOCKTAG		tag;

	SET_LOCKTAG_TUPLE(tag,
					  relation->rd_lockInfo.lockRelId.dbId,
					  relation->rd_lockInfo.lockRelId.relId,
					  ItemPointerGetBlockNumber(tid),
					  ItemPointerGetOffsetNumber(tid));

	(void) LockAcquire(&tag, relation->rd_istemp, lockmode, false, false);
}

/*
 *		ConditionalLockTuple
 *
 * As above, but only lock if we can get the lock without blocking.
 * Returns TRUE iff the lock was acquired.
 */
bool
ConditionalLockTuple(Relation relation, ItemPointer tid, LOCKMODE lockmode)
{
	LOCKTAG		tag;

	SET_LOCKTAG_TUPLE(tag,
					  relation->rd_lockInfo.lockRelId.dbId,
					  relation->rd_lockInfo.lockRelId.relId,
					  ItemPointerGetBlockNumber(tid),
					  ItemPointerGetOffsetNumber(tid));

	return (LockAcquire(&tag, relation->rd_istemp,
						lockmode, false, true) != LOCKACQUIRE_NOT_AVAIL);
}

/*
 *		UnlockTuple
 */
void
UnlockTuple(Relation relation, ItemPointer tid, LOCKMODE lockmode)
{
	LOCKTAG		tag;

	SET_LOCKTAG_TUPLE(tag,
					  relation->rd_lockInfo.lockRelId.dbId,
					  relation->rd_lockInfo.lockRelId.relId,
					  ItemPointerGetBlockNumber(tid),
					  ItemPointerGetOffsetNumber(tid));

	LockRelease(&tag, lockmode, false);
}

/*
 *		XactLockTableInsert
 *
 * Insert a lock showing that the given transaction ID is running ---
 * this is done during xact startup.  The lock can then be used to wait
 * for the transaction to finish.
 */
void
XactLockTableInsert(TransactionId xid)
{
	LOCKTAG		tag;

	SET_LOCKTAG_TRANSACTION(tag, xid);

	(void) LockAcquire(&tag, false, ExclusiveLock, false, false);
}

/*
 *		XactLockTableDelete
 *
 * Delete the lock showing that the given transaction ID is running.
 * (This is never used for main transaction IDs; those locks are only
 * released implicitly at transaction end.	But we do use it for subtrans
 * IDs.)
 */
void
XactLockTableDelete(TransactionId xid)
{
	LOCKTAG		tag;

	SET_LOCKTAG_TRANSACTION(tag, xid);

	LockRelease(&tag, ExclusiveLock, false);
}

/*
 *		XactLockTableWait
 *
 * Wait for the specified transaction to commit or abort.
 *
 * Note that this does the right thing for subtransactions: if we wait on a
 * subtransaction, we will exit as soon as it aborts or its top parent commits.
 * It takes some extra work to ensure this, because to save on shared memory
 * the XID lock of a subtransaction is released when it ends, whether
 * successfully or unsuccessfully.	So we have to check if it's "still running"
 * and if so wait for its parent.
 */
void
XactLockTableWait(TransactionId xid)
{
	LOCKTAG		tag;

	for (;;)
	{
		Assert(TransactionIdIsValid(xid));
		Assert(!TransactionIdEquals(xid, GetTopTransactionId()));

		SET_LOCKTAG_TRANSACTION(tag, xid);

		(void) LockAcquire(&tag, false, ShareLock, false, false);

		LockRelease(&tag, ShareLock, false);

		if (!TransactionIdIsInProgress(xid))
			break;
		xid = SubTransGetParent(xid);
	}
}

/*
 *		ConditionalXactLockTableWait
 *
 * As above, but only lock if we can get the lock without blocking.
 * Returns TRUE if the lock was acquired.
 */
bool
ConditionalXactLockTableWait(TransactionId xid)
{
	LOCKTAG		tag;

	for (;;)
	{
		Assert(TransactionIdIsValid(xid));
		Assert(!TransactionIdEquals(xid, GetTopTransactionId()));

		SET_LOCKTAG_TRANSACTION(tag, xid);

		if (LockAcquire(&tag, false,
						ShareLock, false, true) == LOCKACQUIRE_NOT_AVAIL)
			return false;

		LockRelease(&tag, ShareLock, false);

		if (!TransactionIdIsInProgress(xid))
			break;
		xid = SubTransGetParent(xid);
	}

	return true;
}

/*
 *		LockDatabaseObject
 *
 * Obtain a lock on a general object of the current database.  Don't use
 * this for shared objects (such as tablespaces).  It's unwise to apply it
 * to relations, also, since a lock taken this way will NOT conflict with
 * LockRelation, and also may be wrongly marked if the relation is temp.
 * (If we ever invent temp objects that aren't tables, we'll want to extend
 * the API of this routine to include an isTempObject flag.)
 */
void
LockDatabaseObject(Oid classid, Oid objid, uint16 objsubid,
				   LOCKMODE lockmode)
{
	LOCKTAG		tag;

	SET_LOCKTAG_OBJECT(tag,
					   MyDatabaseId,
					   classid,
					   objid,
					   objsubid);

	(void) LockAcquire(&tag, false, lockmode, false, false);
}

/*
 *		UnlockDatabaseObject
 */
void
UnlockDatabaseObject(Oid classid, Oid objid, uint16 objsubid,
					 LOCKMODE lockmode)
{
	LOCKTAG		tag;

	SET_LOCKTAG_OBJECT(tag,
					   MyDatabaseId,
					   classid,
					   objid,
					   objsubid);

	LockRelease(&tag, lockmode, false);
}

/*
 *		LockSharedObject
 *
 * Obtain a lock on a shared-across-databases object.
 */
void
LockSharedObject(Oid classid, Oid objid, uint16 objsubid,
				 LOCKMODE lockmode)
{
	LOCKTAG		tag;

	SET_LOCKTAG_OBJECT(tag,
					   InvalidOid,
					   classid,
					   objid,
					   objsubid);

	(void) LockAcquire(&tag, false, lockmode, false, false);

	/* Make sure syscaches are up-to-date with any changes we waited for */
	AcceptInvalidationMessages();
}

/*
 *		UnlockSharedObject
 */
void
UnlockSharedObject(Oid classid, Oid objid, uint16 objsubid,
				   LOCKMODE lockmode)
{
	LOCKTAG		tag;

	SET_LOCKTAG_OBJECT(tag,
					   InvalidOid,
					   classid,
					   objid,
					   objsubid);

	LockRelease(&tag, lockmode, false);
}
