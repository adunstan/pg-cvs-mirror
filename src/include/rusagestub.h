/*-------------------------------------------------------------------------
 *
 * rusagestub.h
 *	  Stubs for getrusage(3).
 *
 *
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/rusagestub.h,v 1.18 2007/01/05 22:19:50 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef RUSAGESTUB_H
#define RUSAGESTUB_H

#include <sys/time.h>			/* for struct timeval */
#ifndef WIN32
#include <sys/times.h>			/* for struct tms */
#endif
#include <limits.h>				/* for CLK_TCK */

#define RUSAGE_SELF		0
#define RUSAGE_CHILDREN (-1)

struct rusage
{
	struct timeval ru_utime;	/* user time used */
	struct timeval ru_stime;	/* system time used */
};

extern int	getrusage(int who, struct rusage * rusage);

#endif   /* RUSAGESTUB_H */
