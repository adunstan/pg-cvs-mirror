/*-------------------------------------------------------------------------
 *
 * pg_depend.c
 *	  routines to support manipulation of the pg_depend relation
 *
 * Portions Copyright (c) 1996-2002, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  $Header: /home/cvsmirror/pg/pgsql/src/backend/catalog/pg_depend.c,v 1.1 2002/07/12 18:43:15 tgl Exp $
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include "access/genam.h"
#include "access/heapam.h"
#include "catalog/catname.h"
#include "catalog/indexing.h"
#include "catalog/dependency.h"
#include "catalog/pg_depend.h"
#include "miscadmin.h"
#include "utils/fmgroids.h"


static bool isObjectPinned(const ObjectAddress *object, Relation rel);


/*
 * Record a dependency between 2 objects via their respective objectAddress.
 * The first argument is the dependent object, the second the one it
 * references.
 *
 * This simply creates an entry in pg_depend, without any other processing.
 */
void
recordDependencyOn(const ObjectAddress *depender,
				   const ObjectAddress *referenced,
				   DependencyType behavior)
{
	Relation	dependDesc;
	HeapTuple	tup;
	int			i;
	char		nulls[Natts_pg_depend];
	Datum		values[Natts_pg_depend];
	Relation	idescs[Num_pg_depend_indices];

	/*
	 * During bootstrap, do nothing since pg_depend may not exist yet.
	 * initdb will fill in appropriate pg_depend entries after bootstrap.
	 */
	if (IsBootstrapProcessingMode())
		return;

	dependDesc = heap_openr(DependRelationName, RowExclusiveLock);

	/*
	 * If the referenced object is pinned by the system, there's no real
	 * need to record dependencies on it.  This saves lots of space in
	 * pg_depend, so it's worth the time taken to check.
	 */
	if (!isObjectPinned(referenced, dependDesc))
	{
		/*
		 * Record the Dependency.  Note we don't bother to check for
		 * duplicate dependencies; there's no harm in them.
		 */
		for (i = 0; i < Natts_pg_depend; ++i)
		{
			nulls[i] = ' ';
			values[i] = (Datum) 0;
		}

		values[Anum_pg_depend_classid - 1]	= ObjectIdGetDatum(depender->classId);
		values[Anum_pg_depend_objid - 1]	= ObjectIdGetDatum(depender->objectId);
		values[Anum_pg_depend_objsubid - 1]	= Int32GetDatum(depender->objectSubId);

		values[Anum_pg_depend_refclassid - 1]	= ObjectIdGetDatum(referenced->classId);
		values[Anum_pg_depend_refobjid - 1]		= ObjectIdGetDatum(referenced->objectId);
		values[Anum_pg_depend_refobjsubid - 1]	= Int32GetDatum(referenced->objectSubId);

		values[Anum_pg_depend_deptype -1] = CharGetDatum((char) behavior);

		tup = heap_formtuple(dependDesc->rd_att, values, nulls);

		simple_heap_insert(dependDesc, tup);

		/*
		 * Keep indices current
		 */
		CatalogOpenIndices(Num_pg_depend_indices, Name_pg_depend_indices, idescs);
		CatalogIndexInsert(idescs, Num_pg_depend_indices, dependDesc, tup);
		CatalogCloseIndices(Num_pg_depend_indices, idescs);
	}

	heap_close(dependDesc, RowExclusiveLock);
}


/*
 * isObjectPinned()
 *
 * Test if an object is required for basic database functionality.
 * Caller must already have opened pg_depend.
 *
 * The passed subId, if any, is ignored; we assume that only whole objects
 * are pinned (and that this implies pinning their components).
 */
static bool
isObjectPinned(const ObjectAddress *object, Relation rel)
{
	bool		ret = false;
	SysScanDesc	scan;
	HeapTuple	tup;
	ScanKeyData key[2];

	ScanKeyEntryInitialize(&key[0], 0x0,
						   Anum_pg_depend_refclassid, F_OIDEQ,
						   ObjectIdGetDatum(object->classId));

	ScanKeyEntryInitialize(&key[1], 0x0,
						   Anum_pg_depend_refobjid, F_OIDEQ,
						   ObjectIdGetDatum(object->objectId));

	scan = systable_beginscan(rel, DependReferenceIndex, true,
							  SnapshotNow, 2, key);

	/*
	 * Since we won't generate additional pg_depend entries for pinned
	 * objects, there can be at most one entry referencing a pinned
	 * object.  Hence, it's sufficient to look at the first returned
	 * tuple; we don't need to loop.
	 */
	tup = systable_getnext(scan);
	if (HeapTupleIsValid(tup))
	{
		Form_pg_depend	foundDep = (Form_pg_depend) GETSTRUCT(tup);

		if (foundDep->deptype == DEPENDENCY_PIN)
			ret = true;
	}

	systable_endscan(scan);

	return ret;
}
