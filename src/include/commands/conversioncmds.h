/*-------------------------------------------------------------------------
 *
 * conversioncmds.h
 *	  prototypes for conversioncmds.c.
 *
 *
 * Portions Copyright (c) 1996-2003, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql-server/src/include/commands/conversioncmds.h,v 1.6 2003/11/29 22:40:59 pgsql Exp $
 *
 *-------------------------------------------------------------------------
 */

#ifndef CONVERSIONCMDS_H
#define CONVERSIONCMDS_H

#include "nodes/parsenodes.h"

extern void CreateConversionCommand(CreateConversionStmt *parsetree);
extern void DropConversionCommand(List *conversion_name, DropBehavior behavior);
extern void RenameConversion(List *name, const char *newname);
extern void AlterConversionOwner(List *name, AclId newOwnerSysId);

#endif   /* CONVERSIONCMDS_H */
