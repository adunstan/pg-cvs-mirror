/*-------------------------------------------------------------------------
 *
 * srandom.c
 *	  srandom() wrapper
 *
 * Portions Copyright (c) 1996-2004, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  $PostgreSQL: pgsql-server/src/port/srandom.c,v 1.3 2003/11/29 19:52:13 pgsql Exp $
 *
 *-------------------------------------------------------------------------
 */

#include "c.h"

#include <stdlib.h>
#include <math.h>
#include <errno.h>

void
srandom(unsigned int seed)
{
	srand48((long int) seed);
}
