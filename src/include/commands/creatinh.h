/*-------------------------------------------------------------------------
 *
 * creatinh.h
 *	  prototypes for creatinh.c.
 *
 *
 * Portions Copyright (c) 1996-2001, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $Id: creatinh.h,v 1.15 2001/10/25 05:49:58 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef CREATINH_H
#define CREATINH_H

#include "nodes/parsenodes.h"

extern void DefineRelation(CreateStmt *stmt, char relkind);
extern void RemoveRelation(char *name);
extern void TruncateRelation(char *name);
#endif	 /* CREATINH_H */
