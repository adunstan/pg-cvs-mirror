/*
 * psql - the PostgreSQL interactive terminal
 *
 * Copyright 2000 by PostgreSQL Global Development Group
 *
 * $Header: /home/cvsmirror/pg/pgsql/src/bin/psql/common.h,v 1.11 2000/08/27 19:00:36 petere Exp $
 */
#ifndef COMMON_H
#define COMMON_H

#include "postgres.h"
#include <signal.h>
#include "pqsignal.h"
#include "libpq-fe.h"

char	   *xstrdup(const char *string);

bool		setQFout(const char *fname);

#ifndef __GNUC__
void		psql_error(const char *fmt,...);

#else
/* This checks the format string for consistency. */
void		psql_error(const char *fmt,...) __attribute__((format(printf, 1, 2)));

#endif

void		NoticeProcessor(void *arg, const char *message);

char	   *simple_prompt(const char *prompt, int maxlen, bool echo);

extern volatile bool cancel_pressed;
extern PGconn *cancelConn;

#ifndef WIN32
void		handle_sigint(int signum);

#endif	 /* not WIN32 */

PGresult   *PSQLexec(const char *query);

bool		SendQuery(const char *query);

#endif	 /* COMMON_H */
