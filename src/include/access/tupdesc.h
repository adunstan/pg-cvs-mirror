/*-------------------------------------------------------------------------
 *
 * tupdesc.h--
 *    POSTGRES tuple descriptor definitions.
 *
 *
 * Copyright (c) 1994, Regents of the University of California
 *
 * $Id: tupdesc.h,v 1.2 1996/09/16 05:33:13 scrappy Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef	TUPDESC_H
#define TUPDESC_H

#include "postgres.h"
#include "access/attnum.h"
#include "nodes/pg_list.h"	/* for List */
#include "catalog/pg_attribute.h"


typedef struct tupleDesc {
/*------------------------------------------------------------------------ 
  This structure contains all the attribute information (i.e. from Class 
  pg_attribute) for a tuple. 
-------------------------------------------------------------------------*/
    int  natts;      
      /* Number of attributes in the tuple */
    AttributeTupleForm *attrs;
      /* attrs[N] is a pointer to the description of Attribute Number N+1.  */
} *TupleDesc;

extern TupleDesc CreateTemplateTupleDesc(int natts);

extern TupleDesc CreateTupleDesc(int natts, AttributeTupleForm *attrs);

extern TupleDesc CreateTupleDescCopy(TupleDesc tupdesc);

extern bool TupleDescInitEntry(TupleDesc desc,
			       AttrNumber attributeNumber,
			       char *attributeName, 
			       char *typeName, 
			       int attdim, 
			       bool attisset);

extern TupleDesc BuildDescForRelation(List *schema, char *relname);

#endif	/* TUPDESC_H */
