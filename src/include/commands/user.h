/*-------------------------------------------------------------------------
 *
 * user.h
 *	  Commands for manipulating users and groups.
 *
 *
 * $PostgreSQL: pgsql/src/include/commands/user.h,v 1.25 2004/09/16 16:58:39 tgl Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef USER_H
#define USER_H

#include "nodes/parsenodes.h"


extern void CreateUser(CreateUserStmt *stmt);
extern void AlterUser(AlterUserStmt *stmt);
extern void AlterUserSet(AlterUserSetStmt *stmt);
extern void DropUser(DropUserStmt *stmt);
extern void RenameUser(const char *oldname, const char *newname);

extern void CreateGroup(CreateGroupStmt *stmt);
extern void AlterGroup(AlterGroupStmt *stmt, const char *tag);
extern void DropGroup(DropGroupStmt *stmt);
extern void RenameGroup(const char *oldname, const char *newname);

#endif   /* USER_H */
