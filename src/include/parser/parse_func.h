/*-------------------------------------------------------------------------
 *
 * parse_func.h
 *
 *
 *
 * Copyright (c) 1994, Regents of the University of California
 *
 * $Id: parse_func.h,v 1.20 1999/12/10 07:37:33 tgl Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef PARSER_FUNC_H
#define PARSER_FUNC_H

#include "parser/parse_node.h"


#define MAXFARGS 8				/* max # args to a c or postquel function */

/*
 *	This structure is used to explore the inheritance hierarchy above
 *	nodes in the type tree in order to disambiguate among polymorphic
 *	functions.
 */
typedef struct _InhPaths
{
	int			nsupers;		/* number of superclasses */
	Oid			self;			/* this class */
	Oid		   *supervec;		/* vector of superclasses */
} InhPaths;

/*
 *	This structure holds a list of possible functions or operators that
 *	agree with the known name and argument types of the function/operator.
 */
typedef struct _CandidateList
{
	Oid		   *args;
	struct _CandidateList *next;
}		   *CandidateList;

extern Node *ParseNestedFuncOrColumn(ParseState *pstate, Attr *attr,
						int *curr_resno, int precedence);
extern Node *ParseFuncOrColumn(ParseState *pstate,
							   char *funcname, List *fargs,
							   bool agg_star, bool agg_distinct,
							   int *curr_resno, int precedence);

extern List *setup_base_tlist(Oid typeid);

extern void func_error(char *caller, char *funcname,
					   int nargs, Oid *argtypes, char *msg);

#endif	 /* PARSE_FUNC_H */
