/*-------------------------------------------------------------------------
 *
 * trigger.h
 *	  Declarations for trigger handling.
 *
 * Portions Copyright (c) 1996-2004, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql-server/src/include/commands/trigger.h,v 1.46 2004/07/01 00:51:40 tgl Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef TRIGGER_H
#define TRIGGER_H

#include "nodes/execnodes.h"
#include "nodes/parsenodes.h"

/*
 * TriggerData is the node type that is passed as fmgr "context" info
 * when a function is called by the trigger manager.
 */

#define CALLED_AS_TRIGGER(fcinfo) \
	((fcinfo)->context != NULL && IsA((fcinfo)->context, TriggerData))

typedef uint32 TriggerEvent;

typedef struct TriggerData
{
	NodeTag		type;
	TriggerEvent tg_event;
	Relation	tg_relation;
	HeapTuple	tg_trigtuple;
	HeapTuple	tg_newtuple;
	Trigger    *tg_trigger;
} TriggerData;

/* TriggerEvent bit flags */

#define TRIGGER_EVENT_INSERT			0x00000000
#define TRIGGER_EVENT_DELETE			0x00000001
#define TRIGGER_EVENT_UPDATE			0x00000002
#define TRIGGER_EVENT_OPMASK			0x00000003
#define TRIGGER_EVENT_ROW				0x00000004
#define TRIGGER_EVENT_BEFORE			0x00000008

#define TRIGGER_DEFERRED_DONE			0x00000010
#define TRIGGER_DEFERRED_CANCELED		0x00000020
#define TRIGGER_DEFERRED_DEFERRABLE		0x00000040
#define TRIGGER_DEFERRED_INITDEFERRED	0x00000080
#define TRIGGER_DEFERRED_HAS_BEFORE		0x00000100

#define TRIGGER_FIRED_BY_INSERT(event)	\
		(((TriggerEvent) (event) & TRIGGER_EVENT_OPMASK) == \
												TRIGGER_EVENT_INSERT)

#define TRIGGER_FIRED_BY_DELETE(event)	\
		(((TriggerEvent) (event) & TRIGGER_EVENT_OPMASK) == \
												TRIGGER_EVENT_DELETE)

#define TRIGGER_FIRED_BY_UPDATE(event)	\
		(((TriggerEvent) (event) & TRIGGER_EVENT_OPMASK) == \
												TRIGGER_EVENT_UPDATE)

#define TRIGGER_FIRED_FOR_ROW(event)			\
		((TriggerEvent) (event) & TRIGGER_EVENT_ROW)

#define TRIGGER_FIRED_FOR_STATEMENT(event)		\
		(!TRIGGER_FIRED_FOR_ROW (event))

#define TRIGGER_FIRED_BEFORE(event)				\
		((TriggerEvent) (event) & TRIGGER_EVENT_BEFORE)

#define TRIGGER_FIRED_AFTER(event)				\
		(!TRIGGER_FIRED_BEFORE (event))

/*
 * RI trigger function arguments are stored in pg_trigger.tgargs bytea
 *
 *	 constrname\0fkrel\0pkrel\0matchtype\0fkatt\0pkatt\0fkatt\0pkatt\0...
 *
 * There are one or more pairs of fkatt/pkatt names.
 *
 * The relation names are no longer of much use since they are not
 * guaranteed unique; they are present only for backwards compatibility.
 * Use the tgrelid and tgconstrrelid fields to identify the referenced
 * relations, instead.	(But note that which is which will depend on which
 * trigger you are looking at!)
 */
#define RI_CONSTRAINT_NAME_ARGNO		0
#define RI_FK_RELNAME_ARGNO				1
#define RI_PK_RELNAME_ARGNO				2
#define RI_MATCH_TYPE_ARGNO				3
#define RI_FIRST_ATTNAME_ARGNO			4		/* first attname pair
												 * starts here */

#define RI_KEYPAIR_FK_IDX				0
#define RI_KEYPAIR_PK_IDX				1

#define RI_MAX_NUMKEYS					INDEX_MAX_KEYS
#define RI_MAX_ARGUMENTS		(RI_FIRST_ATTNAME_ARGNO + (RI_MAX_NUMKEYS * 2))


extern Oid	CreateTrigger(CreateTrigStmt *stmt, bool forConstraint);

extern void DropTrigger(Oid relid, const char *trigname,
			DropBehavior behavior);
extern void RemoveTriggerById(Oid trigOid);

extern void renametrig(Oid relid, const char *oldname, const char *newname);

extern void RelationBuildTriggers(Relation relation);

extern TriggerDesc *CopyTriggerDesc(TriggerDesc *trigdesc);

extern void FreeTriggerDesc(TriggerDesc *trigdesc);

extern void ExecBSInsertTriggers(EState *estate,
					 ResultRelInfo *relinfo);
extern void ExecASInsertTriggers(EState *estate,
					 ResultRelInfo *relinfo);
extern HeapTuple ExecBRInsertTriggers(EState *estate,
					 ResultRelInfo *relinfo,
					 HeapTuple trigtuple);
extern void ExecARInsertTriggers(EState *estate,
					 ResultRelInfo *relinfo,
					 HeapTuple trigtuple);
extern void ExecBSDeleteTriggers(EState *estate,
					 ResultRelInfo *relinfo);
extern void ExecASDeleteTriggers(EState *estate,
					 ResultRelInfo *relinfo);
extern bool ExecBRDeleteTriggers(EState *estate,
					 ResultRelInfo *relinfo,
					 ItemPointer tupleid,
					 CommandId cid);
extern void ExecARDeleteTriggers(EState *estate,
					 ResultRelInfo *relinfo,
					 ItemPointer tupleid);
extern void ExecBSUpdateTriggers(EState *estate,
					 ResultRelInfo *relinfo);
extern void ExecASUpdateTriggers(EState *estate,
					 ResultRelInfo *relinfo);
extern HeapTuple ExecBRUpdateTriggers(EState *estate,
					 ResultRelInfo *relinfo,
					 ItemPointer tupleid,
					 HeapTuple newtuple,
					 CommandId cid);
extern void ExecARUpdateTriggers(EState *estate,
					 ResultRelInfo *relinfo,
					 ItemPointer tupleid,
					 HeapTuple newtuple);

extern void DeferredTriggerBeginXact(void);
extern void DeferredTriggerEndQuery(void);
extern void DeferredTriggerEndXact(void);
extern void DeferredTriggerAbortXact(void);
extern void DeferredTriggerBeginSubXact(void);
extern void DeferredTriggerEndSubXact(bool isCommit);

extern void DeferredTriggerSetState(ConstraintsSetStmt *stmt);


/*
 * in utils/adt/ri_triggers.c
 */
extern bool RI_FKey_keyequal_upd(TriggerData *trigdata);
extern bool RI_Initial_Check(FkConstraint *fkconstraint, 
							 Relation rel, 
							 Relation pkrel);

#endif   /* TRIGGER_H */
