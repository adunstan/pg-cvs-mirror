/*-------------------------------------------------------------------------
 *
 * pqsignal.h
 *	  prototypes for the reliable BSD-style signal(2) routine.
 *
 *
 * Portions Copyright (c) 1996-2001, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $Id: pqsignal.h,v 1.12 2001/10/28 06:26:12 momjian Exp $
 *
 * NOTES
 *	  This shouldn't be in libpq, but the monitor and some other
 *	  things need it...
 *
 *-------------------------------------------------------------------------
 */
#ifndef PQSIGNAL_H
#define PQSIGNAL_H

#include "postgres_fe.h"

typedef void (*pqsigfunc) (int);

extern pqsigfunc pqsignal(int signo, pqsigfunc func);

#endif	 /* PQSIGNAL_H */
