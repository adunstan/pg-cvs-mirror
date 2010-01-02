/*-------------------------------------------------------------------------
 *
 * shmqueue.c
 *	  shared memory linked lists
 *
 * Portions Copyright (c) 1996-2010, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  $PostgreSQL: pgsql/src/backend/storage/ipc/shmqueue.c,v 1.33 2009/01/01 17:23:47 momjian Exp $
 *
 * NOTES
 *
 * Package for managing doubly-linked lists in shared memory.
 * The only tricky thing is that SHM_QUEUE will usually be a field
 * in a larger record.	SHMQueueNext has to return a pointer
 * to the record itself instead of a pointer to the SHMQueue field
 * of the record.  It takes an extra parameter and does some extra
 * pointer arithmetic to do this correctly.
 *
 * NOTE: These are set up so they can be turned into macros some day.
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include "storage/shmem.h"


/*
 * ShmemQueueInit -- make the head of a new queue point
 *		to itself
 */
void
SHMQueueInit(SHM_QUEUE *queue)
{
	Assert(ShmemAddrIsValid(queue));
	queue->prev = queue->next = queue;
}

/*
 * SHMQueueIsDetached -- TRUE if element is not currently
 *		in a queue.
 */
#ifdef NOT_USED
bool
SHMQueueIsDetached(SHM_QUEUE *queue)
{
	Assert(ShmemAddrIsValid(queue));
	return (queue->prev == NULL);
}
#endif

/*
 * SHMQueueElemInit -- clear an element's links
 */
void
SHMQueueElemInit(SHM_QUEUE *queue)
{
	Assert(ShmemAddrIsValid(queue));
	queue->prev = queue->next = NULL;
}

/*
 * SHMQueueDelete -- remove an element from the queue and
 *		close the links
 */
void
SHMQueueDelete(SHM_QUEUE *queue)
{
	SHM_QUEUE  *nextElem = queue->next;
	SHM_QUEUE  *prevElem = queue->prev;

	Assert(ShmemAddrIsValid(queue));
	Assert(ShmemAddrIsValid(nextElem));
	Assert(ShmemAddrIsValid(prevElem));

	prevElem->next = queue->next;
	nextElem->prev = queue->prev;

	queue->prev = queue->next = NULL;
}

/*
 * SHMQueueInsertBefore -- put elem in queue before the given queue
 *		element.  Inserting "before" the queue head puts the elem
 *		at the tail of the queue.
 */
void
SHMQueueInsertBefore(SHM_QUEUE *queue, SHM_QUEUE *elem)
{
	SHM_QUEUE  *prevPtr = queue->prev;

	Assert(ShmemAddrIsValid(queue));
	Assert(ShmemAddrIsValid(elem));

	elem->next = prevPtr->next;
	elem->prev = queue->prev;
	queue->prev = elem;
	prevPtr->next = elem;
}

/*
 * SHMQueueInsertAfter -- put elem in queue after the given queue
 *		element.  Inserting "after" the queue head puts the elem
 *		at the head of the queue.
 */
#ifdef NOT_USED
void
SHMQueueInsertAfter(SHM_QUEUE *queue, SHM_QUEUE *elem)
{
	SHM_QUEUE  *nextPtr = queue->next;

	Assert(ShmemAddrIsValid(queue));
	Assert(ShmemAddrIsValid(elem));

	elem->prev = nextPtr->prev;
	elem->next = queue->next;
	queue->next = elem;
	nextPtr->prev = elem;
}
#endif   /* NOT_USED */

/*--------------------
 * SHMQueueNext -- Get the next element from a queue
 *
 * To start the iteration, pass the queue head as both queue and curElem.
 * Returns NULL if no more elements.
 *
 * Next element is at curElem->next.  If SHMQueue is part of
 * a larger structure, we want to return a pointer to the
 * whole structure rather than a pointer to its SHMQueue field.
 * For example,
 * struct {
 *		int				stuff;
 *		SHMQueue		elem;
 * } ELEMType;
 * When this element is in a queue, prevElem->next points at struct.elem.
 * We subtract linkOffset to get the correct start address of the structure.
 *
 * calls to SHMQueueNext should take these parameters:
 *	 &(queueHead), &(queueHead), offsetof(ELEMType, elem)
 * or
 *	 &(queueHead), &(curElem->elem), offsetof(ELEMType, elem)
 *--------------------
 */
Pointer
SHMQueueNext(SHM_QUEUE *queue, SHM_QUEUE *curElem, Size linkOffset)
{
	SHM_QUEUE  *elemPtr = curElem->next;

	Assert(ShmemAddrIsValid(curElem));

	if (elemPtr == queue)		/* back to the queue head? */
		return NULL;

	return (Pointer) (((char *) elemPtr) - linkOffset);
}

/*
 * SHMQueueEmpty -- TRUE if queue head is only element, FALSE otherwise
 */
bool
SHMQueueEmpty(SHM_QUEUE *queue)
{
	Assert(ShmemAddrIsValid(queue));

	if (queue->prev == queue)
	{
		Assert(queue->next == queue);
		return TRUE;
	}
	return FALSE;
}
