/*-------------------------------------------------------------------------
 *
 * pg_version.c--
 *    
 *
 * Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *    $Header: /home/cvsmirror/pg/pgsql/src/bin/pg_version/Attic/pg_version.c,v 1.2 1996/07/31 02:19:23 scrappy Exp $
 *
 *-------------------------------------------------------------------------
 */
#include <stdio.h>

int Noversion = 0;
char *DataDir = (char *) NULL;

int
main(int argc, char **argv)
{
    if (argc < 2) {
	fprintf(stderr, "pg_version: missing argument\n");
	exit(1);
    }
    SetPgVersion(argv[1]);
    exit(0);
}

elog() {}

GetDataHome()
{
	return(0);
}
