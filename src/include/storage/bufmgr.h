/*-------------------------------------------------------------------------
 *
 * bufmgr.h
 *	  POSTGRES buffer manager definitions.
 *
 *
 * Portions Copyright (c) 1996-2000, PostgreSQL, Inc
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $Id: bufmgr.h,v 1.41 2000/10/20 11:01:21 vadim Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef BUFMGR_H
#define BUFMGR_H

#include "storage/buf_internals.h"


typedef void *Block;

/* special pageno for bget */
#define P_NEW	InvalidBlockNumber		/* grow the file to get a new page */


/**********************************************************************

  the rest is function defns in the bufmgr that are externally callable

 **********************************************************************/

/*
 * These routines are beaten on quite heavily, hence the macroization.
 * See buf_internals.h for a related comment.
 */
#define BufferDescriptorGetBuffer(bdesc) ((bdesc)->buf_id + 1)

extern int	ShowPinTrace;

/*
 * Buffer context lock modes
 */
#define BUFFER_LOCK_UNLOCK		0
#define BUFFER_LOCK_SHARE		1
#define BUFFER_LOCK_EXCLUSIVE	2

#define UnlockAndReleaseBuffer(buffer)	\
( \
	LockBuffer(buffer, BUFFER_LOCK_UNLOCK), \
	ReleaseBuffer(buffer) \
)

#define UnlockAndWriteBuffer(buffer)	\
( \
	LockBuffer(buffer, BUFFER_LOCK_UNLOCK), \
	WriteBuffer(buffer) \
)

/*
 * BufferIsValid
 *		True iff the given buffer number is valid (either as a shared
 *		or local buffer).
 *
 * Note:
 *		BufferIsValid(InvalidBuffer) is False.
 *		BufferIsValid(UnknownBuffer) is False.
 *
 * Note: For a long time this was defined the same as BufferIsPinned,
 * that is it would say False if you didn't hold a pin on the buffer.
 * I believe this was bogus and served only to mask logic errors.
 * Code should always know whether it has a buffer reference,
 * independently of the pin state.
 */
#define BufferIsValid(bufnum) \
( \
	BufferIsLocal(bufnum) ? \
		((bufnum) >= -NLocBuffer) \
	: \
		(! BAD_BUFFER_ID(bufnum)) \
)

/*
 * BufferIsPinned
 *		True iff the buffer is pinned (also checks for valid buffer number).
 *
 *		NOTE: what we check here is that *this* backend holds a pin on
 *		the buffer.  We do not care whether some other backend does.
 */
#define BufferIsPinned(bufnum) \
( \
	BufferIsLocal(bufnum) ? \
		((bufnum) >= -NLocBuffer && LocalRefCount[-(bufnum) - 1] > 0) \
	: \
	( \
		BAD_BUFFER_ID(bufnum) ? \
			false \
		: \
			(PrivateRefCount[(bufnum) - 1] > 0) \
	) \
)

/*
 * IncrBufferRefCount
 *		Increment the pin count on a buffer that we have *already* pinned
 *		at least once.
 *
 *		This macro cannot be used on a buffer we do not have pinned,
 *		because it doesn't change the shared buffer state.  Therefore the
 *		Assert checks are for refcount > 0.  Someone got this wrong once...
 */
#define IncrBufferRefCount(buffer) \
( \
	BufferIsLocal(buffer) ? \
	( \
		(void) AssertMacro((buffer) >= -NLocBuffer), \
		(void) AssertMacro(LocalRefCount[-(buffer) - 1] > 0), \
		(void) LocalRefCount[-(buffer) - 1]++ \
	) \
	: \
	( \
		(void) AssertMacro(!BAD_BUFFER_ID(buffer)), \
		(void) AssertMacro(PrivateRefCount[(buffer) - 1] > 0), \
		(void) PrivateRefCount[(buffer) - 1]++ \
	) \
)

/*
 * BufferGetBlock
 *		Returns a reference to a disk page image associated with a buffer.
 *
 * Note:
 *		Assumes buffer is valid.
 */
#define BufferGetBlock(buffer) \
( \
	AssertMacro(BufferIsValid(buffer)), \
	BufferIsLocal(buffer) ? \
		((Block) MAKE_PTR(LocalBufferDescriptors[-(buffer) - 1].data)) \
	: \
		((Block) MAKE_PTR(BufferDescriptors[(buffer) - 1].data)) \
)


/*
 * prototypes for functions in bufmgr.c
 */
extern Buffer RelationGetBufferWithBuffer(Relation relation,
							BlockNumber blockNumber, Buffer buffer);
extern Buffer ReadBuffer(Relation reln, BlockNumber blockNum);
extern int	WriteBuffer(Buffer buffer);
extern int	WriteNoReleaseBuffer(Buffer buffer);
extern Buffer ReleaseAndReadBuffer(Buffer buffer, Relation relation,
					 BlockNumber blockNum);
extern int	FlushBuffer(Buffer buffer, bool release);

extern void InitBufferPool(IPCKey key);
extern void PrintBufferUsage(FILE *statfp);
extern void ResetBufferUsage(void);
extern void ResetBufferPool(bool isCommit);
extern int	BufferPoolCheckLeak(void);
extern void FlushBufferPool(void);
extern BlockNumber BufferGetBlockNumber(Buffer buffer);
extern BlockNumber RelationGetNumberOfBlocks(Relation relation);
extern int	FlushRelationBuffers(Relation rel, BlockNumber firstDelBlock);
extern void ReleaseRelationBuffers(Relation rel);
extern void DropBuffers(Oid dbid);
extern void PrintPinnedBufs(void);
extern int	BufferShmemSize(void);
extern int	ReleaseBuffer(Buffer buffer);

extern void SetBufferCommitInfoNeedsSave(Buffer buffer);

extern void UnlockBuffers(void);
extern void LockBuffer(Buffer buffer, int mode);
extern void AbortBufferIO(void);

extern bool BufferIsUpdatable(Buffer buffer);
extern void MarkBufferForCleanup(Buffer buffer, void (*CleanupFunc)(Buffer));

#endif
