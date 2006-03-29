/*
 * xlogutils.h
 *
 * PostgreSQL transaction log manager utility routines
 *
 * Portions Copyright (c) 1996-2006, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/access/xlogutils.h,v 1.19 2006/03/05 15:58:54 momjian Exp $
 */
#ifndef XLOG_UTILS_H
#define XLOG_UTILS_H

#include "storage/buf.h"
#include "utils/rel.h"


extern void XLogInitRelationCache(void);
extern void XLogCloseRelationCache(void);

extern Relation XLogOpenRelation(RelFileNode rnode);
extern void XLogDropRelation(RelFileNode rnode);
extern void XLogDropDatabase(Oid dbid);

extern Buffer XLogReadBuffer(Relation reln, BlockNumber blkno, bool init);

#endif
