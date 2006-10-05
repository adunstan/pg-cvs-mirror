/* $PostgreSQL: pgsql/src/include/port/irix.h,v 1.3 2006/03/11 04:38:38 momjian Exp $ */

/*
 * IRIX 6.5.26f and 6.5.22f (at least) have a strtod() that accepts
 * "infinity", but leaves endptr pointing to "inity".
 */
#define HAVE_BUGGY_IRIX_STRTOD
