/*-------------------------------------------------------------------------
 *
 * rewriteManip.h
 *
 *
 *
 * Copyright (c) 1994, Regents of the University of California
 *
 * $Id: rewriteManip.h,v 1.18 1999/10/01 04:08:15 tgl Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef REWRITEMANIP_H
#define REWRITEMANIP_H

#include "rewrite/rewriteHandler.h"

/* RewriteManip.c */
extern void OffsetVarNodes(Node *node, int offset, int sublevels_up);
extern void ChangeVarNodes(Node *node, int old_varno, int new_varno,
						   int sublevels_up);
extern void AddQual(Query *parsetree, Node *qual);
extern void AddHavingQual(Query *parsetree, Node *havingQual);
extern void AddNotQual(Query *parsetree, Node *qual);
extern void AddGroupClause(Query *parsetree, List *group_by, List *tlist);

extern void FixNew(RewriteInfo *info, Query *parsetree);

extern void HandleRIRAttributeRule(Query *parsetree, List *rtable,
								   List *targetlist, int rt_index,
								   int attr_num, int *modified, int *badsql);

#endif	 /* REWRITEMANIP_H */
