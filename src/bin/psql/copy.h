/*
 * psql - the PostgreSQL interactive terminal
 *
 * Copyright 2000 by PostgreSQL Global Development Group
 *
 * $Header: /home/cvsmirror/pg/pgsql/src/bin/psql/copy.h,v 1.11 2001/10/28 06:25:58 momjian Exp $
 */
#ifndef COPY_H
#define COPY_H

#include "libpq-fe.h"

extern bool copy_in_state;

/* handler for \copy */
bool		do_copy(const char *args);

/* lower level processors for copy in/out streams */

bool		handleCopyOut(PGconn *conn, FILE *copystream);
bool		handleCopyIn(PGconn *conn, FILE *copystream, const char *prompt);

#endif
