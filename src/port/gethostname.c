/*-------------------------------------------------------------------------
 *
 * gethostname.c
 *	  gethostname using uname
 *
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * IDENTIFICATION
 *	  $PostgreSQL: pgsql/src/port/gethostname.c,v 1.9 2007/01/05 22:20:02 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */

#include "c.h"

#include <sys/utsname.h>

int
gethostname(char *name, int namelen)
{
	static struct utsname mname;
	static int	called = 0;

	if (!called)
	{
		called++;
		uname(&mname);
	}
	strncpy(name, mname.nodename, (SYS_NMLN < namelen ? SYS_NMLN : namelen));

	return 0;
}
