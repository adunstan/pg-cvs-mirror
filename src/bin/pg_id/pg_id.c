/*-------------------------------------------------------------------------
 *
 * pg_id.c
 *	  Print the user ID for the login name passed as argument,
 *	  or the real user ID of the caller if no argument.  If the
 *	  login name doesn't exist, print "NOUSER" and exit 1.
 *
 * Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  $Header: /home/cvsmirror/pg/pgsql/src/bin/pg_id/Attic/pg_id.c,v 1.9 1999/07/19 02:27:09 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include "postgres.h"
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

int
main(int argc, char **argv)
{
	struct passwd *pw;
	int			ch;
	extern int	optind;

	while ((ch = getopt(argc, argv, "")) != EOF)
		switch (ch)
		{
			case '?':
			default:
				fprintf(stderr, "usage: pg_id [login]\n");
				exit(1);
		}
	argc -= optind;
	argv += optind;

	if (argc > 0)
	{
		if (argc > 1)
		{
			fprintf(stderr, "usage: pg_id [login]\n");
			exit(1);
		}
		if ((pw = getpwnam(argv[0])) == NULL)
		{
			printf("NOUSER\n");
			exit(1);
		}
		printf("%ld\n", (long) pw->pw_uid);
	}
	else
		printf("%ld\n", (long) getuid());

	exit(0);
}
