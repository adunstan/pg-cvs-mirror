/*-------------------------------------------------------------------------
 *
 * portalcmds.h
 *	  prototypes for portalcmds.c.
 *
 *
 * Portions Copyright (c) 1996-2007, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/commands/portalcmds.h,v 1.21 2007/02/20 17:32:17 tgl Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef PORTALCMDS_H
#define PORTALCMDS_H

#include "nodes/parsenodes.h"
#include "utils/portal.h"


extern void PerformCursorOpen(DeclareCursorStmt *stmt, ParamListInfo params,
							  const char *queryString, bool isTopLevel);

extern void PerformPortalFetch(FetchStmt *stmt, DestReceiver *dest,
				   char *completionTag);

extern void PerformPortalClose(const char *name);

extern void PortalCleanup(Portal portal);

extern void PersistHoldablePortal(Portal portal);

#endif   /* PORTALCMDS_H */
