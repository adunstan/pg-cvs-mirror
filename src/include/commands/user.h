/*-------------------------------------------------------------------------
 *
 * user.h
 *	  Commands for manipulating roles (formerly called users).
 *
 *
 * $PostgreSQL: pgsql/src/include/commands/user.h,v 1.29 2005/11/22 18:17:30 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef USER_H
#define USER_H

#include "nodes/parsenodes.h"


extern void CreateRole(CreateRoleStmt *stmt);
extern void AlterRole(AlterRoleStmt *stmt);
extern void AlterRoleSet(AlterRoleSetStmt *stmt);
extern void DropRole(DropRoleStmt *stmt);
extern void GrantRole(GrantRoleStmt *stmt);
extern void RenameRole(const char *oldname, const char *newname);
extern void DropOwnedObjects(DropOwnedStmt *stmt);
extern void ReassignOwnedObjects(ReassignOwnedStmt *stmt);

#endif   /* USER_H */
