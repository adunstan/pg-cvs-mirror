/*-------------------------------------------------------------------------
 *
 * heap.c
 *	  code to create and destroy POSTGRES heap relations
 *
 * Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  $Header: /home/cvsmirror/pg/pgsql/src/backend/catalog/heap.c,v 1.98 1999/09/24 00:24:11 tgl Exp $
 *
 *
 * INTERFACE ROUTINES
 *		heap_create()			- Create an uncataloged heap relation
 *		heap_create_with_catalog() - Create a cataloged relation
 *		heap_destroy_with_catalog() - Removes named relation from catalogs
 *
 * NOTES
 *	  this code taken from access/heap/create.c, which contains
 *	  the old heap_create_with_catalog, amcreate, and amdestroy.
 *	  those routines will soon call these routines using the function
 *	  manager,
 *	  just like the poorly named "NewXXX" routines do.	The
 *	  "New" routines are all going to die soon, once and for all!
 *		-cim 1/13/91
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"
#include "miscadmin.h"

#include "access/heapam.h"
#include "access/genam.h"
#include "access/xact.h"
#include "catalog/catalog.h"
#include "catalog/catname.h"
#include "catalog/heap.h"
#include "catalog/index.h"
#include "catalog/indexing.h"
#include "catalog/pg_attrdef.h"
#include "catalog/pg_index.h"
#include "catalog/pg_inherits.h"
#include "catalog/pg_ipl.h"
#include "catalog/pg_proc.h"
#include "catalog/pg_relcheck.h"
#include "commands/trigger.h"
#include "optimizer/tlist.h"
#include "parser/parse_coerce.h"
#include "parser/parse_expr.h"
#include "rewrite/rewriteRemove.h"
#include "storage/smgr.h"
#include "tcop/tcopprot.h"
#include "utils/builtins.h"
#include "utils/portal.h"
#include "utils/relcache.h"
#include "utils/syscache.h"
#include "utils/temprel.h"


static void AddNewRelationTuple(Relation pg_class_desc,
				  Relation new_rel_desc, Oid new_rel_oid, unsigned natts,
					char relkind, char *temp_relname);
static void AddToNoNameRelList(Relation r);
static void DeleteAttributeTuples(Relation rel);
static void DeleteRelationTuple(Relation rel);
static void DeleteTypeTuple(Relation rel);
static void RelationRemoveIndexes(Relation relation);
static void RelationRemoveInheritance(Relation relation);
static void RemoveFromNoNameRelList(Relation r);
static void AddNewRelationType(char *typeName, Oid new_rel_oid);
static void StoreConstraints(Relation rel);
static void RemoveConstraints(Relation rel);


/* ----------------------------------------------------------------
 *				XXX UGLY HARD CODED BADNESS FOLLOWS XXX
 *
 *		these should all be moved to someplace in the lib/catalog
 *		module, if not obliterated first.
 * ----------------------------------------------------------------
 */


/*
 * Note:
 *		Should the executor special case these attributes in the future?
 *		Advantage:	consume 1/2 the space in the ATTRIBUTE relation.
 *		Disadvantage:  having rules to compute values in these tuples may
 *				be more difficult if not impossible.
 */

static FormData_pg_attribute a1 = {
	0xffffffff, {"ctid"}, TIDOID, 0, sizeof(ItemPointerData),
	SelfItemPointerAttributeNumber, 0, -1, -1, '\0', '\0', 'i', '\0', '\0'
};

static FormData_pg_attribute a2 = {
	0xffffffff, {"oid"}, OIDOID, 0, sizeof(Oid),
	ObjectIdAttributeNumber, 0, -1, -1, '\001', '\0', 'i', '\0', '\0'
};

static FormData_pg_attribute a3 = {
	0xffffffff, {"xmin"}, XIDOID, 0, sizeof(TransactionId),
	MinTransactionIdAttributeNumber, 0, -1, -1, '\001', '\0', 'i', '\0', '\0'
};

static FormData_pg_attribute a4 = {
	0xffffffff, {"cmin"}, CIDOID, 0, sizeof(CommandId),
	MinCommandIdAttributeNumber, 0, -1, -1, '\001', '\0', 'i', '\0', '\0'
};

static FormData_pg_attribute a5 = {
	0xffffffff, {"xmax"}, XIDOID, 0, sizeof(TransactionId),
	MaxTransactionIdAttributeNumber, 0, -1, -1, '\001', '\0', 'i', '\0', '\0'
};

static FormData_pg_attribute a6 = {
	0xffffffff, {"cmax"}, CIDOID, 0, sizeof(CommandId),
	MaxCommandIdAttributeNumber, 0, -1, -1, '\001', '\0', 'i', '\0', '\0'
};

static Form_pg_attribute HeapAtt[] = {&a1, &a2, &a3, &a4, &a5, &a6};

/* ----------------------------------------------------------------
 *				XXX END OF UGLY HARD CODED BADNESS XXX
 * ----------------------------------------------------------------
 */

/* the tempRelList holds
   the list of temporary uncatalogued relations that are created.
   these relations should be destroyed at the end of transactions
*/
typedef struct tempRelList
{
	Relation   *rels;			/* array of relation descriptors */
	int			num;			/* number of temporary relations */
	int			size;			/* size of space allocated for the rels
								 * array */
} TempRelList;

#define NONAME_REL_LIST_SIZE	32

static TempRelList *tempRels = NULL;


/* ----------------------------------------------------------------
 *		heap_create		- Create an uncataloged heap relation
 *
 *		Fields relpages, reltuples, reltuples, relkeys, relhistory,
 *		relisindexed, and relkind of rel->rd_rel are initialized
 *		to all zeros, as are rd_last and rd_hook.  Rd_refcnt is set to 1.
 *
 *		Remove the system relation specific code to elsewhere eventually.
 *
 *		Eventually, must place information about this temporary relation
 *		into the transaction context block.
 *
 *
 * if heap_create is called with "" as the name, then heap_create will create
 * a temporary name "pg_noname.$PID.$SEQUENCE" for the relation
 * ----------------------------------------------------------------
 */
Relation
heap_create(char *relname,
			TupleDesc tupDesc,
			bool isnoname,
			bool istemp)
{
	unsigned	i;
	Oid			relid;
	Relation	rel;
	int			len;
	bool		nailme = false;
	int			natts = tupDesc->natts;
	static unsigned int uniqueId = 0;

	extern GlobalMemory CacheCxt;
	MemoryContext oldcxt;


	/* ----------------
	 *	sanity checks
	 * ----------------
	 */
	AssertArg(natts > 0);

	if (relname && !allowSystemTableMods && IsSystemRelationName(relname) && IsNormalProcessingMode())
	{
		elog(ERROR, "Illegal class name '%s'"
			 "\n\tThe 'pg_' name prefix is reserved for system catalogs",
			 relname);
	}

	/* ----------------
	 *	switch to the cache context so that we don't lose
	 *	allocations at the end of this transaction, I guess.
	 *	-cim 6/14/90
	 * ----------------
	 */
	if (!CacheCxt)
		CacheCxt = CreateGlobalMemory("Cache");

	oldcxt = MemoryContextSwitchTo((MemoryContext) CacheCxt);

	/* ----------------
	 *	real ugly stuff to assign the proper relid in the relation
	 *	descriptor follows.
	 * ----------------
	 */
	if (relname && !strcmp(RelationRelationName, relname))
	{
		relid = RelOid_pg_class;
		nailme = true;
	}
	else if (relname && !strcmp(AttributeRelationName, relname))
	{
		relid = RelOid_pg_attribute;
		nailme = true;
	}
	else if (relname && !strcmp(ProcedureRelationName, relname))
	{
		relid = RelOid_pg_proc;
		nailme = true;
	}
	else if (relname && !strcmp(TypeRelationName, relname))
	{
		relid = RelOid_pg_type;
		nailme = true;
	}
	else
		relid = newoid();

	if (isnoname)
	{
		Assert(!relname);
		relname = palloc(NAMEDATALEN);
		snprintf(relname, NAMEDATALEN, "pg_noname.%d.%u",
				 (int) MyProcPid, uniqueId++);
	}

	if (istemp)
	{
		/* replace relname of caller */
		snprintf(relname, NAMEDATALEN, "pg_temp.%d.%u", MyProcPid, uniqueId++);
	}

	/* ----------------
	 *	allocate a new relation descriptor.
	 * ----------------
	 */
	len = sizeof(RelationData);

	rel = (Relation) palloc(len);
	MemSet((char *) rel, 0, len);

	/*
	 * create a new tuple descriptor from the one passed in
	 */
	rel->rd_att = CreateTupleDescCopyConstr(tupDesc);

	/* ----------------
	 *	nail the reldesc if this is a bootstrap create reln and
	 *	we may need it in the cache later on in the bootstrap
	 *	process so we don't ever want it kicked out.  e.g. pg_attribute!!!
	 * ----------------
	 */
	if (nailme)
		rel->rd_isnailed = true;

	RelationSetReferenceCount(rel, 1);

	rel->rd_rel = (Form_pg_class) palloc(sizeof *rel->rd_rel);

	/* ----------------
	 *	initialize the fields of our new relation descriptor
	 * ----------------
	 */
	MemSet((char *) rel->rd_rel, 0, sizeof *rel->rd_rel);
	namestrcpy(&(rel->rd_rel->relname), relname);
	rel->rd_rel->relkind = RELKIND_UNCATALOGED;
	rel->rd_rel->relnatts = natts;
	if (tupDesc->constr)
		rel->rd_rel->relchecks = tupDesc->constr->num_check;

	for (i = 0; i < natts; i++)
		rel->rd_att->attrs[i]->attrelid = relid;

	RelationGetRelid(rel) = relid;

	if (nailme)
	{
		/* for system relations, set the reltype field here */
		rel->rd_rel->reltype = relid;
	}

	/* ----------------
	 *	remember if this is a noname relation
	 * ----------------
	 */
	rel->rd_isnoname = isnoname;

	/* ----------------
	 *	have the storage manager create the relation.
	 * ----------------
	 */

	rel->rd_nonameunlinked = TRUE;		/* change once table is created */
	rel->rd_fd = (File) smgrcreate(DEFAULT_SMGR, rel);
	rel->rd_nonameunlinked = FALSE;

	RelationRegisterRelation(rel);

	MemoryContextSwitchTo(oldcxt);

	/*
	 * add all noname relations to the tempRels list so they can be
	 * properly disposed of at the end of transaction
	 */
	if (isnoname)
		AddToNoNameRelList(rel);

	return rel;
}


/* ----------------------------------------------------------------
 *		heap_create_with_catalog		- Create a cataloged relation
 *
 *		this is done in 6 steps:
 *
 *		1) CheckAttributeNames() is used to make certain the tuple
 *		   descriptor contains a valid set of attribute names
 *
 *		2) pg_class is opened and RelationFindRelid()
 *		   preforms a scan to ensure that no relation with the
 *		   same name already exists.
 *
 *		3) heap_create_with_catalog() is called to create the new relation
 *		   on disk.
 *
 *		4) TypeDefine() is called to define a new type corresponding
 *		   to the new relation.
 *
 *		5) AddNewAttributeTuples() is called to register the
 *		   new relation's schema in pg_attribute.
 *
 *		6) AddNewRelationTuple() is called to register the
 *		   relation itself in the catalogs.
 *
 *		7) StoreConstraints is called ()		- vadim 08/22/97
 *
 *		8) the relations are closed and the new relation's oid
 *		   is returned.
 *
 * old comments:
 *		A new relation is inserted into the RELATION relation
 *		with the specified attribute(s) (newly inserted into
 *		the ATTRIBUTE relation).  How does concurrency control
 *		work?  Is it automatic now?  Expects the caller to have
 *		attname, atttypid, atttyparg, attproc, and attlen domains filled.
 *		Create fills the attnum domains sequentually from zero,
 *		fills the attdisbursion domains with zeros, and fills the
 *		attrelid fields with the relid.
 *
 *		scan relation catalog for name conflict
 *		scan type catalog for typids (if not arg)
 *		create and insert attribute(s) into attribute catalog
 *		create new relation
 *		insert new relation into attribute catalog
 *
 *		Should coordinate with heap_create_with_catalog(). Either
 *		it should not be called or there should be a way to prevent
 *		the relation from being removed at the end of the
 *		transaction if it is successful ('u'/'r' may be enough).
 *		Also, if the transaction does not commit, then the
 *		relation should be removed.
 *
 *		XXX amcreate ignores "off" when inserting (for now).
 *		XXX amcreate (like the other utilities) needs to understand indexes.
 *
 * ----------------------------------------------------------------
 */

/* --------------------------------
 *		CheckAttributeNames
 *
 *		this is used to make certain the tuple descriptor contains a
 *		valid set of attribute names.  a problem simply generates
 *		elog(ERROR) which aborts the current transaction.
 * --------------------------------
 */
static void
CheckAttributeNames(TupleDesc tupdesc)
{
	unsigned	i;
	unsigned	j;
	int			natts = tupdesc->natts;

	/* ----------------
	 *	first check for collision with system attribute names
	 * ----------------
	 *
	 *	 also, warn user if attribute to be created has
	 *	 an unknown typid  (usually as a result of a 'retrieve into'
	 *	  - jolly
	 */
	for (i = 0; i < natts; i += 1)
	{
		for (j = 0; j < sizeof HeapAtt / sizeof HeapAtt[0]; j += 1)
		{
			if (nameeq(&(HeapAtt[j]->attname),
					   &(tupdesc->attrs[i]->attname)))
			{
				elog(ERROR, "Attribute '%s' has a name conflict"
					 "\n\tName matches an existing system attribute",
					 HeapAtt[j]->attname.data);
			}
		}
		if (tupdesc->attrs[i]->atttypid == UNKNOWNOID)
		{
			elog(NOTICE, "Attribute '%s' has an unknown type"
				 "\n\tRelation created; continue",
				 tupdesc->attrs[i]->attname.data);
		}
	}

	/* ----------------
	 *	next check for repeated attribute names
	 * ----------------
	 */
	for (i = 1; i < natts; i += 1)
	{
		for (j = 0; j < i; j += 1)
		{
			if (nameeq(&(tupdesc->attrs[j]->attname),
					   &(tupdesc->attrs[i]->attname)))
			{
				elog(ERROR, "Attribute '%s' is repeated",
					 tupdesc->attrs[j]->attname.data);
			}
		}
	}
}

/* --------------------------------
 *		RelnameFindRelid
 *
 *		this preforms a scan of pg_class to ensure that
 *		no relation with the same name already exists.
 * --------------------------------
 */
Oid
RelnameFindRelid(char *relname)
{
	HeapTuple	tuple;
	Oid			relid;

	/*
	 * If this is not bootstrap (initdb) time, use the catalog index on
	 * pg_class.
	 */
	if (!IsBootstrapProcessingMode())
	{
		tuple = SearchSysCacheTuple(RELNAME,
									PointerGetDatum(relname),
									0, 0, 0);
		if (HeapTupleIsValid(tuple))
			relid = tuple->t_data->t_oid;
		else
			relid = InvalidOid;
	}
	else
	{
		Relation	pg_class_desc;
		ScanKeyData key;
		HeapScanDesc pg_class_scan;

		pg_class_desc = heap_openr(RelationRelationName, AccessShareLock);

		/* ----------------
		 *	At bootstrap time, we have to do this the hard way.  Form the
		 *	scan key.
		 * ----------------
		 */
		ScanKeyEntryInitialize(&key,
							   0,
							   (AttrNumber) Anum_pg_class_relname,
							   (RegProcedure) F_NAMEEQ,
							   (Datum) relname);

		/* ----------------
		 *	begin the scan
		 * ----------------
		 */
		pg_class_scan = heap_beginscan(pg_class_desc,
									   0,
									   SnapshotNow,
									   1,
									   &key);

		/* ----------------
		 *	get a tuple.  if the tuple is NULL then it means we
		 *	didn't find an existing relation.
		 * ----------------
		 */
		tuple = heap_getnext(pg_class_scan, 0);

		if (HeapTupleIsValid(tuple))
			relid = tuple->t_data->t_oid;
		else
			relid = InvalidOid;

		heap_endscan(pg_class_scan);

		heap_close(pg_class_desc, AccessShareLock);
	}
	return relid;
}

/* --------------------------------
 *		AddNewAttributeTuples
 *
 *		this registers the new relation's schema by adding
 *		tuples to pg_attribute.
 * --------------------------------
 */
static void
AddNewAttributeTuples(Oid new_rel_oid,
					  TupleDesc tupdesc)
{
	Form_pg_attribute *dpp;
	unsigned	i;
	HeapTuple	tup;
	Relation	rel;
	bool		hasindex;
	Relation	idescs[Num_pg_attr_indices];
	int			natts = tupdesc->natts;

	/* ----------------
	 *	open pg_attribute
	 * ----------------
	 */
	rel = heap_openr(AttributeRelationName, RowExclusiveLock);

	/* -----------------
	 * Check if we have any indices defined on pg_attribute.
	 * -----------------
	 */
	hasindex = RelationGetForm(rel)->relhasindex;
	if (hasindex)
		CatalogOpenIndices(Num_pg_attr_indices, Name_pg_attr_indices, idescs);

	/* ----------------
	 *	initialize tuple descriptor.  Note we use setheapoverride()
	 *	so that we can see the effects of our TypeDefine() done
	 *	previously.
	 * ----------------
	 */
	setheapoverride(true);
	fillatt(tupdesc);
	setheapoverride(false);

	/* ----------------
	 *	first we add the user attributes..
	 * ----------------
	 */
	dpp = tupdesc->attrs;
	for (i = 0; i < natts; i++)
	{
		(*dpp)->attrelid = new_rel_oid;
		(*dpp)->attdisbursion = 0;

		tup = heap_addheader(Natts_pg_attribute,
							 ATTRIBUTE_TUPLE_SIZE,
							 (char *) *dpp);

		heap_insert(rel, tup);

		if (hasindex)
			CatalogIndexInsert(idescs, Num_pg_attr_indices, rel, tup);

		pfree(tup);
		dpp++;
	}

	/* ----------------
	 *	next we add the system attributes..
	 * ----------------
	 */
	dpp = HeapAtt;
	for (i = 0; i < -1 - FirstLowInvalidHeapAttributeNumber; i++)
	{
		(*dpp)->attrelid = new_rel_oid;
		/* (*dpp)->attdisbursion = 0;	   unneeded */

		tup = heap_addheader(Natts_pg_attribute,
							 ATTRIBUTE_TUPLE_SIZE,
							 (char *) *dpp);

		heap_insert(rel, tup);

		if (hasindex)
			CatalogIndexInsert(idescs, Num_pg_attr_indices, rel, tup);

		pfree(tup);
		dpp++;
	}

	heap_close(rel, RowExclusiveLock);

	/*
	 * close pg_attribute indices
	 */
	if (hasindex)
		CatalogCloseIndices(Num_pg_attr_indices, idescs);
}

/* --------------------------------
 *		AddNewRelationTuple
 *
 *		this registers the new relation in the catalogs by
 *		adding a tuple to pg_class.
 * --------------------------------
 */
static void
AddNewRelationTuple(Relation pg_class_desc,
					Relation new_rel_desc,
					Oid new_rel_oid,
					unsigned natts,
					char relkind,
					char *temp_relname)
{
	Form_pg_class new_rel_reltup;
	HeapTuple	tup;
	Relation	idescs[Num_pg_class_indices];
	bool		isBootstrap;

	/* ----------------
	 *	first we munge some of the information in our
	 *	uncataloged relation's relation descriptor.
	 * ----------------
	 */
	new_rel_reltup = new_rel_desc->rd_rel;

	/* CHECK should get new_rel_oid first via an insert then use XXX */

	/* ----------------
	 * Here we insert bogus estimates of the size of the new relation.
	 * In reality, of course, the new relation has 0 tuples and pages,
	 * and if we were tracking these statistics accurately then we'd
	 * set the fields that way.  But at present the stats will be updated
	 * only by VACUUM or CREATE INDEX, and the user might insert a lot of
	 * tuples before he gets around to doing either of those.  So, instead
	 * of saying the relation is empty, we insert guesstimates.  The point
	 * is to keep the optimizer from making really stupid choices on
	 * never-yet-vacuumed tables; so the estimates need only be large
	 * enough to discourage the optimizer from using nested-loop plans.
	 * With this hack, nested-loop plans will be preferred only after
	 * the table has been proven to be small by VACUUM or CREATE INDEX.
	 * Maintaining the stats on-the-fly would solve the problem more cleanly,
	 * but the overhead of that would likely cost more than it'd save.
	 * (NOTE: CREATE INDEX inserts the same bogus estimates if it finds the
	 * relation has 0 rows and pages. See index.c.)
	 * ----------------
	 */
	new_rel_reltup->relpages = 10;		/* bogus estimates */
	new_rel_reltup->reltuples = 1000;

	new_rel_reltup->relowner = GetUserId();
	new_rel_reltup->relkind = relkind;
	new_rel_reltup->relnatts = natts;

	/* ----------------
	 *	now form a tuple to add to pg_class
	 *	XXX Natts_pg_class_fixed is a hack - see pg_class.h
	 * ----------------
	 */
	tup = heap_addheader(Natts_pg_class_fixed,
						 CLASS_TUPLE_SIZE,
						 (char *) new_rel_reltup);
	tup->t_data->t_oid = new_rel_oid;

	/* ----------------
	 *	finally insert the new tuple and free it.
	 *
	 *	Note: I have no idea why we do a
	 *			SetProcessingMode(BootstrapProcessing);
	 *		  here -cim 6/14/90
	 * ----------------
	 */
	isBootstrap = IsBootstrapProcessingMode() ? true : false;

	SetProcessingMode(BootstrapProcessing);

	heap_insert(pg_class_desc, tup);

	if (temp_relname)
		create_temp_relation(temp_relname, tup);

	if (!isBootstrap)
	{

		/*
		 * First, open the catalog indices and insert index tuples for the
		 * new relation.
		 */

		CatalogOpenIndices(Num_pg_class_indices, Name_pg_class_indices, idescs);
		CatalogIndexInsert(idescs, Num_pg_class_indices, pg_class_desc, tup);
		CatalogCloseIndices(Num_pg_class_indices, idescs);
		/* now restore processing mode */
		SetProcessingMode(NormalProcessing);
	}

	pfree(tup);
}


/* --------------------------------
 *		AddNewRelationType -
 *
 *		define a complex type corresponding to the new relation
 * --------------------------------
 */
static void
AddNewRelationType(char *typeName, Oid new_rel_oid)
{
	Oid			new_type_oid;

	/*
	 * The sizes are set to oid size because it makes implementing sets
	 * MUCH easier, and no one (we hope) uses these fields to figure out
	 * how much space to allocate for the type. An oid is the type used
	 * for a set definition.  When a user requests a set, what they
	 * actually get is the oid of a tuple in the pg_proc catalog, so the
	 * size of the "set" is the size of an oid. Similarly, byval being
	 * true makes sets much easier, and it isn't used by anything else.
	 * Note the assumption that OIDs are the same size as int4s.
	 */
	new_type_oid = TypeCreate(typeName, /* type name */
							  new_rel_oid,		/* relation oid */
							  typeLen(typeidType(OIDOID)),		/* internal size */
							  typeLen(typeidType(OIDOID)),		/* external size */
							  'c',		/* type-type (catalog) */
							  ',',		/* default array delimiter */
							  "int4in", /* input procedure */
							  "int4out",		/* output procedure */
							  "int4in", /* receive procedure */
							  "int4out",		/* send procedure */
							  NULL,		/* array element type - irrelevent */
							  "-",		/* default type value */
							  (bool) 1, /* passed by value */
							  'i');		/* default alignment */
}

/* --------------------------------
 *		heap_create_with_catalog
 *
 *		creates a new cataloged relation.  see comments above.
 * --------------------------------
 */
Oid
heap_create_with_catalog(char *relname,
						 TupleDesc tupdesc,
						 char relkind,
						 bool istemp)
{
	Relation	pg_class_desc;
	Relation	new_rel_desc;
	Oid			new_rel_oid;
	int			natts = tupdesc->natts;
	char	   *temp_relname = NULL;

	/* ----------------
	 *	sanity checks
	 * ----------------
	 */
	Assert(IsNormalProcessingMode() || IsBootstrapProcessingMode());
	if (natts == 0 || natts > MaxHeapAttributeNumber)
		elog(ERROR, "Number of attributes is out of range"
			 "\n\tFrom 1 to %d attributes may be specified",
			 MaxHeapAttributeNumber);

	CheckAttributeNames(tupdesc);

	/* temp tables can mask non-temp tables */
	if ((!istemp && RelnameFindRelid(relname)) ||
		(istemp && get_temp_rel_by_name(relname) != NULL))
		elog(ERROR, "Relation '%s' already exists", relname);

	/* save user relation name because heap_create changes it */
	if (istemp)
	{
		temp_relname = pstrdup(relname);		/* save original value */
		relname = palloc(NAMEDATALEN);
		strcpy(relname, temp_relname);	/* heap_create will change this */
	}

	/* ----------------
	 *	ok, relation does not already exist so now we
	 *	create an uncataloged relation and pull its relation oid
	 *	from the newly formed relation descriptor.
	 *
	 *	Note: The call to heap_create() does all the "real" work
	 *	of creating the disk file for the relation.
	 *	This changes relname for noname and temp tables.
	 * ----------------
	 */
	new_rel_desc = heap_create(relname, tupdesc, false, istemp);

	new_rel_oid = new_rel_desc->rd_att->attrs[0]->attrelid;

	/* ----------------
	 *	since defining a relation also defines a complex type,
	 *	we add a new system type corresponding to the new relation.
	 * ----------------
	 */
	AddNewRelationType(relname, new_rel_oid);

	/* ----------------
	 *	now add tuples to pg_attribute for the attributes in
	 *	our new relation.
	 * ----------------
	 */
	AddNewAttributeTuples(new_rel_oid, tupdesc);

	/* ----------------
	 *	now update the information in pg_class.
	 * ----------------
	 */
	pg_class_desc = heap_openr(RelationRelationName, RowExclusiveLock);

	AddNewRelationTuple(pg_class_desc,
						new_rel_desc,
						new_rel_oid,
						natts,
						relkind,
						temp_relname);

	StoreConstraints(new_rel_desc);

	if (istemp)
	{
		pfree(relname);
		pfree(temp_relname);
	}

	/* ----------------
	 *	ok, the relation has been cataloged, so close our relations
	 *	and return the oid of the newly created relation.
	 *
	 *	SOMEDAY: fill the STATISTIC relation properly.
	 * ----------------
	 */
	heap_close(new_rel_desc, NoLock); /* do not unlock till end of xact */
	heap_close(pg_class_desc, RowExclusiveLock);

	return new_rel_oid;
}


/* ----------------------------------------------------------------
 *		heap_destroy_with_catalog	- removes all record of named relation from catalogs
 *
 *		1)	open relation, check for existence, etc.
 *		2)	remove inheritance information
 *		3)	remove indexes
 *		4)	remove pg_class tuple
 *		5)	remove pg_attribute tuples
 *		6)	remove pg_type tuples
 *		7)	RemoveConstraints ()
 *		8)	unlink relation
 *
 * old comments
 *		Except for vital relations, removes relation from
 *		relation catalog, and related attributes from
 *		attribute catalog (needed?).  (Anything else?)
 *
 *		get proper relation from relation catalog (if not arg)
 *		check if relation is vital (strcmp()/reltype?)
 *		scan attribute catalog deleting attributes of reldesc
 *				(necessary?)
 *		delete relation from relation catalog
 *		(How are the tuples of the relation discarded?)
 *
 *		XXX Must fix to work with indexes.
 *		There may be a better order for doing things.
 *		Problems with destroying a deleted database--cannot create
 *		a struct reldesc without having an open file descriptor.
 * ----------------------------------------------------------------
 */

/* --------------------------------
 *		RelationRemoveInheritance
 *
 *		Note: for now, we cause an exception if relation is a
 *		superclass.  Someday, we may want to allow this and merge
 *		the type info into subclass procedures....	this seems like
 *		lots of work.
 * --------------------------------
 */
static void
RelationRemoveInheritance(Relation relation)
{
	Relation	catalogRelation;
	HeapTuple	tuple;
	HeapScanDesc scan;
	ScanKeyData entry;
	bool		found = false;

	/* ----------------
	 *	open pg_inherits
	 * ----------------
	 */
	catalogRelation = heap_openr(InheritsRelationName, RowExclusiveLock);

	/* ----------------
	 *	form a scan key for the subclasses of this class
	 *	and begin scanning
	 * ----------------
	 */
	ScanKeyEntryInitialize(&entry, 0x0, Anum_pg_inherits_inhparent,
						   F_OIDEQ,
						   ObjectIdGetDatum(RelationGetRelid(relation)));

	scan = heap_beginscan(catalogRelation,
						  false,
						  SnapshotNow,
						  1,
						  &entry);

	/* ----------------
	 *	if any subclasses exist, then we disallow the deletion.
	 * ----------------
	 */
	tuple = heap_getnext(scan, 0);
	if (HeapTupleIsValid(tuple))
	{
		Oid		subclass = ((Form_pg_inherits) GETSTRUCT(tuple))->inhrel;

		heap_endscan(scan);
		heap_close(catalogRelation, RowExclusiveLock);

		elog(ERROR, "Relation '%u' inherits '%s'",
			 subclass, RelationGetRelationName(relation));
	}
	heap_endscan(scan);

	/* ----------------
	 *	If we get here, it means the relation has no subclasses
	 *	so we can trash it.  First we remove dead INHERITS tuples.
	 * ----------------
	 */
	entry.sk_attno = Anum_pg_inherits_inhrel;

	scan = heap_beginscan(catalogRelation,
						  false,
						  SnapshotNow,
						  1,
						  &entry);

	while (HeapTupleIsValid(tuple = heap_getnext(scan, 0)))
	{
		heap_delete(catalogRelation, &tuple->t_self, NULL);
		found = true;
	}

	heap_endscan(scan);
	heap_close(catalogRelation, RowExclusiveLock);

	/* ----------------
	 *	now remove dead IPL tuples
	 * ----------------
	 */
	catalogRelation = heap_openr(InheritancePrecidenceListRelationName,
								 RowExclusiveLock);

	entry.sk_attno = Anum_pg_ipl_iplrel;

	scan = heap_beginscan(catalogRelation,
						  false,
						  SnapshotNow,
						  1,
						  &entry);

	while (HeapTupleIsValid(tuple = heap_getnext(scan, 0)))
		heap_delete(catalogRelation, &tuple->t_self, NULL);

	heap_endscan(scan);
	heap_close(catalogRelation, RowExclusiveLock);
}

/* --------------------------------
 *		RelationRemoveIndexes
 *
 * --------------------------------
 */
static void
RelationRemoveIndexes(Relation relation)
{
	Relation	indexRelation;
	HeapTuple	tuple;
	HeapScanDesc scan;
	ScanKeyData entry;

	indexRelation = heap_openr(IndexRelationName, RowExclusiveLock);

	ScanKeyEntryInitialize(&entry, 0x0, Anum_pg_index_indrelid,
						   F_OIDEQ,
						   ObjectIdGetDatum(RelationGetRelid(relation)));

	scan = heap_beginscan(indexRelation,
						  false,
						  SnapshotNow,
						  1,
						  &entry);

	while (HeapTupleIsValid(tuple = heap_getnext(scan, 0)))
		index_destroy(((Form_pg_index) GETSTRUCT(tuple))->indexrelid);

	heap_endscan(scan);
	heap_close(indexRelation, RowExclusiveLock);
}

/* --------------------------------
 *		DeleteRelationTuple
 *
 * --------------------------------
 */
static void
DeleteRelationTuple(Relation rel)
{
	Relation	pg_class_desc;
	HeapTuple	tup;

	/* ----------------
	 *	open pg_class
	 * ----------------
	 */
	pg_class_desc = heap_openr(RelationRelationName, RowExclusiveLock);

	tup = SearchSysCacheTupleCopy(RELOID,
					   ObjectIdGetDatum(rel->rd_att->attrs[0]->attrelid),
								  0, 0, 0);
	if (!HeapTupleIsValid(tup))
	{
		heap_close(pg_class_desc, RowExclusiveLock);
		elog(ERROR, "Relation '%s' does not exist",
			 &rel->rd_rel->relname);
	}

	/* ----------------
	 *	delete the relation tuple from pg_class, and finish up.
	 * ----------------
	 */
	heap_delete(pg_class_desc, &tup->t_self, NULL);
	pfree(tup);

	heap_close(pg_class_desc, RowExclusiveLock);
}

/* --------------------------------
 * RelationTruncateIndexes - This routine is used to truncate all
 * indices associated with the heap relation to zero tuples.
 * The routine will truncate and then reconstruct the indices on
 * the relation specified by the heapRelation parameter.
 * --------------------------------
 */
static void
RelationTruncateIndexes(Relation heapRelation)
{
	Relation indexRelation, currentIndex;
	ScanKeyData entry;
	HeapScanDesc scan;
	HeapTuple indexTuple, procTuple, classTuple;
	Form_pg_index index;
	Oid heapId, indexId, procId, accessMethodId;
	Node *oldPred = NULL;
	PredInfo *predInfo;
	List *cnfPred = NULL;
	AttrNumber *attributeNumberA;
	FuncIndexInfo fInfo, *funcInfo = NULL;
	int i, numberOfAttributes;
	char *predString;

	heapId = RelationGetRelid(heapRelation);

	/* Scan pg_index to find indexes on heapRelation */

	indexRelation = heap_openr(IndexRelationName, AccessShareLock);
	ScanKeyEntryInitialize(&entry, 0, Anum_pg_index_indrelid, F_OIDEQ,
						   ObjectIdGetDatum(heapId));
	scan = heap_beginscan(indexRelation, false, SnapshotNow, 1, &entry);
	while (HeapTupleIsValid(indexTuple = heap_getnext(scan, 0)))
	{
		/*
		 * For each index, fetch index attributes so we can apply index_build
		 */
		index = (Form_pg_index) GETSTRUCT(indexTuple);
		indexId = index->indexrelid;
		procId = index->indproc;

		for (i = 0; i < INDEX_MAX_KEYS; i++)
		{
			if (index->indkey[i] == InvalidAttrNumber)
				break;
		}
		numberOfAttributes = i;

		/* If a valid where predicate, compute predicate Node */
		if (VARSIZE(&index->indpred) != 0)
		{
			predString = fmgr(F_TEXTOUT, &index->indpred);
			oldPred = stringToNode(predString);
			pfree(predString);
		}
		predInfo = (PredInfo *) palloc(sizeof(PredInfo));
		predInfo->pred = (Node *) cnfPred;
		predInfo->oldPred = oldPred;

		/* Assign Index keys to attributes array */
		attributeNumberA = (AttrNumber *) palloc(numberOfAttributes *
												 sizeof(AttrNumber));
		for (i = 0; i < numberOfAttributes; i++)
			attributeNumberA[i] = index->indkey[i];

		/* If this is a procedural index, initialize our FuncIndexInfo */
		if (procId != InvalidOid)
		{
			funcInfo = &fInfo;
			FIsetnArgs(funcInfo, numberOfAttributes);
			procTuple = SearchSysCacheTuple(PROOID, ObjectIdGetDatum(procId),
											0, 0, 0);
			if (!HeapTupleIsValid(procTuple))
				elog(ERROR, "RelationTruncateIndexes: index procedure not found");
			namecpy(&(funcInfo->funcName),
					&(((Form_pg_proc) GETSTRUCT(procTuple))->proname));
			FIsetProcOid(funcInfo, procTuple->t_data->t_oid);
		}

		/* Fetch the classTuple associated with this index */
		classTuple = SearchSysCacheTupleCopy(RELOID, ObjectIdGetDatum(indexId),
											 0, 0, 0);
		if (!HeapTupleIsValid(classTuple))
			elog(ERROR, "RelationTruncateIndexes: index access method not found");
		accessMethodId = ((Form_pg_class) GETSTRUCT(classTuple))->relam;

		/* Open our index relation */
		currentIndex = index_open(indexId);
		if (currentIndex == NULL)
			elog(ERROR, "RelationTruncateIndexes: can't open index relation");

		/* Obtain exclusive lock on it, just to be sure */
		LockRelation(currentIndex, AccessExclusiveLock);

		/*
		 * Release any buffers associated with this index.  If they're dirty,
		 * they're just dropped without bothering to flush to disk.
		 */
		ReleaseRelationBuffers(currentIndex);
		if (FlushRelationBuffers(currentIndex, (BlockNumber) 0, false) < 0)
			elog(ERROR, "RelationTruncateIndexes: unable to flush index from buffer pool");

		/* Now truncate the actual data and set blocks to zero */
		smgrtruncate(DEFAULT_SMGR, currentIndex, 0);
		currentIndex->rd_nblocks = 0;

		/* Initialize the index and rebuild */
		InitIndexStrategy(numberOfAttributes, currentIndex, accessMethodId);
		index_build(heapRelation, currentIndex, numberOfAttributes,
					attributeNumberA, 0, NULL, funcInfo, predInfo);

		/*
		 * index_build will close both the heap and index relations
		 * (but not give up the locks we hold on them).  That's fine
		 * for the index, but we need to open the heap again.  We need
		 * no new lock, since this backend still has the exclusive lock
		 * grabbed by heap_truncate.
		 */
		heapRelation = heap_open(heapId, NoLock);
		Assert(heapRelation != NULL);
	}

	/* Complete the scan and close pg_index */
    heap_endscan(scan);
	heap_close(indexRelation, AccessShareLock);
}

/* ----------------------------
 *   heap_truncate
 *
 *   This routine is used to truncate the data from the
 *   storage manager of any data within the relation handed
 *   to this routine.
 * ----------------------------
 */

void
heap_truncate(char *relname)
{
	Relation rel;
	Oid rid;

	/* Open relation for processing, and grab exclusive access on it. */

	rel = heap_openr(relname, AccessExclusiveLock);
	rid = rel->rd_id;

	/* ----------------
	 *	TRUNCATE TABLE within a transaction block is dangerous, because
	 *	if the transaction is later rolled back we have no way to
	 *	undo truncation of the relation's physical file.  For now, allow it
	 *	but emit a warning message.
	 *	Someday we might want to consider postponing the physical truncate
	 *	until transaction commit, but that's a lot of work...
	 *	The only case that actually works right is for relations created
	 *	in the current transaction, since the post-abort state would be that
	 *	they don't exist anyway.  So, no warning in that case.
	 * ----------------
	 */
	if (IsTransactionBlock() && ! rel->rd_myxactonly)
		elog(NOTICE, "Caution: TRUNCATE TABLE cannot be rolled back, so don't abort now");

	/*
	 * Release any buffers associated with this relation.  If they're dirty,
	 * they're just dropped without bothering to flush to disk.
	 */

	ReleaseRelationBuffers(rel);
	if (FlushRelationBuffers(rel, (BlockNumber) 0, false) < 0)
		elog(ERROR, "heap_truncate: unable to flush relation from buffer pool");

	/* Now truncate the actual data and set blocks to zero */

	smgrtruncate(DEFAULT_SMGR, rel, 0);
	rel->rd_nblocks = 0;

	/* If this relation has indexes, truncate the indexes too */
	if (rel->rd_rel->relhasindex)
		RelationTruncateIndexes(rel);

	/*
	 * Close the relation, but keep exclusive lock on it until commit.
	 */
	heap_close(rel, NoLock);

	/*
	 * Is this really necessary?
	 */
	RelationForgetRelation(rid);
}


/* --------------------------------
 *		DeleteAttributeTuples
 *
 * --------------------------------
 */
static void
DeleteAttributeTuples(Relation rel)
{
	Relation	pg_attribute_desc;
	HeapTuple	tup;
	int2		attnum;

	/* ----------------
	 *	open pg_attribute
	 * ----------------
	 */
	pg_attribute_desc = heap_openr(AttributeRelationName, RowExclusiveLock);

	for (attnum = FirstLowInvalidHeapAttributeNumber + 1;
		 attnum <= rel->rd_att->natts;
		 attnum++)
	{
		if (HeapTupleIsValid(tup = SearchSysCacheTupleCopy(ATTNUM,
								 ObjectIdGetDatum(RelationGetRelid(rel)),
												   Int16GetDatum(attnum),
														   0, 0)))
		{
			heap_delete(pg_attribute_desc, &tup->t_self, NULL);
			pfree(tup);
		}
	}

	heap_close(pg_attribute_desc, RowExclusiveLock);
}

/* --------------------------------
 *		DeleteTypeTuple
 *
 *		If the user attempts to destroy a relation and there
 *		exists attributes in other relations of type
 *		"relation we are deleting", then we have to do something
 *		special.  presently we disallow the destroy.
 * --------------------------------
 */
static void
DeleteTypeTuple(Relation rel)
{
	Relation	pg_type_desc;
	HeapScanDesc pg_type_scan;
	Relation	pg_attribute_desc;
	HeapScanDesc pg_attribute_scan;
	ScanKeyData key;
	ScanKeyData attkey;
	HeapTuple	tup;
	HeapTuple	atttup;
	Oid			typoid;

	/* ----------------
	 *	open pg_type
	 * ----------------
	 */
	pg_type_desc = heap_openr(TypeRelationName, RowExclusiveLock);

	/* ----------------
	 *	create a scan key to locate the type tuple corresponding
	 *	to this relation.
	 * ----------------
	 */
	ScanKeyEntryInitialize(&key, 0,
						   Anum_pg_type_typrelid,
						   F_OIDEQ,
						   ObjectIdGetDatum(RelationGetRelid(rel)));

	pg_type_scan = heap_beginscan(pg_type_desc,
								  0,
								  SnapshotNow,
								  1,
								  &key);

	/* ----------------
	 *	use heap_getnext() to fetch the pg_type tuple.	If this
	 *	tuple is not valid then something's wrong.
	 * ----------------
	 */
	tup = heap_getnext(pg_type_scan, 0);

	if (!HeapTupleIsValid(tup))
	{
		heap_endscan(pg_type_scan);
		heap_close(pg_type_desc, RowExclusiveLock);
		elog(ERROR, "DeleteTypeTuple: %s type nonexistent",
			 &rel->rd_rel->relname);
	}

	/* ----------------
	 *	now scan pg_attribute.	if any other relations have
	 *	attributes of the type of the relation we are deleteing
	 *	then we have to disallow the deletion.	should talk to
	 *	stonebraker about this.  -cim 6/19/90
	 * ----------------
	 */
	typoid = tup->t_data->t_oid;

	pg_attribute_desc = heap_openr(AttributeRelationName, RowExclusiveLock);

	ScanKeyEntryInitialize(&attkey,
						   0,
						   Anum_pg_attribute_atttypid,
						   F_OIDEQ,
						   typoid);

	pg_attribute_scan = heap_beginscan(pg_attribute_desc,
									   0,
									   SnapshotNow,
									   1,
									   &attkey);

	/* ----------------
	 *	try and get a pg_attribute tuple.  if we succeed it means
	 *	we can't delete the relation because something depends on
	 *	the schema.
	 * ----------------
	 */
	atttup = heap_getnext(pg_attribute_scan, 0);

	if (HeapTupleIsValid(atttup))
	{
		Oid			relid = ((Form_pg_attribute) GETSTRUCT(atttup))->attrelid;

		heap_endscan(pg_attribute_scan);
		heap_close(pg_attribute_desc, RowExclusiveLock);
		heap_endscan(pg_type_scan);
		heap_close(pg_type_desc, RowExclusiveLock);

		elog(ERROR, "DeleteTypeTuple: att of type %s exists in relation %u",
			 &rel->rd_rel->relname, relid);
	}
	heap_endscan(pg_attribute_scan);
	heap_close(pg_attribute_desc, RowExclusiveLock);

	/* ----------------
	 *	Ok, it's safe so we delete the relation tuple
	 *	from pg_type and finish up.  But first end the scan so that
	 *	we release the read lock on pg_type.  -mer 13 Aug 1991
	 * ----------------
	 */
	heap_delete(pg_type_desc, &tup->t_self, NULL);

	heap_endscan(pg_type_scan);
	heap_close(pg_type_desc, RowExclusiveLock);
}

/* --------------------------------
 *		heap_destroy_with_catalog
 *
 * --------------------------------
 */
void
heap_destroy_with_catalog(char *relname)
{
	Relation	rel;
	Oid			rid;
	bool		istemp = (get_temp_rel_by_name(relname) != NULL);

	/* ----------------
	 *	Open and lock the relation.
	 * ----------------
	 */
	rel = heap_openr(relname, AccessExclusiveLock);
	rid = rel->rd_id;

	/* ----------------
	 *	prevent deletion of system relations
	 * ----------------
	 */
	/* allow temp of pg_class? Guess so. */
	if (!istemp && !allowSystemTableMods &&
		IsSystemRelationName(RelationGetRelationName(rel)->data))
		elog(ERROR, "System relation '%s' cannot be destroyed",
			 &rel->rd_rel->relname);

	/* ----------------
	 *	DROP TABLE within a transaction block is dangerous, because
	 *	if the transaction is later rolled back there will be no way to
	 *	undo the unlink of the relation's physical file.  For now, allow it
	 *	but emit a warning message.
	 *	Someday we might want to consider postponing the physical unlink
	 *	until transaction commit, but that's a lot of work...
	 *	The only case that actually works right is for relations created
	 *	in the current transaction, since the post-abort state would be that
	 *	they don't exist anyway.  So, no warning in that case.
	 * ----------------
	 */
	if (IsTransactionBlock() && ! rel->rd_myxactonly)
		elog(NOTICE, "Caution: DROP TABLE cannot be rolled back, so don't abort now");

	/* ----------------
	 *	remove inheritance information
	 * ----------------
	 */
	RelationRemoveInheritance(rel);

	/* ----------------
	 *	remove indexes if necessary
	 * ----------------
	 */
	if (rel->rd_rel->relhasindex)
		RelationRemoveIndexes(rel);

	/* ----------------
	 *	remove rules if necessary
	 * ----------------
	 */
	if (rel->rd_rules != NULL)
		RelationRemoveRules(rid);

	/* triggers */
	if (rel->rd_rel->reltriggers > 0)
		RelationRemoveTriggers(rel);

	/* ----------------
	 *	delete attribute tuples
	 * ----------------
	 */
	DeleteAttributeTuples(rel);

	if (istemp)
		remove_temp_relation(rid);

	/* ----------------
	 *	delete type tuple.	here we want to see the effects
	 *	of the deletions we just did, so we use setheapoverride().
	 * ----------------
	 */
	setheapoverride(true);
	DeleteTypeTuple(rel);
	setheapoverride(false);

	/* ----------------
	 *	delete relation tuple
	 * ----------------
	 */
	/* must delete fake tuple in cache */
	DeleteRelationTuple(rel);

	/*
	 * release dirty buffers of this relation
	 */
	ReleaseRelationBuffers(rel);

	RemoveConstraints(rel);

	/* ----------------
	 *	unlink the relation's physical file and finish up.
	 * ----------------
	 */
	if (!(rel->rd_isnoname) || !(rel->rd_nonameunlinked))
		smgrunlink(DEFAULT_SMGR, rel);

	rel->rd_nonameunlinked = TRUE;

	/*
	 * Close relcache entry, but *keep* AccessExclusiveLock on the
	 * relation until transaction commit.  This ensures no one else
	 * will try to do something with the doomed relation.
	 */
	heap_close(rel, NoLock);

	/* ----------------
	 *	flush the relation from the relcache
	 * ----------------
	 */
	RelationForgetRelation(rid);
}

/*
 * heap_destroy
 *	  destroy and close temporary relations
 *
 */

void
heap_destroy(Relation rel)
{
	ReleaseRelationBuffers(rel);
	if (!(rel->rd_isnoname) || !(rel->rd_nonameunlinked))
		smgrunlink(DEFAULT_SMGR, rel);
	rel->rd_nonameunlinked = TRUE;
	heap_close(rel, NoLock);
	RemoveFromNoNameRelList(rel);
}


/**************************************************************
  functions to deal with the list of temporary relations
**************************************************************/

/* --------------
   InitTempRellist():

   initialize temporary relations list
   the tempRelList is a list of temporary relations that
   are created in the course of the transactions
   they need to be destroyed properly at the end of the transactions

   MODIFIES the global variable tempRels

 >> NOTE <<

   malloc is used instead of palloc because we KNOW when we are
   going to free these things.	Keeps us away from the memory context
   hairyness

*/
void
InitNoNameRelList(void)
{
	if (tempRels)
	{
		free(tempRels->rels);
		free(tempRels);
	}

	tempRels = (TempRelList *) malloc(sizeof(TempRelList));
	tempRels->size = NONAME_REL_LIST_SIZE;
	tempRels->rels = (Relation *) malloc(sizeof(Relation) * tempRels->size);
	MemSet(tempRels->rels, 0, sizeof(Relation) * tempRels->size);
	tempRels->num = 0;
}

/*
   removes a relation from the TempRelList

   MODIFIES the global variable tempRels
	  we don't really remove it, just mark it as NULL
	  and DestroyNoNameRels will look for NULLs
*/
static void
RemoveFromNoNameRelList(Relation r)
{
	int			i;

	if (!tempRels)
		return;

	for (i = 0; i < tempRels->num; i++)
	{
		if (tempRels->rels[i] == r)
		{
			tempRels->rels[i] = NULL;
			break;
		}
	}
}

/*
   add a temporary relation to the TempRelList

   MODIFIES the global variable tempRels
*/
static void
AddToNoNameRelList(Relation r)
{
	if (!tempRels)
		return;

	if (tempRels->num == tempRels->size)
	{
		tempRels->size += NONAME_REL_LIST_SIZE;
		tempRels->rels = realloc(tempRels->rels,
								 sizeof(Relation) * tempRels->size);
	}
	tempRels->rels[tempRels->num] = r;
	tempRels->num++;
}

/*
   go through the tempRels list and destroy each of the relations
*/
void
DestroyNoNameRels(void)
{
	int			i;
	Relation	rel;

	if (!tempRels)
		return;

	for (i = 0; i < tempRels->num; i++)
	{
		rel = tempRels->rels[i];
		/* rel may be NULL if it has been removed from the list already */
		if (rel)
			heap_destroy(rel);
	}
	free(tempRels->rels);
	free(tempRels);
	tempRels = NULL;
}


static void
StoreAttrDefault(Relation rel, AttrDefault *attrdef)
{
	char		str[MAX_PARSE_BUFFER];
	char		cast[2 * NAMEDATALEN] = {0};
	Form_pg_attribute atp = rel->rd_att->attrs[attrdef->adnum - 1];
	List	   *queryTree_list;
	List	   *planTree_list;
	Query	   *query;
	TargetEntry *te;
	Resdom	   *resdom;
	Node	   *expr;
	Oid			type;
	char	   *adbin;
	MemoryContext oldcxt;
	Relation	adrel;
	Relation	idescs[Num_pg_attrdef_indices];
	HeapTuple	tuple;
	Datum		values[4];
	char		nulls[4] = {' ', ' ', ' ', ' '};
	extern GlobalMemory CacheCxt;

start:

	/*
	 * Surround table name with double quotes to allow mixed-case and
	 * whitespaces in names. - BGA 1998-11-14
	 */
	snprintf(str, MAX_PARSE_BUFFER,
			 "select %s%s from \"%.*s\"", attrdef->adsrc, cast,
			 NAMEDATALEN, rel->rd_rel->relname.data);
	setheapoverride(true);
	planTree_list = pg_parse_and_plan(str, NULL, 0,
									  &queryTree_list, None, FALSE);
	setheapoverride(false);
	query = (Query *) lfirst(queryTree_list);

	if (length(query->rtable) > 1 ||
		flatten_tlist(query->targetList) != NIL)
		elog(ERROR, "Cannot use attribute(s) in DEFAULT clause");
	te = (TargetEntry *) lfirst(query->targetList);
	resdom = te->resdom;
	expr = te->expr;
	type = exprType(expr);

	if (type != atp->atttypid)
	{
		if (IS_BINARY_COMPATIBLE(type, atp->atttypid))
			;					/* use without change */
		else if (can_coerce_type(1, &(type), &(atp->atttypid)))
			expr = coerce_type(NULL, (Node *) expr, type, atp->atttypid,
							   atp->atttypmod);
		else if (IsA(expr, Const))
		{
			if (*cast != 0)
				elog(ERROR, "DEFAULT clause const type '%s' mismatched with column type '%s'",
					 typeidTypeName(type), typeidTypeName(atp->atttypid));
			snprintf(cast, 2 * NAMEDATALEN, ":: %s", typeidTypeName(atp->atttypid));
			goto start;
		}
		else
			elog(ERROR, "DEFAULT clause type '%s' mismatched with column type '%s'",
				 typeidTypeName(type), typeidTypeName(atp->atttypid));
	}

	adbin = nodeToString(expr);
	oldcxt = MemoryContextSwitchTo((MemoryContext) CacheCxt);
	attrdef->adbin = pstrdup(adbin);
	(void) MemoryContextSwitchTo(oldcxt);
	pfree(adbin);

	values[Anum_pg_attrdef_adrelid - 1] = rel->rd_id;
	values[Anum_pg_attrdef_adnum - 1] = attrdef->adnum;
	values[Anum_pg_attrdef_adbin - 1] = PointerGetDatum(textin(attrdef->adbin));
	values[Anum_pg_attrdef_adsrc - 1] = PointerGetDatum(textin(attrdef->adsrc));
	adrel = heap_openr(AttrDefaultRelationName, RowExclusiveLock);
	tuple = heap_formtuple(adrel->rd_att, values, nulls);
	CatalogOpenIndices(Num_pg_attrdef_indices, Name_pg_attrdef_indices, idescs);
	heap_insert(adrel, tuple);
	CatalogIndexInsert(idescs, Num_pg_attrdef_indices, adrel, tuple);
	CatalogCloseIndices(Num_pg_attrdef_indices, idescs);
	heap_close(adrel, RowExclusiveLock);

	pfree(DatumGetPointer(values[Anum_pg_attrdef_adbin - 1]));
	pfree(DatumGetPointer(values[Anum_pg_attrdef_adsrc - 1]));
	pfree(tuple);

}

static void
StoreRelCheck(Relation rel, ConstrCheck *check)
{
	char		str[MAX_PARSE_BUFFER];
	List	   *queryTree_list;
	List	   *planTree_list;
	Query	   *query;
	Plan	   *plan;
	List	   *qual;
	char	   *ccbin;
	MemoryContext oldcxt;
	Relation	rcrel;
	Relation	idescs[Num_pg_relcheck_indices];
	HeapTuple	tuple;
	Datum		values[4];
	char		nulls[4] = {' ', ' ', ' ', ' '};
	extern GlobalMemory CacheCxt;

	/*
	 * Check for table's existance. Surround table name with double-quotes
	 * to allow mixed-case and whitespace names. - thomas 1998-11-12
	 */
	snprintf(str, MAX_PARSE_BUFFER,
			 "select 1 from \"%.*s\" where %s",
			 NAMEDATALEN, rel->rd_rel->relname.data, check->ccsrc);
	setheapoverride(true);
	planTree_list = pg_parse_and_plan(str, NULL, 0,
									  &queryTree_list, None, FALSE);
	setheapoverride(false);
	query = (Query *) lfirst(queryTree_list);

	if (length(query->rtable) > 1)
		elog(ERROR, "Only relation '%.*s' can be referenced",
			 NAMEDATALEN, rel->rd_rel->relname.data);

	plan = (Plan *) lfirst(planTree_list);
	qual = plan->qual;

	ccbin = nodeToString(qual);
	oldcxt = MemoryContextSwitchTo((MemoryContext) CacheCxt);
	check->ccbin = (char *) palloc(strlen(ccbin) + 1);
	strcpy(check->ccbin, ccbin);
	(void) MemoryContextSwitchTo(oldcxt);
	pfree(ccbin);

	values[Anum_pg_relcheck_rcrelid - 1] = rel->rd_id;
	values[Anum_pg_relcheck_rcname - 1] = PointerGetDatum(namein(check->ccname));
	values[Anum_pg_relcheck_rcbin - 1] = PointerGetDatum(textin(check->ccbin));
	values[Anum_pg_relcheck_rcsrc - 1] = PointerGetDatum(textin(check->ccsrc));
	rcrel = heap_openr(RelCheckRelationName, RowExclusiveLock);
	tuple = heap_formtuple(rcrel->rd_att, values, nulls);
	CatalogOpenIndices(Num_pg_relcheck_indices, Name_pg_relcheck_indices, idescs);
	heap_insert(rcrel, tuple);
	CatalogIndexInsert(idescs, Num_pg_relcheck_indices, rcrel, tuple);
	CatalogCloseIndices(Num_pg_relcheck_indices, idescs);
	heap_close(rcrel, RowExclusiveLock);

	pfree(DatumGetPointer(values[Anum_pg_relcheck_rcname - 1]));
	pfree(DatumGetPointer(values[Anum_pg_relcheck_rcbin - 1]));
	pfree(DatumGetPointer(values[Anum_pg_relcheck_rcsrc - 1]));
	pfree(tuple);
}

static void
StoreConstraints(Relation rel)
{
	TupleConstr *constr = rel->rd_att->constr;
	int			i;

	if (!constr)
		return;

	if (constr->num_defval > 0)
	{
		for (i = 0; i < constr->num_defval; i++)
			StoreAttrDefault(rel, &(constr->defval[i]));
	}

	if (constr->num_check > 0)
	{
		for (i = 0; i < constr->num_check; i++)
			StoreRelCheck(rel, &(constr->check[i]));
	}

	return;
}

static void
RemoveAttrDefault(Relation rel)
{
	Relation	adrel;
	HeapScanDesc adscan;
	ScanKeyData key;
	HeapTuple	tup;

	adrel = heap_openr(AttrDefaultRelationName, RowExclusiveLock);

	ScanKeyEntryInitialize(&key, 0, Anum_pg_attrdef_adrelid,
						   F_OIDEQ, rel->rd_id);

	adscan = heap_beginscan(adrel, 0, SnapshotNow, 1, &key);

	while (HeapTupleIsValid(tup = heap_getnext(adscan, 0)))
		heap_delete(adrel, &tup->t_self, NULL);

	heap_endscan(adscan);
	heap_close(adrel, RowExclusiveLock);
}

static void
RemoveRelCheck(Relation rel)
{
	Relation	rcrel;
	HeapScanDesc rcscan;
	ScanKeyData key;
	HeapTuple	tup;

	rcrel = heap_openr(RelCheckRelationName, RowExclusiveLock);

	ScanKeyEntryInitialize(&key, 0, Anum_pg_relcheck_rcrelid,
						   F_OIDEQ, rel->rd_id);

	rcscan = heap_beginscan(rcrel, 0, SnapshotNow, 1, &key);

	while (HeapTupleIsValid(tup = heap_getnext(rcscan, 0)))
		heap_delete(rcrel, &tup->t_self, NULL);

	heap_endscan(rcscan);
	heap_close(rcrel, RowExclusiveLock);
}

static void
RemoveConstraints(Relation rel)
{
	TupleConstr *constr = rel->rd_att->constr;

	if (!constr)
		return;

	if (constr->num_defval > 0)
		RemoveAttrDefault(rel);

	if (constr->num_check > 0)
		RemoveRelCheck(rel);
}
