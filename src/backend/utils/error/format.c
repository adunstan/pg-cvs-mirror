/*-------------------------------------------------------------------------
 *
 * format.c
 *	  a wrapper around code that does what vsprintf does.
 *
 * Portions Copyright (c) 1996-2002, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  $Header: /home/cvsmirror/pg/pgsql/src/backend/utils/error/Attic/format.c,v 1.17 2002/06/20 20:29:39 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#define FormMaxSize		1024
#define FormMinSize		(FormMaxSize / 8)

static char FormBuf[FormMaxSize];


/* ----------------
 *		vararg_format
 * ----------------
 */
char *
vararg_format(const char *fmt,...)
{
	va_list		args;

	va_start(args, fmt);
	vsnprintf(FormBuf, FormMaxSize - 1, fmt, args);
	va_end(args);
	return FormBuf;
}
