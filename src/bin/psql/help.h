/*
 * psql - the PostgreSQL interactive terminal
 *
 * Copyright (c) 2000-2004, PostgreSQL Global Development Group
 *
 * $PostgreSQL: pgsql-server/src/bin/psql/help.h,v 1.14 2003/11/29 19:52:06 pgsql Exp $
 */
#ifndef HELP_H
#define HELP_H

void		usage(void);

void		slashUsage(unsigned short int pager);

void		helpSQL(const char *topic, unsigned short int pager);

void		print_copyright(void);

#endif
