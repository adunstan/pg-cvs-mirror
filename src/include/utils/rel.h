/*-------------------------------------------------------------------------
 *
 * rel.h
 *	  POSTGRES relation descriptor definitions.
 *
 *
 * Portions Copyright (c) 1996-2000, PostgreSQL, Inc
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $Id: rel.h,v 1.33 2000/01/26 05:58:38 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef REL_H
#define REL_H

#include "access/strat.h"
#include "access/tupdesc.h"
#include "catalog/pg_am.h"
#include "catalog/pg_class.h"
#include "rewrite/prs2lock.h"
#include "storage/fd.h"

/*
 * LockRelId and LockInfo really belong to lmgr.h, but it's more convenient
 * to declare them here so we can have a LockInfoData field in a Relation.
 */

typedef struct LockRelId
{
	Oid			relId;			/* a relation identifier */
	Oid			dbId;			/* a database identifier */
} LockRelId;

typedef struct LockInfoData
{
	LockRelId	lockRelId;
} LockInfoData;

typedef LockInfoData *LockInfo;


typedef struct Trigger
{
	Oid			tgoid;
	char	   *tgname;
	Oid			tgfoid;
	FmgrInfo	tgfunc;
	int16		tgtype;
	bool		tgenabled;
	bool		tgisconstraint;
	bool		tgdeferrable;
	bool		tginitdeferred;
	int16		tgnargs;
	int16		tgattr[FUNC_MAX_ARGS];
	char	  **tgargs;
} Trigger;

typedef struct TriggerDesc
{
	uint16		n_before_statement[4];
	uint16		n_before_row[4];
	uint16		n_after_row[4];
	uint16		n_after_statement[4];
	Trigger   **tg_before_statement[4];
	Trigger   **tg_before_row[4];
	Trigger   **tg_after_row[4];
	Trigger   **tg_after_statement[4];
	Trigger    *triggers;
} TriggerDesc;


typedef struct RelationData
{
	File		rd_fd;			/* open file descriptor */
	int			rd_nblocks;		/* number of blocks in rel */
	uint16		rd_refcnt;		/* reference count */
	bool		rd_myxactonly;	/* rel uses the local buffer mgr */
	bool		rd_isnailed;	/* rel is nailed in cache */
	bool		rd_isnoname;	/* rel has no name */
	bool		rd_unlinked;	/* rel already unlinked or not created yet */
	Form_pg_am	rd_am;			/* AM tuple */
	Form_pg_class rd_rel;		/* RELATION tuple */
	Oid			rd_id;			/* relation's object id */
	LockInfoData rd_lockInfo;	/* lock manager's info for locking relation */
	TupleDesc	rd_att;			/* tuple descriptor */
	RuleLock   *rd_rules;		/* rewrite rules */
	IndexStrategy rd_istrat;
	RegProcedure *rd_support;
	TriggerDesc *trigdesc;
} RelationData;

typedef RelationData *Relation;


/* ----------------
 *		RelationPtr is used in the executor to support index scans
 *		where we have to keep track of several index relations in an
 *		array.	-cim 9/10/89
 * ----------------
 */
typedef Relation *RelationPtr;


/*
 * RelationIsValid
 *		True iff relation descriptor is valid.
 */
#define RelationIsValid(relation) PointerIsValid(relation)

#define InvalidRelation ((Relation) NULL)

/*
 * RelationGetSystemPort
 *		Returns system port of a relation.
 *
 * Note:
 *		Assumes relation descriptor is valid.
 */
#define RelationGetSystemPort(relation) ((relation)->rd_fd)

/*
 * RelationHasReferenceCountZero
 *		True iff relation reference count is zero.
 *
 * Note:
 *		Assumes relation descriptor is valid.
 */
#define RelationHasReferenceCountZero(relation) \
		((bool)((relation)->rd_refcnt == 0))

/*
 * RelationSetReferenceCount
 *		Sets relation reference count.
 */
#define RelationSetReferenceCount(relation,count) ((relation)->rd_refcnt = (count))

/*
 * RelationIncrementReferenceCount
 *		Increments relation reference count.
 */
#define RelationIncrementReferenceCount(relation) ((relation)->rd_refcnt += 1)

/*
 * RelationDecrementReferenceCount
 *		Decrements relation reference count.
 */
#define RelationDecrementReferenceCount(relation) ((relation)->rd_refcnt -= 1)

/*
 * RelationGetForm
 *		Returns relation attribute values for a relation.
 *
 * Note:
 *		Assumes relation descriptor is valid.
 */
#define RelationGetForm(relation) ((relation)->rd_rel)

/*
 * RelationGetRelid
 *
 *	returns the object id of the relation
 *
 */
#define RelationGetRelid(relation) ((relation)->rd_id)

/*
 * RelationGetFile
 *
 *	  Returns the open File decscriptor
 */
#define RelationGetFile(relation) ((relation)->rd_fd)

/*
 * RelationGetRelationName
 *
 *	  Returns a Relation Name
 */
/* added to prevent circular dependency.  bjm 1999/11/15 */
char 	   *get_temp_rel_by_physicalname(const char *relname);
#define RelationGetRelationName(relation) \
(\
	(strncmp(RelationGetPhysicalRelationName(relation), \
	 "pg_temp.", strlen("pg_temp.")) != 0) \
	? \
		RelationGetPhysicalRelationName(relation) \
	: \
		get_temp_rel_by_physicalname( \
			RelationGetPhysicalRelationName(relation)) \
)


/*
 * RelationGetPhysicalRelationName
 *
 *	  Returns a Relation Name
 */
#define RelationGetPhysicalRelationName(relation) (NameStr((relation)->rd_rel->relname))

/*
 * RelationGetNumberOfAttributes
 *
 *	  Returns the number of attributes.
 */
#define RelationGetNumberOfAttributes(relation) ((relation)->rd_rel->relnatts)

/*
 * RelationGetDescr
 *		Returns tuple descriptor for a relation.
 */
#define RelationGetDescr(relation) ((relation)->rd_att)


extern IndexStrategy RelationGetIndexStrategy(Relation relation);

extern void RelationSetIndexSupport(Relation relation, IndexStrategy strategy,
						RegProcedure *support);

#endif	 /* REL_H */
