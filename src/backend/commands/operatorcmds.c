/*-------------------------------------------------------------------------
 *
 * operatorcmds.c
 *
 *	  Routines for operator manipulation commands
 *
 * Portions Copyright (c) 1996-2002, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  $Header: /home/cvsmirror/pg/pgsql/src/backend/commands/operatorcmds.c,v 1.5 2002/07/12 18:43:16 tgl Exp $
 *
 * DESCRIPTION
 *	  The "DefineFoo" routines take the parse tree and pick out the
 *	  appropriate arguments/flags, passing the results to the
 *	  corresponding "FooDefine" routines (in src/catalog) that do
 *	  the actual catalog-munging.  These routines also verify permission
 *	  of the user to execute the command.
 *
 * NOTES
 *	  These things must be defined and committed in the following order:
 *		"create function":
 *				input/output, recv/send procedures
 *		"create type":
 *				type
 *		"create operator":
 *				operators
 *
 *		Most of the parse-tree manipulation routines are defined in
 *		commands/manip.c.
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include "access/heapam.h"
#include "catalog/catname.h"
#include "catalog/dependency.h"
#include "catalog/namespace.h"
#include "catalog/pg_operator.h"
#include "commands/defrem.h"
#include "miscadmin.h"
#include "parser/parse_oper.h"
#include "parser/parse_type.h"
#include "utils/acl.h"
#include "utils/lsyscache.h"
#include "utils/syscache.h"


/*
 * DefineOperator
 *		this function extracts all the information from the
 *		parameter list generated by the parser and then has
 *		OperatorCreate() do all the actual work.
 *
 * 'parameters' is a list of DefElem
 */
void
DefineOperator(List *names, List *parameters)
{
	char	   *oprName;
	Oid			oprNamespace;
	AclResult	aclresult;
	uint16		precedence = 0;			/* operator precedence */
	bool		canHash = false;		/* operator hashes */
	bool		canMerge = false;		/* operator merges */
	bool		isLeftAssociative = true;		/* operator is left
												 * associative */
	List	   *functionName = NIL;		/* function for operator */
	TypeName   *typeName1 = NULL;		/* first type name */
	TypeName   *typeName2 = NULL;		/* second type name */
	Oid			typeId1 = InvalidOid;	/* types converted to OID */
	Oid			typeId2 = InvalidOid;
	List	   *commutatorName = NIL;	/* optional commutator operator
										 * name */
	List	   *negatorName = NIL;		/* optional negator operator name */
	List	   *restrictionName = NIL;	/* optional restrict. sel.
										 * procedure */
	List	   *joinName = NIL;			/* optional join sel. procedure */
	List	   *leftSortName = NIL;		/* optional left sort operator */
	List	   *rightSortName = NIL;	/* optional right sort operator */
	List	   *ltCompareName = NIL;	/* optional < compare operator */
	List	   *gtCompareName = NIL;	/* optional > compare operator */
	List	   *pl;

	/* Convert list of names to a name and namespace */
	oprNamespace = QualifiedNameGetCreationNamespace(names, &oprName);

	/* Check we have creation rights in target namespace */
	aclresult = pg_namespace_aclcheck(oprNamespace, GetUserId(), ACL_CREATE);
	if (aclresult != ACLCHECK_OK)
		aclcheck_error(aclresult, get_namespace_name(oprNamespace));

	/*
	 * loop over the definition list and extract the information we need.
	 */
	foreach(pl, parameters)
	{
		DefElem    *defel = (DefElem *) lfirst(pl);

		if (strcasecmp(defel->defname, "leftarg") == 0)
		{
			typeName1 = defGetTypeName(defel);
			if (typeName1->setof)
				elog(ERROR, "setof type not implemented for leftarg");
		}
		else if (strcasecmp(defel->defname, "rightarg") == 0)
		{
			typeName2 = defGetTypeName(defel);
			if (typeName2->setof)
				elog(ERROR, "setof type not implemented for rightarg");
		}
		else if (strcasecmp(defel->defname, "procedure") == 0)
			functionName = defGetQualifiedName(defel);
		else if (strcasecmp(defel->defname, "precedence") == 0)
		{
			/* NOT IMPLEMENTED (never worked in v4.2) */
			elog(NOTICE, "CREATE OPERATOR: precedence not implemented");
		}
		else if (strcasecmp(defel->defname, "associativity") == 0)
		{
			/* NOT IMPLEMENTED (never worked in v4.2) */
			elog(NOTICE, "CREATE OPERATOR: associativity not implemented");
		}
		else if (strcasecmp(defel->defname, "commutator") == 0)
			commutatorName = defGetQualifiedName(defel);
		else if (strcasecmp(defel->defname, "negator") == 0)
			negatorName = defGetQualifiedName(defel);
		else if (strcasecmp(defel->defname, "restrict") == 0)
			restrictionName = defGetQualifiedName(defel);
		else if (strcasecmp(defel->defname, "join") == 0)
			joinName = defGetQualifiedName(defel);
		else if (strcasecmp(defel->defname, "hashes") == 0)
			canHash = TRUE;
		else if (strcasecmp(defel->defname, "merges") == 0)
			canMerge = TRUE;
		else if (strcasecmp(defel->defname, "sort1") == 0)
			leftSortName = defGetQualifiedName(defel);
		else if (strcasecmp(defel->defname, "sort2") == 0)
			rightSortName = defGetQualifiedName(defel);
		else if (strcasecmp(defel->defname, "ltcmp") == 0)
			ltCompareName = defGetQualifiedName(defel);
		else if (strcasecmp(defel->defname, "gtcmp") == 0)
			gtCompareName = defGetQualifiedName(defel);
		else
		{
			elog(WARNING, "DefineOperator: attribute \"%s\" not recognized",
				 defel->defname);
		}
	}

	/*
	 * make sure we have our required definitions
	 */
	if (functionName == NIL)
		elog(ERROR, "Define: \"procedure\" unspecified");

	/* Transform type names to type OIDs */
	if (typeName1)
		typeId1 = typenameTypeId(typeName1);
	if (typeName2)
		typeId2 = typenameTypeId(typeName2);

	/*
	 * If any of the mergejoin support operators were given, then canMerge
	 * is implicit.  If canMerge is specified or implicit, fill in default
	 * operator names for any missing mergejoin support operators.
	 */
	if (leftSortName || rightSortName || ltCompareName || gtCompareName)
		canMerge = true;

	if (canMerge)
	{
		if (!leftSortName)
			leftSortName = makeList1(makeString("<"));
		if (!rightSortName)
			rightSortName = makeList1(makeString("<"));
		if (!ltCompareName)
			ltCompareName = makeList1(makeString("<"));
		if (!gtCompareName)
			gtCompareName = makeList1(makeString(">"));
	}

	/*
	 * now have OperatorCreate do all the work..
	 */
	OperatorCreate(oprName,		/* operator name */
				   oprNamespace, /* namespace */
				   typeId1,		/* left type id */
				   typeId2,		/* right type id */
				   functionName,	/* function for operator */
				   precedence,	/* operator precedence */
				   isLeftAssociative,	/* operator is left associative */
				   commutatorName,		/* optional commutator operator
										 * name */
				   negatorName, /* optional negator operator name */
				   restrictionName,		/* optional restrict. sel.
										 * procedure */
				   joinName,	/* optional join sel. procedure name */
				   canHash,		/* operator hashes */
				   leftSortName,	/* optional left sort operator */
				   rightSortName,	/* optional right sort operator */
				   ltCompareName,	/* optional < comparison op */
				   gtCompareName);	/* optional < comparison op */
}


/*
 * RemoveOperator
 *		Deletes an operator.
 */
void
RemoveOperator(RemoveOperStmt *stmt)
{
	List *operatorName = stmt->opname;
	TypeName *typeName1 = (TypeName *) lfirst(stmt->args);
	TypeName *typeName2 = (TypeName *) lsecond(stmt->args);
	Oid			operOid;
	HeapTuple	tup;
	ObjectAddress object;

	operOid = LookupOperNameTypeNames(operatorName, typeName1, typeName2,
									  "RemoveOperator");

	tup = SearchSysCache(OPEROID,
						 ObjectIdGetDatum(operOid),
						 0, 0, 0);
	if (!HeapTupleIsValid(tup))	/* should not happen */
		elog(ERROR, "RemoveOperator: failed to find tuple for operator '%s'",
			 NameListToString(operatorName));

	/* Permission check: must own operator or its namespace */
	if (!pg_oper_ownercheck(operOid, GetUserId()) &&
		!pg_namespace_ownercheck(((Form_pg_operator) GETSTRUCT(tup))->oprnamespace,
								 GetUserId()))
		aclcheck_error(ACLCHECK_NOT_OWNER, NameListToString(operatorName));

	ReleaseSysCache(tup);

	/*
	 * Do the deletion
	 */
	object.classId = get_system_catalog_relid(OperatorRelationName);
	object.objectId = operOid;
	object.objectSubId = 0;

	performDeletion(&object, stmt->behavior);
}

/*
 * Guts of operator deletion.
 */
void
RemoveOperatorById(Oid operOid)
{
	Relation	relation;
	HeapTuple	tup;

	relation = heap_openr(OperatorRelationName, RowExclusiveLock);

	tup = SearchSysCache(OPEROID,
						 ObjectIdGetDatum(operOid),
						 0, 0, 0);
	if (!HeapTupleIsValid(tup))	/* should not happen */
		elog(ERROR, "RemoveOperatorById: failed to find tuple for operator %u",
			 operOid);

	simple_heap_delete(relation, &tup->t_self);

	ReleaseSysCache(tup);

	heap_close(relation, RowExclusiveLock);
}
