/*-------------------------------------------------------------------------
 *
 * dynloader.c
 *	  dynamic loader for HP-UX using the shared library mechanism
 *
 * Portions Copyright (c) 1996-2003, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  $PostgreSQL: pgsql-server/src/backend/port/dynloader/hpux.c,v 1.23 2003/11/29 19:51:54 pgsql Exp $
 *
 *	NOTES
 *		all functions are defined here -- it's impossible to trace the
 *		shl_* routines from the bundled HP-UX debugger.
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

/* System includes */
#include <a.out.h>

#include "dl.h"
#include "dynloader.h"
#include "utils/dynamic_loader.h"

void *
pg_dlopen(char *filename)
{
	/*
	 * Use BIND_IMMEDIATE so that undefined symbols cause a failure return
	 * from shl_load(), rather than an abort() later on when we attempt to
	 * call the library!
	 */
	shl_t		handle = shl_load(filename,
							BIND_IMMEDIATE | BIND_VERBOSE | DYNAMIC_PATH,
								  0L);

	return (void *) handle;
}

PGFunction
pg_dlsym(void *handle, char *funcname)
{
	PGFunction	f;

	if (shl_findsym((shl_t *) & handle, funcname, TYPE_PROCEDURE, &f) == -1)
		f = (PGFunction) NULL;
	return f;
}

void
pg_dlclose(void *handle)
{
	shl_unload((shl_t) handle);
}

char *
pg_dlerror(void)
{
	static char errmsg[] = "shl_load failed";

	if (errno)
		return strerror(errno);

	return errmsg;
}
