/*-------------------------------------------------------------------------
 *
 * dynloader.c
 *	  This dynamic loader uses Andrew Yu's libdl-1.0 package for Ultrix 4.x.
 *
 * Portions Copyright (c) 1996-2001, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  $Header: /home/cvsmirror/pg/pgsql/src/backend/port/dynloader/ultrix4.c,v 1.13 2001/02/10 02:31:26 tgl Exp $
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include "dl.h"
#include "utils/dynamic_loader.h"

extern char pg_pathname[];

void *
pg_dlopen(char *filename)
{
	static int	dl_initialized = 0;
	void	   *handle;

	/*
	 * initializes the dynamic loader with the executable's pathname.
	 * (only needs to do this the first time pg_dlopen is called.)
	 */
	if (!dl_initialized)
	{
		if (!dl_init(pg_pathname))
			return NULL;

		/*
		 * if there are undefined symbols, we want dl to search from the
		 * following libraries also.
		 */
		dl_setLibraries("/usr/lib/libm_G0.a:/usr/lib/libc_G0.a");
		dl_initialized = 1;
	}

	/*
	 * open the file. We do the symbol resolution right away so that we
	 * will know if there are undefined symbols. (This is in fact the same
	 * semantics as "ld -A". ie. you cannot have undefined symbols.
	 */
	if ((handle = dl_open(filename, DL_NOW)) == NULL)
	{
		int			count;
		char	  **list = dl_undefinedSymbols(&count);

		/* list the undefined symbols, if any */
		if (count)
		{
			elog(NOTICE, "dl: Undefined:");
			while (*list)
			{
				elog(NOTICE, "  %s", *list);
				list++;
			}
		}
	}

	return (void *) handle;
}
