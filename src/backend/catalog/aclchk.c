/*-------------------------------------------------------------------------
 *
 * aclchk.c
 *	  Routines to check access control permissions.
 *
 * Portions Copyright (c) 1996-2001, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  $Header: /home/cvsmirror/pg/pgsql/src/backend/catalog/aclchk.c,v 1.56 2002/03/06 06:09:24 momjian Exp $
 *
 * NOTES
 *	  See acl.h.
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include "access/heapam.h"
#include "access/transam.h"
#include "catalog/catalog.h"
#include "catalog/catname.h"
#include "catalog/indexing.h"
#include "catalog/pg_aggregate.h"
#include "catalog/pg_group.h"
#include "catalog/pg_language.h"
#include "catalog/pg_operator.h"
#include "catalog/pg_proc.h"
#include "catalog/pg_shadow.h"
#include "catalog/pg_type.h"
#include "miscadmin.h"
#include "nodes/parsenodes.h"
#include "parser/keywords.h"
#include "parser/parse.h"
#include "parser/parse_agg.h"
#include "parser/parse_func.h"
#include "parser/parse_expr.h"
#include "utils/acl.h"
#include "utils/syscache.h"
#include "utils/temprel.h"


static void ExecuteGrantStmt_Table(GrantStmt *stmt);
static void ExecuteGrantStmt_Function(GrantStmt *stmt);
static void ExecuteGrantStmt_Lang(GrantStmt *stmt);

static const char *privilege_token_string(int token);

static int32 aclcheck(Acl *acl, AclId id, AclIdType idtype, AclMode mode);

/* warning messages, now more explicit. */
/* MUST correspond to the order of the ACLCHK_* result codes in acl.h. */
char	   *aclcheck_error_strings[] = {
	"No error.",
	"Permission denied.",
	"Table does not exist.",
	"Must be table owner."
};


#ifdef ACLDEBUG
static
dumpacl(Acl *acl)
{
	int			i;
	AclItem    *aip;

	elog(LOG, "acl size = %d, # acls = %d",
		 ACL_SIZE(acl), ACL_NUM(acl));
	aip = ACL_DAT(acl);
	for (i = 0; i < ACL_NUM(acl); ++i)
		elog(LOG, "	acl[%d]: %s", i,
			 DatumGetCString(DirectFunctionCall1(aclitemout,
											 PointerGetDatum(aip + i))));
}
#endif   /* ACLDEBUG */


/*
 * If is_grant is true, adds the given privileges for the list of
 * grantees to the existing old_acl.  If is_grant is false, the
 * privileges for the given grantees are removed from old_acl.
 */
static Acl*
merge_acl_with_grant(Acl *old_acl, bool is_grant, List *grantees, char *privileges)
{
	List	   *j;
	Acl		   *new_acl;

#ifdef ACLDEBUG
	dumpacl(old_acl);
#endif
	new_acl = old_acl;

	foreach(j, grantees)
	{
		PrivGrantee *grantee = (PrivGrantee *) lfirst(j);
		char	   *granteeString;
		char	   *aclString;
		AclItem aclitem;
		unsigned	modechg;

		if (grantee->username)
			granteeString = aclmakeuser("U", grantee->username);
		else if (grantee->groupname)
			granteeString = aclmakeuser("G", grantee->groupname);
		else
			granteeString = aclmakeuser("A", "");

		aclString = makeAclString(privileges, granteeString,
								  is_grant ? '+' : '-');

		/* Convert string ACL spec into internal form */
		aclparse(aclString, &aclitem, &modechg);
		new_acl = aclinsert3(new_acl, &aclitem, modechg);

#ifdef ACLDEBUG
		dumpacl(new_acl);
#endif
	}

	return new_acl;
}


/*
 * Called to execute the utility commands GRANT and REVOKE
 */
void
ExecuteGrantStmt(GrantStmt *stmt)
{
	/* see comment in pg_type.h */
	Assert(ACLITEMSIZE == sizeof(AclItem));

	switch(stmt->objtype)
	{
		case TABLE:
			ExecuteGrantStmt_Table(stmt);
			break;
		case FUNCTION:
			ExecuteGrantStmt_Function(stmt);
			break;
		case LANGUAGE:
			ExecuteGrantStmt_Lang(stmt);
			break;
		default:
			elog(ERROR, "bogus GrantStmt.objtype %d", stmt->objtype);
	}
}


static void
ExecuteGrantStmt_Table(GrantStmt *stmt)
{
	List	   *i;
	char	   *privstring;

	if (lfirsti(stmt->privileges) == ALL)
		privstring = aclmakepriv(ACL_MODE_STR, 0);
	else
	{
		privstring = "";
		foreach(i, stmt->privileges)
		{
			int c = 0;

			switch(lfirsti(i))
			{
				case SELECT:
					c = ACL_MODE_SELECT_CHR;
					break;
				case INSERT:
					c = ACL_MODE_INSERT_CHR;
					break;
				case UPDATE:
					c = ACL_MODE_UPDATE_CHR;
					break;
				case DELETE:
					c = ACL_MODE_DELETE_CHR;
					break;
				case RULE:
					c = ACL_MODE_RULE_CHR;
					break;
				case REFERENCES:
					c = ACL_MODE_REFERENCES_CHR;
					break;
				case TRIGGER:
					c = ACL_MODE_TRIGGER_CHR;
					break;
				default:
					elog(ERROR, "invalid privilege type %s for table object",
						 privilege_token_string(lfirsti(i)));
			}

			privstring = aclmakepriv(privstring, c);
		}
	}


	foreach(i, stmt->objects)
	{
		char	   *relname = strVal(lfirst(i));
		Relation	relation;
		HeapTuple	tuple;
		Form_pg_class pg_class_tuple;
		Datum		aclDatum;
		bool		isNull;
		Acl		   *old_acl;
		Acl		   *new_acl;
		unsigned	i;
		HeapTuple	newtuple;
		Datum		values[Natts_pg_class];
		char		nulls[Natts_pg_class];
		char		replaces[Natts_pg_class];


		if (!pg_ownercheck(GetUserId(), relname, RELNAME))
			elog(ERROR, "permission denied");

		/* open pg_class */
		relation = heap_openr(RelationRelationName, RowExclusiveLock);
		tuple = SearchSysCache(RELNAME,
							   PointerGetDatum(relname),
							   0, 0, 0);
		if (!HeapTupleIsValid(tuple))
		{
			heap_close(relation, RowExclusiveLock);
			elog(ERROR, "relation \"%s\" not found",
				 relname);
		}
		pg_class_tuple = (Form_pg_class) GETSTRUCT(tuple);

		if (pg_class_tuple->relkind == RELKIND_INDEX)
			elog(ERROR, "\"%s\" is an index",
				 relname);

		/*
		 * If there's no ACL, create a default using the pg_class.relowner
		 * field.
		 */
		aclDatum = SysCacheGetAttr(RELNAME, tuple, Anum_pg_class_relacl,
								   &isNull);
		if (isNull)
			old_acl = acldefault(pg_class_tuple->relowner);
		else
			/* get a detoasted copy of the rel's ACL */
			old_acl = DatumGetAclPCopy(aclDatum);

		new_acl = merge_acl_with_grant(old_acl, stmt->is_grant,
									   stmt->grantees, privstring);

		/* finished building new ACL value, now insert it */
		for (i = 0; i < Natts_pg_class; ++i)
		{
			replaces[i] = ' ';
			nulls[i] = ' ';		/* ignored if replaces[i]==' ' anyway */
			values[i] = (Datum) NULL;	/* ignored if replaces[i]==' '
										 * anyway */
		}
		replaces[Anum_pg_class_relacl - 1] = 'r';
		values[Anum_pg_class_relacl - 1] = PointerGetDatum(new_acl);
		newtuple = heap_modifytuple(tuple, relation, values, nulls, replaces);

		ReleaseSysCache(tuple);

		simple_heap_update(relation, &newtuple->t_self, newtuple);

		{
			/* keep the catalog indexes up to date */
			Relation	idescs[Num_pg_class_indices];

			CatalogOpenIndices(Num_pg_class_indices, Name_pg_class_indices,
							   idescs);
			CatalogIndexInsert(idescs, Num_pg_class_indices, relation, newtuple);
			CatalogCloseIndices(Num_pg_class_indices, idescs);
		}

		pfree(old_acl);
		pfree(new_acl);

		heap_close(relation, RowExclusiveLock);
	}
}


static Oid
find_function_with_arglist(char *name, List *arguments)
{
	Oid		oid;
	Oid		argoids[FUNC_MAX_ARGS];
	int		i;
	int16	argcount;

	MemSet(argoids, 0, FUNC_MAX_ARGS * sizeof(Oid));
	argcount = length(arguments);
	if (argcount > FUNC_MAX_ARGS)
		elog(ERROR, "functions cannot have more than %d arguments",
			 FUNC_MAX_ARGS);

	for (i = 0; i < argcount; i++)
	{
		TypeName   *t = (TypeName *) lfirst(arguments);
		char       *typnam = TypeNameToInternalName(t);

		arguments = lnext(arguments);

		if (strcmp(typnam, "opaque") == 0)
			argoids[i] = InvalidOid;
		else
		{
			argoids[i] = GetSysCacheOid(TYPENAME,
										PointerGetDatum(typnam),
										0, 0, 0);
			if (!OidIsValid(argoids[i]))
				elog(ERROR, "type '%s' not found", typnam);
		}
	}

	oid = GetSysCacheOid(PROCNAME,
						 PointerGetDatum(name),
						 Int16GetDatum(argcount),
						 PointerGetDatum(argoids),
						 0);

	if (!OidIsValid(oid))
		func_error(NULL, name, argcount, argoids, NULL);

	return oid;
}


static void
ExecuteGrantStmt_Function(GrantStmt *stmt)
{
	List	   *i;
	char	   *privstring = NULL;

	if (lfirsti(stmt->privileges) == ALL)
		privstring = aclmakepriv("", ACL_MODE_SELECT_CHR);
	else
	{
		foreach(i, stmt->privileges)
		{
			if (lfirsti(i) != EXECUTE)
				elog(ERROR, "invalid privilege type %s for function object",
					 privilege_token_string(lfirsti(i)));
		}

		privstring = aclmakepriv("", ACL_MODE_SELECT_CHR);
	}

	foreach(i, stmt->objects)
	{
		FuncWithArgs *func = (FuncWithArgs *) lfirst(i);
		Oid			oid;
		Relation	relation;
		HeapTuple	tuple;
		Form_pg_proc pg_proc_tuple;
		Datum		aclDatum;
		bool		isNull;
		Acl		   *old_acl;
		Acl		   *new_acl;
		unsigned	i;
		HeapTuple	newtuple;
		Datum		values[Natts_pg_proc];
		char		nulls[Natts_pg_proc];
		char		replaces[Natts_pg_proc];

		oid = find_function_with_arglist(func->funcname, func->funcargs);
		relation = heap_openr(ProcedureRelationName, RowExclusiveLock);
		tuple = SearchSysCache(PROCOID, ObjectIdGetDatum(oid), 0, 0, 0);
		if (!HeapTupleIsValid(tuple))
		{
			heap_close(relation, RowExclusiveLock);
			elog(ERROR, "function %u not found", oid);
		}
		pg_proc_tuple = (Form_pg_proc) GETSTRUCT(tuple);

		if (pg_proc_tuple->proowner != GetUserId())
			elog(ERROR, "permission denied");

		/*
		 * If there's no ACL, create a default using the pg_proc.proowner
		 * field.
		 */
		aclDatum = SysCacheGetAttr(PROCOID, tuple, Anum_pg_proc_proacl,
								   &isNull);
		if (isNull)
			old_acl = acldefault(pg_proc_tuple->proowner);
		else
			/* get a detoasted copy of the rel's ACL */
			old_acl = DatumGetAclPCopy(aclDatum);

		new_acl = merge_acl_with_grant(old_acl, stmt->is_grant,
									   stmt->grantees, privstring);

		/* finished building new ACL value, now insert it */
		for (i = 0; i < Natts_pg_proc; ++i)
		{
			replaces[i] = ' ';
			nulls[i] = ' ';		/* ignored if replaces[i]==' ' anyway */
			values[i] = (Datum) NULL;	/* ignored if replaces[i]==' '
										 * anyway */
		}
		replaces[Anum_pg_proc_proacl - 1] = 'r';
		values[Anum_pg_proc_proacl - 1] = PointerGetDatum(new_acl);
		newtuple = heap_modifytuple(tuple, relation, values, nulls, replaces);

		ReleaseSysCache(tuple);

		simple_heap_update(relation, &newtuple->t_self, newtuple);

		{
			/* keep the catalog indexes up to date */
			Relation	idescs[Num_pg_proc_indices];

			CatalogOpenIndices(Num_pg_proc_indices, Name_pg_proc_indices,
							   idescs);
			CatalogIndexInsert(idescs, Num_pg_proc_indices, relation, newtuple);
			CatalogCloseIndices(Num_pg_proc_indices, idescs);
		}

		pfree(old_acl);
		pfree(new_acl);

		heap_close(relation, RowExclusiveLock);
	}
}


static void
ExecuteGrantStmt_Lang(GrantStmt *stmt)
{
	List	   *i;
	char	   *privstring = NULL;

	if (lfirsti(stmt->privileges) == ALL)
		privstring = aclmakepriv("", ACL_MODE_SELECT_CHR);
	else
	{
		foreach(i, stmt->privileges)
		{
			if (lfirsti(i) != USAGE)
				elog(ERROR, "invalid privilege type %s for language object",
					 privilege_token_string(lfirsti(i)));
		}

		privstring = aclmakepriv("", ACL_MODE_SELECT_CHR);
	}

	foreach(i, stmt->objects)
	{
		char	   *langname = strVal(lfirst(i));
		Relation	relation;
		HeapTuple	tuple;
		Form_pg_language pg_language_tuple;
		Datum		aclDatum;
		bool		isNull;
		Acl		   *old_acl;
		Acl		   *new_acl;
		unsigned	i;
		HeapTuple	newtuple;
		Datum		values[Natts_pg_language];
		char		nulls[Natts_pg_language];
		char		replaces[Natts_pg_language];

		if (!superuser())
			elog(ERROR, "permission denied");

		relation = heap_openr(LanguageRelationName, RowExclusiveLock);
		tuple = SearchSysCache(LANGNAME, PointerGetDatum(langname), 0, 0, 0);
		if (!HeapTupleIsValid(tuple))
		{
			heap_close(relation, RowExclusiveLock);
			elog(ERROR, "language \"%s\" not found", langname);
		}
		pg_language_tuple = (Form_pg_language) GETSTRUCT(tuple);

		if (!pg_language_tuple->lanpltrusted)
		{
			heap_close(relation, RowExclusiveLock);
			elog(ERROR, "language \"%s\" is not trusted", langname);
		}

		/*
		 * If there's no ACL, create a default.
		 */
		aclDatum = SysCacheGetAttr(LANGNAME, tuple, Anum_pg_language_lanacl,
								   &isNull);
		if (isNull)
			old_acl = acldefault(InvalidOid);
		else
			/* get a detoasted copy of the rel's ACL */
			old_acl = DatumGetAclPCopy(aclDatum);

		new_acl = merge_acl_with_grant(old_acl, stmt->is_grant,
									   stmt->grantees, privstring);

		/* finished building new ACL value, now insert it */
		for (i = 0; i < Natts_pg_language; ++i)
		{
			replaces[i] = ' ';
			nulls[i] = ' ';		/* ignored if replaces[i]==' ' anyway */
			values[i] = (Datum) NULL;	/* ignored if replaces[i]==' '
										 * anyway */
		}
		replaces[Anum_pg_language_lanacl - 1] = 'r';
		values[Anum_pg_language_lanacl - 1] = PointerGetDatum(new_acl);
		newtuple = heap_modifytuple(tuple, relation, values, nulls, replaces);

		ReleaseSysCache(tuple);

		simple_heap_update(relation, &newtuple->t_self, newtuple);

		{
			/* keep the catalog indexes up to date */
			Relation	idescs[Num_pg_language_indices];

			CatalogOpenIndices(Num_pg_language_indices, Name_pg_language_indices,
							   idescs);
			CatalogIndexInsert(idescs, Num_pg_language_indices, relation, newtuple);
			CatalogCloseIndices(Num_pg_language_indices, idescs);
		}

		pfree(old_acl);
		pfree(new_acl);

		heap_close(relation, RowExclusiveLock);
	}
}



static const char *
privilege_token_string(int token)
{
	const char *s = TokenString(token);

	if (s)
		return s;
	else
		elog(ERROR, "privilege_token_string: invalid token number");
	return NULL; /* appease compiler */
}



AclId
get_grosysid(char *groname)
{
	HeapTuple	tuple;
	AclId		id = 0;

	tuple = SearchSysCache(GRONAME,
						   PointerGetDatum(groname),
						   0, 0, 0);
	if (HeapTupleIsValid(tuple))
	{
		id = ((Form_pg_group) GETSTRUCT(tuple))->grosysid;
		ReleaseSysCache(tuple);
	}
	else
		elog(ERROR, "non-existent group \"%s\"", groname);
	return id;
}

/*
 * Convert group ID to name, or return NULL if group can't be found
 */
char *
get_groname(AclId grosysid)
{
	HeapTuple	tuple;
	char	   *name = NULL;

	tuple = SearchSysCache(GROSYSID,
						   ObjectIdGetDatum(grosysid),
						   0, 0, 0);
	if (HeapTupleIsValid(tuple))
	{
		name = pstrdup(NameStr(((Form_pg_group) GETSTRUCT(tuple))->groname));
		ReleaseSysCache(tuple);
	}
	return name;
}

/*
 * Is user a member of group?
 */
static bool
in_group(AclId uid, AclId gid)
{
	bool		result = false;
	HeapTuple	tuple;
	Datum		att;
	bool		isNull;
	IdList	   *glist;
	AclId	   *aidp;
	int			i,
				num;

	tuple = SearchSysCache(GROSYSID,
						   ObjectIdGetDatum(gid),
						   0, 0, 0);
	if (HeapTupleIsValid(tuple))
	{
		att = SysCacheGetAttr(GROSYSID,
							  tuple,
							  Anum_pg_group_grolist,
							  &isNull);
		if (!isNull)
		{
			/* be sure the IdList is not toasted */
			glist = DatumGetIdListP(att);
			/* scan it */
			num = IDLIST_NUM(glist);
			aidp = IDLIST_DAT(glist);
			for (i = 0; i < num; ++i)
			{
				if (aidp[i] == uid)
				{
					result = true;
					break;
				}
			}
			/* if IdList was toasted, free detoasted copy */
			if ((Pointer) glist != DatumGetPointer(att))
				pfree(glist);
		}
		ReleaseSysCache(tuple);
	}
	else
		elog(WARNING, "in_group: group %u not found", gid);
	return result;
}

/*
 * aclcheck
 *
 * Returns ACLCHECK_OK if the 'id' of type 'idtype' has ACL entries in 'acl'
 * to satisfy any one of the requirements of 'mode'.  Returns an appropriate
 * ACLCHECK_* error code otherwise.
 *
 * The ACL list is expected to be sorted in standard order.
 */
static int32
aclcheck(Acl *acl, AclId id, AclIdType idtype, AclMode mode)
{
	AclItem    *aip,
			   *aidat;
	int			i,
				num;

	/*
	 * If ACL is null, default to "OK" --- this should not happen, since
	 * caller should have inserted appropriate default
	 */
	if (!acl)
	{
		elog(LOG, "aclcheck: null ACL, returning OK");
		return ACLCHECK_OK;
	}

	num = ACL_NUM(acl);
	aidat = ACL_DAT(acl);

	/*
	 * We'll treat the empty ACL like that, too, although this is more
	 * like an error (i.e., you manually blew away your ACL array) -- the
	 * system never creates an empty ACL, since there must always be a
	 * "world" entry in the first slot.
	 */
	if (num < 1)
	{
		elog(LOG, "aclcheck: zero-length ACL, returning OK");
		return ACLCHECK_OK;
	}

	/*
	 * "World" rights are applicable regardless of the passed-in ID, and
	 * since they're much the cheapest to check, check 'em first.
	 */
	if (aidat->ai_idtype != ACL_IDTYPE_WORLD)
		elog(ERROR, "aclcheck: first entry in ACL is not 'world' entry");
	if (aidat->ai_mode & mode)
	{
#ifdef ACLDEBUG
		elog(LOG, "aclcheck: using world=%d", aidat->ai_mode);
#endif
		return ACLCHECK_OK;
	}

	switch (idtype)
	{
		case ACL_IDTYPE_UID:
			/* See if permission is granted directly to user */
			for (i = 1, aip = aidat + 1;		/* skip world entry */
				 i < num && aip->ai_idtype == ACL_IDTYPE_UID;
				 ++i, ++aip)
			{
				if (aip->ai_id == id)
				{
#ifdef ACLDEBUG
					elog(LOG, "aclcheck: found user %u/%d",
						 aip->ai_id, aip->ai_mode);
#endif
					if (aip->ai_mode & mode)
						return ACLCHECK_OK;
				}
			}
			/* See if he has the permission via any group */
			for (;
				 i < num && aip->ai_idtype == ACL_IDTYPE_GID;
				 ++i, ++aip)
			{
				if (aip->ai_mode & mode)
				{
					if (in_group(id, aip->ai_id))
					{
#ifdef ACLDEBUG
						elog(LOG, "aclcheck: found group %u/%d",
							 aip->ai_id, aip->ai_mode);
#endif
						return ACLCHECK_OK;
					}
				}
			}
			break;
		case ACL_IDTYPE_GID:
			/* Look for this group ID */
			for (i = 1, aip = aidat + 1;		/* skip world entry */
				 i < num && aip->ai_idtype == ACL_IDTYPE_UID;
				 ++i, ++aip)
				 /* skip UID entry */ ;
			for (;
				 i < num && aip->ai_idtype == ACL_IDTYPE_GID;
				 ++i, ++aip)
			{
				if (aip->ai_id == id)
				{
#ifdef ACLDEBUG
					elog(LOG, "aclcheck: found group %u/%d",
						 aip->ai_id, aip->ai_mode);
#endif
					if (aip->ai_mode & mode)
						return ACLCHECK_OK;
				}
			}
			break;
		case ACL_IDTYPE_WORLD:
			/* Only check the world entry */
			break;
		default:
			elog(ERROR, "aclcheck: bogus ACL id type: %d", idtype);
			break;
	}

	/* If get here, he doesn't have the privilege nohow */
	return ACLCHECK_NO_PRIV;
}

/*
 * Exported routine for checking a user's access privileges to a table
 *
 * Returns an ACLCHECK_* result code.
 */
int32
pg_aclcheck(char *relname, Oid userid, AclMode mode)
{
	int32		result;
	HeapTuple	tuple;
	char	   *usename;
	Datum		aclDatum;
	bool		isNull;
	Acl		   *acl;

	/*
	 * Validate userid, find out if he is superuser
	 */
	tuple = SearchSysCache(SHADOWSYSID,
						   ObjectIdGetDatum(userid),
						   0, 0, 0);
	if (!HeapTupleIsValid(tuple))
		elog(ERROR, "pg_aclcheck: invalid user id %u",
			 (unsigned) userid);

	usename = NameStr(((Form_pg_shadow) GETSTRUCT(tuple))->usename);

	/*
	 * Deny anyone permission to update a system catalog unless
	 * pg_shadow.usecatupd is set.	(This is to let superusers protect
	 * themselves from themselves.)
	 */
	if ((mode & (ACL_INSERT | ACL_UPDATE | ACL_DELETE)) &&
		!allowSystemTableMods && IsSystemRelationName(relname) &&
		!is_temp_relname(relname) &&
		!((Form_pg_shadow) GETSTRUCT(tuple))->usecatupd)
	{
#ifdef ACLDEBUG
		elog(LOG, "pg_aclcheck: catalog update to \"%s\": permission denied",
			 relname);
#endif
		ReleaseSysCache(tuple);
		return ACLCHECK_NO_PRIV;
	}

	/*
	 * Otherwise, superusers bypass all permission-checking.
	 */
	if (((Form_pg_shadow) GETSTRUCT(tuple))->usesuper)
	{
#ifdef ACLDEBUG
		elog(LOG, "pg_aclcheck: \"%s\" is superuser",
			 usename);
#endif
		ReleaseSysCache(tuple);
		return ACLCHECK_OK;
	}

	ReleaseSysCache(tuple);
	/* caution: usename is inaccessible beyond this point... */

	/*
	 * Normal case: get the relation's ACL from pg_class
	 */
	tuple = SearchSysCache(RELNAME,
						   PointerGetDatum(relname),
						   0, 0, 0);
	if (!HeapTupleIsValid(tuple))
		elog(ERROR, "pg_aclcheck: class \"%s\" not found", relname);

	aclDatum = SysCacheGetAttr(RELNAME, tuple, Anum_pg_class_relacl,
							   &isNull);
	if (isNull)
	{
		/* No ACL, so build default ACL for rel */
		AclId		ownerId;

		ownerId = ((Form_pg_class) GETSTRUCT(tuple))->relowner;
		acl = acldefault(ownerId);
		aclDatum = (Datum) 0;
	}
	else
	{
		/* detoast rel's ACL if necessary */
		acl = DatumGetAclP(aclDatum);
	}

	result = aclcheck(acl, userid, (AclIdType) ACL_IDTYPE_UID, mode);

	/* if we have a detoasted copy, free it */
	if (acl && (Pointer) acl != DatumGetPointer(aclDatum))
		pfree(acl);

	ReleaseSysCache(tuple);

	return result;
}

/*
 * Check ownership of an object identified by name (which will be looked
 * up in the system cache identified by cacheid).
 *
 * Returns true if userid owns the item, or should be allowed to modify
 * the item as if he owned it.
 */
bool
pg_ownercheck(Oid userid,
			  const char *name,
			  int cacheid)
{
	HeapTuple	tuple;
	AclId		owner_id;
	char	   *usename;

	tuple = SearchSysCache(SHADOWSYSID,
						   ObjectIdGetDatum(userid),
						   0, 0, 0);
	if (!HeapTupleIsValid(tuple))
		elog(ERROR, "pg_ownercheck: invalid user id %u",
			 (unsigned) userid);
	usename = NameStr(((Form_pg_shadow) GETSTRUCT(tuple))->usename);

	/*
	 * Superusers bypass all permission-checking.
	 */
	if (((Form_pg_shadow) GETSTRUCT(tuple))->usesuper)
	{
#ifdef ACLDEBUG
		elog(LOG, "pg_ownercheck: user \"%s\" is superuser",
			 usename);
#endif
		ReleaseSysCache(tuple);
		return true;
	}

	ReleaseSysCache(tuple);
	/* caution: usename is inaccessible beyond this point... */

	tuple = SearchSysCache(cacheid,
						   PointerGetDatum(name),
						   0, 0, 0);
	switch (cacheid)
	{
		case RELNAME:
			if (!HeapTupleIsValid(tuple))
				elog(ERROR, "pg_ownercheck: class \"%s\" not found",
					 name);
			owner_id = ((Form_pg_class) GETSTRUCT(tuple))->relowner;
			break;
		case TYPENAME:
			if (!HeapTupleIsValid(tuple))
				elog(ERROR, "pg_ownercheck: type \"%s\" not found",
					 name);
			owner_id = ((Form_pg_type) GETSTRUCT(tuple))->typowner;
			break;
		default:
			elog(ERROR, "pg_ownercheck: invalid cache id: %d", cacheid);
			owner_id = 0;		/* keep compiler quiet */
			break;
	}

	ReleaseSysCache(tuple);

	return userid == owner_id;
}

/*
 * Ownership check for an operator (specified by OID).
 */
bool
pg_oper_ownercheck(Oid userid, Oid oprid)
{
	HeapTuple	tuple;
	AclId		owner_id;
	char	   *usename;

	tuple = SearchSysCache(SHADOWSYSID,
						   ObjectIdGetDatum(userid),
						   0, 0, 0);
	if (!HeapTupleIsValid(tuple))
		elog(ERROR, "pg_oper_ownercheck: invalid user id %u",
			 (unsigned) userid);
	usename = NameStr(((Form_pg_shadow) GETSTRUCT(tuple))->usename);

	/*
	 * Superusers bypass all permission-checking.
	 */
	if (((Form_pg_shadow) GETSTRUCT(tuple))->usesuper)
	{
#ifdef ACLDEBUG
		elog(LOG, "pg_ownercheck: user \"%s\" is superuser",
			 usename);
#endif
		ReleaseSysCache(tuple);
		return true;
	}

	ReleaseSysCache(tuple);
	/* caution: usename is inaccessible beyond this point... */

	tuple = SearchSysCache(OPEROID,
						   ObjectIdGetDatum(oprid),
						   0, 0, 0);
	if (!HeapTupleIsValid(tuple))
		elog(ERROR, "pg_ownercheck: operator %u not found",
			 oprid);

	owner_id = ((Form_pg_operator) GETSTRUCT(tuple))->oprowner;

	ReleaseSysCache(tuple);

	return userid == owner_id;
}

/*
 * Ownership check for a function (specified by name and argument types).
 */
bool
pg_func_ownercheck(Oid userid,
				   char *funcname,
				   int nargs,
				   Oid *arglist)
{
	HeapTuple	tuple;
	AclId		owner_id;
	char	   *usename;

	tuple = SearchSysCache(SHADOWSYSID,
						   ObjectIdGetDatum(userid),
						   0, 0, 0);
	if (!HeapTupleIsValid(tuple))
		elog(ERROR, "pg_func_ownercheck: invalid user id %u",
			 (unsigned) userid);
	usename = NameStr(((Form_pg_shadow) GETSTRUCT(tuple))->usename);

	/*
	 * Superusers bypass all permission-checking.
	 */
	if (((Form_pg_shadow) GETSTRUCT(tuple))->usesuper)
	{
#ifdef ACLDEBUG
		elog(LOG, "pg_ownercheck: user \"%s\" is superuser",
			 usename);
#endif
		ReleaseSysCache(tuple);
		return true;
	}

	ReleaseSysCache(tuple);
	/* caution: usename is inaccessible beyond this point... */

	tuple = SearchSysCache(PROCNAME,
						   PointerGetDatum(funcname),
						   Int32GetDatum(nargs),
						   PointerGetDatum(arglist),
						   0);
	if (!HeapTupleIsValid(tuple))
		func_error("pg_func_ownercheck", funcname, nargs, arglist, NULL);

	owner_id = ((Form_pg_proc) GETSTRUCT(tuple))->proowner;

	ReleaseSysCache(tuple);

	return userid == owner_id;
}

/*
 * Ownership check for an aggregate function (specified by name and
 * argument type).
 */
bool
pg_aggr_ownercheck(Oid userid,
				   char *aggname,
				   Oid basetypeID)
{
	HeapTuple	tuple;
	AclId		owner_id;
	char	   *usename;

	tuple = SearchSysCache(SHADOWSYSID,
						   PointerGetDatum(userid),
						   0, 0, 0);
	if (!HeapTupleIsValid(tuple))
		elog(ERROR, "pg_aggr_ownercheck: invalid user id %u",
			 (unsigned) userid);
	usename = NameStr(((Form_pg_shadow) GETSTRUCT(tuple))->usename);

	/*
	 * Superusers bypass all permission-checking.
	 */
	if (((Form_pg_shadow) GETSTRUCT(tuple))->usesuper)
	{
#ifdef ACLDEBUG
		elog(LOG, "pg_aggr_ownercheck: user \"%s\" is superuser",
			 usename);
#endif
		ReleaseSysCache(tuple);
		return true;
	}

	ReleaseSysCache(tuple);
	/* caution: usename is inaccessible beyond this point... */

	tuple = SearchSysCache(AGGNAME,
						   PointerGetDatum(aggname),
						   ObjectIdGetDatum(basetypeID),
						   0, 0);
	if (!HeapTupleIsValid(tuple))
		agg_error("pg_aggr_ownercheck", aggname, basetypeID);

	owner_id = ((Form_pg_aggregate) GETSTRUCT(tuple))->aggowner;

	ReleaseSysCache(tuple);

	return userid == owner_id;
}



/*
 * Exported routine for checking a user's access privileges to a function
 *
 * Returns an ACLCHECK_* result code.
 */
int32
pg_proc_aclcheck(Oid proc_oid, Oid userid)
{
	int32		result;
	HeapTuple	tuple;
	Datum		aclDatum;
	bool		isNull;
	Acl		   *acl;

	if (superuser_arg(userid))
		return ACLCHECK_OK;

	/*
	 * Validate userid
	 */
	tuple = SearchSysCache(SHADOWSYSID,
						   ObjectIdGetDatum(userid),
						   0, 0, 0);
	if (!HeapTupleIsValid(tuple))
		elog(ERROR, "pg_proc_aclcheck: invalid user id %u",
			 (unsigned) userid);
	ReleaseSysCache(tuple);

	/*
	 * Normal case: get the function's ACL from pg_proc
	 */
	tuple = SearchSysCache(PROCOID,
						   ObjectIdGetDatum(proc_oid),
						   0, 0, 0);
	if (!HeapTupleIsValid(tuple))
		elog(ERROR, "pg_proc_aclcheck: function %u not found", proc_oid);

	aclDatum = SysCacheGetAttr(PROCOID, tuple, Anum_pg_proc_proacl,
							   &isNull);
	if (isNull)
	{
		/* No ACL, so build default ACL */
		AclId		ownerId;

		ownerId = ((Form_pg_proc) GETSTRUCT(tuple))->proowner;
		acl = acldefault(ownerId);
		aclDatum = (Datum) 0;
	}
	else
	{
		/* detoast ACL if necessary */
		acl = DatumGetAclP(aclDatum);
	}

	/*
	 * Functions only have one kind of privilege, which is encoded as
	 * "SELECT" here.
	 */
	result = aclcheck(acl, userid, (AclIdType) ACL_IDTYPE_UID, ACL_SELECT);

	/* if we have a detoasted copy, free it */
	if (acl && (Pointer) acl != DatumGetPointer(aclDatum))
		pfree(acl);

	ReleaseSysCache(tuple);

	return result;
}



/*
 * Exported routine for checking a user's access privileges to a language
 *
 * Returns an ACLCHECK_* result code.
 */
int32
pg_language_aclcheck(Oid lang_oid, Oid userid)
{
	int32		result;
	HeapTuple	tuple;
	Datum		aclDatum;
	bool		isNull;
	Acl		   *acl;

	if (superuser_arg(userid))
		return ACLCHECK_OK;

	/*
	 * Validate userid
	 */
	tuple = SearchSysCache(SHADOWSYSID,
						   ObjectIdGetDatum(userid),
						   0, 0, 0);
	if (!HeapTupleIsValid(tuple))
		elog(ERROR, "pg_language_aclcheck: invalid user id %u",
			 (unsigned) userid);
	ReleaseSysCache(tuple);

	/*
	 * Normal case: get the function's ACL from pg_language
	 */
	tuple = SearchSysCache(LANGOID,
						   ObjectIdGetDatum(lang_oid),
						   0, 0, 0);
	if (!HeapTupleIsValid(tuple))
		elog(ERROR, "pg_language_aclcheck: language %u not found", lang_oid);

	aclDatum = SysCacheGetAttr(LANGOID, tuple, Anum_pg_language_lanacl,
							   &isNull);
	if (isNull)
	{
		/* No ACL, so build default ACL */
		acl = acldefault(InvalidOid);
		aclDatum = (Datum) 0;
	}
	else
	{
		/* detoast ACL if necessary */
		acl = DatumGetAclP(aclDatum);
	}

	/*
	 * Languages only have one kind of privilege, which is encoded as
	 * "SELECT" here.
	 */
	result = aclcheck(acl, userid, (AclIdType) ACL_IDTYPE_UID, ACL_SELECT);

	/* if we have a detoasted copy, free it */
	if (acl && (Pointer) acl != DatumGetPointer(aclDatum))
		pfree(acl);

	ReleaseSysCache(tuple);

	return result;
}
