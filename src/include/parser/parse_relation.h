/*-------------------------------------------------------------------------
 *
 * parse_relation.h
 *	  prototypes for parse_relation.c.
 *
 *
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/parser/parse_relation.h,v 1.58 2008/09/01 20:42:45 tgl Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef PARSE_RELATION_H
#define PARSE_RELATION_H

#include "parser/parse_node.h"

extern bool add_missing_from;

extern RangeTblEntry *refnameRangeTblEntry(ParseState *pstate,
					 const char *schemaname,
					 const char *refname,
					 int location,
					 int *sublevels_up);
extern void checkNameSpaceConflicts(ParseState *pstate, List *namespace1,
						List *namespace2);
extern int RTERangeTablePosn(ParseState *pstate,
				  RangeTblEntry *rte,
				  int *sublevels_up);
extern RangeTblEntry *GetRTEByRangeTablePosn(ParseState *pstate,
					   int varno,
					   int sublevels_up);
extern CommonTableExpr *GetCTEForRTE(ParseState *pstate, RangeTblEntry *rte);
extern Node *scanRTEForColumn(ParseState *pstate, RangeTblEntry *rte,
				 char *colname, int location);
extern Node *colNameToVar(ParseState *pstate, char *colname, bool localonly,
			 int location);
extern Node *qualifiedNameToVar(ParseState *pstate,
				   char *schemaname,
				   char *refname,
				   char *colname,
				   bool implicitRTEOK,
				   int location);
extern Relation parserOpenTable(ParseState *pstate, const RangeVar *relation,
								int lockmode);
extern RangeTblEntry *addRangeTableEntry(ParseState *pstate,
				   RangeVar *relation,
				   Alias *alias,
				   bool inh,
				   bool inFromCl);
extern RangeTblEntry *addRangeTableEntryForRelation(ParseState *pstate,
							  Relation rel,
							  Alias *alias,
							  bool inh,
							  bool inFromCl);
extern RangeTblEntry *addRangeTableEntryForSubquery(ParseState *pstate,
							  Query *subquery,
							  Alias *alias,
							  bool inFromCl);
extern RangeTblEntry *addRangeTableEntryForFunction(ParseState *pstate,
							  char *funcname,
							  Node *funcexpr,
							  RangeFunction *rangefunc,
							  bool inFromCl);
extern RangeTblEntry *addRangeTableEntryForValues(ParseState *pstate,
							List *exprs,
							Alias *alias,
							bool inFromCl);
extern RangeTblEntry *addRangeTableEntryForJoin(ParseState *pstate,
						  List *colnames,
						  JoinType jointype,
						  List *aliasvars,
						  Alias *alias,
						  bool inFromCl);
extern RangeTblEntry *addRangeTableEntryForCTE(ParseState *pstate,
						 CommonTableExpr *cte,
						 Index levelsup,
						 Alias *alias,
						 bool inFromCl);
extern void addRTEtoQuery(ParseState *pstate, RangeTblEntry *rte,
			  bool addToJoinList,
			  bool addToRelNameSpace, bool addToVarNameSpace);
extern RangeTblEntry *addImplicitRTE(ParseState *pstate, RangeVar *relation);
extern void expandRTE(RangeTblEntry *rte, int rtindex, int sublevels_up,
		  int location, bool include_dropped,
		  List **colnames, List **colvars);
extern List *expandRelAttrs(ParseState *pstate, RangeTblEntry *rte,
			   int rtindex, int sublevels_up, int location);
extern int	attnameAttNum(Relation rd, const char *attname, bool sysColOK);
extern Name attnumAttName(Relation rd, int attid);
extern Oid	attnumTypeId(Relation rd, int attid);

#endif   /* PARSE_RELATION_H */
