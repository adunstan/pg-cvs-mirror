/*-------------------------------------------------------------------------
 *
 * genam.h
 *	  POSTGRES general access method definitions.
 *
 *
 * Copyright (c) 1994, Regents of the University of California
 *
 * $Id: genam.h,v 1.18.2.1 1999/07/30 18:26:58 scrappy Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef GENAM_H
#define GENAM_H

#include "access/funcindex.h"
#include "access/itup.h"
#include "access/relscan.h"
#include "access/sdir.h"

/* ----------------
 *		generalized index_ interface routines
 * ----------------
 */
extern Relation index_open(Oid relationId);
extern Relation index_openr(char *relationName);
extern void index_close(Relation relation);
extern InsertIndexResult index_insert(Relation relation,
			 Datum *datum, char *nulls,
			 ItemPointer heap_t_ctid,
			 Relation heapRel);
extern void index_delete(Relation relation, ItemPointer indexItem);
extern IndexScanDesc index_beginscan(Relation relation, bool scanFromEnd,
				uint16 numberOfKeys, ScanKey key);
extern void index_rescan(IndexScanDesc scan, bool scanFromEnd, ScanKey key);
extern void index_endscan(IndexScanDesc scan);
extern void index_markpos(IndexScanDesc scan);
extern void index_restrpos(IndexScanDesc scan);
extern RetrieveIndexResult index_getnext(IndexScanDesc scan,
			  ScanDirection direction);
extern RegProcedure index_getprocid(Relation irel, AttrNumber attnum,
				uint16 procnum);
extern Datum GetIndexValue(HeapTuple tuple, TupleDesc hTupDesc,
			  int attOff, AttrNumber *attrNums, FuncIndexInfo *fInfo,
			  bool *attNull);

/* in genam.c */
extern IndexScanDesc RelationGetIndexScan(Relation relation, bool scanFromEnd,
					 uint16 numberOfKeys, ScanKey key);

#endif	 /* GENAM_H */
