/*-------------------------------------------------------------------------
 *
 * execnodes.h
 *	  definitions for executor state nodes
 *
 *
 * Portions Copyright (c) 1996-2002, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $Id: execnodes.h,v 1.89 2003/01/10 21:08:15 tgl Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef EXECNODES_H
#define EXECNODES_H

#include "access/relscan.h"
#include "executor/hashjoin.h"
#include "executor/tuptable.h"
#include "fmgr.h"
#include "nodes/params.h"
#include "nodes/plannodes.h"
#include "utils/tuplestore.h"


/* ----------------
 *	  IndexInfo information
 *
 *		this class holds the information needed to construct new index
 *		entries for a particular index.  Used for both index_build and
 *		retail creation of index entries.
 *
 *		NumIndexAttrs		number of columns in this index
 *							(1 if a func. index, else same as NumKeyAttrs)
 *		NumKeyAttrs			number of key attributes for this index
 *							(ie, number of attrs from underlying relation)
 *		KeyAttrNumbers		underlying-rel attribute numbers used as keys
 *		Predicate			partial-index predicate, or NIL if none
 *		PredicateState		exec state for predicate, or NIL if none
 *		FuncOid				OID of function, or InvalidOid if not f. index
 *		FuncInfo			fmgr lookup data for function, if FuncOid valid
 *		Unique				is it a unique index?
 * ----------------
 */
typedef struct IndexInfo
{
	NodeTag		type;
	int			ii_NumIndexAttrs;
	int			ii_NumKeyAttrs;
	AttrNumber	ii_KeyAttrNumbers[INDEX_MAX_KEYS];
	List	   *ii_Predicate;	/* list of Expr */
	List	   *ii_PredicateState; /* list of ExprState */
	Oid			ii_FuncOid;
	FmgrInfo	ii_FuncInfo;
	bool		ii_Unique;
} IndexInfo;

/* ----------------
 *	  ExprContext_CB
 *
 *		List of callbacks to be called at ExprContext shutdown.
 * ----------------
 */
typedef void (*ExprContextCallbackFunction) (Datum arg);

typedef struct ExprContext_CB
{
	struct ExprContext_CB *next;
	ExprContextCallbackFunction function;
	Datum		arg;
} ExprContext_CB;

/* ----------------
 *	  ExprContext
 *
 *		This class holds the "current context" information
 *		needed to evaluate expressions for doing tuple qualifications
 *		and tuple projections.	For example, if an expression refers
 *		to an attribute in the current inner tuple then we need to know
 *		what the current inner tuple is and so we look at the expression
 *		context.
 *
 *	There are two memory contexts associated with an ExprContext:
 *	* ecxt_per_query_memory is a query-lifespan context, typically the same
 *	  context the ExprContext node itself is allocated in.  This context
 *	  can be used for purposes such as storing function call cache info.
 *	* ecxt_per_tuple_memory is a short-term context for expression results.
 *	  As the name suggests, it will typically be reset once per tuple,
 *	  before we begin to evaluate expressions for that tuple.  Each
 *	  ExprContext normally has its very own per-tuple memory context.
 *
 *	CurrentMemoryContext should be set to ecxt_per_tuple_memory before
 *	calling ExecEvalExpr() --- see ExecEvalExprSwitchContext().
 * ----------------
 */
typedef struct ExprContext
{
	NodeTag		type;

	/* Tuples that Var nodes in expression may refer to */
	TupleTableSlot *ecxt_scantuple;
	TupleTableSlot *ecxt_innertuple;
	TupleTableSlot *ecxt_outertuple;

	/* Memory contexts for expression evaluation --- see notes above */
	MemoryContext ecxt_per_query_memory;
	MemoryContext ecxt_per_tuple_memory;

	/* Values to substitute for Param nodes in expression */
	ParamExecData *ecxt_param_exec_vals;		/* for PARAM_EXEC params */
	ParamListInfo ecxt_param_list_info; /* for other param types */

	/* Values to substitute for Aggref nodes in expression */
	Datum	   *ecxt_aggvalues; /* precomputed values for Aggref nodes */
	bool	   *ecxt_aggnulls;	/* null flags for Aggref nodes */

	/* Value to substitute for ConstraintTestValue nodes in expression */
	Datum		domainValue_datum;
	bool		domainValue_isNull;

	/* Link to containing EState */
	struct EState *ecxt_estate;

	/* Functions to call back when ExprContext is shut down */
	ExprContext_CB *ecxt_callbacks;
} ExprContext;

/*
 * Set-result status returned by ExecEvalExpr()
 */
typedef enum
{
	ExprSingleResult,			/* expression does not return a set */
	ExprMultipleResult,			/* this result is an element of a set */
	ExprEndResult				/* there are no more elements in the set */
} ExprDoneCond;

/*
 * Return modes for functions returning sets.  Note values must be chosen
 * as separate bits so that a bitmask can be formed to indicate supported
 * modes.
 */
typedef enum
{
	SFRM_ValuePerCall = 0x01,	/* one value returned per call */
	SFRM_Materialize = 0x02		/* result set instantiated in Tuplestore */
} SetFunctionReturnMode;

/*
 * When calling a function that might return a set (multiple rows),
 * a node of this type is passed as fcinfo->resultinfo to allow
 * return status to be passed back.  A function returning set should
 * raise an error if no such resultinfo is provided.
 */
typedef struct ReturnSetInfo
{
	NodeTag		type;
	/* values set by caller: */
	ExprContext *econtext;		/* context function is being called in */
	TupleDesc	expectedDesc;	/* tuple descriptor expected by caller */
	int			allowedModes;	/* bitmask: return modes caller can handle */
	/* result status from function (but pre-initialized by caller): */
	SetFunctionReturnMode returnMode;	/* actual return mode */
	ExprDoneCond isDone;		/* status for ValuePerCall mode */
	/* fields filled by function in Materialize return mode: */
	Tuplestorestate *setResult; /* holds the complete returned tuple set */
	TupleDesc	setDesc;		/* actual descriptor for returned tuples */
} ReturnSetInfo;

/* ----------------
 *		ProjectionInfo node information
 *
 *		This is all the information needed to perform projections
 *		on a tuple.  Nodes which need to do projections create one
 *		of these.  In theory, when a node wants to perform a projection
 *		it should just update this information as necessary and then
 *		call ExecProject().  -cim 6/3/91
 *
 *		targetlist		target list for projection
 *		len				length of target list
 *		tupValue		array of pointers to projection results
 *		exprContext		expression context for ExecTargetList
 *		slot			slot to place projection result in
 * ----------------
 */
typedef struct ProjectionInfo
{
	NodeTag		type;
	List	   *pi_targetlist;
	int			pi_len;
	Datum	   *pi_tupValue;
	ExprContext *pi_exprContext;
	TupleTableSlot *pi_slot;
} ProjectionInfo;

/* ----------------
 *	  JunkFilter
 *
 *	  This class is used to store information regarding junk attributes.
 *	  A junk attribute is an attribute in a tuple that is needed only for
 *	  storing intermediate information in the executor, and does not belong
 *	  in emitted tuples.	For example, when we do an UPDATE query,
 *	  the planner adds a "junk" entry to the targetlist so that the tuples
 *	  returned to ExecutePlan() contain an extra attribute: the ctid of
 *	  the tuple to be updated.	This is needed to do the update, but we
 *	  don't want the ctid to be part of the stored new tuple!  So, we
 *	  apply a "junk filter" to remove the junk attributes and form the
 *	  real output tuple.
 *
 *	  targetList:		the original target list (including junk attributes).
 *	  length:			the length of 'targetList'.
 *	  tupType:			the tuple descriptor for the "original" tuple
 *						(including the junk attributes).
 *	  cleanTargetList:	the "clean" target list (junk attributes removed).
 *	  cleanLength:		the length of 'cleanTargetList'
 *	  cleanTupType:		the tuple descriptor of the "clean" tuple (with
 *						junk attributes removed).
 *	  cleanMap:			A map with the correspondence between the non-junk
 *						attribute numbers of the "original" tuple and the
 *						attribute numbers of the "clean" tuple.
 *	  resultSlot:		tuple slot that can be used to hold cleaned tuple.
 * ----------------
 */
typedef struct JunkFilter
{
	NodeTag		type;
	List	   *jf_targetList;
	int			jf_length;
	TupleDesc	jf_tupType;
	List	   *jf_cleanTargetList;
	int			jf_cleanLength;
	TupleDesc	jf_cleanTupType;
	AttrNumber *jf_cleanMap;
	TupleTableSlot *jf_resultSlot;
} JunkFilter;

/* ----------------
 *	  ResultRelInfo information
 *
 *		Whenever we update an existing relation, we have to
 *		update indices on the relation, and perhaps also fire triggers.
 *		The ResultRelInfo class is used to hold all the information needed
 *		about a result relation, including indices.. -cim 10/15/89
 *
 *		RangeTableIndex			result relation's range table index
 *		RelationDesc			relation descriptor for result relation
 *		NumIndices				# of indices existing on result relation
 *		IndexRelationDescs		array of relation descriptors for indices
 *		IndexRelationInfo		array of key/attr info for indices
 *		TrigDesc				triggers to be fired, if any
 *		TrigFunctions			cached lookup info for trigger functions
 *		ConstraintExprs			array of constraint-checking expr states
 *		junkFilter				for removing junk attributes from tuples
 * ----------------
 */
typedef struct ResultRelInfo
{
	NodeTag		type;
	Index		ri_RangeTableIndex;
	Relation	ri_RelationDesc;
	int			ri_NumIndices;
	RelationPtr ri_IndexRelationDescs;
	IndexInfo **ri_IndexRelationInfo;
	TriggerDesc *ri_TrigDesc;
	FmgrInfo   *ri_TrigFunctions;
	List	  **ri_ConstraintExprs;
	JunkFilter *ri_junkFilter;
} ResultRelInfo;

/* ----------------
 *	  EState information
 *
 * Master working state for an Executor invocation
 * ----------------
 */
typedef struct EState
{
	NodeTag		type;

	/* Basic state for all query types: */
	ScanDirection es_direction;	/* current scan direction */
	Snapshot	es_snapshot;	/* time qual to use */
	List	   *es_range_table;	/* List of RangeTableEntrys */

	/* Info about target table for insert/update/delete queries: */
	ResultRelInfo *es_result_relations; /* array of ResultRelInfos */
	int			es_num_result_relations;		/* length of array */
	ResultRelInfo *es_result_relation_info;		/* currently active array
												 * elt */
	JunkFilter *es_junkFilter;	/* currently active junk filter */
	Relation	es_into_relation_descriptor; /* for SELECT INTO */

	/* Parameter info: */
	ParamListInfo es_param_list_info;	/* values of external params */
	ParamExecData *es_param_exec_vals;	/* values of internal params */

	/* Other working state: */
	MemoryContext es_query_cxt; /* per-query context in which EState lives */

	TupleTable	es_tupleTable;	/* Array of TupleTableSlots */

	uint32		es_processed;	/* # of tuples processed */
	Oid			es_lastoid;		/* last oid processed (by INSERT) */
	List	   *es_rowMark;		/* not good place, but there is no other */

	bool		es_instrument;	/* true requests runtime instrumentation */

	List	   *es_exprcontexts; /* List of ExprContexts within EState */

	/*
	 * this ExprContext is for per-output-tuple operations, such as
	 * constraint checks and index-value computations.	It will be reset
	 * for each output tuple.  Note that it will be created only if
	 * needed.
	 */
	ExprContext *es_per_tuple_exprcontext;

	/* Below is to re-evaluate plan qual in READ COMMITTED mode */
	Plan	   *es_topPlan;		/* link to top of plan tree */
	struct evalPlanQual *es_evalPlanQual; /* chain of PlanQual states */
	bool	   *es_evTupleNull;	/* local array of EPQ status */
	HeapTuple  *es_evTuple;		/* shared array of EPQ substitute tuples */
	bool		es_useEvalPlan;	/* evaluating EPQ tuples? */
} EState;


/* ----------------------------------------------------------------
 *				 Expression State Trees
 *
 * Each executable expression tree has a parallel ExprState tree.
 *
 * Unlike PlanState, there is not an exact one-for-one correspondence between
 * ExprState node types and Expr node types.  Many Expr node types have no
 * need for node-type-specific run-time state, and so they can use plain
 * ExprState or GenericExprState as their associated ExprState node type.
 * ----------------------------------------------------------------
 */

/* ----------------
 *		ExprState node
 *
 * ExprState is the common superclass for all ExprState-type nodes.
 *
 * It can also be instantiated directly for leaf Expr nodes that need no
 * local run-time state (such as Var, Const, or Param).
 * ----------------
 */
typedef struct ExprState
{
	NodeTag		type;
	Expr	   *expr;			/* associated Expr node */
} ExprState;

/* ----------------
 *		GenericExprState node
 *
 * This is used for Expr node types that need no local run-time state,
 * but have one child Expr node.
 * ----------------
 */
typedef struct GenericExprState
{
	ExprState	xprstate;
	ExprState  *arg;			/* state of my child node */
} GenericExprState;

/* ----------------
 *		AggrefExprState node
 * ----------------
 */
typedef struct AggrefExprState
{
	ExprState	xprstate;
	ExprState  *target;			/* state of my child node */
	int			aggno;			/* ID number for agg within its plan node */
} AggrefExprState;

/* ----------------
 *		ArrayRefExprState node
 * ----------------
 */
typedef struct ArrayRefExprState
{
	ExprState	xprstate;
	List	   *refupperindexpr; /* states for child nodes */
	List	   *reflowerindexpr;
	ExprState  *refexpr;
	ExprState  *refassgnexpr;
} ArrayRefExprState;

/* ----------------
 *		FuncExprState node
 *
 * Although named for FuncExpr, this is also used for OpExpr and DistinctExpr
 * nodes; be careful to check what xprstate.expr is actually pointing at!
 * ----------------
 */
typedef struct FuncExprState
{
	ExprState	xprstate;
	List	   *args;			/* states of argument expressions */

	/*
	 * Function manager's lookup info for the target function.  If func.fn_oid
	 * is InvalidOid, we haven't initialized it yet.
	 */
	FmgrInfo	func;

	/*
	 * We also need to store argument values across calls when evaluating a
	 * function-returning-set.
	 *
	 * setArgsValid is true when we are evaluating a set-valued function
	 * and we are in the middle of a call series; we want to pass the same
	 * argument values to the function again (and again, until it returns
	 * ExprEndResult).
	 */
	bool		setArgsValid;

	/*
	 * Flag to remember whether we found a set-valued argument to the
	 * function. This causes the function result to be a set as well.
	 * Valid only when setArgsValid is true.
	 */
	bool		setHasSetArg;	/* some argument returns a set */

	/*
	 * Current argument data for a set-valued function; contains valid
	 * data only if setArgsValid is true.
	 */
	FunctionCallInfoData setArgs;
} FuncExprState;

/* ----------------
 *		BoolExprState node
 * ----------------
 */
typedef struct BoolExprState
{
	ExprState	xprstate;
	List	   *args;			/* states of argument expression(s) */
} BoolExprState;

/* ----------------
 *		SubPlanState node
 * ----------------
 */
/* this struct is private in nodeSubplan.c: */
typedef struct SubPlanHashTableData *SubPlanHashTable;

typedef struct SubPlanState
{
	ExprState	xprstate;
	EState	   *sub_estate;		/* subselect plan has its own EState */
	struct PlanState *planstate; /* subselect plan's state tree */
	List	   *exprs;			/* states of combining expression(s) */
	List	   *args;			/* states of argument expression(s) */
	bool		needShutdown;	/* TRUE = need to shutdown subplan */
	HeapTuple	curTuple;		/* copy of most recent tuple from subplan */
	/* these are used when hashing the subselect's output: */
	SubPlanHashTable hashtable;	/* hash table for no-nulls subselect rows */
	SubPlanHashTable hashnulls;	/* hash table for rows with null(s) */
} SubPlanState;

/* ----------------
 *		CaseExprState node
 * ----------------
 */
typedef struct CaseExprState
{
	ExprState	xprstate;
	List	   *args;			/* the arguments (list of WHEN clauses) */
	ExprState  *defresult;		/* the default result (ELSE clause) */
} CaseExprState;

/* ----------------
 *		CaseWhenState node
 * ----------------
 */
typedef struct CaseWhenState
{
	ExprState	xprstate;
	ExprState  *expr;			/* condition expression */
	ExprState  *result;			/* substitution result */
} CaseWhenState;

/* ----------------
 *		ConstraintTestState node
 * ----------------
 */
typedef struct ConstraintTestState
{
	ExprState	xprstate;
	ExprState  *arg;			/* input expression */
	ExprState  *check_expr;		/* for CHECK test, a boolean expression */
} ConstraintTestState;


/* ----------------------------------------------------------------
 *				 Executor State Trees
 *
 * An executing query has a PlanState tree paralleling the Plan tree
 * that describes the plan.
 * ----------------------------------------------------------------
 */

/* ----------------
 *		PlanState node
 *
 * We never actually instantiate any PlanState nodes; this is just the common
 * abstract superclass for all PlanState-type nodes.
 * ----------------
 */
typedef struct PlanState
{
	NodeTag		type;

	Plan	   *plan;			/* associated Plan node */

	EState	   *state;			/* at execution time, state's of
								 * individual nodes point to one EState
								 * for the whole top-level plan */

	struct Instrumentation *instrument; /* Optional runtime stats for this
										 * plan node */

	/*
	 * Common structural data for all Plan types.  These links to subsidiary
	 * state trees parallel links in the associated plan tree (except for
	 * the subPlan list, which does not exist in the plan tree).
	 */
	List	   *targetlist;		/* target list to be computed at this node */
	List	   *qual;			/* implicitly-ANDed qual conditions */
	struct PlanState *lefttree;	/* input plan tree(s) */
	struct PlanState *righttree;
	List	   *initPlan;		/* Init SubPlanState nodes (un-correlated
								 * expr subselects) */
	List	   *subPlan;		/* SubPlanState nodes in my expressions */

	/*
	 * State for management of parameter-change-driven rescanning
	 */
	List	   *chgParam;		/* integer list of IDs of changed Params */

	/*
	 * Other run-time state needed by most if not all node types.
	 */
	TupleTableSlot *ps_OuterTupleSlot; /* slot for current "outer" tuple */
	TupleTableSlot *ps_ResultTupleSlot;	/* slot for my result tuples */
	ExprContext *ps_ExprContext; /* node's expression-evaluation context */
	ProjectionInfo *ps_ProjInfo; /* info for doing tuple projection */
	bool		ps_TupFromTlist; /* state flag for processing set-valued
								  * functions in targetlist */
} PlanState;

/* ----------------
 *	these are are defined to avoid confusion problems with "left"
 *	and "right" and "inner" and "outer".  The convention is that
 *	the "left" plan is the "outer" plan and the "right" plan is
 *	the inner plan, but these make the code more readable.
 * ----------------
 */
#define innerPlanState(node)		(((PlanState *)(node))->righttree)
#define outerPlanState(node)		(((PlanState *)(node))->lefttree)


/* ----------------
 *	 ResultState information
 * ----------------
 */
typedef struct ResultState
{
	PlanState	ps;				/* its first field is NodeTag */
	ExprState  *resconstantqual;
	bool		rs_done;		/* are we done? */
	bool		rs_checkqual;	/* do we need to check the qual? */
} ResultState;

/* ----------------
 *	 AppendState information
 *
 *		nplans			how many plans are in the list
 *		whichplan		which plan is being executed (0 .. n-1)
 *		firstplan		first plan to execute (usually 0)
 *		lastplan		last plan to execute (usually n-1)
 * ----------------
 */
typedef struct AppendState
{
	PlanState	ps;				/* its first field is NodeTag */
	PlanState **appendplans;	/* array of PlanStates for my inputs */
	int			as_nplans;
	int			as_whichplan;
	int			as_firstplan;
	int			as_lastplan;
} AppendState;

/* ----------------------------------------------------------------
 *				 Scan State Information
 * ----------------------------------------------------------------
 */

/* ----------------
 *	 ScanState information
 *
 *		ScanState extends PlanState for node types that represent
 *		scans of an underlying relation.  It can also be used for nodes
 *		that scan the output of an underlying plan node --- in that case,
 *		only ScanTupleSlot is actually useful, and it refers to the tuple
 *		retrieved from the subplan.
 *
 *		currentRelation    relation being scanned (NULL if none)
 *		currentScanDesc    current scan descriptor for scan (NULL if none)
 *		ScanTupleSlot	   pointer to slot in tuple table holding scan tuple
 * ----------------
 */
typedef struct ScanState
{
	PlanState	ps;				/* its first field is NodeTag */
	Relation	ss_currentRelation;
	HeapScanDesc ss_currentScanDesc;
	TupleTableSlot *ss_ScanTupleSlot;
} ScanState;

/*
 * SeqScan uses a bare ScanState as its state node, since it needs
 * no additional fields.
 */
typedef ScanState SeqScanState;

/* ----------------
 *	 IndexScanState information
 *
 *		NumIndices		   number of indices in this scan
 *		IndexPtr		   current index in use
 *		ScanKeys		   Skey structures to scan index rels
 *		NumScanKeys		   array of no of keys in each Skey struct
 *		RuntimeKeyInfo	   array of array of exprstates for Skeys
 *						   that will be evaluated at runtime
 *		RuntimeContext	   expr context for evaling runtime Skeys
 *		RuntimeKeysReady   true if runtime Skeys have been computed
 *		RelationDescs	   ptr to array of relation descriptors
 *		ScanDescs		   ptr to array of scan descriptors
 * ----------------
 */
typedef struct IndexScanState
{
	ScanState	ss;				/* its first field is NodeTag */
	List	   *indxqual;
	List	   *indxqualorig;
	int			iss_NumIndices;
	int			iss_IndexPtr;
	int			iss_MarkIndexPtr;
	ScanKey    *iss_ScanKeys;
	int		   *iss_NumScanKeys;
	ExprState ***iss_RuntimeKeyInfo;
	ExprContext *iss_RuntimeContext;
	bool		iss_RuntimeKeysReady;
	RelationPtr iss_RelationDescs;
	IndexScanDescPtr iss_ScanDescs;
} IndexScanState;

/* ----------------
 *	 TidScanState information
 *
 *		NumTids		   number of tids in this scan
 *		TidPtr		   current tid in use
 *		TidList		   evaluated item pointers
 * ----------------
 */
typedef struct TidScanState
{
	ScanState	ss;				/* its first field is NodeTag */
	List	   *tss_tideval;	/* list of ExprState nodes */
	int			tss_NumTids;
	int			tss_TidPtr;
	int			tss_MarkTidPtr;
	ItemPointerData *tss_TidList;
	HeapTupleData tss_htup;
} TidScanState;

/* ----------------
 *	 SubqueryScanState information
 *
 *		SubqueryScanState is used for scanning a sub-query in the range table.
 *		The sub-query will have its own EState, which we save here.
 *		ScanTupleSlot references the current output tuple of the sub-query.
 *
 *		SubEState		   exec state for sub-query
 * ----------------
 */
typedef struct SubqueryScanState
{
	ScanState	ss;				/* its first field is NodeTag */
	PlanState  *subplan;
	EState	   *sss_SubEState;
} SubqueryScanState;

/* ----------------
 *	 FunctionScanState information
 *
 *		Function nodes are used to scan the results of a
 *		function appearing in FROM (typically a function returning set).
 *
 *		tupdesc				expected return tuple description
 *		tuplestorestate		private state of tuplestore.c
 *		funcexpr			state for function expression being evaluated
 * ----------------
 */
typedef struct FunctionScanState
{
	ScanState	ss;				/* its first field is NodeTag */
	TupleDesc	tupdesc;
	Tuplestorestate *tuplestorestate;
	ExprState  *funcexpr;
} FunctionScanState;

/* ----------------------------------------------------------------
 *				 Join State Information
 * ----------------------------------------------------------------
 */

/* ----------------
 *	 JoinState information
 *
 *		Superclass for state nodes of join plans.
 * ----------------
 */
typedef struct JoinState
{
	PlanState	ps;
	JoinType	jointype;
	List	   *joinqual;		/* JOIN quals (in addition to ps.qual) */
} JoinState;

/* ----------------
 *	 NestLoopState information
 *
 *		NeedNewOuter	   true if need new outer tuple on next call
 *		MatchedOuter	   true if found a join match for current outer tuple
 *		NullInnerTupleSlot prepared null tuple for left outer joins
 * ----------------
 */
typedef struct NestLoopState
{
	JoinState	js;				/* its first field is NodeTag */
	bool		nl_NeedNewOuter;
	bool		nl_MatchedOuter;
	TupleTableSlot *nl_NullInnerTupleSlot;
} NestLoopState;

/* ----------------
 *	 MergeJoinState information
 *
 *		OuterSkipQual	   outerKey1 < innerKey1 ...
 *		InnerSkipQual	   outerKey1 > innerKey1 ...
 *		JoinState		   current "state" of join. see executor.h
 *		MatchedOuter	   true if found a join match for current outer tuple
 *		MatchedInner	   true if found a join match for current inner tuple
 *		OuterTupleSlot	   pointer to slot in tuple table for cur outer tuple
 *		InnerTupleSlot	   pointer to slot in tuple table for cur inner tuple
 *		MarkedTupleSlot    pointer to slot in tuple table for marked tuple
 *		NullOuterTupleSlot prepared null tuple for right outer joins
 *		NullInnerTupleSlot prepared null tuple for left outer joins
 * ----------------
 */
typedef struct MergeJoinState
{
	JoinState	js;				/* its first field is NodeTag */
	List	   *mergeclauses;		/* list of ExprState nodes */
	List	   *mj_OuterSkipQual;	/* list of ExprState nodes */
	List	   *mj_InnerSkipQual;	/* list of ExprState nodes */
	int			mj_JoinState;
	bool		mj_MatchedOuter;
	bool		mj_MatchedInner;
	TupleTableSlot *mj_OuterTupleSlot;
	TupleTableSlot *mj_InnerTupleSlot;
	TupleTableSlot *mj_MarkedTupleSlot;
	TupleTableSlot *mj_NullOuterTupleSlot;
	TupleTableSlot *mj_NullInnerTupleSlot;
} MergeJoinState;

/* ----------------
 *	 HashJoinState information
 *
 *		hj_HashTable			hash table for the hashjoin
 *		hj_CurBucketNo			bucket# for current outer tuple
 *		hj_CurTuple				last inner tuple matched to current outer
 *								tuple, or NULL if starting search
 *								(CurBucketNo and CurTuple are meaningless
 *								 unless OuterTupleSlot is nonempty!)
 *		hj_OuterHashKeys		the outer hash keys in the hashjoin condition
 *		hj_InnerHashKeys		the inner hash keys in the hashjoin condition
 *		hj_OuterTupleSlot		tuple slot for outer tuples
 *		hj_HashTupleSlot		tuple slot for hashed tuples
 *		hj_NullInnerTupleSlot	prepared null tuple for left outer joins
 *		hj_NeedNewOuter			true if need new outer tuple on next call
 *		hj_MatchedOuter			true if found a join match for current outer
 *		hj_hashdone				true if hash-table-build phase is done
 * ----------------
 */
typedef struct HashJoinState
{
	JoinState	js;				/* its first field is NodeTag */
	List	   *hashclauses;	/* list of ExprState nodes */
	HashJoinTable hj_HashTable;
	int			hj_CurBucketNo;
	HashJoinTuple hj_CurTuple;
	List	   *hj_OuterHashKeys;	/* list of ExprState nodes */
	List	   *hj_InnerHashKeys;	/* list of ExprState nodes */
	TupleTableSlot *hj_OuterTupleSlot;
	TupleTableSlot *hj_HashTupleSlot;
	TupleTableSlot *hj_NullInnerTupleSlot;
	bool		hj_NeedNewOuter;
	bool		hj_MatchedOuter;
	bool		hj_hashdone;
} HashJoinState;


/* ----------------------------------------------------------------
 *				 Materialization State Information
 * ----------------------------------------------------------------
 */

/* ----------------
 *	 MaterialState information
 *
 *		materialize nodes are used to materialize the results
 *		of a subplan into a temporary file.
 *
 *		ss.ss_ScanTupleSlot refers to output of underlying plan.
 *
 *		tuplestorestate		private state of tuplestore.c
 * ----------------
 */
typedef struct MaterialState
{
	ScanState	ss;				/* its first field is NodeTag */
	void	   *tuplestorestate;
} MaterialState;

/* ----------------
 *	 SortState information
 * ----------------
 */
typedef struct SortState
{
	ScanState	ss;				/* its first field is NodeTag */
	bool		sort_Done;		/* sort completed yet? */
	void	   *tuplesortstate;	/* private state of tuplesort.c */
} SortState;

/* ---------------------
 *	GroupState information
 * -------------------------
 */
typedef struct GroupState
{
	ScanState	ss;				/* its first field is NodeTag */
	FmgrInfo   *eqfunctions;	/* per-field lookup data for equality fns */
	HeapTuple	grp_firstTuple;	/* copy of first tuple of current group */
	bool		grp_done;		/* indicates completion of Group scan */
} GroupState;

/* ---------------------
 *	AggState information
 *
 *	ss.ss_ScanTupleSlot refers to output of underlying plan.
 *
 *	Note: ss.ps.ps_ExprContext contains ecxt_aggvalues and
 *	ecxt_aggnulls arrays, which hold the computed agg values for the current
 *	input group during evaluation of an Agg node's output tuple(s).  We
 *	create a second ExprContext, tmpcontext, in which to evaluate input
 *	expressions and run the aggregate transition functions.
 * -------------------------
 */
/* these structs are private in nodeAgg.c: */
typedef struct AggStatePerAggData *AggStatePerAgg;
typedef struct AggStatePerGroupData *AggStatePerGroup;
typedef struct AggHashEntryData *AggHashEntry;
typedef struct AggHashTableData *AggHashTable;

typedef struct AggState
{
	ScanState	ss;				/* its first field is NodeTag */
	List	   *aggs;			/* all Aggref nodes in targetlist & quals */
	int			numaggs;		/* length of list (could be zero!) */
	FmgrInfo   *eqfunctions;	/* per-grouping-field equality fns */
	AggStatePerAgg peragg;		/* per-Aggref information */
	MemoryContext aggcontext;	/* memory context for long-lived data */
	ExprContext *tmpcontext;	/* econtext for input expressions */
	bool		agg_done;		/* indicates completion of Agg scan */
	/* these fields are used in AGG_PLAIN and AGG_SORTED modes: */
	AggStatePerGroup pergroup;	/* per-Aggref-per-group working state */
	HeapTuple	grp_firstTuple;	/* copy of first tuple of current group */
	/* these fields are used in AGG_HASHED mode: */
	AggHashTable hashtable;		/* hash table with one entry per group */
	bool		table_filled;	/* hash table filled yet? */
	AggHashEntry next_hash_entry; /* next entry in current chain */
	int			next_hash_bucket; /* next chain */
} AggState;

/* ----------------
 *	 UniqueState information
 *
 *		Unique nodes are used "on top of" sort nodes to discard
 *		duplicate tuples returned from the sort phase.	Basically
 *		all it does is compare the current tuple from the subplan
 *		with the previously fetched tuple stored in priorTuple.
 *		If the two are identical in all interesting fields, then
 *		we just fetch another tuple from the sort and try again.
 * ----------------
 */
typedef struct UniqueState
{
	PlanState	ps;				/* its first field is NodeTag */
	FmgrInfo   *eqfunctions;	/* per-field lookup data for equality fns */
	HeapTuple	priorTuple;		/* most recently returned tuple, or NULL */
	MemoryContext tempContext;	/* short-term context for comparisons */
} UniqueState;

/* ----------------
 *	 HashState information
 * ----------------
 */
typedef struct HashState
{
	PlanState	ps;				/* its first field is NodeTag */
	HashJoinTable hashtable;	/* hash table for the hashjoin */
	List	   *hashkeys;		/* list of ExprState nodes */
	/* hashkeys is same as parent's hj_InnerHashKeys */
} HashState;

/* ----------------
 *	 SetOpState information
 *
 *		SetOp nodes are used "on top of" sort nodes to discard
 *		duplicate tuples returned from the sort phase.	These are
 *		more complex than a simple Unique since we have to count
 *		how many duplicates to return.
 * ----------------
 */
typedef struct SetOpState
{
	PlanState	ps;				/* its first field is NodeTag */
	FmgrInfo   *eqfunctions;	/* per-field lookup data for equality fns */
	bool		subplan_done;	/* has subplan returned EOF? */
	long		numLeft;		/* number of left-input dups of cur group */
	long		numRight;		/* number of right-input dups of cur group */
	long		numOutput;		/* number of dups left to output */
	MemoryContext tempContext;	/* short-term context for comparisons */
} SetOpState;

/* ----------------
 *	 LimitState information
 *
 *		Limit nodes are used to enforce LIMIT/OFFSET clauses.
 *		They just select the desired subrange of their subplan's output.
 *
 * offset is the number of initial tuples to skip (0 does nothing).
 * count is the number of tuples to return after skipping the offset tuples.
 * If no limit count was specified, count is undefined and noCount is true.
 * When lstate == LIMIT_INITIAL, offset/count/noCount haven't been set yet.
 * ----------------
 */
typedef enum
{
	LIMIT_INITIAL,				/* initial state for LIMIT node */
	LIMIT_EMPTY,				/* there are no returnable rows */
	LIMIT_INWINDOW,				/* have returned a row in the window */
	LIMIT_SUBPLANEOF,			/* at EOF of subplan (within window) */
	LIMIT_WINDOWEND,			/* stepped off end of window */
	LIMIT_WINDOWSTART			/* stepped off beginning of window */
} LimitStateCond;

typedef struct LimitState
{
	PlanState	ps;				/* its first field is NodeTag */
	ExprState  *limitOffset;	/* OFFSET parameter, or NULL if none */
	ExprState  *limitCount;		/* COUNT parameter, or NULL if none */
	long		offset;			/* current OFFSET value */
	long		count;			/* current COUNT, if any */
	bool		noCount;		/* if true, ignore count */
	LimitStateCond lstate;		/* state machine status, as above */
	long		position;		/* 1-based index of last tuple returned */
	TupleTableSlot *subSlot;	/* tuple last obtained from subplan */
} LimitState;

#endif   /* EXECNODES_H */
