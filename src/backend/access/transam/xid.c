/*-------------------------------------------------------------------------
 *
 * xid.c--
 *	  POSTGRES transaction identifier code.
 *
 * Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  $Header: /home/cvsmirror/pg/pgsql/src/backend/access/transam/Attic/xid.c,v 1.13 1998/03/30 17:22:28 momjian Exp $
 *
 * OLD COMMENTS
 * XXX WARNING
 *		Much of this file will change when we change our representation
 *		of transaction ids -cim 3/23/90
 *
 * It is time to make the switch from 5 byte to 4 byte transaction ids
 * This file was totally reworked. -mer 5/22/92
 *
 *-------------------------------------------------------------------------
 */

#include <stdio.h>

#include <postgres.h>
#include <access/xact.h>

extern TransactionId NullTransactionId;
extern TransactionId DisabledTransactionId;
extern TransactionId AmiTransactionId;
extern TransactionId FirstTransactionId;

/* XXX name for catalogs */
TransactionId
xidin(char *representation)
{
	return (atol(representation));
}

/* XXX name for catalogs */
char *
xidout(TransactionId transactionId)
{
/*	  return(TransactionIdFormString(transactionId)); */
	char	   *representation;

	/* maximum 32 bit unsigned integer representation takes 10 chars */
	representation = palloc(11);

	sprintf(representation, "%u", transactionId);

	return (representation);

}

/* ----------------------------------------------------------------
 *		TransactionIdEquals
 * ----------------------------------------------------------------
 */
bool
TransactionIdEquals(TransactionId id1, TransactionId id2)
{
	return ((bool) (id1 == id2));
}

/* ----------------------------------------------------------------
 *		TransactionIdIsLessThan
 * ----------------------------------------------------------------
 */
bool
TransactionIdIsLessThan(TransactionId id1, TransactionId id2)
{
	return ((bool) (id1 < id2));
}

/* ----------------------------------------------------------------
 *		xideq
 * ----------------------------------------------------------------
 */

/*
 *		xideq			- returns 1, iff xid1 == xid2
 *								  0  else;
 */
bool
xideq(TransactionId xid1, TransactionId xid2)
{
	return ((bool) (xid1 == xid2));
}



/* ----------------------------------------------------------------
 *		TransactionIdIncrement
 * ----------------------------------------------------------------
 */
#ifdef NOT_USED
void
TransactionIdIncrement(TransactionId *transactionId)
{

	(*transactionId)++;
	if (*transactionId == DisabledTransactionId)
		elog(FATAL, "TransactionIdIncrement: exhausted XID's");
	return;
}

#endif

/* ----------------------------------------------------------------
 *		TransactionIdAdd
 * ----------------------------------------------------------------
 */
void
TransactionIdAdd(TransactionId *xid, int value)
{
	*xid += value;
	return;
}
