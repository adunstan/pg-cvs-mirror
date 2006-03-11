/* $PostgreSQL: pgsql/src/backend/utils/misc/guc.c,v 1.314 2006/03/07 02:54:23 momjian Exp $ */

#define __darwin__	1

#if HAVE_DECL_F_FULLFSYNC		/* not present before OS X 10.3 */
#define HAVE_FSYNC_WRITETHROUGH
#endif
