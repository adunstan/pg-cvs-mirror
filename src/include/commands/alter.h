/*-------------------------------------------------------------------------
 *
 * alter.h
 *	  prototypes for commands/alter.c
 *
 *
 * Portions Copyright (c) 1996-2003, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql-server/src/include/commands/alter.h,v 1.3 2003/11/29 22:40:59 pgsql Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef ALTER_H
#define ALTER_H

#include "nodes/parsenodes.h"

extern void ExecRenameStmt(RenameStmt *stmt);

extern void ExecAlterOwnerStmt(AlterOwnerStmt *stmt);

#endif   /* ALTER_H */
