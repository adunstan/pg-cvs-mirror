/*-------------------------------------------------------------------------
 *
 * rewriteHandler.h
 *		External interface to query rewriter.
 *
 *
 * Portions Copyright (c) 1996-2004, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql-server/src/include/rewrite/rewriteHandler.h,v 1.22 2003/11/29 22:41:11 pgsql Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef REWRITEHANDLER_H
#define REWRITEHANDLER_H

#include "nodes/parsenodes.h"

extern List *QueryRewrite(Query *parsetree);
extern Node *build_column_default(Relation rel, int attrno);

#endif   /* REWRITEHANDLER_H */
