/*-------------------------------------------------------------------------
 *
 * nodeMaterial.c
 *	  Routines to handle materialization nodes.
 *
 * Portions Copyright (c) 1996-2000, PostgreSQL, Inc
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  $Header: /home/cvsmirror/pg/pgsql/src/backend/executor/nodeMaterial.c,v 1.30.2.2 2000/10/06 01:28:47 tgl Exp $
 *
 *-------------------------------------------------------------------------
 */
/*
 * INTERFACE ROUTINES
 *		ExecMaterial			- generate a temporary relation
 *		ExecInitMaterial		- initialize node and subnodes..
 *		ExecEndMaterial			- shutdown node and subnodes
 *
 */
#include "postgres.h"


#include "access/heapam.h"
#include "catalog/heap.h"
#include "executor/executor.h"
#include "executor/nodeMaterial.h"
#include "optimizer/internal.h"

/* ----------------------------------------------------------------
 *		ExecMaterial
 *
 *		The first time this is called, ExecMaterial retrieves tuples
 *		from this node's outer subplan and inserts them into a temporary
 *		relation.  After this is done, a flag is set indicating that
 *		the subplan has been materialized.	Once the relation is
 *		materialized, the first tuple is then returned.  Successive
 *		calls to ExecMaterial return successive tuples from the temp
 *		relation.
 *
 * ----------------------------------------------------------------
 */
TupleTableSlot *				/* result tuple from subplan */
ExecMaterial(Material *node)
{
	EState	   *estate;
	MaterialState *matstate;
	Plan	   *outerNode;
	ScanDirection dir;
	Relation	tempRelation;
	Relation	currentRelation;
	HeapScanDesc currentScanDesc;
	HeapTuple	heapTuple;
	TupleTableSlot *slot;

	/* ----------------
	 *	get state info from node
	 * ----------------
	 */
	matstate = node->matstate;
	estate = node->plan.state;
	dir = estate->es_direction;

	/* ----------------
	 *	the first time we call this, we retrieve all tuples
	 *	from the subplan into a temporary relation and then
	 *	we sort the relation.  Subsequent calls return tuples
	 *	from the temporary relation.
	 * ----------------
	 */

	if (matstate->mat_Flag == false)
	{
		TupleDesc	tupType;

		/* ----------------
		 *	get type information needed for ExecCreatR
		 * ----------------
		 */
		tupType = ExecGetScanType(&matstate->csstate);

		/* ----------------
		 *	ExecCreatR wants its second argument to be an object id of
		 *	a relation in the range table or a _NONAME_RELATION_ID
		 *	indicating that the relation is not in the range table.
		 *
		 *	In the second case ExecCreatR creates a temp relation.
		 *	(currently this is the only case we support -cim 10/16/89)
		 * ----------------
		 */
		/* ----------------
		 *	create the temporary relation
		 * ----------------
		 */
		tempRelation = ExecCreatR(tupType, _NONAME_RELATION_ID_);

		/* ----------------
		 *	 if we couldn't create the temp relation then
		 *	 we print a warning and return NULL.
		 * ----------------
		 */
		if (tempRelation == NULL)
		{
			elog(DEBUG, "ExecMaterial: temp relation is NULL! aborting...");
			return NULL;
		}

		/* ----------------
		 *	save the relation descriptor in the sortstate
		 * ----------------
		 */
		matstate->mat_TempRelation = tempRelation;
		matstate->csstate.css_currentRelation = NULL;

		/* ----------------
		 *	set all relations to be scanned in the forward direction
		 *	while creating the temporary relation.
		 * ----------------
		 */
		estate->es_direction = ForwardScanDirection;

		/* ----------------
		 *	 retrieve tuples from the subplan and
		 *	 insert them in the temporary relation
		 * ----------------
		 */
		outerNode = outerPlan((Plan *) node);
		for (;;)
		{
			slot = ExecProcNode(outerNode, (Plan *) node);

			if (TupIsNull(slot))
				break;

			heap_insert(tempRelation, slot->val);

			ExecClearTuple(slot);
		}

		/* ----------------
		 *	 restore to user specified direction
		 * ----------------
		 */
		estate->es_direction = dir;

		/* ----------------
		 *	 now initialize the scan descriptor to scan the
		 *	 sorted relation and update the sortstate information
		 * ----------------
		 */
		currentRelation = tempRelation;
		currentScanDesc = heap_beginscan(currentRelation,		/* relation */
										 ScanDirectionIsBackward(dir),
										 SnapshotSelf,	/* seeself */
										 0,		/* num scan keys */
										 NULL); /* scan keys */
		matstate->csstate.css_currentRelation = currentRelation;
		matstate->csstate.css_currentScanDesc = currentScanDesc;

		/* ----------------
		 *	finally set the sorted flag to true
		 * ----------------
		 */
		matstate->mat_Flag = true;
	}

	/* ----------------
	 *	at this point we know we have a sorted relation so
	 *	we perform a simple scan on it with amgetnext()..
	 * ----------------
	 */
	currentScanDesc = matstate->csstate.css_currentScanDesc;

	heapTuple = heap_getnext(currentScanDesc, ScanDirectionIsBackward(dir));

	/* ----------------
	 *	put the tuple into the scan tuple slot and return the slot.
	 *	Note: since the tuple is really a pointer to a page, we don't want
	 *	to call pfree() on it..
	 * ----------------
	 */
	slot = (TupleTableSlot *) matstate->csstate.css_ScanTupleSlot;

	return ExecStoreTuple(heapTuple,	/* tuple to store */
						  slot, /* slot to store in */
						  currentScanDesc->rs_cbuf,		/* buffer for this tuple */
						  false);		/* don't pfree this pointer */

}

/* ----------------------------------------------------------------
 *		ExecInitMaterial
 * ----------------------------------------------------------------
 */
bool							/* initialization status */
ExecInitMaterial(Material *node, EState *estate, Plan *parent)
{
	MaterialState *matstate;
	Plan	   *outerPlan;

	/* ----------------
	 *	assign the node's execution state
	 * ----------------
	 */
	node->plan.state = estate;

	/* ----------------
	 * create state structure
	 * ----------------
	 */
	matstate = makeNode(MaterialState);
	matstate->mat_Flag = false;
	matstate->mat_TempRelation = NULL;
	node->matstate = matstate;

	/* ----------------
	 *	Miscellanious initialization
	 *
	 *		 +	assign node's base_id
	 *		 +	assign debugging hooks and
	 *		 +	assign result tuple slot
	 *
	 *	Materialization nodes don't need ExprContexts because
	 *	they never call ExecQual or ExecTargetList.
	 * ----------------
	 */
	ExecAssignNodeBaseInfo(estate, &matstate->csstate.cstate, parent);

#define MATERIAL_NSLOTS 1
	/* ----------------
	 * tuple table initialization
	 * ----------------
	 */
	ExecInitScanTupleSlot(estate, &matstate->csstate);

	/* ----------------
	 * initializes child nodes
	 * ----------------
	 */
	outerPlan = outerPlan((Plan *) node);
	ExecInitNode(outerPlan, estate, (Plan *) node);

	/* ----------------
	 *	initialize tuple type.	no need to initialize projection
	 *	info because this node doesn't do projections.
	 * ----------------
	 */
	ExecAssignScanTypeFromOuterPlan((Plan *) node, &matstate->csstate);
	matstate->csstate.cstate.cs_ProjInfo = NULL;

	return TRUE;
}

int
ExecCountSlotsMaterial(Material *node)
{
	return ExecCountSlotsNode(outerPlan((Plan *) node)) +
	ExecCountSlotsNode(innerPlan((Plan *) node)) +
	MATERIAL_NSLOTS;
}

/* ----------------------------------------------------------------
 *		ExecEndMaterial
 *
 * old comments
 *		destroys the temporary relation.
 * ----------------------------------------------------------------
 */
void
ExecEndMaterial(Material *node)
{
	MaterialState *matstate;
	Relation	tempRelation;
	Plan	   *outerPlan;

	/* ----------------
	 *	get info from the material state
	 * ----------------
	 */
	matstate = node->matstate;
	tempRelation = matstate->mat_TempRelation;

	/* ----------------
	 *	shut down the scan, but don't close the temp relation
	 * ----------------
	 */
	matstate->csstate.css_currentRelation = NULL;
	ExecCloseR((Plan *) node);

	/* ----------------
	 *	shut down the subplan
	 * ----------------
	 */
	outerPlan = outerPlan((Plan *) node);
	ExecEndNode(outerPlan, (Plan *) node);

	/* ----------------
	 *	clean out the tuple table
	 * ----------------
	 */
	ExecClearTuple(matstate->csstate.css_ScanTupleSlot);

	/* ----------------
	 *	delete the temp relation
	 * ----------------
	 */
	if (tempRelation != NULL)
		heap_drop(tempRelation);
}

/* ----------------------------------------------------------------
 *		ExecMaterialReScan
 *
 *		Rescans the temporary relation.
 * ----------------------------------------------------------------
 */
void
ExecMaterialReScan(Material *node, ExprContext *exprCtxt, Plan *parent)
{
	MaterialState *matstate = node->matstate;

	/*
	 * If we haven't materialized yet, just return. If outerplan' chgParam is
	 * not NULL then it will be re-scanned by ExecProcNode, else - no
	 * reason to re-scan it at all.
	 */
	if (matstate->mat_Flag == false)
		return;

	/*
	 * If subnode is to be rescanned then we forget previous stored results;
	 * we have to re-read the subplan and re-store.
	 *
	 * Otherwise we can just rewind and rescan the stored output.
	 */
	if (((Plan *) node)->lefttree->chgParam != NULL)
	{
		Relation	tempRelation = matstate->mat_TempRelation;

		matstate->csstate.css_currentRelation = NULL;
		ExecCloseR((Plan *) node);
		ExecClearTuple(matstate->csstate.css_ScanTupleSlot);
		if (tempRelation != NULL)
			heap_drop(tempRelation);
		matstate->mat_TempRelation = NULL;
		matstate->mat_Flag = false;
	}
	else
	{
		matstate->csstate.css_currentScanDesc =
			ExecReScanR(matstate->csstate.css_currentRelation,
						matstate->csstate.css_currentScanDesc,
						node->plan.state->es_direction, 0, NULL);
	}
}

/* ----------------------------------------------------------------
 *		ExecMaterialMarkPos
 * ----------------------------------------------------------------
 */
void
ExecMaterialMarkPos(Material *node)
{
	MaterialState *matstate;
	HeapScanDesc scan;

	/* ----------------
	 *	if we haven't materialized yet, just return.
	 * ----------------
	 */
	matstate = node->matstate;
	if (matstate->mat_Flag == false)
		return;

	/* ----------------
	 *	mark the scan position
	 * ----------------
	 */
	scan = matstate->csstate.css_currentScanDesc;
	heap_markpos(scan);
}

/* ----------------------------------------------------------------
 *		ExecMaterialRestrPos
 * ----------------------------------------------------------------
 */
void
ExecMaterialRestrPos(Material *node)
{
	MaterialState *matstate;
	HeapScanDesc scan;

	/* ----------------
	 *	if we haven't materialized yet, just return.
	 * ----------------
	 */
	matstate = node->matstate;
	if (matstate->mat_Flag == false)
		return;

	/* ----------------
	 *	restore the scan to the previously marked position
	 * ----------------
	 */
	scan = matstate->csstate.css_currentScanDesc;
	heap_restrpos(scan);
}
