/*-------------------------------------------------------------------------
 *
 * readfuncs.h
 *	  header file for read.c and readfuncs.c. These functions are internal
 *	  to the stringToNode interface and should not be used by anyone else.
 *
 * Portions Copyright (c) 1996-2004, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql-server/src/include/nodes/readfuncs.h,v 1.19 2004/05/06 14:01:33 tgl Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef READFUNCS_H
#define READFUNCS_H

#include "nodes/nodes.h"

/*
 * prototypes for functions in read.c (the lisp token parser)
 */
extern char *pg_strtok(int *length);
extern char *debackslash(char *token, int length);
extern void *nodeRead(char *token, int tok_len);

/*
 * prototypes for functions in readfuncs.c
 */
extern Node *parseNodeString(void);

#endif   /* READFUNCS_H */
