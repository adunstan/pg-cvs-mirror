/* $PostgreSQL: pgsql/src/bin/psql/mbprint.h,v 1.7 2003/11/29 22:40:49 pgsql Exp $ */
#ifndef MBPRINT_H
#define MBPRINT_H

#include "mb/pg_wchar.h"

extern char *mbvalidate(char *pwcs, int encoding);

extern int	pg_wcswidth(const char *pwcs, size_t len, int encoding);

#endif   /* MBPRINT_H */
