/*
 * psql - the PostgreSQL interactive terminal
 *
 * Copyright (c) 2000-2003, PostgreSQL Global Development Group
 *
 * $Header: /home/cvsmirror/pg/pgsql/src/bin/psql/common.h,v 1.28 2003/08/08 04:52:21 momjian Exp $
 */
#ifndef COMMON_H
#define COMMON_H

#include "postgres_fe.h"
#include <signal.h>
#include "pqsignal.h"
#include "libpq-fe.h"

extern char *xstrdup(const char *string);

extern bool setQFout(const char *fname);

extern void
psql_error(const char *fmt,...)
/* This lets gcc check the format string for consistency. */
__attribute__((format(printf, 1, 2)));

extern void NoticeProcessor(void *arg, const char *message);

extern volatile bool cancel_pressed;

extern void ResetCancelConn(void);

#ifndef WIN32
extern void handle_sigint(SIGNAL_ARGS);
#endif   /* not WIN32 */

extern PGresult *PSQLexec(const char *query, bool start_xact);

extern bool SendQuery(const char *query);

extern bool is_superuser(void);

/* Parse a numeric character code from the string pointed at by *buf, e.g.
 * one written as 0x0c (hexadecimal) or 015 (octal); advance *buf to the last
 * character of the numeric character code.
 */
extern char parse_char(char **buf);

/* Used for all Win32 popen/pclose calls */
#ifdef WIN32
#define popen(x,y) _popen(x,y)
#define pclose(x) _pclose(x)
#endif

#endif   /* COMMON_H */
