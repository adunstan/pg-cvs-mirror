/*-------------------------------------------------------------------------
 *
 * strdup.c
 *	  copies a null-terminated string.
 *
 * Portions Copyright (c) 1996-2004, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  $PostgreSQL: pgsql-server/src/port/strdup.c,v 1.4 2003/11/29 19:52:13 pgsql Exp $
 *
 *-------------------------------------------------------------------------
 */

#include <string.h>
#include <stdlib.h>
#include "strdup.h"

char *
strdup(char const * string)
{
	char	   *nstr;

	nstr = strcpy((char *) malloc(strlen(string) + 1), string);
	return nstr;
}
