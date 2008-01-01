/*-------------------------------------------------------------------------
 *
 * postgres_fe.h
 *	  Primary include file for PostgreSQL client-side .c files
 *
 * This should be the first file included by PostgreSQL client libraries and
 * application programs --- but not by backend modules, which should include
 * postgres.h.
 *
 *
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
 * Portions Copyright (c) 1995, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/postgres_fe.h,v 1.13 2007/09/27 19:53:44 tgl Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef POSTGRES_FE_H
#define POSTGRES_FE_H

#ifndef FRONTEND
#define FRONTEND 1
#endif

#include "c.h"

#endif   /* POSTGRES_FE_H */
