/*-------------------------------------------------------------------------
 *
 * format.c
 *	  a wrapper around code that does what vsprintf does.
 *
 * Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  $Header: /home/cvsmirror/pg/pgsql/src/backend/utils/error/Attic/format.c,v 1.10 1999/02/13 23:19:49 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdarg.h>
#include "postgres.h"

#define FormMaxSize		1024
#define FormMinSize		(FormMaxSize / 8)

static char FormBuf[FormMaxSize];


/* ----------------
 *		form
 * ----------------
 */
char *
form(const char *fmt,...)
{
	va_list		args;
	va_start(args, fmt);
	vsnprintf(FormBuf, FormMaxSize - 1, fmt, args);
	va_end(args);
	return FormBuf;
}
