/*-------------------------------------------------------------------------
 *
 * typecmds.h
 *	  prototypes for typecmds.c.
 *
 *
 * Portions Copyright (c) 1996-2010, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/commands/typecmds.h,v 1.26 2009/12/24 22:09:24 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef TYPECMDS_H
#define TYPECMDS_H

#include "nodes/parsenodes.h"


#define DEFAULT_TYPDELIM		','

extern void DefineType(List *names, List *parameters);
extern void RemoveTypes(DropStmt *drop);
extern void RemoveTypeById(Oid typeOid);
extern void DefineDomain(CreateDomainStmt *stmt);
extern void DefineEnum(CreateEnumStmt *stmt);
extern Oid	DefineCompositeType(const RangeVar *typevar, List *coldeflist);
extern Oid	AssignTypeArrayOid(void);

extern void AlterDomainDefault(List *names, Node *defaultRaw);
extern void AlterDomainNotNull(List *names, bool notNull);
extern void AlterDomainAddConstraint(List *names, Node *constr);
extern void AlterDomainDropConstraint(List *names, const char *constrName,
						  DropBehavior behavior);

extern List *GetDomainConstraints(Oid typeOid);

extern void RenameType(List *names, const char *newTypeName);
extern void AlterTypeOwner(List *names, Oid newOwnerId);
extern void AlterTypeOwnerInternal(Oid typeOid, Oid newOwnerId,
					   bool hasDependEntry);
extern void AlterTypeNamespace(List *names, const char *newschema);
extern void AlterTypeNamespaceInternal(Oid typeOid, Oid nspOid,
						   bool isImplicitArray,
						   bool errorOnTableType);

#endif   /* TYPECMDS_H */
