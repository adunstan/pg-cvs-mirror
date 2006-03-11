/* $PostgreSQL: pgsql/src/backend/utils/misc/guc.c,v 1.314 2006/03/07 02:54:23 momjian Exp $ */

#include "stdlib.h"

static void
Finish(char *msg)
{
	fprintf(stderr, "Error in statement '%s':\n", msg);
	sqlprint();

	/* finish transaction */
	exec sql	rollback;

	/* and remove test table */
	exec sql drop table meskes;
	exec sql	commit;

	exec sql	disconnect;

	exit(-1);
}

static void
warn(void)
{
	fprintf(stderr, "Warning: At least one column was truncated\n");
}

exec sql whenever sqlerror
do
				Finish(msg);
exec sql whenever sqlwarning
do
				warn();
