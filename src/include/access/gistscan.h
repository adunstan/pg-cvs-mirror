/*-------------------------------------------------------------------------
 *
 * gistscan.h
 *	  routines defined in access/gist/gistscan.c
 *
 *
 * Portions Copyright (c) 1996-2005, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/access/gistscan.h,v 1.25 2004/08/29 04:13:03 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef GISTSCAN_H
#define GISTSCAN_H

#include "access/relscan.h"

extern Datum gistbeginscan(PG_FUNCTION_ARGS);
extern Datum gistrescan(PG_FUNCTION_ARGS);
extern Datum gistmarkpos(PG_FUNCTION_ARGS);
extern Datum gistrestrpos(PG_FUNCTION_ARGS);
extern Datum gistendscan(PG_FUNCTION_ARGS);
extern void gistadjscans(Relation r, int op, BlockNumber blkno, OffsetNumber offnum);
extern void ReleaseResources_gist(void);

#endif   /* GISTSCAN_H */
