/*-------------------------------------------------------------------------
 *
 * nodeGroup.c
 *	  Routines to handle group nodes (used for queries with GROUP BY clause).
 *
 * Portions Copyright (c) 1996-2005, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * DESCRIPTION
 *	  The Group node is designed for handling queries with a GROUP BY clause.
 *	  Its outer plan must deliver tuples that are sorted in the order
 *	  specified by the grouping columns (ie. tuples from the same group are
 *	  consecutive).  That way, we just have to compare adjacent tuples to
 *	  locate group boundaries.
 *
 * IDENTIFICATION
 *	  $PostgreSQL: pgsql/src/backend/executor/nodeGroup.c,v 1.59 2004/12/31 21:59:45 pgsql Exp $
 *
 *-------------------------------------------------------------------------
 */

#include "postgres.h"

#include "access/heapam.h"
#include "executor/executor.h"
#include "executor/nodeGroup.h"


/*
 *	 ExecGroup -
 *
 *		Return one tuple for each group of matching input tuples.
 */
TupleTableSlot *
ExecGroup(GroupState *node)
{
	ExprContext *econtext;
	TupleDesc	tupdesc;
	int			numCols;
	AttrNumber *grpColIdx;
	HeapTuple	outerTuple;
	HeapTuple	firsttuple;
	TupleTableSlot *outerslot;

	/*
	 * get state info from node
	 */
	if (node->grp_done)
		return NULL;
	econtext = node->ss.ps.ps_ExprContext;
	tupdesc = ExecGetScanType(&node->ss);
	numCols = ((Group *) node->ss.ps.plan)->numCols;
	grpColIdx = ((Group *) node->ss.ps.plan)->grpColIdx;

	/*
	 * We need not call ResetExprContext here because execTuplesMatch will
	 * reset the per-tuple memory context once per input tuple.
	 */

	/*
	 * If first time through, acquire first input tuple and determine
	 * whether to return it or not.
	 */
	firsttuple = node->grp_firstTuple;
	if (firsttuple == NULL)
	{
		outerslot = ExecProcNode(outerPlanState(node));
		if (TupIsNull(outerslot))
		{
			/* empty input, so return nothing */
			node->grp_done = TRUE;
			return NULL;
		}
		node->grp_firstTuple = firsttuple = heap_copytuple(outerslot->val);
		/* Set up tuple as input for qual test and projection */
		ExecStoreTuple(firsttuple,
					   node->ss.ss_ScanTupleSlot,
					   InvalidBuffer,
					   false);
		econtext->ecxt_scantuple = node->ss.ss_ScanTupleSlot;
		/*
		 * Check the qual (HAVING clause); if the group does not match,
		 * ignore it and fall into scan loop.
		 */
		if (ExecQual(node->ss.ps.qual, econtext, false))
		{
			/*
			 * Form and return a projection tuple using the first input
			 * tuple.
			 */
			return ExecProject(node->ss.ps.ps_ProjInfo, NULL);
		}
	}

	/*
	 * This loop iterates once per input tuple group.  At the head of the
	 * loop, we have finished processing the first tuple of the group and
	 * now need to scan over all the other group members.
	 */
	for (;;)
	{
		/*
		 * Scan over all remaining tuples that belong to this group
		 */
		for (;;)
		{
			outerslot = ExecProcNode(outerPlanState(node));
			if (TupIsNull(outerslot))
			{
				/* no more groups, so we're done */
				node->grp_done = TRUE;
				return NULL;
			}
			outerTuple = outerslot->val;

			/*
			 * Compare with first tuple and see if this tuple is of the same
			 * group.  If so, ignore it and keep scanning.
			 */
			if (!execTuplesMatch(firsttuple, outerTuple,
								 tupdesc,
								 numCols, grpColIdx,
								 node->eqfunctions,
								 econtext->ecxt_per_tuple_memory))
				break;
		}
		/*
		 * We have the first tuple of the next input group.  See if we
		 * want to return it.
		 */
		heap_freetuple(firsttuple);
		node->grp_firstTuple = firsttuple = heap_copytuple(outerTuple);
		/* Set up tuple as input for qual test and projection */
		ExecStoreTuple(firsttuple,
					   node->ss.ss_ScanTupleSlot,
					   InvalidBuffer,
					   false);
		econtext->ecxt_scantuple = node->ss.ss_ScanTupleSlot;
		/*
		 * Check the qual (HAVING clause); if the group does not match,
		 * ignore it and loop back to scan the rest of the group.
		 */
		if (ExecQual(node->ss.ps.qual, econtext, false))
		{
			/*
			 * Form and return a projection tuple using the first input
			 * tuple.
			 */
			return ExecProject(node->ss.ps.ps_ProjInfo, NULL);
		}
	}

	/* NOTREACHED */
	return NULL;
}

/* -----------------
 * ExecInitGroup
 *
 *	Creates the run-time information for the group node produced by the
 *	planner and initializes its outer subtree
 * -----------------
 */
GroupState *
ExecInitGroup(Group *node, EState *estate)
{
	GroupState *grpstate;

	/*
	 * create state structure
	 */
	grpstate = makeNode(GroupState);
	grpstate->ss.ps.plan = (Plan *) node;
	grpstate->ss.ps.state = estate;
	grpstate->grp_firstTuple = NULL;
	grpstate->grp_done = FALSE;

	/*
	 * create expression context
	 */
	ExecAssignExprContext(estate, &grpstate->ss.ps);

#define GROUP_NSLOTS 2

	/*
	 * tuple table initialization
	 */
	ExecInitScanTupleSlot(estate, &grpstate->ss);
	ExecInitResultTupleSlot(estate, &grpstate->ss.ps);

	/*
	 * initialize child expressions
	 */
	grpstate->ss.ps.targetlist = (List *)
		ExecInitExpr((Expr *) node->plan.targetlist,
					 (PlanState *) grpstate);
	grpstate->ss.ps.qual = (List *)
		ExecInitExpr((Expr *) node->plan.qual,
					 (PlanState *) grpstate);

	/*
	 * initialize child nodes
	 */
	outerPlanState(grpstate) = ExecInitNode(outerPlan(node), estate);

	/*
	 * initialize tuple type.
	 */
	ExecAssignScanTypeFromOuterPlan(&grpstate->ss);

	/*
	 * Initialize result tuple type and projection info.
	 */
	ExecAssignResultTypeFromTL(&grpstate->ss.ps);
	ExecAssignProjectionInfo(&grpstate->ss.ps);

	/*
	 * Precompute fmgr lookup data for inner loop
	 */
	grpstate->eqfunctions =
		execTuplesMatchPrepare(ExecGetScanType(&grpstate->ss),
							   node->numCols,
							   node->grpColIdx);

	return grpstate;
}

int
ExecCountSlotsGroup(Group *node)
{
	return ExecCountSlotsNode(outerPlan(node)) + GROUP_NSLOTS;
}

/* ------------------------
 *		ExecEndGroup(node)
 *
 * -----------------------
 */
void
ExecEndGroup(GroupState *node)
{
	PlanState  *outerPlan;

	ExecFreeExprContext(&node->ss.ps);

	/* clean up tuple table */
	ExecClearTuple(node->ss.ss_ScanTupleSlot);

	outerPlan = outerPlanState(node);
	ExecEndNode(outerPlan);
}

void
ExecReScanGroup(GroupState *node, ExprContext *exprCtxt)
{
	node->grp_done = FALSE;
	if (node->grp_firstTuple != NULL)
	{
		heap_freetuple(node->grp_firstTuple);
		node->grp_firstTuple = NULL;
	}

	if (((PlanState *) node)->lefttree &&
		((PlanState *) node)->lefttree->chgParam == NULL)
		ExecReScan(((PlanState *) node)->lefttree, exprCtxt);
}
