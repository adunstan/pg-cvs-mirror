/*-------------------------------------------------------------------------
 *
 * pg_conversion_fn.h
 *	 prototypes for functions in catalog/pg_conversion.c
 *
 *
 * Portions Copyright (c) 1996-2010, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/catalog/pg_conversion_fn.h,v 1.5 2010/01/02 16:58:01 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef PG_CONVERSION_FN_H
#define PG_CONVERSION_FN_H

extern Oid ConversionCreate(const char *conname, Oid connamespace,
				 Oid conowner,
				 int32 conforencoding, int32 contoencoding,
				 Oid conproc, bool def);
extern void RemoveConversionById(Oid conversionOid);
extern Oid	FindDefaultConversion(Oid connamespace, int32 for_encoding, int32 to_encoding);

#endif   /* PG_CONVERSION_FN_H */
