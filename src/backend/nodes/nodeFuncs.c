/*-------------------------------------------------------------------------
 *
 * nodeFuncs.c
 *	  All node routines more complicated than simple access/modification
 *
 * Portions Copyright (c) 1996-2002, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  $Header: /home/cvsmirror/pg/pgsql/src/backend/nodes/nodeFuncs.c,v 1.21 2002/12/13 19:45:56 tgl Exp $
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include "nodes/nodeFuncs.h"
#include "utils/lsyscache.h"

static bool var_is_inner(Var *var);


/*
 * single_node -
 *	  Returns t if node corresponds to a single-noded expression
 */
bool
single_node(Node *node)
{
	if (IsA(node, Const) ||
		IsA(node, Var) ||
		IsA(node, Param))
		return true;
	else
		return false;
}

/*****************************************************************************
 *		VAR nodes
 *****************************************************************************/

/*
 *		var_is_outer
 *		var_is_inner
 *		var_is_mat
 *		var_is_rel
 *
 *		Returns t iff the var node corresponds to (respectively):
 *		the outer relation in a join
 *		the inner relation of a join
 *		a materialized relation
 *		a base relation (i.e., not an attribute reference, a variable from
 *				some lower join level, or a sort result)
 *		var node is an array reference
 *
 */
bool
var_is_outer(Var *var)
{
	return (bool) (var->varno == OUTER);
}

static bool
var_is_inner(Var *var)
{
	return (bool) (var->varno == INNER);
}

bool
var_is_rel(Var *var)
{
	return (bool)
		!(var_is_inner(var) || var_is_outer(var));
}

/*****************************************************************************
 *		OPER nodes
 *****************************************************************************/

/*
 * set_opfuncid -
 *
 *		Set the opfuncid (procedure OID) in an OpExpr node,
 *		if it hasn't been set already.
 */
void
set_opfuncid(OpExpr *opexpr)
{
	if (opexpr->opfuncid == InvalidOid)
		opexpr->opfuncid = get_opcode(opexpr->opno);
}
