/*-------------------------------------------------------------------------
 *
 * parse_exer.h
 *
 *
 *
 * Copyright (c) 1994, Regents of the University of California
 *
 * $Id: parse_expr.h,v 1.6 1998/01/20 05:04:47 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef PARSE_EXPR_H
#define PARSE_EXPR_H

#include <nodes/nodes.h>
#include <nodes/parsenodes.h>
#include <nodes/primnodes.h>
#include <parser/parse_node.h>

extern Node *transformExpr(ParseState *pstate, Node *expr, int precedence);
extern Node *transformIdent(ParseState *pstate, Node *expr, int precedence);
extern Oid exprType(Node *expr);
extern Node *parser_typecast2(Node *expr, Oid exprType, Type tp, int attypmod);

#endif							/* PARSE_EXPR_H */

