/*-------------------------------------------------------------------------
 *
 * parse_target.h
 *	  handle target lists
 *
 *
 * Portions Copyright (c) 1996-2006, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/parser/parse_target.h,v 1.39 2006/03/23 00:19:30 tgl Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef PARSE_TARGET_H
#define PARSE_TARGET_H

#include "parser/parse_node.h"


extern List *transformTargetList(ParseState *pstate, List *targetlist);
extern void markTargetListOrigins(ParseState *pstate, List *targetlist);
extern TargetEntry *transformTargetEntry(ParseState *pstate,
					 Node *node, Node *expr,
					 char *colname, bool resjunk);
extern List *ExpandColumnRefStar(ParseState *pstate, ColumnRef *cref,
								 bool targetlist);
extern List *ExpandIndirectionStar(ParseState *pstate, A_Indirection *ind,
								   bool targetlist);
extern void updateTargetListEntry(ParseState *pstate, TargetEntry *tle,
					  char *colname, int attrno,
					  List *indirection,
					  int location);
extern List *checkInsertTargets(ParseState *pstate, List *cols,
				   List **attrnos);
extern TupleDesc expandRecordVariable(ParseState *pstate, Var *var,
					 int levelsup);
extern char *FigureColname(Node *node);

#endif   /* PARSE_TARGET_H */
