/*-------------------------------------------------------------------------
 *
 * pg_foreign_data_wrapper.h
 *	  definition of the system "foreign-data wrapper" relation (pg_foreign_data_wrapper)
 *	  along with the relation's initial contents.
 *
 *
 * Portions Copyright (c) 1996-2009, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/catalog/pg_foreign_data_wrapper.h,v 1.1 2008/12/19 16:25:18 petere Exp $
 *
 * NOTES
 *	  the genbki.sh script reads this file and generates .bki
 *	  information from the DATA() statements.
 *
 *-------------------------------------------------------------------------
 */
#ifndef PG_FOREIGN_DATA_WRAPPER_H
#define PG_FOREIGN_DATA_WRAPPER_H

#include "catalog/genbki.h"

/* ----------------
 *		pg_foreign_data_wrapper definition.  cpp turns this into
 *		typedef struct FormData_pg_foreign_data_wrapper
 * ----------------
 */
#define ForeignDataWrapperRelationId	2328

CATALOG(pg_foreign_data_wrapper,2328)
{
	NameData	fdwname;		/* foreign-data wrapper name */
	Oid			fdwowner;		/* FDW owner */

	/* VARIABLE LENGTH FIELDS start here. */

	text		fdwlibrary;		/* FDW shared library location */
	aclitem		fdwacl[1];		/* access permissions */
	text		fdwoptions[1];	/* FDW options */
} FormData_pg_foreign_data_wrapper;

/* ----------------
 *		Form_pg_fdw corresponds to a pointer to a tuple with
 *		the format of pg_fdw relation.
 * ----------------
 */
typedef FormData_pg_foreign_data_wrapper *Form_pg_foreign_data_wrapper;

/* ----------------
 *		compiler constants for pg_fdw
 * ----------------
 */

#define Natts_pg_foreign_data_wrapper				5
#define Anum_pg_foreign_data_wrapper_fdwname		1
#define Anum_pg_foreign_data_wrapper_fdwowner		2
#define Anum_pg_foreign_data_wrapper_fdwlibrary		3
#define Anum_pg_foreign_data_wrapper_fdwacl			4
#define Anum_pg_foreign_data_wrapper_fdwoptions		5

#endif   /* PG_FOREIGN_DATA_WRAPPER_H */
