/*-------------------------------------------------------------------------
 *
 * nodes.c
 *	  support code for nodes (now that we have removed the home-brew
 *	  inheritance system, our support code for nodes is much simpler)
 *
 * Portions Copyright (c) 1996-2003, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  $PostgreSQL: pgsql-server/src/backend/nodes/nodes.c,v 1.22 2003/11/29 19:51:49 pgsql Exp $
 *
 * HISTORY
 *	  Andrew Yu			Oct 20, 1994	file creation
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include "nodes/nodes.h"

/*
 * Support for newNode() macro
 */

Node	   *newNodeMacroHolder;
