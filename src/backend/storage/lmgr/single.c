/*-------------------------------------------------------------------------
 *
 * single.c--
 *	  set single locks in the multi-level lock hierarchy
 *
 *	  Sometimes we don't want to set all levels of the multi-level
 *		lock hierarchy at once.  This allows us to set and release
 *		one level at a time.  It's useful in index scans when
 *		you can set an intent lock at the beginning and thereafter
 *		only set page locks.  Tends to speed things up.
 *
 * Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  $Header: /home/cvsmirror/pg/pgsql/src/backend/storage/lmgr/Attic/single.c,v 1.6 1998/06/28 21:17:35 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#include <string.h>

#include "postgres.h"

#include "storage/lmgr.h"		/* where the declarations go */
#include "storage/lock.h"
#include "storage/multilev.h"
#include "utils/rel.h"

/*
 * SingleLockReln -- lock a relation
 *
 * Returns: TRUE if the lock can be set, FALSE otherwise.
 */
bool
SingleLockReln(LockInfo linfo, LOCKTYPE locktype, int action)
{
	LOCKTAG		tag;

	/*
	 * LOCKTAG has two bytes of padding, unfortunately.  The hash function
	 * will return miss if the padding bytes aren't zero'd.
	 */
	MemSet(&tag, 0, sizeof(tag));
	tag.relId = linfo->lRelId.relId;
	tag.dbId = linfo->lRelId.dbId;
	BlockIdSet(&(tag.tupleId.ip_blkid), InvalidBlockNumber);
	tag.tupleId.ip_posid = InvalidOffsetNumber;

	if (action == UNLOCK)
		return (LockRelease(MultiTableId, &tag, locktype));
	else
		return (LockAcquire(MultiTableId, &tag, locktype));
}

/*
 * SingleLockPage -- use multi-level lock table, but lock
 *		only at the page level.
 *
 * Assumes that an INTENT lock has already been set in the
 * multi-level lock table.
 *
 */
bool
SingleLockPage(LockInfo linfo,
			   ItemPointer tidPtr,
			   LOCKTYPE locktype,
			   int action)
{
	LOCKTAG		tag;

	/*
	 * LOCKTAG has two bytes of padding, unfortunately.  The hash function
	 * will return miss if the padding bytes aren't zero'd.
	 */
	MemSet(&tag, 0, sizeof(tag));
	tag.relId = linfo->lRelId.relId;
	tag.dbId = linfo->lRelId.dbId;
	BlockIdCopy(&(tag.tupleId.ip_blkid), &(tidPtr->ip_blkid));
	tag.tupleId.ip_posid = InvalidOffsetNumber;


	if (action == UNLOCK)
		return (LockRelease(MultiTableId, &tag, locktype));
	else
		return (LockAcquire(MultiTableId, &tag, locktype));
}
