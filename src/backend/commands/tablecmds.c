/*-------------------------------------------------------------------------
 *
 * tablecmds.c
 *	  Commands for creating and altering table structures and settings
 *
 * Portions Copyright (c) 1996-2002, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  $Header: /home/cvsmirror/pg/pgsql/src/backend/commands/tablecmds.c,v 1.41 2002/09/12 21:16:42 tgl Exp $
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include "access/genam.h"
#include "access/tuptoaster.h"
#include "catalog/catalog.h"
#include "catalog/catname.h"
#include "catalog/dependency.h"
#include "catalog/heap.h"
#include "catalog/index.h"
#include "catalog/indexing.h"
#include "catalog/namespace.h"
#include "catalog/pg_constraint.h"
#include "catalog/pg_inherits.h"
#include "catalog/pg_namespace.h"
#include "catalog/pg_opclass.h"
#include "catalog/pg_trigger.h"
#include "catalog/pg_type.h"
#include "commands/tablecmds.h"
#include "commands/trigger.h"
#include "executor/executor.h"
#include "miscadmin.h"
#include "nodes/makefuncs.h"
#include "optimizer/clauses.h"
#include "optimizer/plancat.h"
#include "optimizer/planmain.h"
#include "optimizer/prep.h"
#include "parser/gramparse.h"
#include "parser/parse_coerce.h"
#include "parser/parse_expr.h"
#include "parser/parse_relation.h"
#include "parser/parse_type.h"
#include "utils/acl.h"
#include "utils/builtins.h"
#include "utils/fmgroids.h"
#include "utils/lsyscache.h"
#include "utils/syscache.h"
#include "utils/relcache.h"


static List *MergeAttributes(List *schema, List *supers, bool istemp,
				List **supOids, List **supconstr, bool *supHasOids);
static bool change_varattnos_of_a_node(Node *node, const AttrNumber *newattno);
static void StoreCatalogInheritance(Oid relationId, List *supers);
static int	findAttrByName(const char *attributeName, List *schema);
static void setRelhassubclassInRelation(Oid relationId, bool relhassubclass);
static void CheckTupleType(Form_pg_class tuple_class);
static bool needs_toast_table(Relation rel);
static void validateForeignKeyConstraint(FkConstraint *fkconstraint,
							 Relation rel, Relation pkrel);
static Oid createForeignKeyConstraint(Relation rel, Relation pkrel,
						   FkConstraint *fkconstraint);
static void createForeignKeyTriggers(Relation rel, FkConstraint *fkconstraint,
						 Oid constrOid);
static char *fkMatchTypeToString(char match_type);

/* Used by attribute and relation renaming routines: */

#define RI_TRIGGER_PK	1		/* is a trigger on the PK relation */
#define RI_TRIGGER_FK	2		/* is a trigger on the FK relation */
#define RI_TRIGGER_NONE 0		/* is not an RI trigger function */

static int	ri_trigger_type(Oid tgfoid);
static void update_ri_trigger_args(Oid relid,
					   const char *oldname,
					   const char *newname,
					   bool fk_scan,
					   bool update_relname);


/* ----------------------------------------------------------------
 *		DefineRelation
 *				Creates a new relation.
 *
 * If successful, returns the OID of the new relation.
 * ----------------------------------------------------------------
 */
Oid
DefineRelation(CreateStmt *stmt, char relkind)
{
	char		relname[NAMEDATALEN];
	Oid			namespaceId;
	List	   *schema = stmt->tableElts;
	int			numberOfAttributes;
	Oid			relationId;
	Relation	rel;
	TupleDesc	descriptor;
	List	   *inheritOids;
	List	   *old_constraints;
	bool		parentHasOids;
	List	   *rawDefaults;
	List	   *listptr;
	int			i;
	AttrNumber	attnum;

	/*
	 * Truncate relname to appropriate length (probably a waste of time,
	 * as parser should have done this already).
	 */
	StrNCpy(relname, stmt->relation->relname, NAMEDATALEN);

	/*
	 * Look up the namespace in which we are supposed to create the
	 * relation.  Check we have permission to create there. Skip check if
	 * bootstrapping, since permissions machinery may not be working yet.
	 */
	namespaceId = RangeVarGetCreationNamespace(stmt->relation);

	if (!IsBootstrapProcessingMode())
	{
		AclResult	aclresult;

		aclresult = pg_namespace_aclcheck(namespaceId, GetUserId(),
										  ACL_CREATE);
		if (aclresult != ACLCHECK_OK)
			aclcheck_error(aclresult, get_namespace_name(namespaceId));
	}

	/*
	 * Look up inheritance ancestors and generate relation schema,
	 * including inherited attributes.
	 */
	schema = MergeAttributes(schema, stmt->inhRelations,
							 stmt->relation->istemp,
						 &inheritOids, &old_constraints, &parentHasOids);

	numberOfAttributes = length(schema);
	if (numberOfAttributes <= 0)
		elog(ERROR, "DefineRelation: please inherit from a relation or define an attribute");

	/*
	 * Create a relation descriptor from the relation schema and create
	 * the relation.  Note that in this stage only inherited (pre-cooked)
	 * defaults and constraints will be included into the new relation.
	 * (BuildDescForRelation takes care of the inherited defaults, but we
	 * have to copy inherited constraints here.)
	 */
	descriptor = BuildDescForRelation(schema);

	descriptor->tdhasoid = (stmt->hasoids || parentHasOids);

	if (old_constraints != NIL)
	{
		ConstrCheck *check = (ConstrCheck *) palloc(length(old_constraints) *
													sizeof(ConstrCheck));
		int			ncheck = 0;
		int			constr_name_ctr = 0;

		foreach(listptr, old_constraints)
		{
			Constraint *cdef = (Constraint *) lfirst(listptr);

			if (cdef->contype != CONSTR_CHECK)
				continue;

			if (cdef->name != NULL)
			{
				for (i = 0; i < ncheck; i++)
				{
					if (strcmp(check[i].ccname, cdef->name) == 0)
						elog(ERROR, "Duplicate CHECK constraint name: '%s'",
							 cdef->name);
				}
				check[ncheck].ccname = cdef->name;
			}
			else
			{
				/*
				 * Generate a constraint name.	NB: this should match the
				 * form of names that GenerateConstraintName() may produce
				 * for names added later.  We are assured that there is no
				 * name conflict, because MergeAttributes() did not pass
				 * back any names of this form.
				 */
				check[ncheck].ccname = (char *) palloc(NAMEDATALEN);
				snprintf(check[ncheck].ccname, NAMEDATALEN, "$%d",
						 ++constr_name_ctr);
			}
			Assert(cdef->raw_expr == NULL && cdef->cooked_expr != NULL);
			check[ncheck].ccbin = pstrdup(cdef->cooked_expr);
			ncheck++;
		}
		if (ncheck > 0)
		{
			if (descriptor->constr == NULL)
			{
				descriptor->constr = (TupleConstr *) palloc(sizeof(TupleConstr));
				descriptor->constr->defval = NULL;
				descriptor->constr->num_defval = 0;
				descriptor->constr->has_not_null = false;
			}
			descriptor->constr->num_check = ncheck;
			descriptor->constr->check = check;
		}
	}

	relationId = heap_create_with_catalog(relname,
										  namespaceId,
										  descriptor,
										  relkind,
										  false,
										  allowSystemTableMods);

	StoreCatalogInheritance(relationId, inheritOids);

	/*
	 * We must bump the command counter to make the newly-created relation
	 * tuple visible for opening.
	 */
	CommandCounterIncrement();

	/*
	 * Open the new relation and acquire exclusive lock on it.	This isn't
	 * really necessary for locking out other backends (since they can't
	 * see the new rel anyway until we commit), but it keeps the lock
	 * manager from complaining about deadlock risks.
	 */
	rel = relation_open(relationId, AccessExclusiveLock);

	/*
	 * Now add any newly specified column default values and CHECK
	 * constraints to the new relation.  These are passed to us in the
	 * form of raw parsetrees; we need to transform them to executable
	 * expression trees before they can be added. The most convenient way
	 * to do that is to apply the parser's transformExpr routine, but
	 * transformExpr doesn't work unless we have a pre-existing relation.
	 * So, the transformation has to be postponed to this final step of
	 * CREATE TABLE.
	 *
	 * Another task that's conveniently done at this step is to add
	 * dependency links between columns and supporting relations (such as
	 * SERIAL sequences).
	 *
	 * First, scan schema to find new column defaults.
	 */
	rawDefaults = NIL;
	attnum = 0;

	foreach(listptr, schema)
	{
		ColumnDef  *colDef = lfirst(listptr);

		attnum++;

		if (colDef->raw_default != NULL)
		{
			RawColumnDefault *rawEnt;

			Assert(colDef->cooked_default == NULL);

			rawEnt = (RawColumnDefault *) palloc(sizeof(RawColumnDefault));
			rawEnt->attnum = attnum;
			rawEnt->raw_default = colDef->raw_default;
			rawDefaults = lappend(rawDefaults, rawEnt);
		}

		if (colDef->support != NULL)
		{
			/* Create dependency for supporting relation for this column */
			ObjectAddress colobject,
						suppobject;

			colobject.classId = RelOid_pg_class;
			colobject.objectId = relationId;
			colobject.objectSubId = attnum;
			suppobject.classId = RelOid_pg_class;
			suppobject.objectId = RangeVarGetRelid(colDef->support, false);
			suppobject.objectSubId = 0;
			recordDependencyOn(&suppobject, &colobject, DEPENDENCY_INTERNAL);
		}
	}

	/*
	 * Parse and add the defaults/constraints, if any.
	 */
	if (rawDefaults || stmt->constraints)
		AddRelationRawConstraints(rel, rawDefaults, stmt->constraints);

	/*
	 * Clean up.  We keep lock on new relation (although it shouldn't be
	 * visible to anyone else anyway, until commit).
	 */
	relation_close(rel, NoLock);

	return relationId;
}

/*
 * RemoveRelation
 *		Deletes a relation.
 */
void
RemoveRelation(const RangeVar *relation, DropBehavior behavior)
{
	Oid			relOid;
	ObjectAddress object;

	relOid = RangeVarGetRelid(relation, false);

	object.classId = RelOid_pg_class;
	object.objectId = relOid;
	object.objectSubId = 0;

	performDeletion(&object, behavior);
}

/*
 * TruncateRelation
 *				  Removes all the rows from a relation
 *
 * Exceptions:
 *				  BadArg if name is invalid
 *
 * Note:
 *				  Rows are removed, indexes are truncated and reconstructed.
 */
void
TruncateRelation(const RangeVar *relation)
{
	Relation	rel;
	Oid			relid;
	Oid			toastrelid;
	ScanKeyData key;
	Relation	fkeyRel;
	SysScanDesc fkeyScan;
	HeapTuple	tuple;

	/* Grab exclusive lock in preparation for truncate */
	rel = heap_openrv(relation, AccessExclusiveLock);
	relid = RelationGetRelid(rel);

	/* Only allow truncate on regular tables */
	if (rel->rd_rel->relkind != RELKIND_RELATION)
	{
		/* special errors for backwards compatibility */
		if (rel->rd_rel->relkind == RELKIND_SEQUENCE)
			elog(ERROR, "TRUNCATE cannot be used on sequences. '%s' is a sequence",
				 RelationGetRelationName(rel));
		if (rel->rd_rel->relkind == RELKIND_VIEW)
			elog(ERROR, "TRUNCATE cannot be used on views. '%s' is a view",
				 RelationGetRelationName(rel));
		/* else a generic error message will do */
		elog(ERROR, "TRUNCATE can only be used on tables. '%s' is not a table",
			 RelationGetRelationName(rel));
	}

	if (!allowSystemTableMods && IsSystemRelation(rel))
		elog(ERROR, "TRUNCATE cannot be used on system tables. '%s' is a system table",
			 RelationGetRelationName(rel));

	if (!pg_class_ownercheck(relid, GetUserId()))
		aclcheck_error(ACLCHECK_NOT_OWNER, RelationGetRelationName(rel));

	/*
	 * Don't allow truncate on tables which are referenced by foreign keys
	 */
	fkeyRel = heap_openr(ConstraintRelationName, AccessShareLock);

	ScanKeyEntryInitialize(&key, 0,
						   Anum_pg_constraint_confrelid,
						   F_OIDEQ,
						   ObjectIdGetDatum(relid));

	fkeyScan = systable_beginscan(fkeyRel, 0, false,
								  SnapshotNow, 1, &key);

	/*
	 * First foreign key found with us as the reference should throw an
	 * error.
	 */
	while (HeapTupleIsValid(tuple = systable_getnext(fkeyScan)))
	{
		Form_pg_constraint con = (Form_pg_constraint) GETSTRUCT(tuple);

		if (con->contype == 'f' && con->conrelid != relid)
			elog(ERROR, "TRUNCATE cannot be used as table %s references this one via foreign key constraint %s",
				 get_rel_name(con->conrelid),
				 NameStr(con->conname));
	}

	systable_endscan(fkeyScan);
	heap_close(fkeyRel, AccessShareLock);

	toastrelid = rel->rd_rel->reltoastrelid;

	/* Keep the lock until transaction commit */
	heap_close(rel, NoLock);

	/* Truncate the table proper */
	heap_truncate(relid);

	/* If it has a toast table, truncate that too */
	if (OidIsValid(toastrelid))
		heap_truncate(toastrelid);
}

/*----------
 * MergeAttributes
 *		Returns new schema given initial schema and superclasses.
 *
 * Input arguments:
 * 'schema' is the column/attribute definition for the table. (It's a list
 *		of ColumnDef's.) It is destructively changed.
 * 'supers' is a list of names (as RangeVar nodes) of parent relations.
 * 'istemp' is TRUE if we are creating a temp relation.
 *
 * Output arguments:
 * 'supOids' receives an integer list of the OIDs of the parent relations.
 * 'supconstr' receives a list of constraints belonging to the parents,
 *		updated as necessary to be valid for the child.
 * 'supHasOids' is set TRUE if any parent has OIDs, else it is set FALSE.
 *
 * Return value:
 * Completed schema list.
 *
 * Notes:
 *	  The order in which the attributes are inherited is very important.
 *	  Intuitively, the inherited attributes should come first. If a table
 *	  inherits from multiple parents, the order of those attributes are
 *	  according to the order of the parents specified in CREATE TABLE.
 *
 *	  Here's an example:
 *
 *		create table person (name text, age int4, location point);
 *		create table emp (salary int4, manager text) inherits(person);
 *		create table student (gpa float8) inherits (person);
 *		create table stud_emp (percent int4) inherits (emp, student);
 *
 *	  The order of the attributes of stud_emp is:
 *
 *							person {1:name, 2:age, 3:location}
 *							/	 \
 *			   {6:gpa}	student   emp {4:salary, 5:manager}
 *							\	 /
 *						   stud_emp {7:percent}
 *
 *	   If the same attribute name appears multiple times, then it appears
 *	   in the result table in the proper location for its first appearance.
 *
 *	   Constraints (including NOT NULL constraints) for the child table
 *	   are the union of all relevant constraints, from both the child schema
 *	   and parent tables.
 *
 *	   The default value for a child column is defined as:
 *		(1) If the child schema specifies a default, that value is used.
 *		(2) If neither the child nor any parent specifies a default, then
 *			the column will not have a default.
 *		(3) If conflicting defaults are inherited from different parents
 *			(and not overridden by the child), an error is raised.
 *		(4) Otherwise the inherited default is used.
 *		Rule (3) is new in Postgres 7.1; in earlier releases you got a
 *		rather arbitrary choice of which parent default to use.
 *----------
 */
static List *
MergeAttributes(List *schema, List *supers, bool istemp,
				List **supOids, List **supconstr, bool *supHasOids)
{
	List	   *entry;
	List	   *inhSchema = NIL;
	List	   *parentOids = NIL;
	List	   *constraints = NIL;
	bool		parentHasOids = false;
	bool		have_bogus_defaults = false;
	char	   *bogus_marker = "Bogus!";		/* marks conflicting
												 * defaults */
	int			child_attno;

	/*
	 * Check for duplicate names in the explicit list of attributes.
	 *
	 * Although we might consider merging such entries in the same way that
	 * we handle name conflicts for inherited attributes, it seems to make
	 * more sense to assume such conflicts are errors.
	 */
	foreach(entry, schema)
	{
		ColumnDef  *coldef = lfirst(entry);
		List	   *rest;

		foreach(rest, lnext(entry))
		{
			ColumnDef  *restdef = lfirst(rest);

			if (strcmp(coldef->colname, restdef->colname) == 0)
				elog(ERROR, "CREATE TABLE: attribute \"%s\" duplicated",
					 coldef->colname);
		}
	}

	/*
	 * Scan the parents left-to-right, and merge their attributes to form
	 * a list of inherited attributes (inhSchema).	Also check to see if
	 * we need to inherit an OID column.
	 */
	child_attno = 0;
	foreach(entry, supers)
	{
		RangeVar   *parent = (RangeVar *) lfirst(entry);
		Relation	relation;
		TupleDesc	tupleDesc;
		TupleConstr *constr;
		AttrNumber *newattno;
		AttrNumber	parent_attno;

		relation = heap_openrv(parent, AccessShareLock);

		if (relation->rd_rel->relkind != RELKIND_RELATION)
			elog(ERROR, "CREATE TABLE: inherited relation \"%s\" is not a table",
				 parent->relname);
		/* Permanent rels cannot inherit from temporary ones */
		if (!istemp && isTempNamespace(RelationGetNamespace(relation)))
			elog(ERROR, "CREATE TABLE: cannot inherit from temp relation \"%s\"",
				 parent->relname);

		/*
		 * We should have an UNDER permission flag for this, but for now,
		 * demand that creator of a child table own the parent.
		 */
		if (!pg_class_ownercheck(RelationGetRelid(relation), GetUserId()))
			aclcheck_error(ACLCHECK_NOT_OWNER,
						   RelationGetRelationName(relation));

		/*
		 * Reject duplications in the list of parents.
		 */
		if (intMember(RelationGetRelid(relation), parentOids))
			elog(ERROR, "CREATE TABLE: inherited relation \"%s\" duplicated",
				 parent->relname);

		parentOids = lappendi(parentOids, RelationGetRelid(relation));
		setRelhassubclassInRelation(RelationGetRelid(relation), true);

		parentHasOids |= relation->rd_rel->relhasoids;

		tupleDesc = RelationGetDescr(relation);
		constr = tupleDesc->constr;

		/*
		 * newattno[] will contain the child-table attribute numbers for
		 * the attributes of this parent table.  (They are not the same
		 * for parents after the first one, nor if we have dropped
		 * columns.)
		 */
		newattno = (AttrNumber *) palloc(tupleDesc->natts * sizeof(AttrNumber));

		for (parent_attno = 1; parent_attno <= tupleDesc->natts;
			 parent_attno++)
		{
			Form_pg_attribute attribute = tupleDesc->attrs[parent_attno - 1];
			char	   *attributeName = NameStr(attribute->attname);
			int			exist_attno;
			ColumnDef  *def;
			TypeName   *typename;

			/*
			 * Ignore dropped columns in the parent.
			 */
			if (attribute->attisdropped)
			{
				/*
				 * change_varattnos_of_a_node asserts that this is greater
				 * than zero, so if anything tries to use it, we should
				 * find out.
				 */
				newattno[parent_attno - 1] = 0;
				continue;
			}

			/*
			 * Does it conflict with some previously inherited column?
			 */
			exist_attno = findAttrByName(attributeName, inhSchema);
			if (exist_attno > 0)
			{
				/*
				 * Yes, try to merge the two column definitions. They must
				 * have the same type and typmod.
				 */
				elog(NOTICE, "CREATE TABLE: merging multiple inherited definitions of attribute \"%s\"",
					 attributeName);
				def = (ColumnDef *) nth(exist_attno - 1, inhSchema);
				if (typenameTypeId(def->typename) != attribute->atttypid ||
					def->typename->typmod != attribute->atttypmod)
					elog(ERROR, "CREATE TABLE: inherited attribute \"%s\" type conflict (%s and %s)",
						 attributeName,
						 TypeNameToString(def->typename),
						 format_type_be(attribute->atttypid));
				/* Merge of NOT NULL constraints = OR 'em together */
				def->is_not_null |= attribute->attnotnull;
				/* Default and other constraints are handled below */
				newattno[parent_attno - 1] = exist_attno;
			}
			else
			{
				/*
				 * No, create a new inherited column
				 */
				def = makeNode(ColumnDef);
				def->colname = pstrdup(attributeName);
				typename = makeNode(TypeName);
				typename->typeid = attribute->atttypid;
				typename->typmod = attribute->atttypmod;
				def->typename = typename;
				def->is_inherited = true;
				def->is_not_null = attribute->attnotnull;
				def->raw_default = NULL;
				def->cooked_default = NULL;
				def->constraints = NIL;
				def->support = NULL;
				inhSchema = lappend(inhSchema, def);
				newattno[parent_attno - 1] = ++child_attno;
			}

			/*
			 * Copy default if any
			 */
			if (attribute->atthasdef)
			{
				char	   *this_default = NULL;
				AttrDefault *attrdef;
				int			i;

				/* Find default in constraint structure */
				Assert(constr != NULL);
				attrdef = constr->defval;
				for (i = 0; i < constr->num_defval; i++)
				{
					if (attrdef[i].adnum == parent_attno)
					{
						this_default = attrdef[i].adbin;
						break;
					}
				}
				Assert(this_default != NULL);

				/*
				 * If default expr could contain any vars, we'd need to
				 * fix 'em, but it can't; so default is ready to apply to
				 * child.
				 *
				 * If we already had a default from some prior parent, check
				 * to see if they are the same.  If so, no problem; if
				 * not, mark the column as having a bogus default. Below,
				 * we will complain if the bogus default isn't overridden
				 * by the child schema.
				 */
				Assert(def->raw_default == NULL);
				if (def->cooked_default == NULL)
					def->cooked_default = pstrdup(this_default);
				else if (strcmp(def->cooked_default, this_default) != 0)
				{
					def->cooked_default = bogus_marker;
					have_bogus_defaults = true;
				}
			}
		}

		/*
		 * Now copy the constraints of this parent, adjusting attnos using
		 * the completed newattno[] map
		 */
		if (constr && constr->num_check > 0)
		{
			ConstrCheck *check = constr->check;
			int			i;

			for (i = 0; i < constr->num_check; i++)
			{
				Constraint *cdef = makeNode(Constraint);
				Node	   *expr;

				cdef->contype = CONSTR_CHECK;

				/*
				 * Do not inherit generated constraint names, since they
				 * might conflict across multiple inheritance parents.
				 * (But conflicts between user-assigned names will cause
				 * an error.)
				 */
				if (ConstraintNameIsGenerated(check[i].ccname))
					cdef->name = NULL;
				else
					cdef->name = pstrdup(check[i].ccname);
				cdef->raw_expr = NULL;
				/* adjust varattnos of ccbin here */
				expr = stringToNode(check[i].ccbin);
				change_varattnos_of_a_node(expr, newattno);
				cdef->cooked_expr = nodeToString(expr);
				constraints = lappend(constraints, cdef);
			}
		}

		pfree(newattno);

		/*
		 * Close the parent rel, but keep our AccessShareLock on it until
		 * xact commit.  That will prevent someone else from deleting or
		 * ALTERing the parent before the child is committed.
		 */
		heap_close(relation, NoLock);
	}

	/*
	 * If we had no inherited attributes, the result schema is just the
	 * explicitly declared columns.  Otherwise, we need to merge the
	 * declared columns into the inherited schema list.
	 */
	if (inhSchema != NIL)
	{
		foreach(entry, schema)
		{
			ColumnDef  *newdef = lfirst(entry);
			char	   *attributeName = newdef->colname;
			int			exist_attno;

			/*
			 * Does it conflict with some previously inherited column?
			 */
			exist_attno = findAttrByName(attributeName, inhSchema);
			if (exist_attno > 0)
			{
				ColumnDef  *def;

				/*
				 * Yes, try to merge the two column definitions. They must
				 * have the same type and typmod.
				 */
				elog(NOTICE, "CREATE TABLE: merging attribute \"%s\" with inherited definition",
					 attributeName);
				def = (ColumnDef *) nth(exist_attno - 1, inhSchema);
				if (typenameTypeId(def->typename) != typenameTypeId(newdef->typename) ||
					def->typename->typmod != newdef->typename->typmod)
					elog(ERROR, "CREATE TABLE: attribute \"%s\" type conflict (%s and %s)",
						 attributeName,
						 TypeNameToString(def->typename),
						 TypeNameToString(newdef->typename));
				/* Merge of NOT NULL constraints = OR 'em together */
				def->is_not_null |= newdef->is_not_null;
				/* If new def has a default, override previous default */
				if (newdef->raw_default != NULL)
				{
					def->raw_default = newdef->raw_default;
					def->cooked_default = newdef->cooked_default;
				}
			}
			else
			{
				/*
				 * No, attach new column to result schema
				 */
				inhSchema = lappend(inhSchema, newdef);
			}
		}

		schema = inhSchema;
	}

	/*
	 * If we found any conflicting parent default values, check to make
	 * sure they were overridden by the child.
	 */
	if (have_bogus_defaults)
	{
		foreach(entry, schema)
		{
			ColumnDef  *def = lfirst(entry);

			if (def->cooked_default == bogus_marker)
				elog(ERROR, "CREATE TABLE: attribute \"%s\" inherits conflicting default values"
					 "\n\tTo resolve the conflict, specify a default explicitly",
					 def->colname);
		}
	}

	*supOids = parentOids;
	*supconstr = constraints;
	*supHasOids = parentHasOids;
	return schema;
}

/*
 * complementary static functions for MergeAttributes().
 *
 * Varattnos of pg_constraint.conbin must be rewritten when subclasses inherit
 * constraints from parent classes, since the inherited attributes could
 * be given different column numbers in multiple-inheritance cases.
 *
 * Note that the passed node tree is modified in place!
 */
static bool
change_varattnos_walker(Node *node, const AttrNumber *newattno)
{
	if (node == NULL)
		return false;
	if (IsA(node, Var))
	{
		Var		   *var = (Var *) node;

		if (var->varlevelsup == 0 && var->varno == 1 &&
			var->varattno > 0)
		{
			/*
			 * ??? the following may be a problem when the node is
			 * multiply referenced though stringToNode() doesn't create
			 * such a node currently.
			 */
			Assert(newattno[var->varattno - 1] > 0);
			var->varattno = newattno[var->varattno - 1];
		}
		return false;
	}
	return expression_tree_walker(node, change_varattnos_walker,
								  (void *) newattno);
}

static bool
change_varattnos_of_a_node(Node *node, const AttrNumber *newattno)
{
	return change_varattnos_walker(node, newattno);
}

/*
 * StoreCatalogInheritance
 *		Updates the system catalogs with proper inheritance information.
 *
 * supers is an integer list of the OIDs of the new relation's direct
 * ancestors.  NB: it is destructively changed to include indirect ancestors.
 */
static void
StoreCatalogInheritance(Oid relationId, List *supers)
{
	Relation	relation;
	TupleDesc	desc;
	int16		seqNumber;
	List	   *entry;
	HeapTuple	tuple;

	/*
	 * sanity checks
	 */
	AssertArg(OidIsValid(relationId));

	if (supers == NIL)
		return;

	/*
	 * Store INHERITS information in pg_inherits using direct ancestors
	 * only. Also enter dependencies on the direct ancestors.
	 */
	relation = heap_openr(InheritsRelationName, RowExclusiveLock);
	desc = RelationGetDescr(relation);

	seqNumber = 1;
	foreach(entry, supers)
	{
		Oid			entryOid = lfirsti(entry);
		Datum		datum[Natts_pg_inherits];
		char		nullarr[Natts_pg_inherits];
		ObjectAddress childobject,
					parentobject;

		datum[0] = ObjectIdGetDatum(relationId);		/* inhrel */
		datum[1] = ObjectIdGetDatum(entryOid);	/* inhparent */
		datum[2] = Int16GetDatum(seqNumber);	/* inhseqno */

		nullarr[0] = ' ';
		nullarr[1] = ' ';
		nullarr[2] = ' ';

		tuple = heap_formtuple(desc, datum, nullarr);

		simple_heap_insert(relation, tuple);

		CatalogUpdateIndexes(relation, tuple);

		heap_freetuple(tuple);

		/*
		 * Store a dependency too
		 */
		parentobject.classId = RelOid_pg_class;
		parentobject.objectId = entryOid;
		parentobject.objectSubId = 0;
		childobject.classId = RelOid_pg_class;
		childobject.objectId = relationId;
		childobject.objectSubId = 0;

		recordDependencyOn(&childobject, &parentobject, DEPENDENCY_NORMAL);

		seqNumber += 1;
	}

	heap_close(relation, RowExclusiveLock);

	/* ----------------
	 * Expand supers list to include indirect ancestors as well.
	 *
	 * Algorithm:
	 *	0. begin with list of direct superclasses.
	 *	1. append after each relationId, its superclasses, recursively.
	 *	2. remove all but last of duplicates.
	 * ----------------
	 */

	/*
	 * 1. append after each relationId, its superclasses, recursively.
	 */
	foreach(entry, supers)
	{
		HeapTuple	tuple;
		Oid			id;
		int16		number;
		List	   *next;
		List	   *current;

		id = (Oid) lfirsti(entry);
		current = entry;
		next = lnext(entry);

		for (number = 1;; number += 1)
		{
			tuple = SearchSysCache(INHRELID,
								   ObjectIdGetDatum(id),
								   Int16GetDatum(number),
								   0, 0);
			if (!HeapTupleIsValid(tuple))
				break;

			lnext(current) = lconsi(((Form_pg_inherits)
									 GETSTRUCT(tuple))->inhparent,
									NIL);

			ReleaseSysCache(tuple);

			current = lnext(current);
		}
		lnext(current) = next;
	}

	/*
	 * 2. remove all but last of duplicates.
	 */
	foreach(entry, supers)
	{
		Oid			thisone;
		bool		found;
		List	   *rest;

again:
		thisone = lfirsti(entry);
		found = false;
		foreach(rest, lnext(entry))
		{
			if (thisone == lfirsti(rest))
			{
				found = true;
				break;
			}
		}
		if (found)
		{
			/*
			 * found a later duplicate, so remove this entry.
			 */
			lfirsti(entry) = lfirsti(lnext(entry));
			lnext(entry) = lnext(lnext(entry));

			goto again;
		}
	}
}

/*
 * Look for an existing schema entry with the given name.
 *
 * Returns the index (starting with 1) if attribute already exists in schema,
 * 0 if it doesn't.
 */
static int
findAttrByName(const char *attributeName, List *schema)
{
	List	   *s;
	int			i = 0;

	foreach(s, schema)
	{
		ColumnDef  *def = lfirst(s);

		++i;
		if (strcmp(attributeName, def->colname) == 0)
			return i;
	}
	return 0;
}

/*
 * Update a relation's pg_class.relhassubclass entry to the given value
 */
static void
setRelhassubclassInRelation(Oid relationId, bool relhassubclass)
{
	Relation	relationRelation;
	HeapTuple	tuple;

	/*
	 * Fetch a modifiable copy of the tuple, modify it, update pg_class.
	 */
	relationRelation = heap_openr(RelationRelationName, RowExclusiveLock);
	tuple = SearchSysCacheCopy(RELOID,
							   ObjectIdGetDatum(relationId),
							   0, 0, 0);
	if (!HeapTupleIsValid(tuple))
		elog(ERROR, "setRelhassubclassInRelation: cache lookup failed for relation %u", relationId);

	((Form_pg_class) GETSTRUCT(tuple))->relhassubclass = relhassubclass;
	simple_heap_update(relationRelation, &tuple->t_self, tuple);

	/* keep the catalog indexes up to date */
	CatalogUpdateIndexes(relationRelation, tuple);

	heap_freetuple(tuple);
	heap_close(relationRelation, RowExclusiveLock);
}


/*
 *		renameatt		- changes the name of a attribute in a relation
 *
 *		Attname attribute is changed in attribute catalog.
 *		No record of the previous attname is kept (correct?).
 *
 *		get proper relrelation from relation catalog (if not arg)
 *		scan attribute catalog
 *				for name conflict (within rel)
 *				for original attribute (if not arg)
 *		modify attname in attribute tuple
 *		insert modified attribute in attribute catalog
 *		delete original attribute from attribute catalog
 */
void
renameatt(Oid myrelid,
		  const char *oldattname,
		  const char *newattname,
		  bool recurse,
		  bool recursing)
{
	Relation	targetrelation;
	Relation	attrelation;
	HeapTuple	atttup;
	Form_pg_attribute attform;
	List	   *indexoidlist;
	List	   *indexoidscan;

	/*
	 * Grab an exclusive lock on the target table, which we will NOT
	 * release until end of transaction.
	 */
	targetrelation = relation_open(myrelid, AccessExclusiveLock);

	/*
	 * permissions checking.  this would normally be done in utility.c,
	 * but this particular routine is recursive.
	 *
	 * normally, only the owner of a class can change its schema.
	 */
	if (!allowSystemTableMods
		&& IsSystemRelation(targetrelation))
		elog(ERROR, "renameatt: class \"%s\" is a system catalog",
			 RelationGetRelationName(targetrelation));
	if (!pg_class_ownercheck(myrelid, GetUserId()))
		aclcheck_error(ACLCHECK_NOT_OWNER,
					   RelationGetRelationName(targetrelation));

	/*
	 * if the 'recurse' flag is set then we are supposed to rename this
	 * attribute in all classes that inherit from 'relname' (as well as in
	 * 'relname').
	 *
	 * any permissions or problems with duplicate attributes will cause the
	 * whole transaction to abort, which is what we want -- all or
	 * nothing.
	 */
	if (recurse)
	{
		List	   *child,
				   *children;

		/* this routine is actually in the planner */
		children = find_all_inheritors(myrelid);

		/*
		 * find_all_inheritors does the recursive search of the
		 * inheritance hierarchy, so all we have to do is process all of
		 * the relids in the list that it returns.
		 */
		foreach(child, children)
		{
			Oid			childrelid = lfirsti(child);

			if (childrelid == myrelid)
				continue;
			/* note we need not recurse again! */
			renameatt(childrelid, oldattname, newattname, false, true);
		}
	}
	else
	{
		/*
		 * If we are told not to recurse, there had better not be any
		 * child tables; else the rename would put them out of step.
		 */
		if (!recursing &&
			find_inheritance_children(myrelid) != NIL)
			elog(ERROR, "Inherited attribute \"%s\" must be renamed in child tables too",
				 oldattname);
	}

	attrelation = heap_openr(AttributeRelationName, RowExclusiveLock);

	atttup = SearchSysCacheCopyAttName(myrelid, oldattname);
	if (!HeapTupleIsValid(atttup))
		elog(ERROR, "renameatt: attribute \"%s\" does not exist",
			 oldattname);
	attform = (Form_pg_attribute) GETSTRUCT(atttup);

	if (attform->attnum < 0)
		elog(ERROR, "renameatt: system attribute \"%s\" may not be renamed",
			 oldattname);

	/*
	 * if the attribute is inherited, forbid the renaming, unless we are
	 * already inside a recursive rename.
	 */
	if (attform->attisinherited && !recursing)
		elog(ERROR, "renameatt: inherited attribute \"%s\" may not be renamed",
			 oldattname);

	/* should not already exist */
	/* this test is deliberately not attisdropped-aware */
	if (SearchSysCacheExists(ATTNAME,
							 ObjectIdGetDatum(myrelid),
							 PointerGetDatum(newattname),
							 0, 0))
		elog(ERROR, "renameatt: attribute \"%s\" exists", newattname);

	namestrcpy(&(attform->attname), newattname);

	simple_heap_update(attrelation, &atttup->t_self, atttup);

	/* keep system catalog indexes current */
	CatalogUpdateIndexes(attrelation, atttup);

	heap_freetuple(atttup);

	/*
	 * Update column names of indexes that refer to the column being
	 * renamed.
	 */
	indexoidlist = RelationGetIndexList(targetrelation);

	foreach(indexoidscan, indexoidlist)
	{
		Oid			indexoid = lfirsti(indexoidscan);
		HeapTuple	indextup;

		/*
		 * First check to see if index is a functional index. If so, its
		 * column name is a function name and shouldn't be renamed here.
		 */
		indextup = SearchSysCache(INDEXRELID,
								  ObjectIdGetDatum(indexoid),
								  0, 0, 0);
		if (!HeapTupleIsValid(indextup))
			elog(ERROR, "renameatt: can't find index id %u", indexoid);
		if (OidIsValid(((Form_pg_index) GETSTRUCT(indextup))->indproc))
		{
			ReleaseSysCache(indextup);
			continue;
		}
		ReleaseSysCache(indextup);

		/*
		 * Okay, look to see if any column name of the index matches the
		 * old attribute name.
		 */
		atttup = SearchSysCacheCopyAttName(indexoid, oldattname);
		if (!HeapTupleIsValid(atttup))
			continue;			/* Nope, so ignore it */

		/*
		 * Update the (copied) attribute tuple.
		 */
		namestrcpy(&(((Form_pg_attribute) GETSTRUCT(atttup))->attname),
				   newattname);

		simple_heap_update(attrelation, &atttup->t_self, atttup);

		/* keep system catalog indexes current */
		CatalogUpdateIndexes(attrelation, atttup);

		heap_freetuple(atttup);
	}

	freeList(indexoidlist);

	heap_close(attrelation, RowExclusiveLock);

	/*
	 * Update att name in any RI triggers associated with the relation.
	 */
	if (targetrelation->rd_rel->reltriggers > 0)
	{
		/* update tgargs column reference where att is primary key */
		update_ri_trigger_args(RelationGetRelid(targetrelation),
							   oldattname, newattname,
							   false, false);
		/* update tgargs column reference where att is foreign key */
		update_ri_trigger_args(RelationGetRelid(targetrelation),
							   oldattname, newattname,
							   true, false);
	}

	relation_close(targetrelation, NoLock);		/* close rel but keep
												 * lock! */
}

/*
 *		renamerel		- change the name of a relation
 *
 *		XXX - When renaming sequences, we don't bother to modify the
 *			  sequence name that is stored within the sequence itself
 *			  (this would cause problems with MVCC). In the future,
 *			  the sequence name should probably be removed from the
 *			  sequence, AFAIK there's no need for it to be there.
 */
void
renamerel(Oid myrelid, const char *newrelname)
{
	Relation	targetrelation;
	Relation	relrelation;	/* for RELATION relation */
	HeapTuple	reltup;
	Oid			namespaceId;
	char	   *oldrelname;
	char		relkind;
	bool		relhastriggers;

	/*
	 * Grab an exclusive lock on the target table or index, which we will
	 * NOT release until end of transaction.
	 */
	targetrelation = relation_open(myrelid, AccessExclusiveLock);

	oldrelname = pstrdup(RelationGetRelationName(targetrelation));
	namespaceId = RelationGetNamespace(targetrelation);

	/* Validity checks */
	if (!allowSystemTableMods &&
		IsSystemRelation(targetrelation))
		elog(ERROR, "renamerel: system relation \"%s\" may not be renamed",
			 oldrelname);

	relkind = targetrelation->rd_rel->relkind;
	relhastriggers = (targetrelation->rd_rel->reltriggers > 0);

	/*
	 * Find relation's pg_class tuple, and make sure newrelname isn't in
	 * use.
	 */
	relrelation = heap_openr(RelationRelationName, RowExclusiveLock);

	reltup = SearchSysCacheCopy(RELOID,
								PointerGetDatum(myrelid),
								0, 0, 0);
	if (!HeapTupleIsValid(reltup))
		elog(ERROR, "renamerel: relation \"%s\" does not exist",
			 oldrelname);

	if (get_relname_relid(newrelname, namespaceId) != InvalidOid)
		elog(ERROR, "renamerel: relation \"%s\" exists", newrelname);

	/*
	 * Update pg_class tuple with new relname.	(Scribbling on reltup is
	 * OK because it's a copy...)
	 */
	namestrcpy(&(((Form_pg_class) GETSTRUCT(reltup))->relname), newrelname);

	simple_heap_update(relrelation, &reltup->t_self, reltup);

	/* keep the system catalog indexes current */
	CatalogUpdateIndexes(relrelation, reltup);

	heap_close(relrelation, NoLock);
	heap_freetuple(reltup);

	/*
	 * Also rename the associated type, if any.
	 */
	if (relkind != RELKIND_INDEX)
		TypeRename(oldrelname, namespaceId, newrelname);

	/*
	 * Update rel name in any RI triggers associated with the relation.
	 */
	if (relhastriggers)
	{
		/* update tgargs where relname is primary key */
		update_ri_trigger_args(myrelid,
							   oldrelname,
							   newrelname,
							   false, true);
		/* update tgargs where relname is foreign key */
		update_ri_trigger_args(myrelid,
							   oldrelname,
							   newrelname,
							   true, true);
	}

	/*
	 * Close rel, but keep exclusive lock!
	 */
	relation_close(targetrelation, NoLock);
}


/*
 * Given a trigger function OID, determine whether it is an RI trigger,
 * and if so whether it is attached to PK or FK relation.
 *
 * XXX this probably doesn't belong here; should be exported by
 * ri_triggers.c
 */
static int
ri_trigger_type(Oid tgfoid)
{
	switch (tgfoid)
	{
		case F_RI_FKEY_CASCADE_DEL:
		case F_RI_FKEY_CASCADE_UPD:
		case F_RI_FKEY_RESTRICT_DEL:
		case F_RI_FKEY_RESTRICT_UPD:
		case F_RI_FKEY_SETNULL_DEL:
		case F_RI_FKEY_SETNULL_UPD:
		case F_RI_FKEY_SETDEFAULT_DEL:
		case F_RI_FKEY_SETDEFAULT_UPD:
		case F_RI_FKEY_NOACTION_DEL:
		case F_RI_FKEY_NOACTION_UPD:
			return RI_TRIGGER_PK;

		case F_RI_FKEY_CHECK_INS:
		case F_RI_FKEY_CHECK_UPD:
			return RI_TRIGGER_FK;
	}

	return RI_TRIGGER_NONE;
}

/*
 * Scan pg_trigger for RI triggers that are on the specified relation
 * (if fk_scan is false) or have it as the tgconstrrel (if fk_scan
 * is true).  Update RI trigger args fields matching oldname to contain
 * newname instead.  If update_relname is true, examine the relname
 * fields; otherwise examine the attname fields.
 */
static void
update_ri_trigger_args(Oid relid,
					   const char *oldname,
					   const char *newname,
					   bool fk_scan,
					   bool update_relname)
{
	Relation	tgrel;
	ScanKeyData skey[1];
	SysScanDesc trigscan;
	HeapTuple	tuple;
	Datum		values[Natts_pg_trigger];
	char		nulls[Natts_pg_trigger];
	char		replaces[Natts_pg_trigger];

	tgrel = heap_openr(TriggerRelationName, RowExclusiveLock);
	if (fk_scan)
	{
		ScanKeyEntryInitialize(&skey[0], 0x0,
							   Anum_pg_trigger_tgconstrrelid,
							   F_OIDEQ,
							   ObjectIdGetDatum(relid));
		trigscan = systable_beginscan(tgrel, TriggerConstrRelidIndex,
									  true, SnapshotNow,
									  1, skey);
	}
	else
	{
		ScanKeyEntryInitialize(&skey[0], 0x0,
							   Anum_pg_trigger_tgrelid,
							   F_OIDEQ,
							   ObjectIdGetDatum(relid));
		trigscan = systable_beginscan(tgrel, TriggerRelidNameIndex,
									  true, SnapshotNow,
									  1, skey);
	}

	while ((tuple = systable_getnext(trigscan)) != NULL)
	{
		Form_pg_trigger pg_trigger = (Form_pg_trigger) GETSTRUCT(tuple);
		bytea	   *val;
		bytea	   *newtgargs;
		bool		isnull;
		int			tg_type;
		bool		examine_pk;
		bool		changed;
		int			tgnargs;
		int			i;
		int			newlen;
		const char *arga[RI_MAX_ARGUMENTS];
		const char *argp;

		tg_type = ri_trigger_type(pg_trigger->tgfoid);
		if (tg_type == RI_TRIGGER_NONE)
		{
			/* Not an RI trigger, forget it */
			continue;
		}

		/*
		 * It is an RI trigger, so parse the tgargs bytea.
		 *
		 * NB: we assume the field will never be compressed or moved out of
		 * line; so does trigger.c ...
		 */
		tgnargs = pg_trigger->tgnargs;
		val = (bytea *) fastgetattr(tuple,
									Anum_pg_trigger_tgargs,
									tgrel->rd_att, &isnull);
		if (isnull || tgnargs < RI_FIRST_ATTNAME_ARGNO ||
			tgnargs > RI_MAX_ARGUMENTS)
		{
			/* This probably shouldn't happen, but ignore busted triggers */
			continue;
		}
		argp = (const char *) VARDATA(val);
		for (i = 0; i < tgnargs; i++)
		{
			arga[i] = argp;
			argp += strlen(argp) + 1;
		}

		/*
		 * Figure out which item(s) to look at.  If the trigger is
		 * primary-key type and attached to my rel, I should look at the
		 * PK fields; if it is foreign-key type and attached to my rel, I
		 * should look at the FK fields.  But the opposite rule holds when
		 * examining triggers found by tgconstrrel search.
		 */
		examine_pk = (tg_type == RI_TRIGGER_PK) == (!fk_scan);

		changed = false;
		if (update_relname)
		{
			/* Change the relname if needed */
			i = examine_pk ? RI_PK_RELNAME_ARGNO : RI_FK_RELNAME_ARGNO;
			if (strcmp(arga[i], oldname) == 0)
			{
				arga[i] = newname;
				changed = true;
			}
		}
		else
		{
			/* Change attname(s) if needed */
			i = examine_pk ? RI_FIRST_ATTNAME_ARGNO + RI_KEYPAIR_PK_IDX :
				RI_FIRST_ATTNAME_ARGNO + RI_KEYPAIR_FK_IDX;
			for (; i < tgnargs; i += 2)
			{
				if (strcmp(arga[i], oldname) == 0)
				{
					arga[i] = newname;
					changed = true;
				}
			}
		}

		if (!changed)
		{
			/* Don't need to update this tuple */
			continue;
		}

		/*
		 * Construct modified tgargs bytea.
		 */
		newlen = VARHDRSZ;
		for (i = 0; i < tgnargs; i++)
			newlen += strlen(arga[i]) + 1;
		newtgargs = (bytea *) palloc(newlen);
		VARATT_SIZEP(newtgargs) = newlen;
		newlen = VARHDRSZ;
		for (i = 0; i < tgnargs; i++)
		{
			strcpy(((char *) newtgargs) + newlen, arga[i]);
			newlen += strlen(arga[i]) + 1;
		}

		/*
		 * Build modified tuple.
		 */
		for (i = 0; i < Natts_pg_trigger; i++)
		{
			values[i] = (Datum) 0;
			replaces[i] = ' ';
			nulls[i] = ' ';
		}
		values[Anum_pg_trigger_tgargs - 1] = PointerGetDatum(newtgargs);
		replaces[Anum_pg_trigger_tgargs - 1] = 'r';

		tuple = heap_modifytuple(tuple, tgrel, values, nulls, replaces);

		/*
		 * Update pg_trigger and its indexes
		 */
		simple_heap_update(tgrel, &tuple->t_self, tuple);

		CatalogUpdateIndexes(tgrel, tuple);

		/* free up our scratch memory */
		pfree(newtgargs);
		heap_freetuple(tuple);
	}

	systable_endscan(trigscan);

	heap_close(tgrel, RowExclusiveLock);

	/*
	 * Increment cmd counter to make updates visible; this is needed in
	 * case the same tuple has to be updated again by next pass (can
	 * happen in case of a self-referential FK relationship).
	 */
	CommandCounterIncrement();
}


/* ----------------
 *		AlterTableAddColumn
 *		(formerly known as PerformAddAttribute)
 *
 *		adds an additional attribute to a relation
 * ----------------
 */
void
AlterTableAddColumn(Oid myrelid,
					bool recurse,
					bool recursing,
					ColumnDef *colDef)
{
	Relation	rel,
				pgclass,
				attrdesc;
	HeapTuple	reltup;
	HeapTuple	newreltup;
	HeapTuple	attributeTuple;
	Form_pg_attribute attribute;
	FormData_pg_attribute attributeD;
	int			i;
	int			minattnum,
				maxatts;
	HeapTuple	typeTuple;
	Form_pg_type tform;
	int			attndims;
	ObjectAddress myself,
				referenced;

	/*
	 * Grab an exclusive lock on the target table, which we will NOT
	 * release until end of transaction.
	 */
	rel = heap_open(myrelid, AccessExclusiveLock);

	if (rel->rd_rel->relkind != RELKIND_RELATION)
		elog(ERROR, "ALTER TABLE: relation \"%s\" is not a table",
			 RelationGetRelationName(rel));

	/*
	 * permissions checking.  this would normally be done in utility.c,
	 * but this particular routine is recursive.
	 *
	 * normally, only the owner of a class can change its schema.
	 */
	if (!allowSystemTableMods
		&& IsSystemRelation(rel))
		elog(ERROR, "ALTER TABLE: relation \"%s\" is a system catalog",
			 RelationGetRelationName(rel));
	if (!pg_class_ownercheck(myrelid, GetUserId()))
		aclcheck_error(ACLCHECK_NOT_OWNER, RelationGetRelationName(rel));

	/*
	 * Recurse to add the column to child classes, if requested.
	 *
	 * any permissions or problems with duplicate attributes will cause the
	 * whole transaction to abort, which is what we want -- all or
	 * nothing.
	 */
	if (recurse)
	{
		List	   *child,
				   *children;
		ColumnDef  *colDefChild = copyObject(colDef);

		colDefChild->is_inherited = true;

		/* this routine is actually in the planner */
		children = find_all_inheritors(myrelid);

		/*
		 * find_all_inheritors does the recursive search of the
		 * inheritance hierarchy, so all we have to do is process all of
		 * the relids in the list that it returns.
		 */
		foreach(child, children)
		{
			Oid			childrelid = lfirsti(child);

			if (childrelid == myrelid)
				continue;

			AlterTableAddColumn(childrelid, false, true, colDefChild);
		}
	}
	else
	{
		/*
		 * If we are told not to recurse, there had better not be any
		 * child tables; else the addition would put them out of step.
		 */
		if (!recursing &&
			find_inheritance_children(myrelid) != NIL)
			elog(ERROR, "Attribute must be added to child tables too");
	}

	/*
	 * OK, get on with it...
	 *
	 * Implementation restrictions: because we don't touch the table rows,
	 * the new column values will initially appear to be NULLs.  (This
	 * happens because the heap tuple access routines always check for
	 * attnum > # of attributes in tuple, and return NULL if so.)
	 * Therefore we can't support a DEFAULT value in SQL92-compliant
	 * fashion, and we also can't allow a NOT NULL constraint.
	 *
	 * We do allow CHECK constraints, even though these theoretically could
	 * fail for NULL rows (eg, CHECK (newcol IS NOT NULL)).
	 */
	if (colDef->raw_default || colDef->cooked_default)
		elog(ERROR, "Adding columns with defaults is not implemented."
			 "\n\tAdd the column, then use ALTER TABLE SET DEFAULT.");

	if (colDef->is_not_null)
		elog(ERROR, "Adding NOT NULL columns is not implemented."
		   "\n\tAdd the column, then use ALTER TABLE ... SET NOT NULL.");

	pgclass = heap_openr(RelationRelationName, RowExclusiveLock);

	reltup = SearchSysCache(RELOID,
							ObjectIdGetDatum(myrelid),
							0, 0, 0);
	if (!HeapTupleIsValid(reltup))
		elog(ERROR, "ALTER TABLE: relation \"%s\" not found",
			 RelationGetRelationName(rel));

	/*
	 * this test is deliberately not attisdropped-aware, since if one
	 * tries to add a column matching a dropped column name, it's gonna
	 * fail anyway.
	 */
	if (SearchSysCacheExists(ATTNAME,
							 ObjectIdGetDatum(myrelid),
							 PointerGetDatum(colDef->colname),
							 0, 0))
		elog(ERROR, "ALTER TABLE: column name \"%s\" already exists in table \"%s\"",
			 colDef->colname, RelationGetRelationName(rel));

	minattnum = ((Form_pg_class) GETSTRUCT(reltup))->relnatts;
	maxatts = minattnum + 1;
	if (maxatts > MaxHeapAttributeNumber)
		elog(ERROR, "ALTER TABLE: relations limited to %d columns",
			 MaxHeapAttributeNumber);
	i = minattnum + 1;

	attrdesc = heap_openr(AttributeRelationName, RowExclusiveLock);

	if (colDef->typename->arrayBounds)
		attndims = length(colDef->typename->arrayBounds);
	else
		attndims = 0;

	typeTuple = typenameType(colDef->typename);
	tform = (Form_pg_type) GETSTRUCT(typeTuple);

	attributeTuple = heap_addheader(Natts_pg_attribute,
									false,
									ATTRIBUTE_TUPLE_SIZE,
									(void *) &attributeD);

	attribute = (Form_pg_attribute) GETSTRUCT(attributeTuple);

	attribute->attrelid = myrelid;
	namestrcpy(&(attribute->attname), colDef->colname);
	attribute->atttypid = HeapTupleGetOid(typeTuple);
	attribute->attstattarget = -1;
	attribute->attlen = tform->typlen;
	attribute->attcacheoff = -1;
	attribute->atttypmod = colDef->typename->typmod;
	attribute->attnum = i;
	attribute->attbyval = tform->typbyval;
	attribute->attndims = attndims;
	attribute->attisset = (bool) (tform->typtype == 'c');
	attribute->attstorage = tform->typstorage;
	attribute->attalign = tform->typalign;
	attribute->attnotnull = colDef->is_not_null;
	attribute->atthasdef = (colDef->raw_default != NULL ||
							colDef->cooked_default != NULL);
	attribute->attisdropped = false;
	attribute->attisinherited = colDef->is_inherited;

	ReleaseSysCache(typeTuple);

	simple_heap_insert(attrdesc, attributeTuple);

	/* Update indexes on pg_attribute */
	CatalogUpdateIndexes(attrdesc, attributeTuple);

	heap_close(attrdesc, RowExclusiveLock);

	/*
	 * Update number of attributes in pg_class tuple
	 */
	newreltup = heap_copytuple(reltup);

	((Form_pg_class) GETSTRUCT(newreltup))->relnatts = maxatts;

	simple_heap_update(pgclass, &newreltup->t_self, newreltup);

	/* keep catalog indexes current */
	CatalogUpdateIndexes(pgclass, newreltup);

	heap_freetuple(newreltup);
	ReleaseSysCache(reltup);

	heap_close(pgclass, NoLock);

	heap_close(rel, NoLock);	/* close rel but keep lock! */

	/*
	 * Add datatype dependency for the new column.
	 */
	myself.classId = RelOid_pg_class;
	myself.objectId = myrelid;
	myself.objectSubId = i;
	referenced.classId = RelOid_pg_type;
	referenced.objectId = attribute->atttypid;
	referenced.objectSubId = 0;
	recordDependencyOn(&myself, &referenced, DEPENDENCY_NORMAL);

	/*
	 * Make our catalog updates visible for subsequent steps.
	 */
	CommandCounterIncrement();

	/*
	 * Add any CHECK constraints attached to the new column.
	 *
	 * To do this we must re-open the rel so that its new attr list gets
	 * loaded into the relcache.
	 */
	if (colDef->constraints != NIL)
	{
		rel = heap_open(myrelid, AccessExclusiveLock);
		AddRelationRawConstraints(rel, NIL, colDef->constraints);
		heap_close(rel, NoLock);
	}

	/*
	 * Automatically create the secondary relation for TOAST if it
	 * formerly had no such but now has toastable attributes.
	 */
	AlterTableCreateToastTable(myrelid, true);
}

/*
 * ALTER TABLE ALTER COLUMN DROP NOT NULL
 */
void
AlterTableAlterColumnDropNotNull(Oid myrelid, bool recurse,
								 const char *colName)
{
	Relation	rel;
	HeapTuple	tuple;
	AttrNumber	attnum;
	Relation	attr_rel;
	List	   *indexoidlist;
	List	   *indexoidscan;

	rel = heap_open(myrelid, AccessExclusiveLock);

	if (rel->rd_rel->relkind != RELKIND_RELATION)
		elog(ERROR, "ALTER TABLE: relation \"%s\" is not a table",
			 RelationGetRelationName(rel));

	if (!allowSystemTableMods
		&& IsSystemRelation(rel))
		elog(ERROR, "ALTER TABLE: relation \"%s\" is a system catalog",
			 RelationGetRelationName(rel));

	if (!pg_class_ownercheck(myrelid, GetUserId()))
		aclcheck_error(ACLCHECK_NOT_OWNER, RelationGetRelationName(rel));

	/*
	 * Propagate to children if desired
	 */
	if (recurse)
	{
		List	   *child,
				   *children;

		/* this routine is actually in the planner */
		children = find_all_inheritors(myrelid);

		/*
		 * find_all_inheritors does the recursive search of the
		 * inheritance hierarchy, so all we have to do is process all of
		 * the relids in the list that it returns.
		 */
		foreach(child, children)
		{
			Oid			childrelid = lfirsti(child);

			if (childrelid == myrelid)
				continue;
			AlterTableAlterColumnDropNotNull(childrelid,
											 false, colName);
		}
	}

	/* -= now do the thing on this relation =- */

	/*
	 * get the number of the attribute
	 */
	attnum = get_attnum(myrelid, colName);
	if (attnum == InvalidAttrNumber)
		elog(ERROR, "Relation \"%s\" has no column \"%s\"",
			 RelationGetRelationName(rel), colName);

	/* Prevent them from altering a system attribute */
	if (attnum < 0)
		elog(ERROR, "ALTER TABLE: Cannot alter system attribute \"%s\"",
			 colName);

	/*
	 * Check that the attribute is not in a primary key
	 */

	/* Loop over all indexes on the relation */
	indexoidlist = RelationGetIndexList(rel);

	foreach(indexoidscan, indexoidlist)
	{
		Oid			indexoid = lfirsti(indexoidscan);
		HeapTuple	indexTuple;
		Form_pg_index indexStruct;
		int			i;

		indexTuple = SearchSysCache(INDEXRELID,
									ObjectIdGetDatum(indexoid),
									0, 0, 0);
		if (!HeapTupleIsValid(indexTuple))
			elog(ERROR, "ALTER TABLE: Index %u not found",
				 indexoid);
		indexStruct = (Form_pg_index) GETSTRUCT(indexTuple);

		/* If the index is not a primary key, skip the check */
		if (indexStruct->indisprimary)
		{
			/*
			 * Loop over each attribute in the primary key and see if it
			 * matches the to-be-altered attribute
			 */
			for (i = 0; i < INDEX_MAX_KEYS &&
				 indexStruct->indkey[i] != InvalidAttrNumber; i++)
			{
				if (indexStruct->indkey[i] == attnum)
					elog(ERROR, "ALTER TABLE: Attribute \"%s\" is in a primary key", colName);
			}
		}

		ReleaseSysCache(indexTuple);
	}

	freeList(indexoidlist);

	/*
	 * Okay, actually perform the catalog change
	 */
	attr_rel = heap_openr(AttributeRelationName, RowExclusiveLock);

	tuple = SearchSysCacheCopyAttName(myrelid, colName);
	if (!HeapTupleIsValid(tuple))		/* shouldn't happen */
		elog(ERROR, "ALTER TABLE: relation \"%s\" has no column \"%s\"",
			 RelationGetRelationName(rel), colName);

	((Form_pg_attribute) GETSTRUCT(tuple))->attnotnull = FALSE;

	simple_heap_update(attr_rel, &tuple->t_self, tuple);

	/* keep the system catalog indexes current */
	CatalogUpdateIndexes(attr_rel, tuple);

	heap_close(attr_rel, RowExclusiveLock);

	heap_close(rel, NoLock);
}

/*
 * ALTER TABLE ALTER COLUMN SET NOT NULL
 */
void
AlterTableAlterColumnSetNotNull(Oid myrelid, bool recurse,
								const char *colName)
{
	Relation	rel;
	HeapTuple	tuple;
	AttrNumber	attnum;
	Relation	attr_rel;
	HeapScanDesc scan;
	TupleDesc	tupdesc;

	rel = heap_open(myrelid, AccessExclusiveLock);

	if (rel->rd_rel->relkind != RELKIND_RELATION)
		elog(ERROR, "ALTER TABLE: relation \"%s\" is not a table",
			 RelationGetRelationName(rel));

	if (!allowSystemTableMods
		&& IsSystemRelation(rel))
		elog(ERROR, "ALTER TABLE: relation \"%s\" is a system catalog",
			 RelationGetRelationName(rel));

	if (!pg_class_ownercheck(myrelid, GetUserId()))
		aclcheck_error(ACLCHECK_NOT_OWNER, RelationGetRelationName(rel));

	/*
	 * Propagate to children if desired
	 */
	if (recurse)
	{
		List	   *child,
				   *children;

		/* this routine is actually in the planner */
		children = find_all_inheritors(myrelid);

		/*
		 * find_all_inheritors does the recursive search of the
		 * inheritance hierarchy, so all we have to do is process all of
		 * the relids in the list that it returns.
		 */
		foreach(child, children)
		{
			Oid			childrelid = lfirsti(child);

			if (childrelid == myrelid)
				continue;
			AlterTableAlterColumnSetNotNull(childrelid,
											false, colName);
		}
	}

	/* -= now do the thing on this relation =- */

	/*
	 * get the number of the attribute
	 */
	attnum = get_attnum(myrelid, colName);
	if (attnum == InvalidAttrNumber)
		elog(ERROR, "Relation \"%s\" has no column \"%s\"",
			 RelationGetRelationName(rel), colName);

	/* Prevent them from altering a system attribute */
	if (attnum < 0)
		elog(ERROR, "ALTER TABLE: Cannot alter system attribute \"%s\"",
			 colName);

	/*
	 * Perform a scan to ensure that there are no NULL values already in
	 * the relation
	 */
	tupdesc = RelationGetDescr(rel);

	scan = heap_beginscan(rel, SnapshotNow, 0, NULL);

	while ((tuple = heap_getnext(scan, ForwardScanDirection)) != NULL)
	{
		Datum		d;
		bool		isnull;

		d = heap_getattr(tuple, attnum, tupdesc, &isnull);

		if (isnull)
			elog(ERROR, "ALTER TABLE: Attribute \"%s\" contains NULL values",
				 colName);
	}

	heap_endscan(scan);

	/*
	 * Okay, actually perform the catalog change
	 */
	attr_rel = heap_openr(AttributeRelationName, RowExclusiveLock);

	tuple = SearchSysCacheCopyAttName(myrelid, colName);
	if (!HeapTupleIsValid(tuple))		/* shouldn't happen */
		elog(ERROR, "ALTER TABLE: relation \"%s\" has no column \"%s\"",
			 RelationGetRelationName(rel), colName);

	((Form_pg_attribute) GETSTRUCT(tuple))->attnotnull = TRUE;

	simple_heap_update(attr_rel, &tuple->t_self, tuple);

	/* keep the system catalog indexes current */
	CatalogUpdateIndexes(attr_rel, tuple);

	heap_close(attr_rel, RowExclusiveLock);

	heap_close(rel, NoLock);
}


/*
 * ALTER TABLE ALTER COLUMN SET/DROP DEFAULT
 */
void
AlterTableAlterColumnDefault(Oid myrelid, bool recurse,
							 const char *colName,
							 Node *newDefault)
{
	Relation	rel;
	AttrNumber	attnum;

	rel = heap_open(myrelid, AccessExclusiveLock);

	/*
	 * We allow defaults on views so that INSERT into a view can have
	 * default-ish behavior.  This works because the rewriter substitutes
	 * default values into INSERTs before it expands rules.
	 */
	if (rel->rd_rel->relkind != RELKIND_RELATION &&
		rel->rd_rel->relkind != RELKIND_VIEW)
		elog(ERROR, "ALTER TABLE: relation \"%s\" is not a table or view",
			 RelationGetRelationName(rel));

	if (!allowSystemTableMods
		&& IsSystemRelation(rel))
		elog(ERROR, "ALTER TABLE: relation \"%s\" is a system catalog",
			 RelationGetRelationName(rel));

	if (!pg_class_ownercheck(myrelid, GetUserId()))
		aclcheck_error(ACLCHECK_NOT_OWNER, RelationGetRelationName(rel));

	/*
	 * Propagate to children if desired
	 */
	if (recurse)
	{
		List	   *child,
				   *children;

		/* this routine is actually in the planner */
		children = find_all_inheritors(myrelid);

		/*
		 * find_all_inheritors does the recursive search of the
		 * inheritance hierarchy, so all we have to do is process all of
		 * the relids in the list that it returns.
		 */
		foreach(child, children)
		{
			Oid			childrelid = lfirsti(child);

			if (childrelid == myrelid)
				continue;
			AlterTableAlterColumnDefault(childrelid,
										 false, colName, newDefault);
		}
	}

	/* -= now do the thing on this relation =- */

	/*
	 * get the number of the attribute
	 */
	attnum = get_attnum(myrelid, colName);
	if (attnum == InvalidAttrNumber)
		elog(ERROR, "Relation \"%s\" has no column \"%s\"",
			 RelationGetRelationName(rel), colName);

	/* Prevent them from altering a system attribute */
	if (attnum < 0)
		elog(ERROR, "ALTER TABLE: Cannot alter system attribute \"%s\"",
			 colName);

	/*
	 * Remove any old default for the column.  We use RESTRICT here for
	 * safety, but at present we do not expect anything to depend on the
	 * default.
	 */
	RemoveAttrDefault(myrelid, attnum, DROP_RESTRICT, false);

	if (newDefault)
	{
		/* SET DEFAULT */
		RawColumnDefault *rawEnt;

		rawEnt = (RawColumnDefault *) palloc(sizeof(RawColumnDefault));
		rawEnt->attnum = attnum;
		rawEnt->raw_default = newDefault;

		/*
		 * This function is intended for CREATE TABLE, so it processes a
		 * _list_ of defaults, but we just do one.
		 */
		AddRelationRawConstraints(rel, makeList1(rawEnt), NIL);
	}

	heap_close(rel, NoLock);
}

/*
 * ALTER TABLE ALTER COLUMN SET STATISTICS / STORAGE
 */
void
AlterTableAlterColumnFlags(Oid myrelid, bool recurse,
						   const char *colName,
						   Node *flagValue, const char *flagType)
{
	Relation	rel;
	int			newtarget = 1;
	char		newstorage = 'p';
	Relation	attrelation;
	HeapTuple	tuple;
	Form_pg_attribute attrtuple;

	rel = heap_open(myrelid, AccessExclusiveLock);

	if (rel->rd_rel->relkind != RELKIND_RELATION)
		elog(ERROR, "ALTER TABLE: relation \"%s\" is not a table",
			 RelationGetRelationName(rel));

	/*
	 * we allow statistics case for system tables
	 */
	if (*flagType != 'S' && !allowSystemTableMods && IsSystemRelation(rel))
		elog(ERROR, "ALTER TABLE: relation \"%s\" is a system catalog",
			 RelationGetRelationName(rel));

	if (!pg_class_ownercheck(myrelid, GetUserId()))
		aclcheck_error(ACLCHECK_NOT_OWNER, RelationGetRelationName(rel));

	/*
	 * Check the supplied parameters before anything else
	 */
	if (*flagType == 'S')
	{
		/* STATISTICS */
		Assert(IsA(flagValue, Integer));
		newtarget = intVal(flagValue);

		/*
		 * Limit target to a sane range
		 */
		if (newtarget < -1)
		{
			elog(ERROR, "ALTER TABLE: statistics target %d is too low",
				 newtarget);
		}
		else if (newtarget > 1000)
		{
			elog(WARNING, "ALTER TABLE: lowering statistics target to 1000");
			newtarget = 1000;
		}
	}
	else if (*flagType == 'M')
	{
		/* STORAGE */
		char	   *storagemode;

		Assert(IsA(flagValue, String));
		storagemode = strVal(flagValue);

		if (strcasecmp(storagemode, "plain") == 0)
			newstorage = 'p';
		else if (strcasecmp(storagemode, "external") == 0)
			newstorage = 'e';
		else if (strcasecmp(storagemode, "extended") == 0)
			newstorage = 'x';
		else if (strcasecmp(storagemode, "main") == 0)
			newstorage = 'm';
		else
			elog(ERROR, "ALTER TABLE: \"%s\" storage not recognized",
				 storagemode);
	}
	else
	{
		elog(ERROR, "ALTER TABLE: Invalid column flag: %c",
			 (int) *flagType);
	}

	/*
	 * Propagate to children if desired
	 */
	if (recurse)
	{
		List	   *child,
				   *children;

		/* this routine is actually in the planner */
		children = find_all_inheritors(myrelid);

		/*
		 * find_all_inheritors does the recursive search of the
		 * inheritance hierarchy, so all we have to do is process all of
		 * the relids in the list that it returns.
		 */
		foreach(child, children)
		{
			Oid			childrelid = lfirsti(child);

			if (childrelid == myrelid)
				continue;
			AlterTableAlterColumnFlags(childrelid,
									false, colName, flagValue, flagType);
		}
	}

	/* -= now do the thing on this relation =- */

	attrelation = heap_openr(AttributeRelationName, RowExclusiveLock);

	tuple = SearchSysCacheCopyAttName(myrelid, colName);
	if (!HeapTupleIsValid(tuple))
		elog(ERROR, "ALTER TABLE: relation \"%s\" has no column \"%s\"",
			 RelationGetRelationName(rel), colName);
	attrtuple = (Form_pg_attribute) GETSTRUCT(tuple);

	if (attrtuple->attnum < 0)
		elog(ERROR, "ALTER TABLE: cannot change system attribute \"%s\"",
			 colName);

	/*
	 * Now change the appropriate field
	 */
	if (*flagType == 'S')
		attrtuple->attstattarget = newtarget;
	else if (*flagType == 'M')
	{
		/*
		 * safety check: do not allow toasted storage modes unless column
		 * datatype is TOAST-aware.
		 */
		if (newstorage == 'p' || TypeIsToastable(attrtuple->atttypid))
			attrtuple->attstorage = newstorage;
		else
			elog(ERROR, "ALTER TABLE: Column datatype %s can only have storage \"plain\"",
				 format_type_be(attrtuple->atttypid));
	}

	simple_heap_update(attrelation, &tuple->t_self, tuple);

	/* keep system catalog indexes current */
	CatalogUpdateIndexes(attrelation, tuple);

	heap_freetuple(tuple);

	heap_close(attrelation, NoLock);
	heap_close(rel, NoLock);	/* close rel, but keep lock! */
}


/*
 * ALTER TABLE DROP COLUMN
 */
void
AlterTableDropColumn(Oid myrelid, bool recurse, bool recursing,
					 const char *colName,
					 DropBehavior behavior)
{
	Relation	rel;
	AttrNumber	attnum;
	AttrNumber	n;
	TupleDesc	tupleDesc;
	bool		success;
	ObjectAddress object;

	rel = heap_open(myrelid, AccessExclusiveLock);

	if (rel->rd_rel->relkind != RELKIND_RELATION)
		elog(ERROR, "ALTER TABLE: relation \"%s\" is not a table",
			 RelationGetRelationName(rel));

	if (!allowSystemTableMods
		&& IsSystemRelation(rel))
		elog(ERROR, "ALTER TABLE: relation \"%s\" is a system catalog",
			 RelationGetRelationName(rel));

	if (!pg_class_ownercheck(myrelid, GetUserId()))
		aclcheck_error(ACLCHECK_NOT_OWNER, RelationGetRelationName(rel));

	/*
	 * get the number of the attribute
	 */
	attnum = get_attnum(myrelid, colName);
	if (attnum == InvalidAttrNumber)
		elog(ERROR, "Relation \"%s\" has no column \"%s\"",
			 RelationGetRelationName(rel), colName);

	/* Can't drop a system attribute */
	if (attnum < 0)
		elog(ERROR, "ALTER TABLE: Cannot drop system attribute \"%s\"",
			 colName);

	/*
	 * Make sure there will be at least one user column left in the
	 * relation after we drop this one.  Zero-length tuples tend to
	 * confuse us.
	 */
	tupleDesc = RelationGetDescr(rel);

	success = false;
	for (n = 1; n <= tupleDesc->natts; n++)
	{
		Form_pg_attribute attribute = tupleDesc->attrs[n - 1];

		if (!attribute->attisdropped && n != attnum)
		{
			success = true;
			break;
		}
	}

	if (!success)
		elog(ERROR, "ALTER TABLE: Cannot drop last column from table \"%s\"",
			 RelationGetRelationName(rel));

	/* Don't drop inherited columns */
	if (tupleDesc->attrs[attnum - 1]->attisinherited && !recursing)
		elog(ERROR, "ALTER TABLE: Cannot drop inherited column \"%s\"",
			 colName);

	/*
	 * If we are asked to drop ONLY in this table (no recursion), we need
	 * to mark the inheritors' attribute as non-inherited.
	 */
	if (!recurse && !recursing)
	{
		Relation	attr_rel;
		List	   *child,
				   *children;

		/* We only want direct inheritors in this case */
		children = find_inheritance_children(myrelid);

		attr_rel = heap_openr(AttributeRelationName, RowExclusiveLock);
		foreach(child, children)
		{
			Oid			childrelid = lfirsti(child);
			Relation	childrel;
			HeapTuple	tuple;

			childrel = heap_open(childrelid, AccessExclusiveLock);

			tuple = SearchSysCacheCopyAttName(childrelid, colName);
			if (!HeapTupleIsValid(tuple))		/* shouldn't happen */
				elog(ERROR, "ALTER TABLE: relation %u has no column \"%s\"",
					 childrelid, colName);

			((Form_pg_attribute) GETSTRUCT(tuple))->attisinherited = false;

			simple_heap_update(attr_rel, &tuple->t_self, tuple);

			/* keep the system catalog indexes current */
			CatalogUpdateIndexes(attr_rel, tuple);

			heap_close(childrel, NoLock);
		}
		heap_close(attr_rel, RowExclusiveLock);
	}

	/*
	 * Propagate to children if desired
	 */
	if (recurse)
	{
		List	   *child,
				   *children;

		/* this routine is actually in the planner */
		children = find_all_inheritors(myrelid);

		/*
		 * find_all_inheritors does the recursive search of the
		 * inheritance hierarchy, so all we have to do is process all of
		 * the relids in the list that it returns.
		 */
		foreach(child, children)
		{
			Oid			childrelid = lfirsti(child);

			if (childrelid == myrelid)
				continue;
			AlterTableDropColumn(childrelid, false, true, colName, behavior);
		}
	}

	/*
	 * Perform the actual deletion
	 */
	object.classId = RelOid_pg_class;
	object.objectId = myrelid;
	object.objectSubId = attnum;

	performDeletion(&object, behavior);

	heap_close(rel, NoLock);	/* close rel, but keep lock! */
}


/*
 * ALTER TABLE ADD CONSTRAINT
 */
void
AlterTableAddConstraint(Oid myrelid, bool recurse,
						List *newConstraints)
{
	Relation	rel;
	List	   *listptr;
	int			counter = 0;

	/*
	 * Grab an exclusive lock on the target table, which we will NOT
	 * release until end of transaction.
	 */
	rel = heap_open(myrelid, AccessExclusiveLock);

	if (rel->rd_rel->relkind != RELKIND_RELATION)
		elog(ERROR, "ALTER TABLE: relation \"%s\" is not a table",
			 RelationGetRelationName(rel));

	if (!allowSystemTableMods
		&& IsSystemRelation(rel))
		elog(ERROR, "ALTER TABLE: relation \"%s\" is a system catalog",
			 RelationGetRelationName(rel));

	if (!pg_class_ownercheck(myrelid, GetUserId()))
		aclcheck_error(ACLCHECK_NOT_OWNER, RelationGetRelationName(rel));

	if (recurse)
	{
		List	   *child,
				   *children;

		/* this routine is actually in the planner */
		children = find_all_inheritors(myrelid);

		/*
		 * find_all_inheritors does the recursive search of the
		 * inheritance hierarchy, so all we have to do is process all of
		 * the relids in the list that it returns.
		 */
		foreach(child, children)
		{
			Oid			childrelid = lfirsti(child);

			if (childrelid == myrelid)
				continue;
			AlterTableAddConstraint(childrelid, false, newConstraints);
		}
	}

	foreach(listptr, newConstraints)
	{
		/*
		 * copy is because we may destructively alter the node below by
		 * inserting a generated name; this name is not necessarily
		 * correct for children or parents.
		 */
		Node	   *newConstraint = copyObject(lfirst(listptr));

		switch (nodeTag(newConstraint))
		{
			case T_Constraint:
				{
					Constraint *constr = (Constraint *) newConstraint;

					/*
					 * Currently, we only expect to see CONSTR_CHECK nodes
					 * arriving here (see the preprocessing done in
					 * parser/analyze.c).  Use a switch anyway to make it
					 * easier to add more code later.
					 */
					switch (constr->contype)
					{
						case CONSTR_CHECK:
							{
								ParseState *pstate;
								bool		successful = true;
								HeapScanDesc scan;
								ExprContext *econtext;
								TupleTableSlot *slot;
								HeapTuple	tuple;
								RangeTblEntry *rte;
								List	   *qual;
								Node	   *expr;

								/*
								 * Assign or validate constraint name
								 */
								if (constr->name)
								{
									if (ConstraintNameIsUsed(RelationGetRelid(rel),
											   RelationGetNamespace(rel),
														   constr->name))
										elog(ERROR, "constraint \"%s\" already exists for relation \"%s\"",
											 constr->name,
										   RelationGetRelationName(rel));
								}
								else
									constr->name = GenerateConstraintName(RelationGetRelid(rel),
											   RelationGetNamespace(rel),
															   &counter);

								/*
								 * We need to make a parse state and range
								 * table to allow us to transformExpr and
								 * fix_opids to get a version of the
								 * expression we can pass to ExecQual
								 */
								pstate = make_parsestate(NULL);
								rte = addRangeTableEntryForRelation(pstate,
																 myrelid,
																	makeAlias(RelationGetRelationName(rel), NIL),
																	false,
																	true);
								addRTEtoQuery(pstate, rte, true, true);

								/*
								 * Convert the A_EXPR in raw_expr into an
								 * EXPR
								 */
								expr = transformExpr(pstate, constr->raw_expr);

								/*
								 * Make sure it yields a boolean result.
								 */
								expr = coerce_to_boolean(expr, "CHECK");

								/*
								 * Make sure no outside relations are
								 * referred to.
								 */
								if (length(pstate->p_rtable) != 1)
									elog(ERROR, "Only relation '%s' can be referenced in CHECK",
										 RelationGetRelationName(rel));

								/*
								 * No subplans or aggregates, either...
								 */
								if (contain_subplans(expr))
									elog(ERROR, "cannot use subselect in CHECK constraint expression");
								if (contain_agg_clause(expr))
									elog(ERROR, "cannot use aggregate function in CHECK constraint expression");

								/*
								 * Might as well try to reduce any
								 * constant expressions.
								 */
								expr = eval_const_expressions(expr);

								/* And fix the opids */
								fix_opids(expr);

								qual = makeList1(expr);

								/* Make tuple slot to hold tuples */
								slot = MakeTupleTableSlot();
								ExecSetSlotDescriptor(slot, RelationGetDescr(rel), false);
								/* Make an expression context for ExecQual */
								econtext = MakeExprContext(slot, CurrentMemoryContext);

								/*
								 * Scan through the rows now, checking the
								 * expression at each row.
								 */
								scan = heap_beginscan(rel, SnapshotNow, 0, NULL);

								while ((tuple = heap_getnext(scan, ForwardScanDirection)) != NULL)
								{
									ExecStoreTuple(tuple, slot, InvalidBuffer, false);
									if (!ExecQual(qual, econtext, true))
									{
										successful = false;
										break;
									}
									ResetExprContext(econtext);
								}

								heap_endscan(scan);

								FreeExprContext(econtext);
								pfree(slot);

								if (!successful)
									elog(ERROR, "AlterTableAddConstraint: rejected due to CHECK constraint %s",
										 constr->name);

								/*
								 * Call AddRelationRawConstraints to do
								 * the real adding -- It duplicates some
								 * of the above, but does not check the
								 * validity of the constraint against
								 * tuples already in the table.
								 */
								AddRelationRawConstraints(rel, NIL,
													  makeList1(constr));

								break;
							}
						default:
							elog(ERROR, "ALTER TABLE / ADD CONSTRAINT is not implemented for that constraint type.");
					}
					break;
				}
			case T_FkConstraint:
				{
					FkConstraint *fkconstraint = (FkConstraint *) newConstraint;
					Relation	pkrel;
					Oid			constrOid;

					/*
					 * Assign or validate constraint name
					 */
					if (fkconstraint->constr_name)
					{
						if (ConstraintNameIsUsed(RelationGetRelid(rel),
											   RelationGetNamespace(rel),
											  fkconstraint->constr_name))
							elog(ERROR, "constraint \"%s\" already exists for relation \"%s\"",
								 fkconstraint->constr_name,
								 RelationGetRelationName(rel));
					}
					else
						fkconstraint->constr_name = GenerateConstraintName(RelationGetRelid(rel),
											   RelationGetNamespace(rel),
															   &counter);

					/*
					 * Grab an exclusive lock on the pk table, so that
					 * someone doesn't delete rows out from under us.
					 * (Although a lesser lock would do for that purpose,
					 * we'll need exclusive lock anyway to add triggers to
					 * the pk table; trying to start with a lesser lock
					 * will just create a risk of deadlock.)
					 */
					pkrel = heap_openrv(fkconstraint->pktable,
										AccessExclusiveLock);

					/*
					 * Validity checks
					 */
					if (pkrel->rd_rel->relkind != RELKIND_RELATION)
						elog(ERROR, "referenced relation \"%s\" is not a table",
							 RelationGetRelationName(pkrel));

					if (!allowSystemTableMods
						&& IsSystemRelation(pkrel))
						elog(ERROR, "ALTER TABLE: relation \"%s\" is a system catalog",
							 RelationGetRelationName(pkrel));

					/* XXX shouldn't there be a permission check too? */

					if (isTempNamespace(RelationGetNamespace(pkrel)) &&
						!isTempNamespace(RelationGetNamespace(rel)))
						elog(ERROR, "ALTER TABLE / ADD CONSTRAINT: Unable to reference temporary table from permanent table constraint");

					/*
					 * Check that the constraint is satisfied by existing
					 * rows (we can skip this during table creation).
					 *
					 * NOTE: we assume parser has already checked for
					 * existence of an appropriate unique index on the
					 * referenced relation, and that the column datatypes
					 * are comparable.
					 */
					if (!fkconstraint->skip_validation)
						validateForeignKeyConstraint(fkconstraint, rel, pkrel);

					/*
					 * Record the FK constraint in pg_constraint.
					 */
					constrOid = createForeignKeyConstraint(rel, pkrel,
														   fkconstraint);

					/*
					 * Create the triggers that will enforce the
					 * constraint.
					 */
					createForeignKeyTriggers(rel, fkconstraint, constrOid);

					/*
					 * Close pk table, but keep lock until we've
					 * committed.
					 */
					heap_close(pkrel, NoLock);

					break;
				}
			default:
				elog(ERROR, "ALTER TABLE / ADD CONSTRAINT unable to determine type of constraint passed");
		}
	}

	/* Close rel, but keep lock till commit */
	heap_close(rel, NoLock);
}

/*
 * Scan the existing rows in a table to verify they meet a proposed FK
 * constraint.
 *
 * Caller must have opened and locked both relations.
 */
static void
validateForeignKeyConstraint(FkConstraint *fkconstraint,
							 Relation rel,
							 Relation pkrel)
{
	HeapScanDesc scan;
	HeapTuple	tuple;
	Trigger		trig;
	List	   *list;
	int			count;

	/*
	 * Scan through each tuple, calling RI_FKey_check_ins (insert trigger)
	 * as if that tuple had just been inserted.  If any of those fail, it
	 * should elog(ERROR) and that's that.
	 */
	MemSet(&trig, 0, sizeof(trig));
	trig.tgoid = InvalidOid;
	trig.tgname = fkconstraint->constr_name;
	trig.tgenabled = TRUE;
	trig.tgisconstraint = TRUE;
	trig.tgconstrrelid = RelationGetRelid(pkrel);
	trig.tgdeferrable = FALSE;
	trig.tginitdeferred = FALSE;

	trig.tgargs = (char **) palloc(sizeof(char *) *
								   (4 + length(fkconstraint->fk_attrs)
									+ length(fkconstraint->pk_attrs)));

	trig.tgargs[0] = trig.tgname;
	trig.tgargs[1] = RelationGetRelationName(rel);
	trig.tgargs[2] = RelationGetRelationName(pkrel);
	trig.tgargs[3] = fkMatchTypeToString(fkconstraint->fk_matchtype);
	count = 4;
	foreach(list, fkconstraint->fk_attrs)
	{
		char	   *fk_at = strVal(lfirst(list));

		trig.tgargs[count] = fk_at;
		count += 2;
	}
	count = 5;
	foreach(list, fkconstraint->pk_attrs)
	{
		char	   *pk_at = strVal(lfirst(list));

		trig.tgargs[count] = pk_at;
		count += 2;
	}
	trig.tgnargs = count - 1;

	scan = heap_beginscan(rel, SnapshotNow, 0, NULL);

	while ((tuple = heap_getnext(scan, ForwardScanDirection)) != NULL)
	{
		FunctionCallInfoData fcinfo;
		TriggerData trigdata;

		/*
		 * Make a call to the trigger function
		 *
		 * No parameters are passed, but we do set a context
		 */
		MemSet(&fcinfo, 0, sizeof(fcinfo));

		/*
		 * We assume RI_FKey_check_ins won't look at flinfo...
		 */
		trigdata.type = T_TriggerData;
		trigdata.tg_event = TRIGGER_EVENT_INSERT | TRIGGER_EVENT_ROW;
		trigdata.tg_relation = rel;
		trigdata.tg_trigtuple = tuple;
		trigdata.tg_newtuple = NULL;
		trigdata.tg_trigger = &trig;

		fcinfo.context = (Node *) &trigdata;

		RI_FKey_check_ins(&fcinfo);
	}

	heap_endscan(scan);

	pfree(trig.tgargs);
}

/*
 * Record an FK constraint in pg_constraint.
 */
static Oid
createForeignKeyConstraint(Relation rel, Relation pkrel,
						   FkConstraint *fkconstraint)
{
	int16	   *fkattr;
	int16	   *pkattr;
	int			fkcount;
	int			pkcount;
	List	   *l;
	int			i;

	/* Convert foreign-key attr names to attr number array */
	fkcount = length(fkconstraint->fk_attrs);
	fkattr = (int16 *) palloc(fkcount * sizeof(int16));
	i = 0;
	foreach(l, fkconstraint->fk_attrs)
	{
		char	   *id = strVal(lfirst(l));
		AttrNumber	attno;

		attno = get_attnum(RelationGetRelid(rel), id);
		if (attno == InvalidAttrNumber)
			elog(ERROR, "Relation \"%s\" has no column \"%s\"",
				 RelationGetRelationName(rel), id);
		fkattr[i++] = attno;
	}

	/* The same for the referenced primary key attrs */
	pkcount = length(fkconstraint->pk_attrs);
	pkattr = (int16 *) palloc(pkcount * sizeof(int16));
	i = 0;
	foreach(l, fkconstraint->pk_attrs)
	{
		char	   *id = strVal(lfirst(l));
		AttrNumber	attno;

		attno = get_attnum(RelationGetRelid(pkrel), id);
		if (attno == InvalidAttrNumber)
			elog(ERROR, "Relation \"%s\" has no column \"%s\"",
				 RelationGetRelationName(pkrel), id);
		pkattr[i++] = attno;
	}

	/* Now we can make the pg_constraint entry */
	return CreateConstraintEntry(fkconstraint->constr_name,
								 RelationGetNamespace(rel),
								 CONSTRAINT_FOREIGN,
								 fkconstraint->deferrable,
								 fkconstraint->initdeferred,
								 RelationGetRelid(rel),
								 fkattr,
								 fkcount,
								 InvalidOid,	/* not a domain constraint */
								 RelationGetRelid(pkrel),
								 pkattr,
								 pkcount,
								 fkconstraint->fk_upd_action,
								 fkconstraint->fk_del_action,
								 fkconstraint->fk_matchtype,
								 NULL,	/* no check constraint */
								 NULL,
								 NULL);
}

/*
 * Create the triggers that implement an FK constraint.
 */
static void
createForeignKeyTriggers(Relation rel, FkConstraint *fkconstraint,
						 Oid constrOid)
{
	RangeVar   *myRel;
	CreateTrigStmt *fk_trigger;
	List	   *fk_attr;
	List	   *pk_attr;
	ObjectAddress trigobj,
				constrobj;

	/*
	 * Reconstruct a RangeVar for my relation (not passed in,
	 * unfortunately).
	 */
	myRel = makeRangeVar(get_namespace_name(RelationGetNamespace(rel)),
						 pstrdup(RelationGetRelationName(rel)));

	/*
	 * Preset objectAddress fields
	 */
	constrobj.classId = get_system_catalog_relid(ConstraintRelationName);
	constrobj.objectId = constrOid;
	constrobj.objectSubId = 0;
	trigobj.classId = get_system_catalog_relid(TriggerRelationName);
	trigobj.objectSubId = 0;

	/* Make changes-so-far visible */
	CommandCounterIncrement();

	/*
	 * Build and execute a CREATE CONSTRAINT TRIGGER statement for the
	 * CHECK action.
	 */
	fk_trigger = makeNode(CreateTrigStmt);
	fk_trigger->trigname = fkconstraint->constr_name;
	fk_trigger->relation = myRel;
	fk_trigger->funcname = SystemFuncName("RI_FKey_check_ins");
	fk_trigger->before = false;
	fk_trigger->row = true;
	fk_trigger->actions[0] = 'i';
	fk_trigger->actions[1] = 'u';
	fk_trigger->actions[2] = '\0';
	fk_trigger->lang = NULL;
	fk_trigger->text = NULL;

	fk_trigger->attr = NIL;
	fk_trigger->when = NULL;
	fk_trigger->isconstraint = true;
	fk_trigger->deferrable = fkconstraint->deferrable;
	fk_trigger->initdeferred = fkconstraint->initdeferred;
	fk_trigger->constrrel = fkconstraint->pktable;

	fk_trigger->args = NIL;
	fk_trigger->args = lappend(fk_trigger->args,
							   makeString(fkconstraint->constr_name));
	fk_trigger->args = lappend(fk_trigger->args,
							   makeString(myRel->relname));
	fk_trigger->args = lappend(fk_trigger->args,
							 makeString(fkconstraint->pktable->relname));
	fk_trigger->args = lappend(fk_trigger->args,
			makeString(fkMatchTypeToString(fkconstraint->fk_matchtype)));
	fk_attr = fkconstraint->fk_attrs;
	pk_attr = fkconstraint->pk_attrs;
	if (length(fk_attr) != length(pk_attr))
		elog(ERROR, "number of key attributes in referenced table must be equal to foreign key"
			 "\n\tIllegal FOREIGN KEY definition references \"%s\"",
			 fkconstraint->pktable->relname);

	while (fk_attr != NIL)
	{
		fk_trigger->args = lappend(fk_trigger->args, lfirst(fk_attr));
		fk_trigger->args = lappend(fk_trigger->args, lfirst(pk_attr));
		fk_attr = lnext(fk_attr);
		pk_attr = lnext(pk_attr);
	}

	trigobj.objectId = CreateTrigger(fk_trigger, true);

	/* Register dependency from trigger to constraint */
	recordDependencyOn(&trigobj, &constrobj, DEPENDENCY_INTERNAL);

	/* Make changes-so-far visible */
	CommandCounterIncrement();

	/*
	 * Build and execute a CREATE CONSTRAINT TRIGGER statement for the ON
	 * DELETE action on the referenced table.
	 */
	fk_trigger = makeNode(CreateTrigStmt);
	fk_trigger->trigname = fkconstraint->constr_name;
	fk_trigger->relation = fkconstraint->pktable;
	fk_trigger->before = false;
	fk_trigger->row = true;
	fk_trigger->actions[0] = 'd';
	fk_trigger->actions[1] = '\0';
	fk_trigger->lang = NULL;
	fk_trigger->text = NULL;

	fk_trigger->attr = NIL;
	fk_trigger->when = NULL;
	fk_trigger->isconstraint = true;
	fk_trigger->deferrable = fkconstraint->deferrable;
	fk_trigger->initdeferred = fkconstraint->initdeferred;
	fk_trigger->constrrel = myRel;
	switch (fkconstraint->fk_del_action)
	{
		case FKCONSTR_ACTION_NOACTION:
			fk_trigger->funcname = SystemFuncName("RI_FKey_noaction_del");
			break;
		case FKCONSTR_ACTION_RESTRICT:
			fk_trigger->deferrable = false;
			fk_trigger->initdeferred = false;
			fk_trigger->funcname = SystemFuncName("RI_FKey_restrict_del");
			break;
		case FKCONSTR_ACTION_CASCADE:
			fk_trigger->funcname = SystemFuncName("RI_FKey_cascade_del");
			break;
		case FKCONSTR_ACTION_SETNULL:
			fk_trigger->funcname = SystemFuncName("RI_FKey_setnull_del");
			break;
		case FKCONSTR_ACTION_SETDEFAULT:
			fk_trigger->funcname = SystemFuncName("RI_FKey_setdefault_del");
			break;
		default:
			elog(ERROR, "Unrecognized ON DELETE action for FOREIGN KEY constraint");
			break;
	}

	fk_trigger->args = NIL;
	fk_trigger->args = lappend(fk_trigger->args,
							   makeString(fkconstraint->constr_name));
	fk_trigger->args = lappend(fk_trigger->args,
							   makeString(myRel->relname));
	fk_trigger->args = lappend(fk_trigger->args,
							 makeString(fkconstraint->pktable->relname));
	fk_trigger->args = lappend(fk_trigger->args,
			makeString(fkMatchTypeToString(fkconstraint->fk_matchtype)));
	fk_attr = fkconstraint->fk_attrs;
	pk_attr = fkconstraint->pk_attrs;
	while (fk_attr != NIL)
	{
		fk_trigger->args = lappend(fk_trigger->args, lfirst(fk_attr));
		fk_trigger->args = lappend(fk_trigger->args, lfirst(pk_attr));
		fk_attr = lnext(fk_attr);
		pk_attr = lnext(pk_attr);
	}

	trigobj.objectId = CreateTrigger(fk_trigger, true);

	/* Register dependency from trigger to constraint */
	recordDependencyOn(&trigobj, &constrobj, DEPENDENCY_INTERNAL);

	/* Make changes-so-far visible */
	CommandCounterIncrement();

	/*
	 * Build and execute a CREATE CONSTRAINT TRIGGER statement for the ON
	 * UPDATE action on the referenced table.
	 */
	fk_trigger = makeNode(CreateTrigStmt);
	fk_trigger->trigname = fkconstraint->constr_name;
	fk_trigger->relation = fkconstraint->pktable;
	fk_trigger->before = false;
	fk_trigger->row = true;
	fk_trigger->actions[0] = 'u';
	fk_trigger->actions[1] = '\0';
	fk_trigger->lang = NULL;
	fk_trigger->text = NULL;

	fk_trigger->attr = NIL;
	fk_trigger->when = NULL;
	fk_trigger->isconstraint = true;
	fk_trigger->deferrable = fkconstraint->deferrable;
	fk_trigger->initdeferred = fkconstraint->initdeferred;
	fk_trigger->constrrel = myRel;
	switch (fkconstraint->fk_upd_action)
	{
		case FKCONSTR_ACTION_NOACTION:
			fk_trigger->funcname = SystemFuncName("RI_FKey_noaction_upd");
			break;
		case FKCONSTR_ACTION_RESTRICT:
			fk_trigger->deferrable = false;
			fk_trigger->initdeferred = false;
			fk_trigger->funcname = SystemFuncName("RI_FKey_restrict_upd");
			break;
		case FKCONSTR_ACTION_CASCADE:
			fk_trigger->funcname = SystemFuncName("RI_FKey_cascade_upd");
			break;
		case FKCONSTR_ACTION_SETNULL:
			fk_trigger->funcname = SystemFuncName("RI_FKey_setnull_upd");
			break;
		case FKCONSTR_ACTION_SETDEFAULT:
			fk_trigger->funcname = SystemFuncName("RI_FKey_setdefault_upd");
			break;
		default:
			elog(ERROR, "Unrecognized ON UPDATE action for FOREIGN KEY constraint");
			break;
	}

	fk_trigger->args = NIL;
	fk_trigger->args = lappend(fk_trigger->args,
							   makeString(fkconstraint->constr_name));
	fk_trigger->args = lappend(fk_trigger->args,
							   makeString(myRel->relname));
	fk_trigger->args = lappend(fk_trigger->args,
							 makeString(fkconstraint->pktable->relname));
	fk_trigger->args = lappend(fk_trigger->args,
			makeString(fkMatchTypeToString(fkconstraint->fk_matchtype)));
	fk_attr = fkconstraint->fk_attrs;
	pk_attr = fkconstraint->pk_attrs;
	while (fk_attr != NIL)
	{
		fk_trigger->args = lappend(fk_trigger->args, lfirst(fk_attr));
		fk_trigger->args = lappend(fk_trigger->args, lfirst(pk_attr));
		fk_attr = lnext(fk_attr);
		pk_attr = lnext(pk_attr);
	}

	trigobj.objectId = CreateTrigger(fk_trigger, true);

	/* Register dependency from trigger to constraint */
	recordDependencyOn(&trigobj, &constrobj, DEPENDENCY_INTERNAL);
}

/*
 * fkMatchTypeToString -
 *	  convert FKCONSTR_MATCH_xxx code to string to use in trigger args
 */
static char *
fkMatchTypeToString(char match_type)
{
	switch (match_type)
	{
		case FKCONSTR_MATCH_FULL:
			return pstrdup("FULL");
		case FKCONSTR_MATCH_PARTIAL:
			return pstrdup("PARTIAL");
		case FKCONSTR_MATCH_UNSPECIFIED:
			return pstrdup("UNSPECIFIED");
		default:
			elog(ERROR, "fkMatchTypeToString: Unknown MATCH TYPE '%c'",
				 match_type);
	}
	return NULL;				/* can't get here */
}

/*
 * ALTER TABLE DROP CONSTRAINT
 */
void
AlterTableDropConstraint(Oid myrelid, bool recurse,
						 const char *constrName,
						 DropBehavior behavior)
{
	Relation	rel;
	int			deleted = 0;

	/*
	 * Acquire an exclusive lock on the target relation for the duration
	 * of the operation.
	 */
	rel = heap_open(myrelid, AccessExclusiveLock);

	/* Disallow DROP CONSTRAINT on views, indexes, sequences, etc */
	if (rel->rd_rel->relkind != RELKIND_RELATION)
		elog(ERROR, "ALTER TABLE: relation \"%s\" is not a table",
			 RelationGetRelationName(rel));

	if (!allowSystemTableMods
		&& IsSystemRelation(rel))
		elog(ERROR, "ALTER TABLE: relation \"%s\" is a system catalog",
			 RelationGetRelationName(rel));

	if (!pg_class_ownercheck(myrelid, GetUserId()))
		aclcheck_error(ACLCHECK_NOT_OWNER, RelationGetRelationName(rel));

	/*
	 * Process child tables if requested.
	 */
	if (recurse)
	{
		List	   *child,
				   *children;

		/* This routine is actually in the planner */
		children = find_all_inheritors(myrelid);

		/*
		 * find_all_inheritors does the recursive search of the
		 * inheritance hierarchy, so all we have to do is process all of
		 * the relids in the list that it returns.
		 */
		foreach(child, children)
		{
			Oid			childrelid = lfirsti(child);
			Relation	inhrel;

			if (childrelid == myrelid)
				continue;
			inhrel = heap_open(childrelid, AccessExclusiveLock);
			/* do NOT count child constraints in deleted. */
			RemoveRelConstraints(inhrel, constrName, behavior);
			heap_close(inhrel, NoLock);
		}
	}

	/*
	 * Now do the thing on this relation.
	 */
	deleted += RemoveRelConstraints(rel, constrName, behavior);

	/* Close the target relation */
	heap_close(rel, NoLock);

	/* If zero constraints deleted, complain */
	if (deleted == 0)
		elog(ERROR, "ALTER TABLE / DROP CONSTRAINT: %s does not exist",
			 constrName);
	/* Otherwise if more than one constraint deleted, notify */
	else if (deleted > 1)
		elog(NOTICE, "Multiple constraints dropped");
}

/*
 * ALTER TABLE OWNER
 */
void
AlterTableOwner(Oid relationOid, int32 newOwnerSysId)
{
	Relation	target_rel;
	Relation	class_rel;
	HeapTuple	tuple;
	Form_pg_class tuple_class;

	/* Get exclusive lock till end of transaction on the target table */
	/* Use relation_open here so that we work on indexes... */
	target_rel = relation_open(relationOid, AccessExclusiveLock);

	/* Get its pg_class tuple, too */
	class_rel = heap_openr(RelationRelationName, RowExclusiveLock);

	tuple = SearchSysCacheCopy(RELOID,
							   ObjectIdGetDatum(relationOid),
							   0, 0, 0);
	if (!HeapTupleIsValid(tuple))
		elog(ERROR, "ALTER TABLE: relation %u not found", relationOid);
	tuple_class = (Form_pg_class) GETSTRUCT(tuple);

	/* Can we change the ownership of this tuple? */
	CheckTupleType(tuple_class);

	/*
	 * Okay, this is a valid tuple: change its ownership and write to the
	 * heap.
	 */
	tuple_class->relowner = newOwnerSysId;
	simple_heap_update(class_rel, &tuple->t_self, tuple);

	/* Keep the catalog indexes up to date */
	CatalogUpdateIndexes(class_rel, tuple);

	/*
	 * If we are operating on a table, also change the ownership of any
	 * indexes that belong to the table, as well as the table's toast
	 * table (if it has one)
	 */
	if (tuple_class->relkind == RELKIND_RELATION ||
		tuple_class->relkind == RELKIND_TOASTVALUE)
	{
		List	   *index_oid_list,
				   *i;

		/* Find all the indexes belonging to this relation */
		index_oid_list = RelationGetIndexList(target_rel);

		/* For each index, recursively change its ownership */
		foreach(i, index_oid_list)
			AlterTableOwner(lfirsti(i), newOwnerSysId);

		freeList(index_oid_list);
	}

	if (tuple_class->relkind == RELKIND_RELATION)
	{
		/* If it has a toast table, recurse to change its ownership */
		if (tuple_class->reltoastrelid != InvalidOid)
			AlterTableOwner(tuple_class->reltoastrelid, newOwnerSysId);
	}

	heap_freetuple(tuple);
	heap_close(class_rel, RowExclusiveLock);
	relation_close(target_rel, NoLock);
}

static void
CheckTupleType(Form_pg_class tuple_class)
{
	switch (tuple_class->relkind)
	{
		case RELKIND_RELATION:
		case RELKIND_INDEX:
		case RELKIND_VIEW:
		case RELKIND_SEQUENCE:
		case RELKIND_TOASTVALUE:
			/* ok to change owner */
			break;
		default:
			elog(ERROR, "ALTER TABLE: relation \"%s\" is not a table, TOAST table, index, view, or sequence",
				 NameStr(tuple_class->relname));
	}
}

/*
 * ALTER TABLE CREATE TOAST TABLE
 */
void
AlterTableCreateToastTable(Oid relOid, bool silent)
{
	Relation	rel;
	HeapTuple	reltup;
	TupleDesc	tupdesc;
	bool		shared_relation;
	Relation	class_rel;
	Oid			toast_relid;
	Oid			toast_idxid;
	char		toast_relname[NAMEDATALEN];
	char		toast_idxname[NAMEDATALEN];
	IndexInfo  *indexInfo;
	Oid			classObjectId[2];
	ObjectAddress baseobject,
				toastobject;

	/*
	 * Grab an exclusive lock on the target table, which we will NOT
	 * release until end of transaction.
	 */
	rel = heap_open(relOid, AccessExclusiveLock);

	/* Check permissions */
	if (rel->rd_rel->relkind != RELKIND_RELATION)
		elog(ERROR, "ALTER TABLE: relation \"%s\" is not a table",
			 RelationGetRelationName(rel));

	if (!pg_class_ownercheck(relOid, GetUserId()))
		aclcheck_error(ACLCHECK_NOT_OWNER, RelationGetRelationName(rel));

	/*
	 * Toast table is shared if and only if its parent is.
	 *
	 * We cannot allow toasting a shared relation after initdb (because
	 * there's no way to mark it toasted in other databases' pg_class).
	 * Unfortunately we can't distinguish initdb from a manually started
	 * standalone backend.	However, we can at least prevent this mistake
	 * under normal multi-user operation.
	 */
	shared_relation = rel->rd_rel->relisshared;
	if (shared_relation && IsUnderPostmaster)
		elog(ERROR, "Shared relations cannot be toasted after initdb");

	/*
	 * Is it already toasted?
	 */
	if (rel->rd_rel->reltoastrelid != InvalidOid)
	{
		if (silent)
		{
			heap_close(rel, NoLock);
			return;
		}

		elog(ERROR, "ALTER TABLE: relation \"%s\" already has a toast table",
			 RelationGetRelationName(rel));
	}

	/*
	 * Check to see whether the table actually needs a TOAST table.
	 */
	if (!needs_toast_table(rel))
	{
		if (silent)
		{
			heap_close(rel, NoLock);
			return;
		}

		elog(ERROR, "ALTER TABLE: relation \"%s\" does not need a toast table",
			 RelationGetRelationName(rel));
	}

	/*
	 * Create the toast table and its index
	 */
	snprintf(toast_relname, NAMEDATALEN, "pg_toast_%u", relOid);
	snprintf(toast_idxname, NAMEDATALEN, "pg_toast_%u_index", relOid);

	/* this is pretty painful...  need a tuple descriptor */
	tupdesc = CreateTemplateTupleDesc(3, false);
	TupleDescInitEntry(tupdesc, (AttrNumber) 1,
					   "chunk_id",
					   OIDOID,
					   -1, 0, false);
	TupleDescInitEntry(tupdesc, (AttrNumber) 2,
					   "chunk_seq",
					   INT4OID,
					   -1, 0, false);
	TupleDescInitEntry(tupdesc, (AttrNumber) 3,
					   "chunk_data",
					   BYTEAOID,
					   -1, 0, false);

	/*
	 * Ensure that the toast table doesn't itself get toasted, or we'll be
	 * toast :-(.  This is essential for chunk_data because type bytea is
	 * toastable; hit the other two just to be sure.
	 */
	tupdesc->attrs[0]->attstorage = 'p';
	tupdesc->attrs[1]->attstorage = 'p';
	tupdesc->attrs[2]->attstorage = 'p';

	/*
	 * Note: the toast relation is placed in the regular pg_toast
	 * namespace even if its master relation is a temp table.  There
	 * cannot be any naming collision, and the toast rel will be destroyed
	 * when its master is, so there's no need to handle the toast rel as
	 * temp.
	 */
	toast_relid = heap_create_with_catalog(toast_relname,
										   PG_TOAST_NAMESPACE,
										   tupdesc,
										   RELKIND_TOASTVALUE,
										   shared_relation,
										   true);

	/* make the toast relation visible, else index creation will fail */
	CommandCounterIncrement();

	/*
	 * Create unique index on chunk_id, chunk_seq.
	 *
	 * NOTE: the normal TOAST access routines could actually function with a
	 * single-column index on chunk_id only. However, the slice access
	 * routines use both columns for faster access to an individual chunk.
	 * In addition, we want it to be unique as a check against the
	 * possibility of duplicate TOAST chunk OIDs. The index might also be
	 * a little more efficient this way, since btree isn't all that happy
	 * with large numbers of equal keys.
	 */

	indexInfo = makeNode(IndexInfo);
	indexInfo->ii_NumIndexAttrs = 2;
	indexInfo->ii_NumKeyAttrs = 2;
	indexInfo->ii_KeyAttrNumbers[0] = 1;
	indexInfo->ii_KeyAttrNumbers[1] = 2;
	indexInfo->ii_Predicate = NIL;
	indexInfo->ii_FuncOid = InvalidOid;
	indexInfo->ii_Unique = true;

	classObjectId[0] = OID_BTREE_OPS_OID;
	classObjectId[1] = INT4_BTREE_OPS_OID;

	toast_idxid = index_create(toast_relid, toast_idxname, indexInfo,
							   BTREE_AM_OID, classObjectId,
							   true, false, true);

	/*
	 * Update toast rel's pg_class entry to show that it has an index. The
	 * index OID is stored into the reltoastidxid field for easy access by
	 * the tuple toaster.
	 */
	setRelhasindex(toast_relid, true, true, toast_idxid);

	/*
	 * Store the toast table's OID in the parent relation's pg_class row
	 */
	class_rel = heap_openr(RelationRelationName, RowExclusiveLock);

	reltup = SearchSysCacheCopy(RELOID,
								ObjectIdGetDatum(relOid),
								0, 0, 0);
	if (!HeapTupleIsValid(reltup))
		elog(ERROR, "ALTER TABLE: relation \"%s\" not found",
			 RelationGetRelationName(rel));

	((Form_pg_class) GETSTRUCT(reltup))->reltoastrelid = toast_relid;

	simple_heap_update(class_rel, &reltup->t_self, reltup);

	/* Keep catalog indexes current */
	CatalogUpdateIndexes(class_rel, reltup);

	heap_freetuple(reltup);

	heap_close(class_rel, RowExclusiveLock);

	/*
	 * Register dependency from the toast table to the master, so that the
	 * toast table will be deleted if the master is.
	 */
	baseobject.classId = RelOid_pg_class;
	baseobject.objectId = relOid;
	baseobject.objectSubId = 0;
	toastobject.classId = RelOid_pg_class;
	toastobject.objectId = toast_relid;
	toastobject.objectSubId = 0;

	recordDependencyOn(&toastobject, &baseobject, DEPENDENCY_INTERNAL);

	/*
	 * Clean up and make changes visible
	 */
	heap_close(rel, NoLock);

	CommandCounterIncrement();
}

/*
 * Check to see whether the table needs a TOAST table.	It does only if
 * (1) there are any toastable attributes, and (2) the maximum length
 * of a tuple could exceed TOAST_TUPLE_THRESHOLD.  (We don't want to
 * create a toast table for something like "f1 varchar(20)".)
 */
static bool
needs_toast_table(Relation rel)
{
	int32		data_length = 0;
	bool		maxlength_unknown = false;
	bool		has_toastable_attrs = false;
	TupleDesc	tupdesc;
	Form_pg_attribute *att;
	int32		tuple_length;
	int			i;

	tupdesc = rel->rd_att;
	att = tupdesc->attrs;

	for (i = 0; i < tupdesc->natts; i++)
	{
		data_length = att_align(data_length, att[i]->attalign);
		if (att[i]->attlen > 0)
		{
			/* Fixed-length types are never toastable */
			data_length += att[i]->attlen;
		}
		else
		{
			int32		maxlen = type_maximum_size(att[i]->atttypid,
												   att[i]->atttypmod);

			if (maxlen < 0)
				maxlength_unknown = true;
			else
				data_length += maxlen;
			if (att[i]->attstorage != 'p')
				has_toastable_attrs = true;
		}
	}
	if (!has_toastable_attrs)
		return false;			/* nothing to toast? */
	if (maxlength_unknown)
		return true;			/* any unlimited-length attrs? */
	tuple_length = MAXALIGN(offsetof(HeapTupleHeaderData, t_bits) +
							BITMAPLEN(tupdesc->natts)) +
		MAXALIGN(data_length);
	return (tuple_length > TOAST_TUPLE_THRESHOLD);
}
