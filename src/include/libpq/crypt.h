/*-------------------------------------------------------------------------
 *
 * crypt.h
 *	  Interface to libpq/crypt.c
 *
 * Portions Copyright (c) 1996-2006, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/libpq/crypt.h,v 1.34 2006/03/05 15:58:56 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef PG_CRYPT_H
#define PG_CRYPT_H

#include "libpq/libpq-be.h"

extern int md5_crypt_verify(const Port *port, const char *user,
				 char *client_pass);

#endif
