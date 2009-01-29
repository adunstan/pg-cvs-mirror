/*-------------------------------------------------------------------------
 *
 *	  ASCII and MULE_INTERNAL
 *
 * Portions Copyright (c) 1996-2003, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * IDENTIFICATION
 *	  $Header: /home/cvsmirror/pg/pgsql/src/backend/utils/mb/conversion_procs/ascii_and_mic/ascii_and_mic.c,v 1.6.4.1 2009/01/29 19:25:12 tgl Exp $
 *
 *-------------------------------------------------------------------------
 */

#include "postgres.h"
#include "fmgr.h"
#include "mb/pg_wchar.h"

PG_FUNCTION_INFO_V1(ascii_to_mic);
PG_FUNCTION_INFO_V1(mic_to_ascii);

extern Datum ascii_to_mic(PG_FUNCTION_ARGS);
extern Datum mic_to_ascii(PG_FUNCTION_ARGS);

/* ----------
 * conv_proc(
 *		INTEGER,	-- source encoding id
 *		INTEGER,	-- destination encoding id
 *		CSTRING,	-- source string (null terminated C string)
 *		CSTRING,	-- destination string (null terminated C string)
 *		INTEGER		-- source string length
 * ) returns VOID;
 * ----------
 */

Datum
ascii_to_mic(PG_FUNCTION_ARGS)
{
	unsigned char *src = PG_GETARG_CSTRING(2);
	unsigned char *dest = PG_GETARG_CSTRING(3);
	int			len = PG_GETARG_INT32(4);

	CHECK_ENCODING_CONVERSION_ARGS(PG_SQL_ASCII, PG_MULE_INTERNAL);

	pg_ascii2mic(src, dest, len);

	PG_RETURN_VOID();
}

Datum
mic_to_ascii(PG_FUNCTION_ARGS)
{
	unsigned char *src = PG_GETARG_CSTRING(2);
	unsigned char *dest = PG_GETARG_CSTRING(3);
	int			len = PG_GETARG_INT32(4);

	CHECK_ENCODING_CONVERSION_ARGS(PG_MULE_INTERNAL, PG_SQL_ASCII);

	pg_mic2ascii(src, dest, len);

	PG_RETURN_VOID();
}
