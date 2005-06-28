/*-------------------------------------------------------------------------
 *
 * user.h
 *	  Commands for manipulating roles (formerly called users).
 *
 *
 * $PostgreSQL: pgsql/src/include/commands/user.h,v 1.26 2005/02/20 02:22:05 tgl Exp $
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

#endif   /* USER_H */
