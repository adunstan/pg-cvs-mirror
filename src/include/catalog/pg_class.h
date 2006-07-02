/*-------------------------------------------------------------------------
 *
 * pg_class.h
 *	  definition of the system "relation" relation (pg_class)
 *	  along with the relation's initial contents.
 *
 *
 * Portions Copyright (c) 1996-2006, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/catalog/pg_class.h,v 1.92 2006/05/28 02:27:08 alvherre Exp $
 *
 * NOTES
 *	  the genbki.sh script reads this file and generates .bki
 *	  information from the DATA() statements.
 *
 *-------------------------------------------------------------------------
 */
#ifndef PG_CLASS_H
#define PG_CLASS_H

/* ----------------
 *		postgres.h contains the system type definitions and the
 *		CATALOG(), BKI_BOOTSTRAP and DATA() sugar words so this file
 *		can be read by both genbki.sh and the C compiler.
 * ----------------
 */

/* ----------------
 *		pg_class definition.  cpp turns this into
 *		typedef struct FormData_pg_class
 * ----------------
 */

/* ----------------
 *		This structure is actually variable-length (the last attribute is
 *		a POSTGRES array).	Hence, sizeof(FormData_pg_class) does not
 *		necessarily match the actual length of the structure.  Furthermore
 *		relacl may be a NULL field.  Hence, you MUST use heap_getattr()
 *		to get the relacl field ... and don't forget to check isNull.
 * ----------------
 */
#define RelationRelationId	1259

CATALOG(pg_class,1259) BKI_BOOTSTRAP
{
	NameData	relname;		/* class name */
	Oid			relnamespace;	/* OID of namespace containing this class */
	Oid			reltype;		/* OID of associated entry in pg_type */
	Oid			relowner;		/* class owner */
	Oid			relam;			/* index access method; 0 if not an index */
	Oid			relfilenode;	/* identifier of physical storage file */
	Oid			reltablespace;	/* identifier of table space for relation */
	int4		relpages;		/* # of blocks (not always up-to-date) */
	float4		reltuples;		/* # of tuples (not always up-to-date) */
	Oid			reltoastrelid;	/* OID of toast table; 0 if none */
	Oid			reltoastidxid;	/* if toast table, OID of chunk_id index */
	bool		relhasindex;	/* T if has (or has had) any indexes */
	bool		relisshared;	/* T if shared across databases */
	char		relkind;		/* see RELKIND_xxx constants below */
	int2		relnatts;		/* number of user attributes */

	/*
	 * Class pg_attribute must contain exactly "relnatts" user attributes
	 * (with attnums ranging from 1 to relnatts) for this class.  It may also
	 * contain entries with negative attnums for system attributes.
	 */
	int2		relchecks;		/* # of CHECK constraints for class */
	int2		reltriggers;	/* # of TRIGGERs */
	int2		relukeys;		/* # of Unique keys (not used) */
	int2		relfkeys;		/* # of FOREIGN KEYs (not used) */
	int2		relrefs;		/* # of references to this rel (not used) */
	bool		relhasoids;		/* T if we generate OIDs for rows of rel */
	bool		relhaspkey;		/* has PRIMARY KEY index */
	bool		relhasrules;	/* has associated rules */
	bool		relhassubclass; /* has derived classes */

	/* following fields may or may not be present, see note above! */
	text		reloptions[1];	/* access method specific data */
	aclitem		relacl[1];		/* we declare this just for the catalog */
} FormData_pg_class;

/* Size of fixed part of pg_class tuples, not counting relacl or padding */
#define CLASS_TUPLE_SIZE \
	 (offsetof(FormData_pg_class,relhassubclass) + sizeof(bool))

/* ----------------
 *		Form_pg_class corresponds to a pointer to a tuple with
 *		the format of pg_class relation.
 * ----------------
 */
typedef FormData_pg_class *Form_pg_class;

/* ----------------
 *		compiler constants for pg_class
 * ----------------
 */

/* ----------------
 *		Natts_pg_class_fixed is used to tell routines that insert new
 *		pg_class tuples (as opposed to replacing old ones) that there's no
 *		relacl field.  This is a kluge.
 * ----------------
 */
#define Natts_pg_class_fixed			24
#define Natts_pg_class					26
#define Anum_pg_class_relname			1
#define Anum_pg_class_relnamespace		2
#define Anum_pg_class_reltype			3
#define Anum_pg_class_relowner			4
#define Anum_pg_class_relam				5
#define Anum_pg_class_relfilenode		6
#define Anum_pg_class_reltablespace		7
#define Anum_pg_class_relpages			8
#define Anum_pg_class_reltuples			9
#define Anum_pg_class_reltoastrelid		10
#define Anum_pg_class_reltoastidxid		11
#define Anum_pg_class_relhasindex		12
#define Anum_pg_class_relisshared		13
#define Anum_pg_class_relkind			14
#define Anum_pg_class_relnatts			15
#define Anum_pg_class_relchecks			16
#define Anum_pg_class_reltriggers		17
#define Anum_pg_class_relukeys			18
#define Anum_pg_class_relfkeys			19
#define Anum_pg_class_relrefs			20
#define Anum_pg_class_relhasoids		21
#define Anum_pg_class_relhaspkey		22
#define Anum_pg_class_relhasrules		23
#define Anum_pg_class_relhassubclass	24
#define Anum_pg_class_reloptions		25
#define Anum_pg_class_relacl			26

/* ----------------
 *		initial contents of pg_class
 *
 * NOTE: only "bootstrapped" relations need to be declared here.  Be sure that
 * the OIDs listed here match those given in their CATALOG macros.
 * ----------------
 */

DATA(insert OID = 1247 (  pg_type		PGNSP 71 PGUID 0 1247 0 0 0 0 0 f f r 23 0 0 0 0 0 t f f f _null_ _null_ ));
DESCR("");
DATA(insert OID = 1249 (  pg_attribute	PGNSP 75 PGUID 0 1249 0 0 0 0 0 f f r 17 0 0 0 0 0 f f f f _null_ _null_ ));
DESCR("");
DATA(insert OID = 1255 (  pg_proc		PGNSP 81 PGUID 0 1255 0 0 0 0 0 f f r 18 0 0 0 0 0 t f f f _null_ _null_ ));
DESCR("");
DATA(insert OID = 1259 (  pg_class		PGNSP 83 PGUID 0 1259 0 0 0 0 0 f f r 26 0 0 0 0 0 t f f f _null_ _null_ ));
DESCR("");

#define		  RELKIND_INDEX			  'i'		/* secondary index */
#define		  RELKIND_RELATION		  'r'		/* ordinary cataloged heap */
#define		  RELKIND_SEQUENCE		  'S'		/* SEQUENCE relation */
#define		  RELKIND_UNCATALOGED	  'u'		/* temporary heap */
#define		  RELKIND_TOASTVALUE	  't'		/* moved off huge values */
#define		  RELKIND_VIEW			  'v'		/* view */
#define		  RELKIND_COMPOSITE_TYPE  'c'		/* composite type */

#endif   /* PG_CLASS_H */
