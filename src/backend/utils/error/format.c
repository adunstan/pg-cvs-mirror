/*-------------------------------------------------------------------------
 *
 * format.c--
 *    a wrapper around code that does what vsprintf does.
 *
 * Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *    $Header: /home/cvsmirror/pg/pgsql/src/backend/utils/error/Attic/format.c,v 1.3 1997/08/12 22:54:47 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdarg.h>
#include "c.h"

#define FormMaxSize	1024
#define FormMinSize	(FormMaxSize / 8)

static	char	FormBuf[FormMaxSize];


/* ----------------
 *	form
 * ----------------
 */
char *
form(const char *fmt, ... )
{
    va_list	args;

    va_start(args, fmt);
    
    vsprintf(FormBuf, fmt, args);

    va_end(args);
    
    return (FormBuf);
}
