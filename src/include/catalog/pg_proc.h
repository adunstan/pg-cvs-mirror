/*-------------------------------------------------------------------------
 *
 * pg_proc.h
 *	  definition of the system "procedure" relation (pg_proc)
 *	  along with the relation's initial contents.
 *
 * Portions Copyright (c) 1996-2007, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/catalog/pg_proc.h,v 1.459 2007/06/06 23:00:41 tgl Exp $
 *
 * NOTES
 *	  The script catalog/genbki.sh reads this file and generates .bki
 *	  information from the DATA() statements.  utils/Gen_fmgrtab.sh
 *	  generates fmgroids.h and fmgrtab.c the same way.
 *
 *	  XXX do NOT break up DATA() statements into multiple lines!
 *		  the scripts are not as smart as you might think...
 *	  XXX (eg. #if 0 #endif won't do what you think)
 *
 *-------------------------------------------------------------------------
 */
#ifndef PG_PROC_H
#define PG_PROC_H

/* ----------------
 *		postgres.h contains the system type definitions and the
 *		CATALOG(), BKI_BOOTSTRAP and DATA() sugar words so this file
 *		can be read by both genbki.sh and the C compiler.
 * ----------------
 */

/* ----------------
 *		pg_proc definition.  cpp turns this into
 *		typedef struct FormData_pg_proc
 * ----------------
 */
#define ProcedureRelationId  1255

CATALOG(pg_proc,1255) BKI_BOOTSTRAP
{
	NameData	proname;		/* procedure name */
	Oid			pronamespace;	/* OID of namespace containing this proc */
	Oid			proowner;		/* procedure owner */
	Oid			prolang;		/* OID of pg_language entry */
	float4		procost;		/* estimated execution cost */
	float4		prorows;		/* estimated # of rows out (if proretset) */
	bool		proisagg;		/* is it an aggregate? */
	bool		prosecdef;		/* security definer */
	bool		proisstrict;	/* strict with respect to NULLs? */
	bool		proretset;		/* returns a set? */
	char		provolatile;	/* see PROVOLATILE_ categories below */
	int2		pronargs;		/* number of arguments */
	Oid			prorettype;		/* OID of result type */

	/* VARIABLE LENGTH FIELDS: */
	oidvector	proargtypes;	/* parameter types (excludes OUT params) */
	Oid			proallargtypes[1];		/* all param types (NULL if IN only) */
	char		proargmodes[1]; /* parameter modes (NULL if IN only) */
	text		proargnames[1]; /* parameter names (NULL if no names) */
	text		prosrc;			/* procedure source text */
	bytea		probin;			/* secondary procedure definition field */
	aclitem		proacl[1];		/* access permissions */
} FormData_pg_proc;

/* ----------------
 *		Form_pg_proc corresponds to a pointer to a tuple with
 *		the format of pg_proc relation.
 * ----------------
 */
typedef FormData_pg_proc *Form_pg_proc;

/* ----------------
 *		compiler constants for pg_proc
 * ----------------
 */
#define Natts_pg_proc					20
#define Anum_pg_proc_proname			1
#define Anum_pg_proc_pronamespace		2
#define Anum_pg_proc_proowner			3
#define Anum_pg_proc_prolang			4
#define Anum_pg_proc_procost			5
#define Anum_pg_proc_prorows			6
#define Anum_pg_proc_proisagg			7
#define Anum_pg_proc_prosecdef			8
#define Anum_pg_proc_proisstrict		9
#define Anum_pg_proc_proretset			10
#define Anum_pg_proc_provolatile		11
#define Anum_pg_proc_pronargs			12
#define Anum_pg_proc_prorettype			13
#define Anum_pg_proc_proargtypes		14
#define Anum_pg_proc_proallargtypes		15
#define Anum_pg_proc_proargmodes		16
#define Anum_pg_proc_proargnames		17
#define Anum_pg_proc_prosrc				18
#define Anum_pg_proc_probin				19
#define Anum_pg_proc_proacl				20

/* ----------------
 *		initial contents of pg_proc
 * ----------------
 */

/* keep the following ordered by OID so that later changes can be made easier */

/* OIDS 1 - 99 */

DATA(insert OID = 1242 (  boolin		   PGNSP PGUID 12 1 0 f f t f i 1 16 "2275" _null_ _null_ _null_ boolin - _null_ ));
DESCR("I/O");
DATA(insert OID = 1243 (  boolout		   PGNSP PGUID 12 1 0 f f t f i 1 2275 "16" _null_ _null_ _null_ boolout - _null_ ));
DESCR("I/O");
DATA(insert OID = 1244 (  byteain		   PGNSP PGUID 12 1 0 f f t f i 1 17 "2275" _null_ _null_ _null_ byteain - _null_ ));
DESCR("I/O");
DATA(insert OID =  31 (  byteaout		   PGNSP PGUID 12 1 0 f f t f i 1 2275 "17" _null_ _null_ _null_ byteaout - _null_ ));
DESCR("I/O");
DATA(insert OID = 1245 (  charin		   PGNSP PGUID 12 1 0 f f t f i 1 18 "2275" _null_ _null_ _null_ charin - _null_ ));
DESCR("I/O");
DATA(insert OID =  33 (  charout		   PGNSP PGUID 12 1 0 f f t f i 1 2275 "18" _null_ _null_ _null_ charout - _null_ ));
DESCR("I/O");
DATA(insert OID =  34 (  namein			   PGNSP PGUID 12 1 0 f f t f i 1 19 "2275" _null_ _null_ _null_ namein - _null_ ));
DESCR("I/O");
DATA(insert OID =  35 (  nameout		   PGNSP PGUID 12 1 0 f f t f i 1 2275 "19" _null_ _null_ _null_ nameout - _null_ ));
DESCR("I/O");
DATA(insert OID =  38 (  int2in			   PGNSP PGUID 12 1 0 f f t f i 1 21 "2275" _null_ _null_ _null_ int2in - _null_ ));
DESCR("I/O");
DATA(insert OID =  39 (  int2out		   PGNSP PGUID 12 1 0 f f t f i 1 2275 "21" _null_ _null_ _null_ int2out - _null_ ));
DESCR("I/O");
DATA(insert OID =  40 (  int2vectorin	   PGNSP PGUID 12 1 0 f f t f i 1 22 "2275" _null_ _null_ _null_ int2vectorin - _null_ ));
DESCR("I/O");
DATA(insert OID =  41 (  int2vectorout	   PGNSP PGUID 12 1 0 f f t f i 1 2275 "22" _null_ _null_ _null_ int2vectorout - _null_ ));
DESCR("I/O");
DATA(insert OID =  42 (  int4in			   PGNSP PGUID 12 1 0 f f t f i 1 23 "2275" _null_ _null_ _null_ int4in - _null_ ));
DESCR("I/O");
DATA(insert OID =  43 (  int4out		   PGNSP PGUID 12 1 0 f f t f i 1 2275 "23" _null_ _null_ _null_ int4out - _null_ ));
DESCR("I/O");
DATA(insert OID =  44 (  regprocin		   PGNSP PGUID 12 1 0 f f t f s 1 24 "2275" _null_ _null_ _null_ regprocin - _null_ ));
DESCR("I/O");
DATA(insert OID =  45 (  regprocout		   PGNSP PGUID 12 1 0 f f t f s 1 2275 "24" _null_ _null_ _null_ regprocout - _null_ ));
DESCR("I/O");
DATA(insert OID =  46 (  textin			   PGNSP PGUID 12 1 0 f f t f i 1 25 "2275" _null_ _null_ _null_ textin - _null_ ));
DESCR("I/O");
DATA(insert OID =  47 (  textout		   PGNSP PGUID 12 1 0 f f t f i 1 2275 "25" _null_ _null_ _null_ textout - _null_ ));
DESCR("I/O");
DATA(insert OID =  48 (  tidin			   PGNSP PGUID 12 1 0 f f t f i 1 27 "2275" _null_ _null_ _null_ tidin - _null_ ));
DESCR("I/O");
DATA(insert OID =  49 (  tidout			   PGNSP PGUID 12 1 0 f f t f i 1 2275 "27" _null_ _null_ _null_ tidout - _null_ ));
DESCR("I/O");
DATA(insert OID =  50 (  xidin			   PGNSP PGUID 12 1 0 f f t f i 1 28 "2275" _null_ _null_ _null_ xidin - _null_ ));
DESCR("I/O");
DATA(insert OID =  51 (  xidout			   PGNSP PGUID 12 1 0 f f t f i 1 2275 "28" _null_ _null_ _null_ xidout - _null_ ));
DESCR("I/O");
DATA(insert OID =  52 (  cidin			   PGNSP PGUID 12 1 0 f f t f i 1 29 "2275" _null_ _null_ _null_ cidin - _null_ ));
DESCR("I/O");
DATA(insert OID =  53 (  cidout			   PGNSP PGUID 12 1 0 f f t f i 1 2275 "29" _null_ _null_ _null_ cidout - _null_ ));
DESCR("I/O");
DATA(insert OID =  54 (  oidvectorin	   PGNSP PGUID 12 1 0 f f t f i 1 30 "2275" _null_ _null_ _null_ oidvectorin - _null_ ));
DESCR("I/O");
DATA(insert OID =  55 (  oidvectorout	   PGNSP PGUID 12 1 0 f f t f i 1 2275 "30" _null_ _null_ _null_ oidvectorout - _null_ ));
DESCR("I/O");
DATA(insert OID =  56 (  boollt			   PGNSP PGUID 12 1 0 f f t f i 2 16 "16 16" _null_ _null_ _null_ boollt - _null_ ));
DESCR("less-than");
DATA(insert OID =  57 (  boolgt			   PGNSP PGUID 12 1 0 f f t f i 2 16 "16 16" _null_ _null_ _null_ boolgt - _null_ ));
DESCR("greater-than");
DATA(insert OID =  60 (  booleq			   PGNSP PGUID 12 1 0 f f t f i 2 16 "16 16" _null_ _null_ _null_ booleq - _null_ ));
DESCR("equal");
DATA(insert OID =  61 (  chareq			   PGNSP PGUID 12 1 0 f f t f i 2 16 "18 18" _null_ _null_ _null_ chareq - _null_ ));
DESCR("equal");
DATA(insert OID =  62 (  nameeq			   PGNSP PGUID 12 1 0 f f t f i 2 16 "19 19" _null_ _null_ _null_ nameeq - _null_ ));
DESCR("equal");
DATA(insert OID =  63 (  int2eq			   PGNSP PGUID 12 1 0 f f t f i 2 16 "21 21" _null_ _null_ _null_ int2eq - _null_ ));
DESCR("equal");
DATA(insert OID =  64 (  int2lt			   PGNSP PGUID 12 1 0 f f t f i 2 16 "21 21" _null_ _null_ _null_ int2lt - _null_ ));
DESCR("less-than");
DATA(insert OID =  65 (  int4eq			   PGNSP PGUID 12 1 0 f f t f i 2 16 "23 23" _null_ _null_ _null_ int4eq - _null_ ));
DESCR("equal");
DATA(insert OID =  66 (  int4lt			   PGNSP PGUID 12 1 0 f f t f i 2 16 "23 23" _null_ _null_ _null_ int4lt - _null_ ));
DESCR("less-than");
DATA(insert OID =  67 (  texteq			   PGNSP PGUID 12 1 0 f f t f i 2 16 "25 25" _null_ _null_ _null_ texteq - _null_ ));
DESCR("equal");
DATA(insert OID =  68 (  xideq			   PGNSP PGUID 12 1 0 f f t f i 2 16 "28 28" _null_ _null_ _null_ xideq - _null_ ));
DESCR("equal");
DATA(insert OID =  69 (  cideq			   PGNSP PGUID 12 1 0 f f t f i 2 16 "29 29" _null_ _null_ _null_ cideq - _null_ ));
DESCR("equal");
DATA(insert OID =  70 (  charne			   PGNSP PGUID 12 1 0 f f t f i 2 16 "18 18" _null_ _null_ _null_ charne - _null_ ));
DESCR("not equal");
DATA(insert OID = 1246 (  charlt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "18 18" _null_ _null_ _null_ charlt - _null_ ));
DESCR("less-than");
DATA(insert OID =  72 (  charle			   PGNSP PGUID 12 1 0 f f t f i 2 16 "18 18" _null_ _null_ _null_ charle - _null_ ));
DESCR("less-than-or-equal");
DATA(insert OID =  73 (  chargt			   PGNSP PGUID 12 1 0 f f t f i 2 16 "18 18" _null_ _null_ _null_ chargt - _null_ ));
DESCR("greater-than");
DATA(insert OID =  74 (  charge			   PGNSP PGUID 12 1 0 f f t f i 2 16 "18 18" _null_ _null_ _null_ charge - _null_ ));
DESCR("greater-than-or-equal");
DATA(insert OID =  77 (  int4			   PGNSP PGUID 12 1 0 f f t f i 1  23  "18" _null_ _null_ _null_	chartoi4 - _null_ ));
DESCR("convert char to int4");
DATA(insert OID =  78 (  char			   PGNSP PGUID 12 1 0 f f t f i 1  18  "23" _null_ _null_ _null_	i4tochar - _null_ ));
DESCR("convert int4 to char");

DATA(insert OID =  79 (  nameregexeq	   PGNSP PGUID 12 1 0 f f t f i 2 16 "19 25" _null_ _null_ _null_ nameregexeq - _null_ ));
DESCR("matches regex., case-sensitive");
DATA(insert OID = 1252 (  nameregexne	   PGNSP PGUID 12 1 0 f f t f i 2 16 "19 25" _null_ _null_ _null_ nameregexne - _null_ ));
DESCR("does not match regex., case-sensitive");
DATA(insert OID = 1254 (  textregexeq	   PGNSP PGUID 12 1 0 f f t f i 2 16 "25 25" _null_ _null_ _null_ textregexeq - _null_ ));
DESCR("matches regex., case-sensitive");
DATA(insert OID = 1256 (  textregexne	   PGNSP PGUID 12 1 0 f f t f i 2 16 "25 25" _null_ _null_ _null_ textregexne - _null_ ));
DESCR("does not match regex., case-sensitive");
DATA(insert OID = 1257 (  textlen		   PGNSP PGUID 12 1 0 f f t f i 1 23 "25" _null_ _null_ _null_	textlen - _null_ ));
DESCR("length");
DATA(insert OID = 1258 (  textcat		   PGNSP PGUID 12 1 0 f f t f i 2 25 "25 25" _null_ _null_ _null_ textcat - _null_ ));
DESCR("concatenate");

DATA(insert OID =  84 (  boolne			   PGNSP PGUID 12 1 0 f f t f i 2 16 "16 16" _null_ _null_ _null_ boolne - _null_ ));
DESCR("not equal");
DATA(insert OID =  89 (  version		   PGNSP PGUID 12 1 0 f f t f s 0 25 "" _null_ _null_ _null_ pgsql_version - _null_ ));
DESCR("PostgreSQL version string");

/* OIDS 100 - 199 */

DATA(insert OID = 101 (  eqsel			   PGNSP PGUID 12 1 0 f f t f s 4 701 "2281 26 2281 23" _null_ _null_ _null_	eqsel - _null_ ));
DESCR("restriction selectivity of = and related operators");
DATA(insert OID = 102 (  neqsel			   PGNSP PGUID 12 1 0 f f t f s 4 701 "2281 26 2281 23" _null_ _null_ _null_	neqsel - _null_ ));
DESCR("restriction selectivity of <> and related operators");
DATA(insert OID = 103 (  scalarltsel	   PGNSP PGUID 12 1 0 f f t f s 4 701 "2281 26 2281 23" _null_ _null_ _null_	scalarltsel - _null_ ));
DESCR("restriction selectivity of < and related operators on scalar datatypes");
DATA(insert OID = 104 (  scalargtsel	   PGNSP PGUID 12 1 0 f f t f s 4 701 "2281 26 2281 23" _null_ _null_ _null_	scalargtsel - _null_ ));
DESCR("restriction selectivity of > and related operators on scalar datatypes");
DATA(insert OID = 105 (  eqjoinsel		   PGNSP PGUID 12 1 0 f f t f s 4 701 "2281 26 2281 21" _null_ _null_ _null_	eqjoinsel - _null_ ));
DESCR("join selectivity of = and related operators");
DATA(insert OID = 106 (  neqjoinsel		   PGNSP PGUID 12 1 0 f f t f s 4 701 "2281 26 2281 21" _null_ _null_ _null_	neqjoinsel - _null_ ));
DESCR("join selectivity of <> and related operators");
DATA(insert OID = 107 (  scalarltjoinsel   PGNSP PGUID 12 1 0 f f t f s 4 701 "2281 26 2281 21" _null_ _null_ _null_	scalarltjoinsel - _null_ ));
DESCR("join selectivity of < and related operators on scalar datatypes");
DATA(insert OID = 108 (  scalargtjoinsel   PGNSP PGUID 12 1 0 f f t f s 4 701 "2281 26 2281 21" _null_ _null_ _null_	scalargtjoinsel - _null_ ));
DESCR("join selectivity of > and related operators on scalar datatypes");

DATA(insert OID =  109 (  unknownin		   PGNSP PGUID 12 1 0 f f t f i 1 705 "2275" _null_ _null_ _null_ unknownin - _null_ ));
DESCR("I/O");
DATA(insert OID =  110 (  unknownout	   PGNSP PGUID 12 1 0 f f t f i 1 2275	"705" _null_ _null_ _null_	unknownout - _null_ ));
DESCR("I/O");
DATA(insert OID = 111 (  numeric_fac	   PGNSP PGUID 12 1 0 f f t f i 1 1700 "20" _null_ _null_ _null_	numeric_fac - _null_ ));

DATA(insert OID = 115 (  box_above_eq	   PGNSP PGUID 12 1 0 f f t f i 2  16 "603 603" _null_ _null_ _null_	box_above_eq - _null_ ));
DESCR("is above (allows touching)");
DATA(insert OID = 116 (  box_below_eq	   PGNSP PGUID 12 1 0 f f t f i 2  16 "603 603" _null_ _null_ _null_	box_below_eq - _null_ ));
DESCR("is below (allows touching)");

DATA(insert OID = 117 (  point_in		   PGNSP PGUID 12 1 0 f f t f i 1 600 "2275" _null_ _null_ _null_  point_in - _null_ ));
DESCR("I/O");
DATA(insert OID = 118 (  point_out		   PGNSP PGUID 12 1 0 f f t f i 1 2275 "600" _null_ _null_ _null_  point_out - _null_ ));
DESCR("I/O");
DATA(insert OID = 119 (  lseg_in		   PGNSP PGUID 12 1 0 f f t f i 1 601 "2275" _null_ _null_ _null_  lseg_in - _null_ ));
DESCR("I/O");
DATA(insert OID = 120 (  lseg_out		   PGNSP PGUID 12 1 0 f f t f i 1 2275 "601" _null_ _null_ _null_  lseg_out - _null_ ));
DESCR("I/O");
DATA(insert OID = 121 (  path_in		   PGNSP PGUID 12 1 0 f f t f i 1 602 "2275" _null_ _null_ _null_  path_in - _null_ ));
DESCR("I/O");
DATA(insert OID = 122 (  path_out		   PGNSP PGUID 12 1 0 f f t f i 1 2275 "602" _null_ _null_ _null_  path_out - _null_ ));
DESCR("I/O");
DATA(insert OID = 123 (  box_in			   PGNSP PGUID 12 1 0 f f t f i 1 603 "2275" _null_ _null_ _null_  box_in - _null_ ));
DESCR("I/O");
DATA(insert OID = 124 (  box_out		   PGNSP PGUID 12 1 0 f f t f i 1 2275 "603" _null_ _null_ _null_  box_out - _null_ ));
DESCR("I/O");
DATA(insert OID = 125 (  box_overlap	   PGNSP PGUID 12 1 0 f f t f i 2 16 "603 603" _null_ _null_ _null_ box_overlap - _null_ ));
DESCR("overlaps");
DATA(insert OID = 126 (  box_ge			   PGNSP PGUID 12 1 0 f f t f i 2 16 "603 603" _null_ _null_ _null_ box_ge - _null_ ));
DESCR("greater-than-or-equal by area");
DATA(insert OID = 127 (  box_gt			   PGNSP PGUID 12 1 0 f f t f i 2 16 "603 603" _null_ _null_ _null_ box_gt - _null_ ));
DESCR("greater-than by area");
DATA(insert OID = 128 (  box_eq			   PGNSP PGUID 12 1 0 f f t f i 2 16 "603 603" _null_ _null_ _null_ box_eq - _null_ ));
DESCR("equal by area");
DATA(insert OID = 129 (  box_lt			   PGNSP PGUID 12 1 0 f f t f i 2 16 "603 603" _null_ _null_ _null_ box_lt - _null_ ));
DESCR("less-than by area");
DATA(insert OID = 130 (  box_le			   PGNSP PGUID 12 1 0 f f t f i 2 16 "603 603" _null_ _null_ _null_ box_le - _null_ ));
DESCR("less-than-or-equal by area");
DATA(insert OID = 131 (  point_above	   PGNSP PGUID 12 1 0 f f t f i 2 16 "600 600" _null_ _null_ _null_ point_above - _null_ ));
DESCR("is above");
DATA(insert OID = 132 (  point_left		   PGNSP PGUID 12 1 0 f f t f i 2 16 "600 600" _null_ _null_ _null_ point_left - _null_ ));
DESCR("is left of");
DATA(insert OID = 133 (  point_right	   PGNSP PGUID 12 1 0 f f t f i 2 16 "600 600" _null_ _null_ _null_ point_right - _null_ ));
DESCR("is right of");
DATA(insert OID = 134 (  point_below	   PGNSP PGUID 12 1 0 f f t f i 2 16 "600 600" _null_ _null_ _null_ point_below - _null_ ));
DESCR("is below");
DATA(insert OID = 135 (  point_eq		   PGNSP PGUID 12 1 0 f f t f i 2 16 "600 600" _null_ _null_ _null_ point_eq - _null_ ));
DESCR("same as?");
DATA(insert OID = 136 (  on_pb			   PGNSP PGUID 12 1 0 f f t f i 2 16 "600 603" _null_ _null_ _null_ on_pb - _null_ ));
DESCR("point inside box?");
DATA(insert OID = 137 (  on_ppath		   PGNSP PGUID 12 1 0 f f t f i 2 16 "600 602" _null_ _null_ _null_ on_ppath - _null_ ));
DESCR("point within closed path, or point on open path");
DATA(insert OID = 138 (  box_center		   PGNSP PGUID 12 1 0 f f t f i 1 600 "603" _null_ _null_ _null_	box_center - _null_ ));
DESCR("center of");
DATA(insert OID = 139 (  areasel		   PGNSP PGUID 12 1 0 f f t f s 4 701 "2281 26 2281 23" _null_ _null_ _null_	areasel - _null_ ));
DESCR("restriction selectivity for area-comparison operators");
DATA(insert OID = 140 (  areajoinsel	   PGNSP PGUID 12 1 0 f f t f s 4 701 "2281 26 2281 21" _null_ _null_ _null_	areajoinsel - _null_ ));
DESCR("join selectivity for area-comparison operators");
DATA(insert OID = 141 (  int4mul		   PGNSP PGUID 12 1 0 f f t f i 2 23 "23 23" _null_ _null_ _null_ int4mul - _null_ ));
DESCR("multiply");
DATA(insert OID = 144 (  int4ne			   PGNSP PGUID 12 1 0 f f t f i 2 16 "23 23" _null_ _null_ _null_ int4ne - _null_ ));
DESCR("not equal");
DATA(insert OID = 145 (  int2ne			   PGNSP PGUID 12 1 0 f f t f i 2 16 "21 21" _null_ _null_ _null_ int2ne - _null_ ));
DESCR("not equal");
DATA(insert OID = 146 (  int2gt			   PGNSP PGUID 12 1 0 f f t f i 2 16 "21 21" _null_ _null_ _null_ int2gt - _null_ ));
DESCR("greater-than");
DATA(insert OID = 147 (  int4gt			   PGNSP PGUID 12 1 0 f f t f i 2 16 "23 23" _null_ _null_ _null_ int4gt - _null_ ));
DESCR("greater-than");
DATA(insert OID = 148 (  int2le			   PGNSP PGUID 12 1 0 f f t f i 2 16 "21 21" _null_ _null_ _null_ int2le - _null_ ));
DESCR("less-than-or-equal");
DATA(insert OID = 149 (  int4le			   PGNSP PGUID 12 1 0 f f t f i 2 16 "23 23" _null_ _null_ _null_ int4le - _null_ ));
DESCR("less-than-or-equal");
DATA(insert OID = 150 (  int4ge			   PGNSP PGUID 12 1 0 f f t f i 2 16 "23 23" _null_ _null_ _null_ int4ge - _null_ ));
DESCR("greater-than-or-equal");
DATA(insert OID = 151 (  int2ge			   PGNSP PGUID 12 1 0 f f t f i 2 16 "21 21" _null_ _null_ _null_ int2ge - _null_ ));
DESCR("greater-than-or-equal");
DATA(insert OID = 152 (  int2mul		   PGNSP PGUID 12 1 0 f f t f i 2 21 "21 21" _null_ _null_ _null_ int2mul - _null_ ));
DESCR("multiply");
DATA(insert OID = 153 (  int2div		   PGNSP PGUID 12 1 0 f f t f i 2 21 "21 21" _null_ _null_ _null_ int2div - _null_ ));
DESCR("divide");
DATA(insert OID = 154 (  int4div		   PGNSP PGUID 12 1 0 f f t f i 2 23 "23 23" _null_ _null_ _null_ int4div - _null_ ));
DESCR("divide");
DATA(insert OID = 155 (  int2mod		   PGNSP PGUID 12 1 0 f f t f i 2 21 "21 21" _null_ _null_ _null_ int2mod - _null_ ));
DESCR("modulus");
DATA(insert OID = 156 (  int4mod		   PGNSP PGUID 12 1 0 f f t f i 2 23 "23 23" _null_ _null_ _null_ int4mod - _null_ ));
DESCR("modulus");
DATA(insert OID = 157 (  textne			   PGNSP PGUID 12 1 0 f f t f i 2 16 "25 25" _null_ _null_ _null_ textne - _null_ ));
DESCR("not equal");
DATA(insert OID = 158 (  int24eq		   PGNSP PGUID 12 1 0 f f t f i 2 16 "21 23" _null_ _null_ _null_ int24eq - _null_ ));
DESCR("equal");
DATA(insert OID = 159 (  int42eq		   PGNSP PGUID 12 1 0 f f t f i 2 16 "23 21" _null_ _null_ _null_ int42eq - _null_ ));
DESCR("equal");
DATA(insert OID = 160 (  int24lt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "21 23" _null_ _null_ _null_ int24lt - _null_ ));
DESCR("less-than");
DATA(insert OID = 161 (  int42lt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "23 21" _null_ _null_ _null_ int42lt - _null_ ));
DESCR("less-than");
DATA(insert OID = 162 (  int24gt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "21 23" _null_ _null_ _null_ int24gt - _null_ ));
DESCR("greater-than");
DATA(insert OID = 163 (  int42gt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "23 21" _null_ _null_ _null_ int42gt - _null_ ));
DESCR("greater-than");
DATA(insert OID = 164 (  int24ne		   PGNSP PGUID 12 1 0 f f t f i 2 16 "21 23" _null_ _null_ _null_ int24ne - _null_ ));
DESCR("not equal");
DATA(insert OID = 165 (  int42ne		   PGNSP PGUID 12 1 0 f f t f i 2 16 "23 21" _null_ _null_ _null_ int42ne - _null_ ));
DESCR("not equal");
DATA(insert OID = 166 (  int24le		   PGNSP PGUID 12 1 0 f f t f i 2 16 "21 23" _null_ _null_ _null_ int24le - _null_ ));
DESCR("less-than-or-equal");
DATA(insert OID = 167 (  int42le		   PGNSP PGUID 12 1 0 f f t f i 2 16 "23 21" _null_ _null_ _null_ int42le - _null_ ));
DESCR("less-than-or-equal");
DATA(insert OID = 168 (  int24ge		   PGNSP PGUID 12 1 0 f f t f i 2 16 "21 23" _null_ _null_ _null_ int24ge - _null_ ));
DESCR("greater-than-or-equal");
DATA(insert OID = 169 (  int42ge		   PGNSP PGUID 12 1 0 f f t f i 2 16 "23 21" _null_ _null_ _null_ int42ge - _null_ ));
DESCR("greater-than-or-equal");
DATA(insert OID = 170 (  int24mul		   PGNSP PGUID 12 1 0 f f t f i 2 23 "21 23" _null_ _null_ _null_ int24mul - _null_ ));
DESCR("multiply");
DATA(insert OID = 171 (  int42mul		   PGNSP PGUID 12 1 0 f f t f i 2 23 "23 21" _null_ _null_ _null_ int42mul - _null_ ));
DESCR("multiply");
DATA(insert OID = 172 (  int24div		   PGNSP PGUID 12 1 0 f f t f i 2 23 "21 23" _null_ _null_ _null_ int24div - _null_ ));
DESCR("divide");
DATA(insert OID = 173 (  int42div		   PGNSP PGUID 12 1 0 f f t f i 2 23 "23 21" _null_ _null_ _null_ int42div - _null_ ));
DESCR("divide");
DATA(insert OID = 174 (  int24mod		   PGNSP PGUID 12 1 0 f f t f i 2 23 "21 23" _null_ _null_ _null_ int24mod - _null_ ));
DESCR("modulus");
DATA(insert OID = 175 (  int42mod		   PGNSP PGUID 12 1 0 f f t f i 2 23 "23 21" _null_ _null_ _null_ int42mod - _null_ ));
DESCR("modulus");
DATA(insert OID = 176 (  int2pl			   PGNSP PGUID 12 1 0 f f t f i 2 21 "21 21" _null_ _null_ _null_ int2pl - _null_ ));
DESCR("add");
DATA(insert OID = 177 (  int4pl			   PGNSP PGUID 12 1 0 f f t f i 2 23 "23 23" _null_ _null_ _null_ int4pl - _null_ ));
DESCR("add");
DATA(insert OID = 178 (  int24pl		   PGNSP PGUID 12 1 0 f f t f i 2 23 "21 23" _null_ _null_ _null_ int24pl - _null_ ));
DESCR("add");
DATA(insert OID = 179 (  int42pl		   PGNSP PGUID 12 1 0 f f t f i 2 23 "23 21" _null_ _null_ _null_ int42pl - _null_ ));
DESCR("add");
DATA(insert OID = 180 (  int2mi			   PGNSP PGUID 12 1 0 f f t f i 2 21 "21 21" _null_ _null_ _null_ int2mi - _null_ ));
DESCR("subtract");
DATA(insert OID = 181 (  int4mi			   PGNSP PGUID 12 1 0 f f t f i 2 23 "23 23" _null_ _null_ _null_ int4mi - _null_ ));
DESCR("subtract");
DATA(insert OID = 182 (  int24mi		   PGNSP PGUID 12 1 0 f f t f i 2 23 "21 23" _null_ _null_ _null_ int24mi - _null_ ));
DESCR("subtract");
DATA(insert OID = 183 (  int42mi		   PGNSP PGUID 12 1 0 f f t f i 2 23 "23 21" _null_ _null_ _null_ int42mi - _null_ ));
DESCR("subtract");
DATA(insert OID = 184 (  oideq			   PGNSP PGUID 12 1 0 f f t f i 2 16 "26 26" _null_ _null_ _null_ oideq - _null_ ));
DESCR("equal");
DATA(insert OID = 185 (  oidne			   PGNSP PGUID 12 1 0 f f t f i 2 16 "26 26" _null_ _null_ _null_ oidne - _null_ ));
DESCR("not equal");
DATA(insert OID = 186 (  box_same		   PGNSP PGUID 12 1 0 f f t f i 2 16 "603 603" _null_ _null_ _null_ box_same - _null_ ));
DESCR("same as?");
DATA(insert OID = 187 (  box_contain	   PGNSP PGUID 12 1 0 f f t f i 2 16 "603 603" _null_ _null_ _null_ box_contain - _null_ ));
DESCR("contains?");
DATA(insert OID = 188 (  box_left		   PGNSP PGUID 12 1 0 f f t f i 2 16 "603 603" _null_ _null_ _null_ box_left - _null_ ));
DESCR("is left of");
DATA(insert OID = 189 (  box_overleft	   PGNSP PGUID 12 1 0 f f t f i 2 16 "603 603" _null_ _null_ _null_ box_overleft - _null_ ));
DESCR("overlaps or is left of");
DATA(insert OID = 190 (  box_overright	   PGNSP PGUID 12 1 0 f f t f i 2 16 "603 603" _null_ _null_ _null_ box_overright - _null_ ));
DESCR("overlaps or is right of");
DATA(insert OID = 191 (  box_right		   PGNSP PGUID 12 1 0 f f t f i 2 16 "603 603" _null_ _null_ _null_ box_right - _null_ ));
DESCR("is right of");
DATA(insert OID = 192 (  box_contained	   PGNSP PGUID 12 1 0 f f t f i 2 16 "603 603" _null_ _null_ _null_ box_contained - _null_ ));
DESCR("is contained by?");

/* OIDS 200 - 299 */

DATA(insert OID = 200 (  float4in		   PGNSP PGUID 12 1 0 f f t f i 1 700 "2275" _null_ _null_ _null_  float4in - _null_ ));
DESCR("I/O");
DATA(insert OID = 201 (  float4out		   PGNSP PGUID 12 1 0 f f t f i 1 2275 "700" _null_ _null_ _null_  float4out - _null_ ));
DESCR("I/O");
DATA(insert OID = 202 (  float4mul		   PGNSP PGUID 12 1 0 f f t f i 2 700 "700 700" _null_ _null_ _null_	float4mul - _null_ ));
DESCR("multiply");
DATA(insert OID = 203 (  float4div		   PGNSP PGUID 12 1 0 f f t f i 2 700 "700 700" _null_ _null_ _null_	float4div - _null_ ));
DESCR("divide");
DATA(insert OID = 204 (  float4pl		   PGNSP PGUID 12 1 0 f f t f i 2 700 "700 700" _null_ _null_ _null_	float4pl - _null_ ));
DESCR("add");
DATA(insert OID = 205 (  float4mi		   PGNSP PGUID 12 1 0 f f t f i 2 700 "700 700" _null_ _null_ _null_	float4mi - _null_ ));
DESCR("subtract");
DATA(insert OID = 206 (  float4um		   PGNSP PGUID 12 1 0 f f t f i 1 700 "700" _null_ _null_ _null_	float4um - _null_ ));
DESCR("negate");
DATA(insert OID = 207 (  float4abs		   PGNSP PGUID 12 1 0 f f t f i 1 700 "700" _null_ _null_ _null_	float4abs - _null_ ));
DESCR("absolute value");
DATA(insert OID = 208 (  float4_accum	   PGNSP PGUID 12 1 0 f f t f i 2 1022 "1022 700" _null_ _null_ _null_	float4_accum - _null_ ));
DESCR("aggregate transition function");
DATA(insert OID = 209 (  float4larger	   PGNSP PGUID 12 1 0 f f t f i 2 700 "700 700" _null_ _null_ _null_	float4larger - _null_ ));
DESCR("larger of two");
DATA(insert OID = 211 (  float4smaller	   PGNSP PGUID 12 1 0 f f t f i 2 700 "700 700" _null_ _null_ _null_	float4smaller - _null_ ));
DESCR("smaller of two");

DATA(insert OID = 212 (  int4um			   PGNSP PGUID 12 1 0 f f t f i 1 23 "23" _null_ _null_ _null_	int4um - _null_ ));
DESCR("negate");
DATA(insert OID = 213 (  int2um			   PGNSP PGUID 12 1 0 f f t f i 1 21 "21" _null_ _null_ _null_	int2um - _null_ ));
DESCR("negate");

DATA(insert OID = 214 (  float8in		   PGNSP PGUID 12 1 0 f f t f i 1 701 "2275" _null_ _null_ _null_  float8in - _null_ ));
DESCR("I/O");
DATA(insert OID = 215 (  float8out		   PGNSP PGUID 12 1 0 f f t f i 1 2275 "701" _null_ _null_ _null_  float8out - _null_ ));
DESCR("I/O");
DATA(insert OID = 216 (  float8mul		   PGNSP PGUID 12 1 0 f f t f i 2 701 "701 701" _null_ _null_ _null_	float8mul - _null_ ));
DESCR("multiply");
DATA(insert OID = 217 (  float8div		   PGNSP PGUID 12 1 0 f f t f i 2 701 "701 701" _null_ _null_ _null_	float8div - _null_ ));
DESCR("divide");
DATA(insert OID = 218 (  float8pl		   PGNSP PGUID 12 1 0 f f t f i 2 701 "701 701" _null_ _null_ _null_	float8pl - _null_ ));
DESCR("add");
DATA(insert OID = 219 (  float8mi		   PGNSP PGUID 12 1 0 f f t f i 2 701 "701 701" _null_ _null_ _null_	float8mi - _null_ ));
DESCR("subtract");
DATA(insert OID = 220 (  float8um		   PGNSP PGUID 12 1 0 f f t f i 1 701 "701" _null_ _null_ _null_	float8um - _null_ ));
DESCR("negate");
DATA(insert OID = 221 (  float8abs		   PGNSP PGUID 12 1 0 f f t f i 1 701 "701" _null_ _null_ _null_	float8abs - _null_ ));
DESCR("absolute value");
DATA(insert OID = 222 (  float8_accum	   PGNSP PGUID 12 1 0 f f t f i 2 1022 "1022 701" _null_ _null_ _null_	float8_accum - _null_ ));
DESCR("aggregate transition function");
DATA(insert OID = 223 (  float8larger	   PGNSP PGUID 12 1 0 f f t f i 2 701 "701 701" _null_ _null_ _null_	float8larger - _null_ ));
DESCR("larger of two");
DATA(insert OID = 224 (  float8smaller	   PGNSP PGUID 12 1 0 f f t f i 2 701 "701 701" _null_ _null_ _null_	float8smaller - _null_ ));
DESCR("smaller of two");

DATA(insert OID = 225 (  lseg_center	   PGNSP PGUID 12 1 0 f f t f i 1 600 "601" _null_ _null_ _null_	lseg_center - _null_ ));
DESCR("center of");
DATA(insert OID = 226 (  path_center	   PGNSP PGUID 12 1 0 f f t f i 1 600 "602" _null_ _null_ _null_	path_center - _null_ ));
DESCR("center of");
DATA(insert OID = 227 (  poly_center	   PGNSP PGUID 12 1 0 f f t f i 1 600 "604" _null_ _null_ _null_	poly_center - _null_ ));
DESCR("center of");

DATA(insert OID = 228 (  dround			   PGNSP PGUID 12 1 0 f f t f i 1 701 "701" _null_ _null_ _null_	dround - _null_ ));
DESCR("round to nearest integer");
DATA(insert OID = 229 (  dtrunc			   PGNSP PGUID 12 1 0 f f t f i 1 701 "701" _null_ _null_ _null_	dtrunc - _null_ ));
DESCR("truncate to integer");
DATA(insert OID = 2308 ( ceil			   PGNSP PGUID 12 1 0 f f t f i 1 701 "701" _null_ _null_ _null_	dceil - _null_ ));
DESCR("smallest integer >= value");
DATA(insert OID = 2320 ( ceiling		   PGNSP PGUID 12 1 0 f f t f i 1 701 "701" _null_ _null_ _null_	dceil - _null_ ));
DESCR("smallest integer >= value");
DATA(insert OID = 2309 ( floor			   PGNSP PGUID 12 1 0 f f t f i 1 701 "701" _null_ _null_ _null_	dfloor - _null_ ));
DESCR("largest integer <= value");
DATA(insert OID = 2310 ( sign			   PGNSP PGUID 12 1 0 f f t f i 1 701 "701" _null_ _null_ _null_	dsign - _null_ ));
DESCR("sign of value");
DATA(insert OID = 230 (  dsqrt			   PGNSP PGUID 12 1 0 f f t f i 1 701 "701" _null_ _null_ _null_	dsqrt - _null_ ));
DESCR("square root");
DATA(insert OID = 231 (  dcbrt			   PGNSP PGUID 12 1 0 f f t f i 1 701 "701" _null_ _null_ _null_	dcbrt - _null_ ));
DESCR("cube root");
DATA(insert OID = 232 (  dpow			   PGNSP PGUID 12 1 0 f f t f i 2 701 "701 701" _null_ _null_ _null_	dpow - _null_ ));
DESCR("exponentiation (x^y)");
DATA(insert OID = 233 (  dexp			   PGNSP PGUID 12 1 0 f f t f i 1 701 "701" _null_ _null_ _null_	dexp - _null_ ));
DESCR("natural exponential (e^x)");
DATA(insert OID = 234 (  dlog1			   PGNSP PGUID 12 1 0 f f t f i 1 701 "701" _null_ _null_ _null_	dlog1 - _null_ ));
DESCR("natural logarithm");
DATA(insert OID = 235 (  float8			   PGNSP PGUID 12 1 0 f f t f i 1 701  "21" _null_ _null_ _null_	i2tod - _null_ ));
DESCR("convert int2 to float8");
DATA(insert OID = 236 (  float4			   PGNSP PGUID 12 1 0 f f t f i 1 700  "21" _null_ _null_ _null_	i2tof - _null_ ));
DESCR("convert int2 to float4");
DATA(insert OID = 237 (  int2			   PGNSP PGUID 12 1 0 f f t f i 1  21 "701" _null_ _null_ _null_	dtoi2 - _null_ ));
DESCR("convert float8 to int2");
DATA(insert OID = 238 (  int2			   PGNSP PGUID 12 1 0 f f t f i 1  21 "700" _null_ _null_ _null_	ftoi2 - _null_ ));
DESCR("convert float4 to int2");
DATA(insert OID = 239 (  line_distance	   PGNSP PGUID 12 1 0 f f t f i 2 701 "628 628" _null_ _null_ _null_	line_distance - _null_ ));
DESCR("distance between");

DATA(insert OID = 240 (  abstimein		   PGNSP PGUID 12 1 0 f f t f s 1 702 "2275" _null_ _null_ _null_  abstimein - _null_ ));
DESCR("I/O");
DATA(insert OID = 241 (  abstimeout		   PGNSP PGUID 12 1 0 f f t f s 1 2275 "702" _null_ _null_ _null_  abstimeout - _null_ ));
DESCR("I/O");
DATA(insert OID = 242 (  reltimein		   PGNSP PGUID 12 1 0 f f t f s 1 703 "2275" _null_ _null_ _null_  reltimein - _null_ ));
DESCR("I/O");
DATA(insert OID = 243 (  reltimeout		   PGNSP PGUID 12 1 0 f f t f s 1 2275 "703" _null_ _null_ _null_  reltimeout - _null_ ));
DESCR("I/O");
DATA(insert OID = 244 (  timepl			   PGNSP PGUID 12 1 0 f f t f i 2 702 "702 703" _null_ _null_ _null_	timepl - _null_ ));
DESCR("add");
DATA(insert OID = 245 (  timemi			   PGNSP PGUID 12 1 0 f f t f i 2 702 "702 703" _null_ _null_ _null_	timemi - _null_ ));
DESCR("subtract");
DATA(insert OID = 246 (  tintervalin	   PGNSP PGUID 12 1 0 f f t f s 1 704 "2275" _null_ _null_ _null_  tintervalin - _null_ ));
DESCR("I/O");
DATA(insert OID = 247 (  tintervalout	   PGNSP PGUID 12 1 0 f f t f s 1 2275 "704" _null_ _null_ _null_  tintervalout - _null_ ));
DESCR("I/O");
DATA(insert OID = 248 (  intinterval	   PGNSP PGUID 12 1 0 f f t f i 2 16 "702 704" _null_ _null_ _null_ intinterval - _null_ ));
DESCR("abstime in tinterval");
DATA(insert OID = 249 (  tintervalrel	   PGNSP PGUID 12 1 0 f f t f i 1 703 "704" _null_ _null_ _null_	tintervalrel - _null_ ));
DESCR("tinterval to reltime");
DATA(insert OID = 250 (  timenow		   PGNSP PGUID 12 1 0 f f t f s 0 702 "" _null_ _null_ _null_  timenow - _null_ ));
DESCR("current date and time (abstime)");
DATA(insert OID = 251 (  abstimeeq		   PGNSP PGUID 12 1 0 f f t f i 2 16 "702 702" _null_ _null_ _null_ abstimeeq - _null_ ));
DESCR("equal");
DATA(insert OID = 252 (  abstimene		   PGNSP PGUID 12 1 0 f f t f i 2 16 "702 702" _null_ _null_ _null_ abstimene - _null_ ));
DESCR("not equal");
DATA(insert OID = 253 (  abstimelt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "702 702" _null_ _null_ _null_ abstimelt - _null_ ));
DESCR("less-than");
DATA(insert OID = 254 (  abstimegt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "702 702" _null_ _null_ _null_ abstimegt - _null_ ));
DESCR("greater-than");
DATA(insert OID = 255 (  abstimele		   PGNSP PGUID 12 1 0 f f t f i 2 16 "702 702" _null_ _null_ _null_ abstimele - _null_ ));
DESCR("less-than-or-equal");
DATA(insert OID = 256 (  abstimege		   PGNSP PGUID 12 1 0 f f t f i 2 16 "702 702" _null_ _null_ _null_ abstimege - _null_ ));
DESCR("greater-than-or-equal");
DATA(insert OID = 257 (  reltimeeq		   PGNSP PGUID 12 1 0 f f t f i 2 16 "703 703" _null_ _null_ _null_ reltimeeq - _null_ ));
DESCR("equal");
DATA(insert OID = 258 (  reltimene		   PGNSP PGUID 12 1 0 f f t f i 2 16 "703 703" _null_ _null_ _null_ reltimene - _null_ ));
DESCR("not equal");
DATA(insert OID = 259 (  reltimelt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "703 703" _null_ _null_ _null_ reltimelt - _null_ ));
DESCR("less-than");
DATA(insert OID = 260 (  reltimegt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "703 703" _null_ _null_ _null_ reltimegt - _null_ ));
DESCR("greater-than");
DATA(insert OID = 261 (  reltimele		   PGNSP PGUID 12 1 0 f f t f i 2 16 "703 703" _null_ _null_ _null_ reltimele - _null_ ));
DESCR("less-than-or-equal");
DATA(insert OID = 262 (  reltimege		   PGNSP PGUID 12 1 0 f f t f i 2 16 "703 703" _null_ _null_ _null_ reltimege - _null_ ));
DESCR("greater-than-or-equal");
DATA(insert OID = 263 (  tintervalsame	   PGNSP PGUID 12 1 0 f f t f i 2 16 "704 704" _null_ _null_ _null_ tintervalsame - _null_ ));
DESCR("same as?");
DATA(insert OID = 264 (  tintervalct	   PGNSP PGUID 12 1 0 f f t f i 2 16 "704 704" _null_ _null_ _null_ tintervalct - _null_ ));
DESCR("contains?");
DATA(insert OID = 265 (  tintervalov	   PGNSP PGUID 12 1 0 f f t f i 2 16 "704 704" _null_ _null_ _null_ tintervalov - _null_ ));
DESCR("overlaps");
DATA(insert OID = 266 (  tintervalleneq    PGNSP PGUID 12 1 0 f f t f i 2 16 "704 703" _null_ _null_ _null_ tintervalleneq - _null_ ));
DESCR("length equal");
DATA(insert OID = 267 (  tintervallenne    PGNSP PGUID 12 1 0 f f t f i 2 16 "704 703" _null_ _null_ _null_ tintervallenne - _null_ ));
DESCR("length not equal to");
DATA(insert OID = 268 (  tintervallenlt    PGNSP PGUID 12 1 0 f f t f i 2 16 "704 703" _null_ _null_ _null_ tintervallenlt - _null_ ));
DESCR("length less-than");
DATA(insert OID = 269 (  tintervallengt    PGNSP PGUID 12 1 0 f f t f i 2 16 "704 703" _null_ _null_ _null_ tintervallengt - _null_ ));
DESCR("length greater-than");
DATA(insert OID = 270 (  tintervallenle    PGNSP PGUID 12 1 0 f f t f i 2 16 "704 703" _null_ _null_ _null_ tintervallenle - _null_ ));
DESCR("length less-than-or-equal");
DATA(insert OID = 271 (  tintervallenge    PGNSP PGUID 12 1 0 f f t f i 2 16 "704 703" _null_ _null_ _null_ tintervallenge - _null_ ));
DESCR("length greater-than-or-equal");
DATA(insert OID = 272 (  tintervalstart    PGNSP PGUID 12 1 0 f f t f i 1 702 "704" _null_ _null_ _null_	tintervalstart - _null_ ));
DESCR("start of interval");
DATA(insert OID = 273 (  tintervalend	   PGNSP PGUID 12 1 0 f f t f i 1 702 "704" _null_ _null_ _null_	tintervalend - _null_ ));
DESCR("end of interval");
DATA(insert OID = 274 (  timeofday		   PGNSP PGUID 12 1 0 f f t f v 0 25 "" _null_ _null_ _null_ timeofday - _null_ ));
DESCR("current date and time - increments during transactions");
DATA(insert OID = 275 (  isfinite		   PGNSP PGUID 12 1 0 f f t f i 1 16 "702" _null_ _null_ _null_ abstime_finite - _null_ ));
DESCR("finite abstime?");

DATA(insert OID = 277 (  inter_sl		   PGNSP PGUID 12 1 0 f f t f i 2 16 "601 628" _null_ _null_ _null_ inter_sl - _null_ ));
DESCR("intersect?");
DATA(insert OID = 278 (  inter_lb		   PGNSP PGUID 12 1 0 f f t f i 2 16 "628 603" _null_ _null_ _null_ inter_lb - _null_ ));
DESCR("intersect?");

DATA(insert OID = 279 (  float48mul		   PGNSP PGUID 12 1 0 f f t f i 2 701 "700 701" _null_ _null_ _null_	float48mul - _null_ ));
DESCR("multiply");
DATA(insert OID = 280 (  float48div		   PGNSP PGUID 12 1 0 f f t f i 2 701 "700 701" _null_ _null_ _null_	float48div - _null_ ));
DESCR("divide");
DATA(insert OID = 281 (  float48pl		   PGNSP PGUID 12 1 0 f f t f i 2 701 "700 701" _null_ _null_ _null_	float48pl - _null_ ));
DESCR("add");
DATA(insert OID = 282 (  float48mi		   PGNSP PGUID 12 1 0 f f t f i 2 701 "700 701" _null_ _null_ _null_	float48mi - _null_ ));
DESCR("subtract");
DATA(insert OID = 283 (  float84mul		   PGNSP PGUID 12 1 0 f f t f i 2 701 "701 700" _null_ _null_ _null_	float84mul - _null_ ));
DESCR("multiply");
DATA(insert OID = 284 (  float84div		   PGNSP PGUID 12 1 0 f f t f i 2 701 "701 700" _null_ _null_ _null_	float84div - _null_ ));
DESCR("divide");
DATA(insert OID = 285 (  float84pl		   PGNSP PGUID 12 1 0 f f t f i 2 701 "701 700" _null_ _null_ _null_	float84pl - _null_ ));
DESCR("add");
DATA(insert OID = 286 (  float84mi		   PGNSP PGUID 12 1 0 f f t f i 2 701 "701 700" _null_ _null_ _null_	float84mi - _null_ ));
DESCR("subtract");

DATA(insert OID = 287 (  float4eq		   PGNSP PGUID 12 1 0 f f t f i 2 16 "700 700" _null_ _null_ _null_ float4eq - _null_ ));
DESCR("equal");
DATA(insert OID = 288 (  float4ne		   PGNSP PGUID 12 1 0 f f t f i 2 16 "700 700" _null_ _null_ _null_ float4ne - _null_ ));
DESCR("not equal");
DATA(insert OID = 289 (  float4lt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "700 700" _null_ _null_ _null_ float4lt - _null_ ));
DESCR("less-than");
DATA(insert OID = 290 (  float4le		   PGNSP PGUID 12 1 0 f f t f i 2 16 "700 700" _null_ _null_ _null_ float4le - _null_ ));
DESCR("less-than-or-equal");
DATA(insert OID = 291 (  float4gt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "700 700" _null_ _null_ _null_ float4gt - _null_ ));
DESCR("greater-than");
DATA(insert OID = 292 (  float4ge		   PGNSP PGUID 12 1 0 f f t f i 2 16 "700 700" _null_ _null_ _null_ float4ge - _null_ ));
DESCR("greater-than-or-equal");

DATA(insert OID = 293 (  float8eq		   PGNSP PGUID 12 1 0 f f t f i 2 16 "701 701" _null_ _null_ _null_ float8eq - _null_ ));
DESCR("equal");
DATA(insert OID = 294 (  float8ne		   PGNSP PGUID 12 1 0 f f t f i 2 16 "701 701" _null_ _null_ _null_ float8ne - _null_ ));
DESCR("not equal");
DATA(insert OID = 295 (  float8lt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "701 701" _null_ _null_ _null_ float8lt - _null_ ));
DESCR("less-than");
DATA(insert OID = 296 (  float8le		   PGNSP PGUID 12 1 0 f f t f i 2 16 "701 701" _null_ _null_ _null_ float8le - _null_ ));
DESCR("less-than-or-equal");
DATA(insert OID = 297 (  float8gt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "701 701" _null_ _null_ _null_ float8gt - _null_ ));
DESCR("greater-than");
DATA(insert OID = 298 (  float8ge		   PGNSP PGUID 12 1 0 f f t f i 2 16 "701 701" _null_ _null_ _null_ float8ge - _null_ ));
DESCR("greater-than-or-equal");

DATA(insert OID = 299 (  float48eq		   PGNSP PGUID 12 1 0 f f t f i 2 16 "700 701" _null_ _null_ _null_ float48eq - _null_ ));
DESCR("equal");

/* OIDS 300 - 399 */

DATA(insert OID = 300 (  float48ne		   PGNSP PGUID 12 1 0 f f t f i 2 16 "700 701" _null_ _null_ _null_ float48ne - _null_ ));
DESCR("not equal");
DATA(insert OID = 301 (  float48lt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "700 701" _null_ _null_ _null_ float48lt - _null_ ));
DESCR("less-than");
DATA(insert OID = 302 (  float48le		   PGNSP PGUID 12 1 0 f f t f i 2 16 "700 701" _null_ _null_ _null_ float48le - _null_ ));
DESCR("less-than-or-equal");
DATA(insert OID = 303 (  float48gt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "700 701" _null_ _null_ _null_ float48gt - _null_ ));
DESCR("greater-than");
DATA(insert OID = 304 (  float48ge		   PGNSP PGUID 12 1 0 f f t f i 2 16 "700 701" _null_ _null_ _null_ float48ge - _null_ ));
DESCR("greater-than-or-equal");
DATA(insert OID = 305 (  float84eq		   PGNSP PGUID 12 1 0 f f t f i 2 16 "701 700" _null_ _null_ _null_ float84eq - _null_ ));
DESCR("equal");
DATA(insert OID = 306 (  float84ne		   PGNSP PGUID 12 1 0 f f t f i 2 16 "701 700" _null_ _null_ _null_ float84ne - _null_ ));
DESCR("not equal");
DATA(insert OID = 307 (  float84lt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "701 700" _null_ _null_ _null_ float84lt - _null_ ));
DESCR("less-than");
DATA(insert OID = 308 (  float84le		   PGNSP PGUID 12 1 0 f f t f i 2 16 "701 700" _null_ _null_ _null_ float84le - _null_ ));
DESCR("less-than-or-equal");
DATA(insert OID = 309 (  float84gt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "701 700" _null_ _null_ _null_ float84gt - _null_ ));
DESCR("greater-than");
DATA(insert OID = 310 (  float84ge		   PGNSP PGUID 12 1 0 f f t f i 2 16 "701 700" _null_ _null_ _null_ float84ge - _null_ ));
DESCR("greater-than-or-equal");
DATA(insert OID = 320 ( width_bucket	   PGNSP PGUID 12 1 0 f f t f i 4 23 "701 701 701 23" _null_ _null_ _null_	width_bucket_float8 - _null_ ));
DESCR("bucket number of operand in equidepth histogram");

DATA(insert OID = 311 (  float8			   PGNSP PGUID 12 1 0 f f t f i 1 701 "700" _null_ _null_ _null_	ftod - _null_ ));
DESCR("convert float4 to float8");
DATA(insert OID = 312 (  float4			   PGNSP PGUID 12 1 0 f f t f i 1 700 "701" _null_ _null_ _null_	dtof - _null_ ));
DESCR("convert float8 to float4");
DATA(insert OID = 313 (  int4			   PGNSP PGUID 12 1 0 f f t f i 1  23  "21" _null_ _null_ _null_	i2toi4 - _null_ ));
DESCR("convert int2 to int4");
DATA(insert OID = 314 (  int2			   PGNSP PGUID 12 1 0 f f t f i 1  21  "23" _null_ _null_ _null_	i4toi2 - _null_ ));
DESCR("convert int4 to int2");
DATA(insert OID = 315 (  int2vectoreq	   PGNSP PGUID 12 1 0 f f t f i 2  16  "22 22" _null_ _null_ _null_ int2vectoreq - _null_ ));
DESCR("equal");
DATA(insert OID = 316 (  float8			   PGNSP PGUID 12 1 0 f f t f i 1 701  "23" _null_ _null_ _null_	i4tod - _null_ ));
DESCR("convert int4 to float8");
DATA(insert OID = 317 (  int4			   PGNSP PGUID 12 1 0 f f t f i 1  23 "701" _null_ _null_ _null_	dtoi4 - _null_ ));
DESCR("convert float8 to int4");
DATA(insert OID = 318 (  float4			   PGNSP PGUID 12 1 0 f f t f i 1 700  "23" _null_ _null_ _null_	i4tof - _null_ ));
DESCR("convert int4 to float4");
DATA(insert OID = 319 (  int4			   PGNSP PGUID 12 1 0 f f t f i 1  23 "700" _null_ _null_ _null_	ftoi4 - _null_ ));
DESCR("convert float4 to int4");

DATA(insert OID = 330 (  btgettuple		   PGNSP PGUID 12 1 0 f f t f v 2 16 "2281 2281" _null_ _null_ _null_  btgettuple - _null_ ));
DESCR("btree(internal)");
DATA(insert OID = 636 (  btgetmulti		   PGNSP PGUID 12 1 0 f f t f v 4 16 "2281 2281 2281 2281" _null_ _null_ _null_  btgetmulti - _null_ ));
DESCR("btree(internal)");
DATA(insert OID = 331 (  btinsert		   PGNSP PGUID 12 1 0 f f t f v 6 16 "2281 2281 2281 2281 2281 2281" _null_ _null_ _null_	btinsert - _null_ ));
DESCR("btree(internal)");
DATA(insert OID = 333 (  btbeginscan	   PGNSP PGUID 12 1 0 f f t f v 3 2281 "2281 2281 2281" _null_ _null_ _null_	btbeginscan - _null_ ));
DESCR("btree(internal)");
DATA(insert OID = 334 (  btrescan		   PGNSP PGUID 12 1 0 f f t f v 2 2278 "2281 2281" _null_ _null_ _null_ btrescan - _null_ ));
DESCR("btree(internal)");
DATA(insert OID = 335 (  btendscan		   PGNSP PGUID 12 1 0 f f t f v 1 2278 "2281" _null_ _null_ _null_	btendscan - _null_ ));
DESCR("btree(internal)");
DATA(insert OID = 336 (  btmarkpos		   PGNSP PGUID 12 1 0 f f t f v 1 2278 "2281" _null_ _null_ _null_	btmarkpos - _null_ ));
DESCR("btree(internal)");
DATA(insert OID = 337 (  btrestrpos		   PGNSP PGUID 12 1 0 f f t f v 1 2278 "2281" _null_ _null_ _null_	btrestrpos - _null_ ));
DESCR("btree(internal)");
DATA(insert OID = 338 (  btbuild		   PGNSP PGUID 12 1 0 f f t f v 3 2281 "2281 2281 2281" _null_ _null_ _null_ btbuild - _null_ ));
DESCR("btree(internal)");
DATA(insert OID = 332 (  btbulkdelete	   PGNSP PGUID 12 1 0 f f t f v 4 2281 "2281 2281 2281 2281" _null_ _null_ _null_ btbulkdelete - _null_ ));
DESCR("btree(internal)");
DATA(insert OID = 972 (  btvacuumcleanup   PGNSP PGUID 12 1 0 f f t f v 2 2281 "2281 2281" _null_ _null_ _null_ btvacuumcleanup - _null_ ));
DESCR("btree(internal)");
DATA(insert OID = 1268 (  btcostestimate   PGNSP PGUID 12 1 0 f f t f v 8 2278 "2281 2281 2281 2281 2281 2281 2281 2281" _null_ _null_ _null_  btcostestimate - _null_ ));
DESCR("btree(internal)");
DATA(insert OID = 2785 (  btoptions		   PGNSP PGUID 12 1 0 f f t f s 2 17 "1009 16" _null_ _null_ _null_  btoptions - _null_ ));
DESCR("btree(internal)");

DATA(insert OID = 339 (  poly_same		   PGNSP PGUID 12 1 0 f f t f i 2 16 "604 604" _null_ _null_ _null_ poly_same - _null_ ));
DESCR("same as?");
DATA(insert OID = 340 (  poly_contain	   PGNSP PGUID 12 1 0 f f t f i 2 16 "604 604" _null_ _null_ _null_ poly_contain - _null_ ));
DESCR("contains?");
DATA(insert OID = 341 (  poly_left		   PGNSP PGUID 12 1 0 f f t f i 2 16 "604 604" _null_ _null_ _null_ poly_left - _null_ ));
DESCR("is left of");
DATA(insert OID = 342 (  poly_overleft	   PGNSP PGUID 12 1 0 f f t f i 2 16 "604 604" _null_ _null_ _null_ poly_overleft - _null_ ));
DESCR("overlaps or is left of");
DATA(insert OID = 343 (  poly_overright    PGNSP PGUID 12 1 0 f f t f i 2 16 "604 604" _null_ _null_ _null_ poly_overright - _null_ ));
DESCR("overlaps or is right of");
DATA(insert OID = 344 (  poly_right		   PGNSP PGUID 12 1 0 f f t f i 2 16 "604 604" _null_ _null_ _null_ poly_right - _null_ ));
DESCR("is right of");
DATA(insert OID = 345 (  poly_contained    PGNSP PGUID 12 1 0 f f t f i 2 16 "604 604" _null_ _null_ _null_ poly_contained - _null_ ));
DESCR("is contained by?");
DATA(insert OID = 346 (  poly_overlap	   PGNSP PGUID 12 1 0 f f t f i 2 16 "604 604" _null_ _null_ _null_ poly_overlap - _null_ ));
DESCR("overlaps");
DATA(insert OID = 347 (  poly_in		   PGNSP PGUID 12 1 0 f f t f i 1 604 "2275" _null_ _null_ _null_  poly_in - _null_ ));
DESCR("I/O");
DATA(insert OID = 348 (  poly_out		   PGNSP PGUID 12 1 0 f f t f i 1 2275 "604" _null_ _null_ _null_  poly_out - _null_ ));
DESCR("I/O");

DATA(insert OID = 350 (  btint2cmp		   PGNSP PGUID 12 1 0 f f t f i 2 23 "21 21" _null_ _null_ _null_ btint2cmp - _null_ ));
DESCR("btree less-equal-greater");
DATA(insert OID = 351 (  btint4cmp		   PGNSP PGUID 12 1 0 f f t f i 2 23 "23 23" _null_ _null_ _null_ btint4cmp - _null_ ));
DESCR("btree less-equal-greater");
DATA(insert OID = 842 (  btint8cmp		   PGNSP PGUID 12 1 0 f f t f i 2 23 "20 20" _null_ _null_ _null_ btint8cmp - _null_ ));
DESCR("btree less-equal-greater");
DATA(insert OID = 354 (  btfloat4cmp	   PGNSP PGUID 12 1 0 f f t f i 2 23 "700 700" _null_ _null_ _null_ btfloat4cmp - _null_ ));
DESCR("btree less-equal-greater");
DATA(insert OID = 355 (  btfloat8cmp	   PGNSP PGUID 12 1 0 f f t f i 2 23 "701 701" _null_ _null_ _null_ btfloat8cmp - _null_ ));
DESCR("btree less-equal-greater");
DATA(insert OID = 356 (  btoidcmp		   PGNSP PGUID 12 1 0 f f t f i 2 23 "26 26" _null_ _null_ _null_ btoidcmp - _null_ ));
DESCR("btree less-equal-greater");
DATA(insert OID = 404 (  btoidvectorcmp    PGNSP PGUID 12 1 0 f f t f i 2 23 "30 30" _null_ _null_ _null_ btoidvectorcmp - _null_ ));
DESCR("btree less-equal-greater");
DATA(insert OID = 357 (  btabstimecmp	   PGNSP PGUID 12 1 0 f f t f i 2 23 "702 702" _null_ _null_ _null_ btabstimecmp - _null_ ));
DESCR("btree less-equal-greater");
DATA(insert OID = 358 (  btcharcmp		   PGNSP PGUID 12 1 0 f f t f i 2 23 "18 18" _null_ _null_ _null_ btcharcmp - _null_ ));
DESCR("btree less-equal-greater");
DATA(insert OID = 359 (  btnamecmp		   PGNSP PGUID 12 1 0 f f t f i 2 23 "19 19" _null_ _null_ _null_ btnamecmp - _null_ ));
DESCR("btree less-equal-greater");
DATA(insert OID = 360 (  bttextcmp		   PGNSP PGUID 12 1 0 f f t f i 2 23 "25 25" _null_ _null_ _null_ bttextcmp - _null_ ));
DESCR("btree less-equal-greater");
DATA(insert OID = 377 (  cash_cmp		   PGNSP PGUID 12 1 0 f f t f i 2 23 "790 790" _null_ _null_ _null_ cash_cmp - _null_ ));
DESCR("btree less-equal-greater");
DATA(insert OID = 380 (  btreltimecmp	   PGNSP PGUID 12 1 0 f f t f i 2 23 "703 703" _null_ _null_ _null_ btreltimecmp - _null_ ));
DESCR("btree less-equal-greater");
DATA(insert OID = 381 (  bttintervalcmp    PGNSP PGUID 12 1 0 f f t f i 2 23 "704 704" _null_ _null_ _null_ bttintervalcmp - _null_ ));
DESCR("btree less-equal-greater");
DATA(insert OID = 382 (  btarraycmp		   PGNSP PGUID 12 1 0 f f t f i 2 23 "2277 2277" _null_ _null_ _null_ btarraycmp - _null_ ));
DESCR("btree less-equal-greater");

DATA(insert OID = 361 (  lseg_distance	   PGNSP PGUID 12 1 0 f f t f i 2 701 "601 601" _null_ _null_ _null_	lseg_distance - _null_ ));
DESCR("distance between");
DATA(insert OID = 362 (  lseg_interpt	   PGNSP PGUID 12 1 0 f f t f i 2 600 "601 601" _null_ _null_ _null_	lseg_interpt - _null_ ));
DESCR("intersection point");
DATA(insert OID = 363 (  dist_ps		   PGNSP PGUID 12 1 0 f f t f i 2 701 "600 601" _null_ _null_ _null_	dist_ps - _null_ ));
DESCR("distance between");
DATA(insert OID = 364 (  dist_pb		   PGNSP PGUID 12 1 0 f f t f i 2 701 "600 603" _null_ _null_ _null_	dist_pb - _null_ ));
DESCR("distance between point and box");
DATA(insert OID = 365 (  dist_sb		   PGNSP PGUID 12 1 0 f f t f i 2 701 "601 603" _null_ _null_ _null_	dist_sb - _null_ ));
DESCR("distance between segment and box");
DATA(insert OID = 366 (  close_ps		   PGNSP PGUID 12 1 0 f f t f i 2 600 "600 601" _null_ _null_ _null_	close_ps - _null_ ));
DESCR("closest point on line segment");
DATA(insert OID = 367 (  close_pb		   PGNSP PGUID 12 1 0 f f t f i 2 600 "600 603" _null_ _null_ _null_	close_pb - _null_ ));
DESCR("closest point on box");
DATA(insert OID = 368 (  close_sb		   PGNSP PGUID 12 1 0 f f t f i 2 600 "601 603" _null_ _null_ _null_	close_sb - _null_ ));
DESCR("closest point to line segment on box");
DATA(insert OID = 369 (  on_ps			   PGNSP PGUID 12 1 0 f f t f i 2 16 "600 601" _null_ _null_ _null_ on_ps - _null_ ));
DESCR("point contained in segment?");
DATA(insert OID = 370 (  path_distance	   PGNSP PGUID 12 1 0 f f t f i 2 701 "602 602" _null_ _null_ _null_	path_distance - _null_ ));
DESCR("distance between paths");
DATA(insert OID = 371 (  dist_ppath		   PGNSP PGUID 12 1 0 f f t f i 2 701 "600 602" _null_ _null_ _null_	dist_ppath - _null_ ));
DESCR("distance between point and path");
DATA(insert OID = 372 (  on_sb			   PGNSP PGUID 12 1 0 f f t f i 2 16 "601 603" _null_ _null_ _null_ on_sb - _null_ ));
DESCR("lseg contained in box?");
DATA(insert OID = 373 (  inter_sb		   PGNSP PGUID 12 1 0 f f t f i 2 16 "601 603" _null_ _null_ _null_ inter_sb - _null_ ));
DESCR("intersect?");

/* OIDS 400 - 499 */

DATA(insert OID =  401 (  text			   PGNSP PGUID 12 1 0 f f t f i 1 25 "1042" _null_ _null_ _null_	rtrim1 - _null_ ));
DESCR("convert char(n) to text");
DATA(insert OID =  406 (  text			   PGNSP PGUID 12 1 0 f f t f i 1 25 "19" _null_ _null_ _null_ name_text - _null_ ));
DESCR("convert name to text");
DATA(insert OID =  407 (  name			   PGNSP PGUID 12 1 0 f f t f i 1 19 "25" _null_ _null_ _null_ text_name - _null_ ));
DESCR("convert text to name");
DATA(insert OID =  408 (  bpchar		   PGNSP PGUID 12 1 0 f f t f i 1 1042 "19" _null_ _null_ _null_ name_bpchar - _null_ ));
DESCR("convert name to char(n)");
DATA(insert OID =  409 (  name			   PGNSP PGUID 12 1 0 f f t f i 1 19 "1042" _null_ _null_ _null_	bpchar_name - _null_ ));
DESCR("convert char(n) to name");

DATA(insert OID = 440 (  hashgettuple	   PGNSP PGUID 12 1 0 f f t f v 2 16 "2281 2281" _null_ _null_ _null_  hashgettuple - _null_ ));
DESCR("hash(internal)");
DATA(insert OID = 637 (  hashgetmulti	   PGNSP PGUID 12 1 0 f f t f v 4 16 "2281 2281 2281 2281" _null_ _null_ _null_  hashgetmulti - _null_ ));
DESCR("hash(internal)");
DATA(insert OID = 441 (  hashinsert		   PGNSP PGUID 12 1 0 f f t f v 6 16 "2281 2281 2281 2281 2281 2281" _null_ _null_ _null_	hashinsert - _null_ ));
DESCR("hash(internal)");
DATA(insert OID = 443 (  hashbeginscan	   PGNSP PGUID 12 1 0 f f t f v 3 2281 "2281 2281 2281" _null_ _null_ _null_	hashbeginscan - _null_ ));
DESCR("hash(internal)");
DATA(insert OID = 444 (  hashrescan		   PGNSP PGUID 12 1 0 f f t f v 2 2278 "2281 2281" _null_ _null_ _null_ hashrescan - _null_ ));
DESCR("hash(internal)");
DATA(insert OID = 445 (  hashendscan	   PGNSP PGUID 12 1 0 f f t f v 1 2278 "2281" _null_ _null_ _null_	hashendscan - _null_ ));
DESCR("hash(internal)");
DATA(insert OID = 446 (  hashmarkpos	   PGNSP PGUID 12 1 0 f f t f v 1 2278 "2281" _null_ _null_ _null_	hashmarkpos - _null_ ));
DESCR("hash(internal)");
DATA(insert OID = 447 (  hashrestrpos	   PGNSP PGUID 12 1 0 f f t f v 1 2278 "2281" _null_ _null_ _null_	hashrestrpos - _null_ ));
DESCR("hash(internal)");
DATA(insert OID = 448 (  hashbuild		   PGNSP PGUID 12 1 0 f f t f v 3 2281 "2281 2281 2281" _null_ _null_ _null_ hashbuild - _null_ ));
DESCR("hash(internal)");
DATA(insert OID = 442 (  hashbulkdelete    PGNSP PGUID 12 1 0 f f t f v 4 2281 "2281 2281 2281 2281" _null_ _null_ _null_ hashbulkdelete - _null_ ));
DESCR("hash(internal)");
DATA(insert OID = 425 (  hashvacuumcleanup PGNSP PGUID 12 1 0 f f t f v 2 2281 "2281 2281" _null_ _null_ _null_ hashvacuumcleanup - _null_ ));
DESCR("hash(internal)");
DATA(insert OID = 438 (  hashcostestimate  PGNSP PGUID 12 1 0 f f t f v 8 2278 "2281 2281 2281 2281 2281 2281 2281 2281" _null_ _null_ _null_  hashcostestimate - _null_ ));
DESCR("hash(internal)");
DATA(insert OID = 2786 (  hashoptions	   PGNSP PGUID 12 1 0 f f t f s 2 17 "1009 16" _null_ _null_ _null_  hashoptions - _null_ ));
DESCR("hash(internal)");

DATA(insert OID = 449 (  hashint2		   PGNSP PGUID 12 1 0 f f t f i 1 23 "21" _null_ _null_ _null_	hashint2 - _null_ ));
DESCR("hash");
DATA(insert OID = 450 (  hashint4		   PGNSP PGUID 12 1 0 f f t f i 1 23 "23" _null_ _null_ _null_	hashint4 - _null_ ));
DESCR("hash");
DATA(insert OID = 949 (  hashint8		   PGNSP PGUID 12 1 0 f f t f i 1 23 "20" _null_ _null_ _null_	hashint8 - _null_ ));
DESCR("hash");
DATA(insert OID = 451 (  hashfloat4		   PGNSP PGUID 12 1 0 f f t f i 1 23 "700" _null_ _null_ _null_ hashfloat4 - _null_ ));
DESCR("hash");
DATA(insert OID = 452 (  hashfloat8		   PGNSP PGUID 12 1 0 f f t f i 1 23 "701" _null_ _null_ _null_ hashfloat8 - _null_ ));
DESCR("hash");
DATA(insert OID = 453 (  hashoid		   PGNSP PGUID 12 1 0 f f t f i 1 23 "26" _null_ _null_ _null_	hashoid - _null_ ));
DESCR("hash");
DATA(insert OID = 454 (  hashchar		   PGNSP PGUID 12 1 0 f f t f i 1 23 "18" _null_ _null_ _null_	hashchar - _null_ ));
DESCR("hash");
DATA(insert OID = 455 (  hashname		   PGNSP PGUID 12 1 0 f f t f i 1 23 "19" _null_ _null_ _null_	hashname - _null_ ));
DESCR("hash");
DATA(insert OID = 400 (  hashtext		   PGNSP PGUID 12 1 0 f f t f i 1 23 "25" _null_ _null_ _null_ hashtext - _null_ ));
DESCR("hash");
DATA(insert OID = 456 (  hashvarlena	   PGNSP PGUID 12 1 0 f f t f i 1 23 "2281" _null_ _null_ _null_ hashvarlena - _null_ ));
DESCR("hash any varlena type");
DATA(insert OID = 457 (  hashoidvector	   PGNSP PGUID 12 1 0 f f t f i 1 23 "30" _null_ _null_ _null_	hashoidvector - _null_ ));
DESCR("hash");
DATA(insert OID = 329 (  hash_aclitem	   PGNSP PGUID 12 1 0 f f t f i 1 23 "1033" _null_ _null_ _null_	hash_aclitem - _null_ ));
DESCR("hash");
DATA(insert OID = 398 (  hashint2vector    PGNSP PGUID 12 1 0 f f t f i 1 23 "22" _null_ _null_ _null_	hashint2vector - _null_ ));
DESCR("hash");
DATA(insert OID = 399 (  hashmacaddr	   PGNSP PGUID 12 1 0 f f t f i 1 23 "829" _null_ _null_ _null_ hashmacaddr - _null_ ));
DESCR("hash");
DATA(insert OID = 422 (  hashinet		   PGNSP PGUID 12 1 0 f f t f i 1 23 "869" _null_ _null_ _null_ hashinet - _null_ ));
DESCR("hash");
DATA(insert OID = 432 (  hash_numeric	   PGNSP PGUID 12 1 0 f f t f i 1 23 "1700" _null_ _null_ _null_ hash_numeric - _null_ ));
DESCR("hash");
DATA(insert OID = 458 (  text_larger	   PGNSP PGUID 12 1 0 f f t f i 2 25 "25 25" _null_ _null_ _null_ text_larger - _null_ ));
DESCR("larger of two");
DATA(insert OID = 459 (  text_smaller	   PGNSP PGUID 12 1 0 f f t f i 2 25 "25 25" _null_ _null_ _null_ text_smaller - _null_ ));
DESCR("smaller of two");

DATA(insert OID = 460 (  int8in			   PGNSP PGUID 12 1 0 f f t f i 1 20 "2275" _null_ _null_ _null_ int8in - _null_ ));
DESCR("I/O");
DATA(insert OID = 461 (  int8out		   PGNSP PGUID 12 1 0 f f t f i 1 2275 "20" _null_ _null_ _null_ int8out - _null_ ));
DESCR("I/O");
DATA(insert OID = 462 (  int8um			   PGNSP PGUID 12 1 0 f f t f i 1 20 "20" _null_ _null_ _null_	int8um - _null_ ));
DESCR("negate");
DATA(insert OID = 463 (  int8pl			   PGNSP PGUID 12 1 0 f f t f i 2 20 "20 20" _null_ _null_ _null_ int8pl - _null_ ));
DESCR("add");
DATA(insert OID = 464 (  int8mi			   PGNSP PGUID 12 1 0 f f t f i 2 20 "20 20" _null_ _null_ _null_ int8mi - _null_ ));
DESCR("subtract");
DATA(insert OID = 465 (  int8mul		   PGNSP PGUID 12 1 0 f f t f i 2 20 "20 20" _null_ _null_ _null_ int8mul - _null_ ));
DESCR("multiply");
DATA(insert OID = 466 (  int8div		   PGNSP PGUID 12 1 0 f f t f i 2 20 "20 20" _null_ _null_ _null_ int8div - _null_ ));
DESCR("divide");
DATA(insert OID = 467 (  int8eq			   PGNSP PGUID 12 1 0 f f t f i 2 16 "20 20" _null_ _null_ _null_ int8eq - _null_ ));
DESCR("equal");
DATA(insert OID = 468 (  int8ne			   PGNSP PGUID 12 1 0 f f t f i 2 16 "20 20" _null_ _null_ _null_ int8ne - _null_ ));
DESCR("not equal");
DATA(insert OID = 469 (  int8lt			   PGNSP PGUID 12 1 0 f f t f i 2 16 "20 20" _null_ _null_ _null_ int8lt - _null_ ));
DESCR("less-than");
DATA(insert OID = 470 (  int8gt			   PGNSP PGUID 12 1 0 f f t f i 2 16 "20 20" _null_ _null_ _null_ int8gt - _null_ ));
DESCR("greater-than");
DATA(insert OID = 471 (  int8le			   PGNSP PGUID 12 1 0 f f t f i 2 16 "20 20" _null_ _null_ _null_ int8le - _null_ ));
DESCR("less-than-or-equal");
DATA(insert OID = 472 (  int8ge			   PGNSP PGUID 12 1 0 f f t f i 2 16 "20 20" _null_ _null_ _null_ int8ge - _null_ ));
DESCR("greater-than-or-equal");

DATA(insert OID = 474 (  int84eq		   PGNSP PGUID 12 1 0 f f t f i 2 16 "20 23" _null_ _null_ _null_ int84eq - _null_ ));
DESCR("equal");
DATA(insert OID = 475 (  int84ne		   PGNSP PGUID 12 1 0 f f t f i 2 16 "20 23" _null_ _null_ _null_ int84ne - _null_ ));
DESCR("not equal");
DATA(insert OID = 476 (  int84lt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "20 23" _null_ _null_ _null_ int84lt - _null_ ));
DESCR("less-than");
DATA(insert OID = 477 (  int84gt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "20 23" _null_ _null_ _null_ int84gt - _null_ ));
DESCR("greater-than");
DATA(insert OID = 478 (  int84le		   PGNSP PGUID 12 1 0 f f t f i 2 16 "20 23" _null_ _null_ _null_ int84le - _null_ ));
DESCR("less-than-or-equal");
DATA(insert OID = 479 (  int84ge		   PGNSP PGUID 12 1 0 f f t f i 2 16 "20 23" _null_ _null_ _null_ int84ge - _null_ ));
DESCR("greater-than-or-equal");

DATA(insert OID = 480 (  int4			   PGNSP PGUID 12 1 0 f f t f i 1  23 "20" _null_ _null_ _null_ int84 - _null_ ));
DESCR("convert int8 to int4");
DATA(insert OID = 481 (  int8			   PGNSP PGUID 12 1 0 f f t f i 1  20 "23" _null_ _null_ _null_ int48 - _null_ ));
DESCR("convert int4 to int8");
DATA(insert OID = 482 (  float8			   PGNSP PGUID 12 1 0 f f t f i 1 701 "20" _null_ _null_ _null_ i8tod - _null_ ));
DESCR("convert int8 to float8");
DATA(insert OID = 483 (  int8			   PGNSP PGUID 12 1 0 f f t f i 1  20 "701" _null_ _null_ _null_	dtoi8 - _null_ ));
DESCR("convert float8 to int8");

/* OIDS 500 - 599 */

/* OIDS 600 - 699 */

DATA(insert OID = 652 (  float4			   PGNSP PGUID 12 1 0 f f t f i 1 700 "20" _null_ _null_ _null_ i8tof - _null_ ));
DESCR("convert int8 to float4");
DATA(insert OID = 653 (  int8			   PGNSP PGUID 12 1 0 f f t f i 1  20 "700" _null_ _null_ _null_	ftoi8 - _null_ ));
DESCR("convert float4 to int8");

DATA(insert OID = 714 (  int2			   PGNSP PGUID 12 1 0 f f t f i 1  21 "20" _null_ _null_ _null_ int82 - _null_ ));
DESCR("convert int8 to int2");
DATA(insert OID = 754 (  int8			   PGNSP PGUID 12 1 0 f f t f i 1  20 "21" _null_ _null_ _null_ int28 - _null_ ));
DESCR("convert int2 to int8");

DATA(insert OID = 1285 (  int4notin		   PGNSP PGUID 12 1 0 f f t f s 2 16 "23 25" _null_ _null_ _null_ int4notin - _null_ ));
DESCR("not in");
DATA(insert OID = 1286 (  oidnotin		   PGNSP PGUID 12 1 0 f f t f s 2 16 "26 25" _null_ _null_ _null_ oidnotin - _null_ ));
DESCR("not in");
DATA(insert OID = 655 (  namelt			   PGNSP PGUID 12 1 0 f f t f i 2 16 "19 19" _null_ _null_ _null_ namelt - _null_ ));
DESCR("less-than");
DATA(insert OID = 656 (  namele			   PGNSP PGUID 12 1 0 f f t f i 2 16 "19 19" _null_ _null_ _null_ namele - _null_ ));
DESCR("less-than-or-equal");
DATA(insert OID = 657 (  namegt			   PGNSP PGUID 12 1 0 f f t f i 2 16 "19 19" _null_ _null_ _null_ namegt - _null_ ));
DESCR("greater-than");
DATA(insert OID = 658 (  namege			   PGNSP PGUID 12 1 0 f f t f i 2 16 "19 19" _null_ _null_ _null_ namege - _null_ ));
DESCR("greater-than-or-equal");
DATA(insert OID = 659 (  namene			   PGNSP PGUID 12 1 0 f f t f i 2 16 "19 19" _null_ _null_ _null_ namene - _null_ ));
DESCR("not equal");

DATA(insert OID = 668 (  bpchar			   PGNSP PGUID 12 1 0 f f t f i 3 1042 "1042 23 16" _null_ _null_ _null_ bpchar - _null_ ));
DESCR("adjust char() to typmod length");
DATA(insert OID = 669 (  varchar		   PGNSP PGUID 12 1 0 f f t f i 3 1043 "1043 23 16" _null_ _null_ _null_ varchar - _null_ ));
DESCR("adjust varchar() to typmod length");

DATA(insert OID = 676 (  mktinterval	   PGNSP PGUID 12 1 0 f f t f i 2 704 "702 702" _null_ _null_ _null_ mktinterval - _null_ ));
DESCR("convert to tinterval");
DATA(insert OID = 619 (  oidvectorne	   PGNSP PGUID 12 1 0 f f t f i 2 16 "30 30" _null_ _null_ _null_ oidvectorne - _null_ ));
DESCR("not equal");
DATA(insert OID = 677 (  oidvectorlt	   PGNSP PGUID 12 1 0 f f t f i 2 16 "30 30" _null_ _null_ _null_ oidvectorlt - _null_ ));
DESCR("less-than");
DATA(insert OID = 678 (  oidvectorle	   PGNSP PGUID 12 1 0 f f t f i 2 16 "30 30" _null_ _null_ _null_ oidvectorle - _null_ ));
DESCR("less-than-or-equal");
DATA(insert OID = 679 (  oidvectoreq	   PGNSP PGUID 12 1 0 f f t f i 2 16 "30 30" _null_ _null_ _null_ oidvectoreq - _null_ ));
DESCR("equal");
DATA(insert OID = 680 (  oidvectorge	   PGNSP PGUID 12 1 0 f f t f i 2 16 "30 30" _null_ _null_ _null_ oidvectorge - _null_ ));
DESCR("greater-than-or-equal");
DATA(insert OID = 681 (  oidvectorgt	   PGNSP PGUID 12 1 0 f f t f i 2 16 "30 30" _null_ _null_ _null_ oidvectorgt - _null_ ));
DESCR("greater-than");

/* OIDS 700 - 799 */
DATA(insert OID = 710 (  getpgusername	   PGNSP PGUID 12 1 0 f f t f s 0 19 "" _null_ _null_ _null_ current_user - _null_ ));
DESCR("deprecated -- use current_user");
DATA(insert OID = 716 (  oidlt			   PGNSP PGUID 12 1 0 f f t f i 2 16 "26 26" _null_ _null_ _null_ oidlt - _null_ ));
DESCR("less-than");
DATA(insert OID = 717 (  oidle			   PGNSP PGUID 12 1 0 f f t f i 2 16 "26 26" _null_ _null_ _null_ oidle - _null_ ));
DESCR("less-than-or-equal");

DATA(insert OID = 720 (  octet_length	   PGNSP PGUID 12 1 0 f f t f i 1 23 "17" _null_ _null_ _null_	byteaoctetlen - _null_ ));
DESCR("octet length");
DATA(insert OID = 721 (  get_byte		   PGNSP PGUID 12 1 0 f f t f i 2 23 "17 23" _null_ _null_ _null_ byteaGetByte - _null_ ));
DESCR("get byte");
DATA(insert OID = 722 (  set_byte		   PGNSP PGUID 12 1 0 f f t f i 3 17 "17 23 23" _null_ _null_ _null_	byteaSetByte - _null_ ));
DESCR("set byte");
DATA(insert OID = 723 (  get_bit		   PGNSP PGUID 12 1 0 f f t f i 2 23 "17 23" _null_ _null_ _null_ byteaGetBit - _null_ ));
DESCR("get bit");
DATA(insert OID = 724 (  set_bit		   PGNSP PGUID 12 1 0 f f t f i 3 17 "17 23 23" _null_ _null_ _null_	byteaSetBit - _null_ ));
DESCR("set bit");

DATA(insert OID = 725 (  dist_pl		   PGNSP PGUID 12 1 0 f f t f i 2 701 "600 628" _null_ _null_ _null_	dist_pl - _null_ ));
DESCR("distance between point and line");
DATA(insert OID = 726 (  dist_lb		   PGNSP PGUID 12 1 0 f f t f i 2 701 "628 603" _null_ _null_ _null_	dist_lb - _null_ ));
DESCR("distance between line and box");
DATA(insert OID = 727 (  dist_sl		   PGNSP PGUID 12 1 0 f f t f i 2 701 "601 628" _null_ _null_ _null_	dist_sl - _null_ ));
DESCR("distance between lseg and line");
DATA(insert OID = 728 (  dist_cpoly		   PGNSP PGUID 12 1 0 f f t f i 2 701 "718 604" _null_ _null_ _null_	dist_cpoly - _null_ ));
DESCR("distance between");
DATA(insert OID = 729 (  poly_distance	   PGNSP PGUID 12 1 0 f f t f i 2 701 "604 604" _null_ _null_ _null_	poly_distance - _null_ ));
DESCR("distance between");

DATA(insert OID = 740 (  text_lt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "25 25" _null_ _null_ _null_ text_lt - _null_ ));
DESCR("less-than");
DATA(insert OID = 741 (  text_le		   PGNSP PGUID 12 1 0 f f t f i 2 16 "25 25" _null_ _null_ _null_ text_le - _null_ ));
DESCR("less-than-or-equal");
DATA(insert OID = 742 (  text_gt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "25 25" _null_ _null_ _null_ text_gt - _null_ ));
DESCR("greater-than");
DATA(insert OID = 743 (  text_ge		   PGNSP PGUID 12 1 0 f f t f i 2 16 "25 25" _null_ _null_ _null_ text_ge - _null_ ));
DESCR("greater-than-or-equal");

DATA(insert OID = 745 (  current_user	   PGNSP PGUID 12 1 0 f f t f s 0 19 "" _null_ _null_ _null_ current_user - _null_ ));
DESCR("current user name");
DATA(insert OID = 746 (  session_user	   PGNSP PGUID 12 1 0 f f t f s 0 19 "" _null_ _null_ _null_ session_user - _null_ ));
DESCR("session user name");

DATA(insert OID = 744 (  array_eq		   PGNSP PGUID 12 1 0 f f t f i 2 16 "2277 2277" _null_ _null_ _null_ array_eq - _null_ ));
DESCR("array equal");
DATA(insert OID = 390 (  array_ne		   PGNSP PGUID 12 1 0 f f t f i 2 16 "2277 2277" _null_ _null_ _null_ array_ne - _null_ ));
DESCR("array not equal");
DATA(insert OID = 391 (  array_lt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "2277 2277" _null_ _null_ _null_ array_lt - _null_ ));
DESCR("array less than");
DATA(insert OID = 392 (  array_gt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "2277 2277" _null_ _null_ _null_ array_gt - _null_ ));
DESCR("array greater than");
DATA(insert OID = 393 (  array_le		   PGNSP PGUID 12 1 0 f f t f i 2 16 "2277 2277" _null_ _null_ _null_ array_le - _null_ ));
DESCR("array less than or equal");
DATA(insert OID = 396 (  array_ge		   PGNSP PGUID 12 1 0 f f t f i 2 16 "2277 2277" _null_ _null_ _null_ array_ge - _null_ ));
DESCR("array greater than or equal");
DATA(insert OID = 747 (  array_dims		   PGNSP PGUID 12 1 0 f f t f i 1 25 "2277" _null_ _null_ _null_ array_dims - _null_ ));
DESCR("array dimensions");
DATA(insert OID = 750 (  array_in		   PGNSP PGUID 12 1 0 f f t f s 3 2277 "2275 26 23" _null_ _null_ _null_	array_in - _null_ ));
DESCR("I/O");
DATA(insert OID = 751 (  array_out		   PGNSP PGUID 12 1 0 f f t f s 1 2275 "2277" _null_ _null_ _null_	array_out - _null_ ));
DESCR("I/O");
DATA(insert OID = 2091 (  array_lower	   PGNSP PGUID 12 1 0 f f t f i 2 23 "2277 23" _null_ _null_ _null_ array_lower - _null_ ));
DESCR("array lower dimension");
DATA(insert OID = 2092 (  array_upper	   PGNSP PGUID 12 1 0 f f t f i 2 23 "2277 23" _null_ _null_ _null_ array_upper - _null_ ));
DESCR("array upper dimension");
DATA(insert OID = 378 (  array_append	   PGNSP PGUID 12 1 0 f f f f i 2 2277 "2277 2283" _null_ _null_ _null_ array_push - _null_ ));
DESCR("append element onto end of array");
DATA(insert OID = 379 (  array_prepend	   PGNSP PGUID 12 1 0 f f f f i 2 2277 "2283 2277" _null_ _null_ _null_ array_push - _null_ ));
DESCR("prepend element onto front of array");
DATA(insert OID = 383 (  array_cat		   PGNSP PGUID 12 1 0 f f f f i 2 2277 "2277 2277" _null_ _null_ _null_ array_cat - _null_ ));
DESCR("concatenate two arrays");
DATA(insert OID = 394 (  string_to_array   PGNSP PGUID 12 1 0 f f t f i 2 1009 "25 25" _null_ _null_ _null_ text_to_array - _null_ ));
DESCR("split delimited text into text[]");
DATA(insert OID = 395 (  array_to_string   PGNSP PGUID 12 1 0 f f t f i 2 25 "2277 25" _null_ _null_ _null_ array_to_text - _null_ ));
DESCR("concatenate array elements, using delimiter, into text");
DATA(insert OID = 515 (  array_larger	   PGNSP PGUID 12 1 0 f f t f i 2 2277 "2277 2277" _null_ _null_ _null_ array_larger - _null_ ));
DESCR("larger of two");
DATA(insert OID = 516 (  array_smaller	   PGNSP PGUID 12 1 0 f f t f i 2 2277 "2277 2277" _null_ _null_ _null_ array_smaller - _null_ ));
DESCR("smaller of two");

DATA(insert OID = 760 (  smgrin			   PGNSP PGUID 12 1 0 f f t f s 1 210 "2275" _null_ _null_ _null_  smgrin - _null_ ));
DESCR("I/O");
DATA(insert OID = 761 (  smgrout		   PGNSP PGUID 12 1 0 f f t f s 1 2275 "210" _null_ _null_ _null_  smgrout - _null_ ));
DESCR("I/O");
DATA(insert OID = 762 (  smgreq			   PGNSP PGUID 12 1 0 f f t f i 2 16 "210 210" _null_ _null_ _null_ smgreq - _null_ ));
DESCR("storage manager");
DATA(insert OID = 763 (  smgrne			   PGNSP PGUID 12 1 0 f f t f i 2 16 "210 210" _null_ _null_ _null_ smgrne - _null_ ));
DESCR("storage manager");

DATA(insert OID = 764 (  lo_import		   PGNSP PGUID 12 1 0 f f t f v 1 26 "25" _null_ _null_ _null_	lo_import - _null_ ));
DESCR("large object import");
DATA(insert OID = 765 (  lo_export		   PGNSP PGUID 12 1 0 f f t f v 2 23 "26 25" _null_ _null_ _null_ lo_export - _null_ ));
DESCR("large object export");

DATA(insert OID = 766 (  int4inc		   PGNSP PGUID 12 1 0 f f t f i 1 23 "23" _null_ _null_ _null_	int4inc - _null_ ));
DESCR("increment");
DATA(insert OID = 768 (  int4larger		   PGNSP PGUID 12 1 0 f f t f i 2 23 "23 23" _null_ _null_ _null_ int4larger - _null_ ));
DESCR("larger of two");
DATA(insert OID = 769 (  int4smaller	   PGNSP PGUID 12 1 0 f f t f i 2 23 "23 23" _null_ _null_ _null_ int4smaller - _null_ ));
DESCR("smaller of two");
DATA(insert OID = 770 (  int2larger		   PGNSP PGUID 12 1 0 f f t f i 2 21 "21 21" _null_ _null_ _null_ int2larger - _null_ ));
DESCR("larger of two");
DATA(insert OID = 771 (  int2smaller	   PGNSP PGUID 12 1 0 f f t f i 2 21 "21 21" _null_ _null_ _null_ int2smaller - _null_ ));
DESCR("smaller of two");

DATA(insert OID = 774 (  gistgettuple	   PGNSP PGUID 12 1 0 f f t f v 2 16 "2281 2281" _null_ _null_ _null_  gistgettuple - _null_ ));
DESCR("gist(internal)");
DATA(insert OID = 638 (  gistgetmulti	   PGNSP PGUID 12 1 0 f f t f v 4 16 "2281 2281 2281 2281" _null_ _null_ _null_  gistgetmulti - _null_ ));
DESCR("gist(internal)");
DATA(insert OID = 775 (  gistinsert		   PGNSP PGUID 12 1 0 f f t f v 6 16 "2281 2281 2281 2281 2281 2281" _null_ _null_ _null_	gistinsert - _null_ ));
DESCR("gist(internal)");
DATA(insert OID = 777 (  gistbeginscan	   PGNSP PGUID 12 1 0 f f t f v 3 2281 "2281 2281 2281" _null_ _null_ _null_	gistbeginscan - _null_ ));
DESCR("gist(internal)");
DATA(insert OID = 778 (  gistrescan		   PGNSP PGUID 12 1 0 f f t f v 2 2278 "2281 2281" _null_ _null_ _null_ gistrescan - _null_ ));
DESCR("gist(internal)");
DATA(insert OID = 779 (  gistendscan	   PGNSP PGUID 12 1 0 f f t f v 1 2278 "2281" _null_ _null_ _null_	gistendscan - _null_ ));
DESCR("gist(internal)");
DATA(insert OID = 780 (  gistmarkpos	   PGNSP PGUID 12 1 0 f f t f v 1 2278 "2281" _null_ _null_ _null_	gistmarkpos - _null_ ));
DESCR("gist(internal)");
DATA(insert OID = 781 (  gistrestrpos	   PGNSP PGUID 12 1 0 f f t f v 1 2278 "2281" _null_ _null_ _null_	gistrestrpos - _null_ ));
DESCR("gist(internal)");
DATA(insert OID = 782 (  gistbuild		   PGNSP PGUID 12 1 0 f f t f v 3 2281 "2281 2281 2281" _null_ _null_ _null_ gistbuild - _null_ ));
DESCR("gist(internal)");
DATA(insert OID = 776 (  gistbulkdelete    PGNSP PGUID 12 1 0 f f t f v 4 2281 "2281 2281 2281 2281" _null_ _null_ _null_ gistbulkdelete - _null_ ));
DESCR("gist(internal)");
DATA(insert OID = 2561 (  gistvacuumcleanup   PGNSP PGUID 12 1 0 f f t f v 2 2281 "2281 2281" _null_ _null_ _null_ gistvacuumcleanup - _null_ ));
DESCR("gist(internal)");
DATA(insert OID = 772 (  gistcostestimate  PGNSP PGUID 12 1 0 f f t f v 8 2278 "2281 2281 2281 2281 2281 2281 2281 2281" _null_ _null_ _null_  gistcostestimate - _null_ ));
DESCR("gist(internal)");
DATA(insert OID = 2787 (  gistoptions	   PGNSP PGUID 12 1 0 f f t f s 2 17 "1009 16" _null_ _null_ _null_  gistoptions - _null_ ));
DESCR("gist(internal)");

DATA(insert OID = 784 (  tintervaleq	   PGNSP PGUID 12 1 0 f f t f i 2 16 "704 704" _null_ _null_ _null_ tintervaleq - _null_ ));
DESCR("equal");
DATA(insert OID = 785 (  tintervalne	   PGNSP PGUID 12 1 0 f f t f i 2 16 "704 704" _null_ _null_ _null_ tintervalne - _null_ ));
DESCR("not equal");
DATA(insert OID = 786 (  tintervallt	   PGNSP PGUID 12 1 0 f f t f i 2 16 "704 704" _null_ _null_ _null_ tintervallt - _null_ ));
DESCR("less-than");
DATA(insert OID = 787 (  tintervalgt	   PGNSP PGUID 12 1 0 f f t f i 2 16 "704 704" _null_ _null_ _null_ tintervalgt - _null_ ));
DESCR("greater-than");
DATA(insert OID = 788 (  tintervalle	   PGNSP PGUID 12 1 0 f f t f i 2 16 "704 704" _null_ _null_ _null_ tintervalle - _null_ ));
DESCR("less-than-or-equal");
DATA(insert OID = 789 (  tintervalge	   PGNSP PGUID 12 1 0 f f t f i 2 16 "704 704" _null_ _null_ _null_ tintervalge - _null_ ));
DESCR("greater-than-or-equal");

/* OIDS 800 - 899 */

DATA(insert OID =  846 (  cash_mul_flt4    PGNSP PGUID 12 1 0 f f t f i 2 790 "790 700" _null_ _null_ _null_	cash_mul_flt4 - _null_ ));
DESCR("multiply");
DATA(insert OID =  847 (  cash_div_flt4    PGNSP PGUID 12 1 0 f f t f i 2 790 "790 700" _null_ _null_ _null_	cash_div_flt4 - _null_ ));
DESCR("divide");
DATA(insert OID =  848 (  flt4_mul_cash    PGNSP PGUID 12 1 0 f f t f i 2 790 "700 790" _null_ _null_ _null_	flt4_mul_cash - _null_ ));
DESCR("multiply");

DATA(insert OID =  849 (  position		   PGNSP PGUID 12 1 0 f f t f i 2 23 "25 25" _null_ _null_ _null_ textpos - _null_ ));
DESCR("return position of substring");
DATA(insert OID =  850 (  textlike		   PGNSP PGUID 12 1 0 f f t f i 2 16 "25 25" _null_ _null_ _null_ textlike - _null_ ));
DESCR("matches LIKE expression");
DATA(insert OID =  851 (  textnlike		   PGNSP PGUID 12 1 0 f f t f i 2 16 "25 25" _null_ _null_ _null_ textnlike - _null_ ));
DESCR("does not match LIKE expression");

DATA(insert OID =  852 (  int48eq		   PGNSP PGUID 12 1 0 f f t f i 2 16 "23 20" _null_ _null_ _null_ int48eq - _null_ ));
DESCR("equal");
DATA(insert OID =  853 (  int48ne		   PGNSP PGUID 12 1 0 f f t f i 2 16 "23 20" _null_ _null_ _null_ int48ne - _null_ ));
DESCR("not equal");
DATA(insert OID =  854 (  int48lt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "23 20" _null_ _null_ _null_ int48lt - _null_ ));
DESCR("less-than");
DATA(insert OID =  855 (  int48gt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "23 20" _null_ _null_ _null_ int48gt - _null_ ));
DESCR("greater-than");
DATA(insert OID =  856 (  int48le		   PGNSP PGUID 12 1 0 f f t f i 2 16 "23 20" _null_ _null_ _null_ int48le - _null_ ));
DESCR("less-than-or-equal");
DATA(insert OID =  857 (  int48ge		   PGNSP PGUID 12 1 0 f f t f i 2 16 "23 20" _null_ _null_ _null_ int48ge - _null_ ));
DESCR("greater-than-or-equal");

DATA(insert OID =  858 (  namelike		   PGNSP PGUID 12 1 0 f f t f i 2 16 "19 25" _null_ _null_ _null_ namelike - _null_ ));
DESCR("matches LIKE expression");
DATA(insert OID =  859 (  namenlike		   PGNSP PGUID 12 1 0 f f t f i 2 16 "19 25" _null_ _null_ _null_ namenlike - _null_ ));
DESCR("does not match LIKE expression");

DATA(insert OID =  860 (  bpchar		   PGNSP PGUID 12 1 0 f f t f i 1 1042 "18" _null_ _null_ _null_	char_bpchar - _null_ ));
DESCR("convert char to char()");

DATA(insert OID = 861 ( current_database	   PGNSP PGUID 12 1 0 f f t f i 0 19 "" _null_ _null_ _null_ current_database - _null_ ));
DESCR("returns the current database");

DATA(insert OID =  862 (  int4_mul_cash		   PGNSP PGUID 12 1 0 f f t f i 2 790 "23 790" _null_ _null_ _null_ int4_mul_cash - _null_ ));
DESCR("multiply");
DATA(insert OID =  863 (  int2_mul_cash		   PGNSP PGUID 12 1 0 f f t f i 2 790 "21 790" _null_ _null_ _null_ int2_mul_cash - _null_ ));
DESCR("multiply");
DATA(insert OID =  864 (  cash_mul_int4		   PGNSP PGUID 12 1 0 f f t f i 2 790 "790 23" _null_ _null_ _null_ cash_mul_int4 - _null_ ));
DESCR("multiply");
DATA(insert OID =  865 (  cash_div_int4		   PGNSP PGUID 12 1 0 f f t f i 2 790 "790 23" _null_ _null_ _null_ cash_div_int4 - _null_ ));
DESCR("divide");
DATA(insert OID =  866 (  cash_mul_int2		   PGNSP PGUID 12 1 0 f f t f i 2 790 "790 21" _null_ _null_ _null_ cash_mul_int2 - _null_ ));
DESCR("multiply");
DATA(insert OID =  867 (  cash_div_int2		   PGNSP PGUID 12 1 0 f f t f i 2 790 "790 21" _null_ _null_ _null_ cash_div_int2 - _null_ ));
DESCR("divide");

DATA(insert OID =  886 (  cash_in		   PGNSP PGUID 12 1 0 f f t f i 1 790 "2275" _null_ _null_ _null_  cash_in - _null_ ));
DESCR("I/O");
DATA(insert OID =  887 (  cash_out		   PGNSP PGUID 12 1 0 f f t f i 1 2275 "790" _null_ _null_ _null_  cash_out - _null_ ));
DESCR("I/O");
DATA(insert OID =  888 (  cash_eq		   PGNSP PGUID 12 1 0 f f t f i 2  16 "790 790" _null_ _null_ _null_	cash_eq - _null_ ));
DESCR("equal");
DATA(insert OID =  889 (  cash_ne		   PGNSP PGUID 12 1 0 f f t f i 2  16 "790 790" _null_ _null_ _null_	cash_ne - _null_ ));
DESCR("not equal");
DATA(insert OID =  890 (  cash_lt		   PGNSP PGUID 12 1 0 f f t f i 2  16 "790 790" _null_ _null_ _null_	cash_lt - _null_ ));
DESCR("less-than");
DATA(insert OID =  891 (  cash_le		   PGNSP PGUID 12 1 0 f f t f i 2  16 "790 790" _null_ _null_ _null_	cash_le - _null_ ));
DESCR("less-than-or-equal");
DATA(insert OID =  892 (  cash_gt		   PGNSP PGUID 12 1 0 f f t f i 2  16 "790 790" _null_ _null_ _null_	cash_gt - _null_ ));
DESCR("greater-than");
DATA(insert OID =  893 (  cash_ge		   PGNSP PGUID 12 1 0 f f t f i 2  16 "790 790" _null_ _null_ _null_	cash_ge - _null_ ));
DESCR("greater-than-or-equal");
DATA(insert OID =  894 (  cash_pl		   PGNSP PGUID 12 1 0 f f t f i 2 790 "790 790" _null_ _null_ _null_	cash_pl - _null_ ));
DESCR("add");
DATA(insert OID =  895 (  cash_mi		   PGNSP PGUID 12 1 0 f f t f i 2 790 "790 790" _null_ _null_ _null_	cash_mi - _null_ ));
DESCR("subtract");
DATA(insert OID =  896 (  cash_mul_flt8    PGNSP PGUID 12 1 0 f f t f i 2 790 "790 701" _null_ _null_ _null_	cash_mul_flt8 - _null_ ));
DESCR("multiply");
DATA(insert OID =  897 (  cash_div_flt8    PGNSP PGUID 12 1 0 f f t f i 2 790 "790 701" _null_ _null_ _null_	cash_div_flt8 - _null_ ));
DESCR("divide");
DATA(insert OID =  898 (  cashlarger	   PGNSP PGUID 12 1 0 f f t f i 2 790 "790 790" _null_ _null_ _null_	cashlarger - _null_ ));
DESCR("larger of two");
DATA(insert OID =  899 (  cashsmaller	   PGNSP PGUID 12 1 0 f f t f i 2 790 "790 790" _null_ _null_ _null_	cashsmaller - _null_ ));
DESCR("smaller of two");
DATA(insert OID =  919 (  flt8_mul_cash    PGNSP PGUID 12 1 0 f f t f i 2 790 "701 790" _null_ _null_ _null_	flt8_mul_cash - _null_ ));
DESCR("multiply");
DATA(insert OID =  935 (  cash_words	   PGNSP PGUID 12 1 0 f f t f i 1  25 "790" _null_ _null_ _null_	cash_words - _null_ ));
DESCR("output amount as words");

/* OIDS 900 - 999 */

DATA(insert OID = 940 (  mod			   PGNSP PGUID 12 1 0 f f t f i 2 21 "21 21" _null_ _null_ _null_ int2mod - _null_ ));
DESCR("modulus");
DATA(insert OID = 941 (  mod			   PGNSP PGUID 12 1 0 f f t f i 2 23 "23 23" _null_ _null_ _null_ int4mod - _null_ ));
DESCR("modulus");
DATA(insert OID = 942 (  mod			   PGNSP PGUID 12 1 0 f f t f i 2 23 "21 23" _null_ _null_ _null_ int24mod - _null_ ));
DESCR("modulus");
DATA(insert OID = 943 (  mod			   PGNSP PGUID 12 1 0 f f t f i 2 23 "23 21" _null_ _null_ _null_ int42mod - _null_ ));
DESCR("modulus");

DATA(insert OID = 945 (  int8mod		   PGNSP PGUID 12 1 0 f f t f i 2 20 "20 20" _null_ _null_ _null_ int8mod - _null_ ));
DESCR("modulus");
DATA(insert OID = 947 (  mod			   PGNSP PGUID 12 1 0 f f t f i 2 20 "20 20" _null_ _null_ _null_ int8mod - _null_ ));
DESCR("modulus");

DATA(insert OID = 944 (  char			   PGNSP PGUID 12 1 0 f f t f i 1 18 "25" _null_ _null_ _null_	text_char - _null_ ));
DESCR("convert text to char");
DATA(insert OID = 946 (  text			   PGNSP PGUID 12 1 0 f f t f i 1 25 "18" _null_ _null_ _null_	char_text - _null_ ));
DESCR("convert char to text");

DATA(insert OID = 950 (  istrue			   PGNSP PGUID 12 1 0 f f f f i 1 16 "16" _null_ _null_ _null_	istrue - _null_ ));
DESCR("bool is true (not false or unknown)");
DATA(insert OID = 951 (  isfalse		   PGNSP PGUID 12 1 0 f f f f i 1 16 "16" _null_ _null_ _null_	isfalse - _null_ ));
DESCR("bool is false (not true or unknown)");

DATA(insert OID = 952 (  lo_open		   PGNSP PGUID 12 1 0 f f t f v 2 23 "26 23" _null_ _null_ _null_ lo_open - _null_ ));
DESCR("large object open");
DATA(insert OID = 953 (  lo_close		   PGNSP PGUID 12 1 0 f f t f v 1 23 "23" _null_ _null_ _null_	lo_close - _null_ ));
DESCR("large object close");
DATA(insert OID = 954 (  loread			   PGNSP PGUID 12 1 0 f f t f v 2 17 "23 23" _null_ _null_ _null_ loread - _null_ ));
DESCR("large object read");
DATA(insert OID = 955 (  lowrite		   PGNSP PGUID 12 1 0 f f t f v 2 23 "23 17" _null_ _null_ _null_ lowrite - _null_ ));
DESCR("large object write");
DATA(insert OID = 956 (  lo_lseek		   PGNSP PGUID 12 1 0 f f t f v 3 23 "23 23 23" _null_ _null_ _null_	lo_lseek - _null_ ));
DESCR("large object seek");
DATA(insert OID = 957 (  lo_creat		   PGNSP PGUID 12 1 0 f f t f v 1 26 "23" _null_ _null_ _null_	lo_creat - _null_ ));
DESCR("large object create");
DATA(insert OID = 715 (  lo_create		   PGNSP PGUID 12 1 0 f f t f v 1 26 "26" _null_ _null_ _null_	lo_create - _null_ ));
DESCR("large object create");
DATA(insert OID = 958 (  lo_tell		   PGNSP PGUID 12 1 0 f f t f v 1 23 "23" _null_ _null_ _null_	lo_tell - _null_ ));
DESCR("large object position");
DATA(insert OID = 1004 (  lo_truncate	   PGNSP PGUID 12 1 0 f f t f v 2 23 "23 23" _null_ _null_ _null_ lo_truncate - _null_ ));
DESCR("truncate large object");

DATA(insert OID = 959 (  on_pl			   PGNSP PGUID 12 1 0 f f t f i 2  16 "600 628" _null_ _null_ _null_	on_pl - _null_ ));
DESCR("point on line?");
DATA(insert OID = 960 (  on_sl			   PGNSP PGUID 12 1 0 f f t f i 2  16 "601 628" _null_ _null_ _null_	on_sl - _null_ ));
DESCR("lseg on line?");
DATA(insert OID = 961 (  close_pl		   PGNSP PGUID 12 1 0 f f t f i 2 600 "600 628" _null_ _null_ _null_	close_pl - _null_ ));
DESCR("closest point on line");
DATA(insert OID = 962 (  close_sl		   PGNSP PGUID 12 1 0 f f t f i 2 600 "601 628" _null_ _null_ _null_	close_sl - _null_ ));
DESCR("closest point to line segment on line");
DATA(insert OID = 963 (  close_lb		   PGNSP PGUID 12 1 0 f f t f i 2 600 "628 603" _null_ _null_ _null_	close_lb - _null_ ));
DESCR("closest point to line on box");

DATA(insert OID = 964 (  lo_unlink		   PGNSP PGUID 12 1 0 f f t f v 1  23 "26" _null_ _null_ _null_ lo_unlink - _null_ ));
DESCR("large object unlink(delete)");

DATA(insert OID = 973 (  path_inter		   PGNSP PGUID 12 1 0 f f t f i 2  16 "602 602" _null_ _null_ _null_	path_inter - _null_ ));
DESCR("intersect?");
DATA(insert OID = 975 (  area			   PGNSP PGUID 12 1 0 f f t f i 1 701 "603" _null_ _null_ _null_	box_area - _null_ ));
DESCR("box area");
DATA(insert OID = 976 (  width			   PGNSP PGUID 12 1 0 f f t f i 1 701 "603" _null_ _null_ _null_	box_width - _null_ ));
DESCR("box width");
DATA(insert OID = 977 (  height			   PGNSP PGUID 12 1 0 f f t f i 1 701 "603" _null_ _null_ _null_	box_height - _null_ ));
DESCR("box height");
DATA(insert OID = 978 (  box_distance	   PGNSP PGUID 12 1 0 f f t f i 2 701 "603 603" _null_ _null_ _null_	box_distance - _null_ ));
DESCR("distance between boxes");
DATA(insert OID = 979 (  area			   PGNSP PGUID 12 1 0 f f t f i 1 701 "602" _null_ _null_ _null_	path_area - _null_ ));
DESCR("area of a closed path");
DATA(insert OID = 980 (  box_intersect	   PGNSP PGUID 12 1 0 f f t f i 2 603 "603 603" _null_ _null_ _null_	box_intersect - _null_ ));
DESCR("box intersection (another box)");
DATA(insert OID = 981 (  diagonal		   PGNSP PGUID 12 1 0 f f t f i 1 601 "603" _null_ _null_ _null_	box_diagonal - _null_ ));
DESCR("box diagonal");
DATA(insert OID = 982 (  path_n_lt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "602 602" _null_ _null_ _null_ path_n_lt - _null_ ));
DESCR("less-than");
DATA(insert OID = 983 (  path_n_gt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "602 602" _null_ _null_ _null_ path_n_gt - _null_ ));
DESCR("greater-than");
DATA(insert OID = 984 (  path_n_eq		   PGNSP PGUID 12 1 0 f f t f i 2 16 "602 602" _null_ _null_ _null_ path_n_eq - _null_ ));
DESCR("equal");
DATA(insert OID = 985 (  path_n_le		   PGNSP PGUID 12 1 0 f f t f i 2 16 "602 602" _null_ _null_ _null_ path_n_le - _null_ ));
DESCR("less-than-or-equal");
DATA(insert OID = 986 (  path_n_ge		   PGNSP PGUID 12 1 0 f f t f i 2 16 "602 602" _null_ _null_ _null_ path_n_ge - _null_ ));
DESCR("greater-than-or-equal");
DATA(insert OID = 987 (  path_length	   PGNSP PGUID 12 1 0 f f t f i 1 701 "602" _null_ _null_ _null_	path_length - _null_ ));
DESCR("sum of path segment lengths");
DATA(insert OID = 988 (  point_ne		   PGNSP PGUID 12 1 0 f f t f i 2 16 "600 600" _null_ _null_ _null_ point_ne - _null_ ));
DESCR("not equal");
DATA(insert OID = 989 (  point_vert		   PGNSP PGUID 12 1 0 f f t f i 2 16 "600 600" _null_ _null_ _null_ point_vert - _null_ ));
DESCR("vertically aligned?");
DATA(insert OID = 990 (  point_horiz	   PGNSP PGUID 12 1 0 f f t f i 2 16 "600 600" _null_ _null_ _null_ point_horiz - _null_ ));
DESCR("horizontally aligned?");
DATA(insert OID = 991 (  point_distance    PGNSP PGUID 12 1 0 f f t f i 2 701 "600 600" _null_ _null_ _null_	point_distance - _null_ ));
DESCR("distance between");
DATA(insert OID = 992 (  slope			   PGNSP PGUID 12 1 0 f f t f i 2 701 "600 600" _null_ _null_ _null_	point_slope - _null_ ));
DESCR("slope between points");
DATA(insert OID = 993 (  lseg			   PGNSP PGUID 12 1 0 f f t f i 2 601 "600 600" _null_ _null_ _null_	lseg_construct - _null_ ));
DESCR("convert points to line segment");
DATA(insert OID = 994 (  lseg_intersect    PGNSP PGUID 12 1 0 f f t f i 2 16 "601 601" _null_ _null_ _null_ lseg_intersect - _null_ ));
DESCR("intersect?");
DATA(insert OID = 995 (  lseg_parallel	   PGNSP PGUID 12 1 0 f f t f i 2 16 "601 601" _null_ _null_ _null_ lseg_parallel - _null_ ));
DESCR("parallel?");
DATA(insert OID = 996 (  lseg_perp		   PGNSP PGUID 12 1 0 f f t f i 2 16 "601 601" _null_ _null_ _null_ lseg_perp - _null_ ));
DESCR("perpendicular?");
DATA(insert OID = 997 (  lseg_vertical	   PGNSP PGUID 12 1 0 f f t f i 1 16 "601" _null_ _null_ _null_ lseg_vertical - _null_ ));
DESCR("vertical?");
DATA(insert OID = 998 (  lseg_horizontal   PGNSP PGUID 12 1 0 f f t f i 1 16 "601" _null_ _null_ _null_ lseg_horizontal - _null_ ));
DESCR("horizontal?");
DATA(insert OID = 999 (  lseg_eq		   PGNSP PGUID 12 1 0 f f t f i 2 16 "601 601" _null_ _null_ _null_ lseg_eq - _null_ ));
DESCR("equal");

/* OIDS 1000 - 1999 */

DATA(insert OID = 1026 (  timezone		   PGNSP PGUID 12 1 0 f f t f i 2 1114 "1186 1184" _null_ _null_ _null_ timestamptz_izone - _null_ ));
DESCR("adjust timestamp to new time zone");

DATA(insert OID = 1029 (  nullvalue		   PGNSP PGUID 12 1 0 f f f f i 1 16 "2276" _null_ _null_ _null_ nullvalue - _null_ ));
DESCR("(internal)");
DATA(insert OID = 1030 (  nonnullvalue	   PGNSP PGUID 12 1 0 f f f f i 1 16 "2276" _null_ _null_ _null_ nonnullvalue - _null_ ));
DESCR("(internal)");
DATA(insert OID = 1031 (  aclitemin		   PGNSP PGUID 12 1 0 f f t f s 1 1033 "2275" _null_ _null_ _null_	aclitemin - _null_ ));
DESCR("I/O");
DATA(insert OID = 1032 (  aclitemout	   PGNSP PGUID 12 1 0 f f t f s 1 2275 "1033" _null_ _null_ _null_	aclitemout - _null_ ));
DESCR("I/O");
DATA(insert OID = 1035 (  aclinsert		   PGNSP PGUID 12 1 0 f f t f i 2 1034 "1034 1033" _null_ _null_ _null_ aclinsert - _null_ ));
DESCR("add/update ACL item");
DATA(insert OID = 1036 (  aclremove		   PGNSP PGUID 12 1 0 f f t f i 2 1034 "1034 1033" _null_ _null_ _null_ aclremove - _null_ ));
DESCR("remove ACL item");
DATA(insert OID = 1037 (  aclcontains	   PGNSP PGUID 12 1 0 f f t f i 2 16 "1034 1033" _null_ _null_ _null_ aclcontains - _null_ ));
DESCR("ACL contains item?");
DATA(insert OID = 1062 (  aclitemeq		   PGNSP PGUID 12 1 0 f f t f i 2 16 "1033 1033" _null_ _null_ _null_ aclitem_eq - _null_ ));
DESCR("equality operator for ACL items");
DATA(insert OID = 1365 (  makeaclitem	   PGNSP PGUID 12 1 0 f f t f i 4 1033 "26 26 25 16" _null_ _null_ _null_ makeaclitem - _null_ ));
DESCR("make ACL item");
DATA(insert OID = 1044 (  bpcharin		   PGNSP PGUID 12 1 0 f f t f i 3 1042 "2275 26 23" _null_ _null_ _null_ bpcharin - _null_ ));
DESCR("I/O");
DATA(insert OID = 1045 (  bpcharout		   PGNSP PGUID 12 1 0 f f t f i 1 2275 "1042" _null_ _null_ _null_	bpcharout - _null_ ));
DESCR("I/O");
DATA(insert OID = 2913 (  bpchartypmodin   PGNSP PGUID 12 1 0 f f t f i 1 23 "1263" _null_ _null_ _null_	bpchartypmodin - _null_ ));
DESCR("I/O typmod");
DATA(insert OID = 2914 (  bpchartypmodout  PGNSP PGUID 12 1 0 f f t f i 1 2275 "23" _null_ _null_ _null_	bpchartypmodout - _null_ ));
DESCR("I/O typmod");
DATA(insert OID = 1046 (  varcharin		   PGNSP PGUID 12 1 0 f f t f i 3 1043 "2275 26 23" _null_ _null_ _null_ varcharin - _null_ ));
DESCR("I/O");
DATA(insert OID = 1047 (  varcharout	   PGNSP PGUID 12 1 0 f f t f i 1 2275 "1043" _null_ _null_ _null_	varcharout - _null_ ));
DESCR("I/O");
DATA(insert OID = 2915 (  varchartypmodin  PGNSP PGUID 12 1 0 f f t f i 1 23 "1263" _null_ _null_ _null_	varchartypmodin - _null_ ));
DESCR("I/O typmod");
DATA(insert OID = 2916 (  varchartypmodout PGNSP PGUID 12 1 0 f f t f i 1 2275 "23" _null_ _null_ _null_	varchartypmodout - _null_ ));
DESCR("I/O typmod");
DATA(insert OID = 1048 (  bpchareq		   PGNSP PGUID 12 1 0 f f t f i 2 16 "1042 1042" _null_ _null_ _null_ bpchareq - _null_ ));
DESCR("equal");
DATA(insert OID = 1049 (  bpcharlt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "1042 1042" _null_ _null_ _null_ bpcharlt - _null_ ));
DESCR("less-than");
DATA(insert OID = 1050 (  bpcharle		   PGNSP PGUID 12 1 0 f f t f i 2 16 "1042 1042" _null_ _null_ _null_ bpcharle - _null_ ));
DESCR("less-than-or-equal");
DATA(insert OID = 1051 (  bpchargt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "1042 1042" _null_ _null_ _null_ bpchargt - _null_ ));
DESCR("greater-than");
DATA(insert OID = 1052 (  bpcharge		   PGNSP PGUID 12 1 0 f f t f i 2 16 "1042 1042" _null_ _null_ _null_ bpcharge - _null_ ));
DESCR("greater-than-or-equal");
DATA(insert OID = 1053 (  bpcharne		   PGNSP PGUID 12 1 0 f f t f i 2 16 "1042 1042" _null_ _null_ _null_ bpcharne - _null_ ));
DESCR("not equal");
DATA(insert OID = 1063 (  bpchar_larger    PGNSP PGUID 12 1 0 f f t f i 2 1042 "1042 1042" _null_ _null_ _null_ bpchar_larger - _null_ ));
DESCR("larger of two");
DATA(insert OID = 1064 (  bpchar_smaller   PGNSP PGUID 12 1 0 f f t f i 2 1042 "1042 1042" _null_ _null_ _null_ bpchar_smaller - _null_ ));
DESCR("smaller of two");
DATA(insert OID = 1078 (  bpcharcmp		   PGNSP PGUID 12 1 0 f f t f i 2 23 "1042 1042" _null_ _null_ _null_ bpcharcmp - _null_ ));
DESCR("less-equal-greater");
DATA(insert OID = 1080 (  hashbpchar	   PGNSP PGUID 12 1 0 f f t f i 1 23 "1042" _null_ _null_ _null_	hashbpchar - _null_ ));
DESCR("hash");
DATA(insert OID = 1081 (  format_type	   PGNSP PGUID 12 1 0 f f f f s 2 25 "26 23" _null_ _null_ _null_ format_type - _null_ ));
DESCR("format a type oid and atttypmod to canonical SQL");
DATA(insert OID = 1084 (  date_in		   PGNSP PGUID 12 1 0 f f t f s 1 1082 "2275" _null_ _null_ _null_	date_in - _null_ ));
DESCR("I/O");
DATA(insert OID = 1085 (  date_out		   PGNSP PGUID 12 1 0 f f t f s 1 2275 "1082" _null_ _null_ _null_	date_out - _null_ ));
DESCR("I/O");
DATA(insert OID = 1086 (  date_eq		   PGNSP PGUID 12 1 0 f f t f i 2 16 "1082 1082" _null_ _null_ _null_ date_eq - _null_ ));
DESCR("equal");
DATA(insert OID = 1087 (  date_lt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "1082 1082" _null_ _null_ _null_ date_lt - _null_ ));
DESCR("less-than");
DATA(insert OID = 1088 (  date_le		   PGNSP PGUID 12 1 0 f f t f i 2 16 "1082 1082" _null_ _null_ _null_ date_le - _null_ ));
DESCR("less-than-or-equal");
DATA(insert OID = 1089 (  date_gt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "1082 1082" _null_ _null_ _null_ date_gt - _null_ ));
DESCR("greater-than");
DATA(insert OID = 1090 (  date_ge		   PGNSP PGUID 12 1 0 f f t f i 2 16 "1082 1082" _null_ _null_ _null_ date_ge - _null_ ));
DESCR("greater-than-or-equal");
DATA(insert OID = 1091 (  date_ne		   PGNSP PGUID 12 1 0 f f t f i 2 16 "1082 1082" _null_ _null_ _null_ date_ne - _null_ ));
DESCR("not equal");
DATA(insert OID = 1092 (  date_cmp		   PGNSP PGUID 12 1 0 f f t f i 2 23 "1082 1082" _null_ _null_ _null_ date_cmp - _null_ ));
DESCR("less-equal-greater");

/* OIDS 1100 - 1199 */

DATA(insert OID = 1102 (  time_lt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "1083 1083" _null_ _null_ _null_ time_lt - _null_ ));
DESCR("less-than");
DATA(insert OID = 1103 (  time_le		   PGNSP PGUID 12 1 0 f f t f i 2 16 "1083 1083" _null_ _null_ _null_ time_le - _null_ ));
DESCR("less-than-or-equal");
DATA(insert OID = 1104 (  time_gt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "1083 1083" _null_ _null_ _null_ time_gt - _null_ ));
DESCR("greater-than");
DATA(insert OID = 1105 (  time_ge		   PGNSP PGUID 12 1 0 f f t f i 2 16 "1083 1083" _null_ _null_ _null_ time_ge - _null_ ));
DESCR("greater-than-or-equal");
DATA(insert OID = 1106 (  time_ne		   PGNSP PGUID 12 1 0 f f t f i 2 16 "1083 1083" _null_ _null_ _null_ time_ne - _null_ ));
DESCR("not equal");
DATA(insert OID = 1107 (  time_cmp		   PGNSP PGUID 12 1 0 f f t f i 2 23 "1083 1083" _null_ _null_ _null_ time_cmp - _null_ ));
DESCR("less-equal-greater");
DATA(insert OID = 1138 (  date_larger	   PGNSP PGUID 12 1 0 f f t f i 2 1082 "1082 1082" _null_ _null_ _null_ date_larger - _null_ ));
DESCR("larger of two");
DATA(insert OID = 1139 (  date_smaller	   PGNSP PGUID 12 1 0 f f t f i 2 1082 "1082 1082" _null_ _null_ _null_ date_smaller - _null_ ));
DESCR("smaller of two");
DATA(insert OID = 1140 (  date_mi		   PGNSP PGUID 12 1 0 f f t f i 2 23 "1082 1082" _null_ _null_ _null_ date_mi - _null_ ));
DESCR("subtract");
DATA(insert OID = 1141 (  date_pli		   PGNSP PGUID 12 1 0 f f t f i 2 1082 "1082 23" _null_ _null_ _null_ date_pli - _null_ ));
DESCR("add");
DATA(insert OID = 1142 (  date_mii		   PGNSP PGUID 12 1 0 f f t f i 2 1082 "1082 23" _null_ _null_ _null_ date_mii - _null_ ));
DESCR("subtract");
DATA(insert OID = 1143 (  time_in		   PGNSP PGUID 12 1 0 f f t f s 3 1083 "2275 26 23" _null_ _null_ _null_ time_in - _null_ ));
DESCR("I/O");
DATA(insert OID = 1144 (  time_out		   PGNSP PGUID 12 1 0 f f t f i 1 2275 "1083" _null_ _null_ _null_	time_out - _null_ ));
DESCR("I/O");
DATA(insert OID = 2909 (  timetypmodin   	PGNSP PGUID 12 1 0 f f t f i 1 23 "1263" _null_ _null_ _null_	timetypmodin - _null_ ));
DESCR("I/O typmod");
DATA(insert OID = 2910 (  timetypmodout  	PGNSP PGUID 12 1 0 f f t f i 1 2275 "23" _null_ _null_ _null_	timetypmodout - _null_ ));
DESCR("I/O typmod");
DATA(insert OID = 1145 (  time_eq		   PGNSP PGUID 12 1 0 f f t f i 2 16 "1083 1083" _null_ _null_ _null_ time_eq - _null_ ));
DESCR("equal");

DATA(insert OID = 1146 (  circle_add_pt    PGNSP PGUID 12 1 0 f f t f i 2 718 "718 600" _null_ _null_ _null_	circle_add_pt - _null_ ));
DESCR("add");
DATA(insert OID = 1147 (  circle_sub_pt    PGNSP PGUID 12 1 0 f f t f i 2 718 "718 600" _null_ _null_ _null_	circle_sub_pt - _null_ ));
DESCR("subtract");
DATA(insert OID = 1148 (  circle_mul_pt    PGNSP PGUID 12 1 0 f f t f i 2 718 "718 600" _null_ _null_ _null_	circle_mul_pt - _null_ ));
DESCR("multiply");
DATA(insert OID = 1149 (  circle_div_pt    PGNSP PGUID 12 1 0 f f t f i 2 718 "718 600" _null_ _null_ _null_	circle_div_pt - _null_ ));
DESCR("divide");

DATA(insert OID = 1150 (  timestamptz_in   PGNSP PGUID 12 1 0 f f t f s 3 1184 "2275 26 23" _null_ _null_ _null_ timestamptz_in - _null_ ));
DESCR("I/O");
DATA(insert OID = 1151 (  timestamptz_out  PGNSP PGUID 12 1 0 f f t f s 1 2275 "1184" _null_ _null_ _null_	timestamptz_out - _null_ ));
DESCR("I/O");
DATA(insert OID = 2907 (  timestamptztypmodin   	PGNSP PGUID 12 1 0 f f t f i 1 23 "1263" _null_ _null_ _null_	timestamptztypmodin - _null_ ));
DESCR("I/O typmod");
DATA(insert OID = 2908 (  timestamptztypmodout  	PGNSP PGUID 12 1 0 f f t f i 1 2275 "23" _null_ _null_ _null_	timestamptztypmodout - _null_ ));
DESCR("I/O typmod");
DATA(insert OID = 1152 (  timestamptz_eq   PGNSP PGUID 12 1 0 f f t f i 2 16 "1184 1184" _null_ _null_ _null_ timestamp_eq - _null_ ));
DESCR("equal");
DATA(insert OID = 1153 (  timestamptz_ne   PGNSP PGUID 12 1 0 f f t f i 2 16 "1184 1184" _null_ _null_ _null_ timestamp_ne - _null_ ));
DESCR("not equal");
DATA(insert OID = 1154 (  timestamptz_lt   PGNSP PGUID 12 1 0 f f t f i 2 16 "1184 1184" _null_ _null_ _null_ timestamp_lt - _null_ ));
DESCR("less-than");
DATA(insert OID = 1155 (  timestamptz_le   PGNSP PGUID 12 1 0 f f t f i 2 16 "1184 1184" _null_ _null_ _null_ timestamp_le - _null_ ));
DESCR("less-than-or-equal");
DATA(insert OID = 1156 (  timestamptz_ge   PGNSP PGUID 12 1 0 f f t f i 2 16 "1184 1184" _null_ _null_ _null_ timestamp_ge - _null_ ));
DESCR("greater-than-or-equal");
DATA(insert OID = 1157 (  timestamptz_gt   PGNSP PGUID 12 1 0 f f t f i 2 16 "1184 1184" _null_ _null_ _null_ timestamp_gt - _null_ ));
DESCR("greater-than");
DATA(insert OID = 1158 (  to_timestamp	   PGNSP PGUID 14 1 0 f f t f i 1 1184 "701" _null_ _null_ _null_ "select (''epoch''::pg_catalog.timestamptz + $1 * ''1 second''::pg_catalog.interval)" - _null_ ));
DESCR("convert UNIX epoch to timestamptz");
DATA(insert OID = 1159 (  timezone		   PGNSP PGUID 12 1 0 f f t f i 2 1114 "25 1184" _null_ _null_ _null_  timestamptz_zone - _null_ ));
DESCR("adjust timestamp to new time zone");

DATA(insert OID = 1160 (  interval_in	   PGNSP PGUID 12 1 0 f f t f s 3 1186 "2275 26 23" _null_ _null_ _null_ interval_in - _null_ ));
DESCR("I/O");
DATA(insert OID = 1161 (  interval_out	   PGNSP PGUID 12 1 0 f f t f i 1 2275 "1186" _null_ _null_ _null_	interval_out - _null_ ));
DESCR("I/O");
DATA(insert OID = 2903 (  intervaltypmodin   	PGNSP PGUID 12 1 0 f f t f i 1 23 "1263" _null_ _null_ _null_	intervaltypmodin - _null_ ));
DESCR("I/O typmod");
DATA(insert OID = 2904 (  intervaltypmodout  	PGNSP PGUID 12 1 0 f f t f i 1 2275 "23" _null_ _null_ _null_	intervaltypmodout - _null_ ));
DESCR("I/O typmod");
DATA(insert OID = 1162 (  interval_eq	   PGNSP PGUID 12 1 0 f f t f i 2 16 "1186 1186" _null_ _null_ _null_ interval_eq - _null_ ));
DESCR("equal");
DATA(insert OID = 1163 (  interval_ne	   PGNSP PGUID 12 1 0 f f t f i 2 16 "1186 1186" _null_ _null_ _null_ interval_ne - _null_ ));
DESCR("not equal");
DATA(insert OID = 1164 (  interval_lt	   PGNSP PGUID 12 1 0 f f t f i 2 16 "1186 1186" _null_ _null_ _null_ interval_lt - _null_ ));
DESCR("less-than");
DATA(insert OID = 1165 (  interval_le	   PGNSP PGUID 12 1 0 f f t f i 2 16 "1186 1186" _null_ _null_ _null_ interval_le - _null_ ));
DESCR("less-than-or-equal");
DATA(insert OID = 1166 (  interval_ge	   PGNSP PGUID 12 1 0 f f t f i 2 16 "1186 1186" _null_ _null_ _null_ interval_ge - _null_ ));
DESCR("greater-than-or-equal");
DATA(insert OID = 1167 (  interval_gt	   PGNSP PGUID 12 1 0 f f t f i 2 16 "1186 1186" _null_ _null_ _null_ interval_gt - _null_ ));
DESCR("greater-than");
DATA(insert OID = 1168 (  interval_um	   PGNSP PGUID 12 1 0 f f t f i 1 1186 "1186" _null_ _null_ _null_	interval_um - _null_ ));
DESCR("subtract");
DATA(insert OID = 1169 (  interval_pl	   PGNSP PGUID 12 1 0 f f t f i 2 1186 "1186 1186" _null_ _null_ _null_ interval_pl - _null_ ));
DESCR("add");
DATA(insert OID = 1170 (  interval_mi	   PGNSP PGUID 12 1 0 f f t f i 2 1186 "1186 1186" _null_ _null_ _null_ interval_mi - _null_ ));
DESCR("subtract");
DATA(insert OID = 1171 (  date_part		   PGNSP PGUID 12 1 0 f f t f s 2  701 "25 1184" _null_ _null_ _null_ timestamptz_part - _null_ ));
DESCR("extract field from timestamp with time zone");
DATA(insert OID = 1172 (  date_part		   PGNSP PGUID 12 1 0 f f t f i 2  701 "25 1186" _null_ _null_ _null_ interval_part - _null_ ));
DESCR("extract field from interval");
DATA(insert OID = 1173 (  timestamptz	   PGNSP PGUID 12 1 0 f f t f i 1 1184 "702" _null_ _null_ _null_ abstime_timestamptz - _null_ ));
DESCR("convert abstime to timestamp with time zone");
DATA(insert OID = 1174 (  timestamptz	   PGNSP PGUID 12 1 0 f f t f s 1 1184 "1082" _null_ _null_ _null_	date_timestamptz - _null_ ));
DESCR("convert date to timestamp with time zone");
DATA(insert OID = 2711 (  justify_interval PGNSP PGUID 12 1 0 f f t f i 1 1186 "1186" _null_ _null_ _null_	interval_justify_interval - _null_ ));
DESCR("promote groups of 24 hours to numbers of days and promote groups of 30 days to numbers of months");
DATA(insert OID = 1175 (  justify_hours    PGNSP PGUID 12 1 0 f f t f i 1 1186 "1186" _null_ _null_ _null_	interval_justify_hours - _null_ ));
DESCR("promote groups of 24 hours to numbers of days");
DATA(insert OID = 1295 (  justify_days	   PGNSP PGUID 12 1 0 f f t f i 1 1186 "1186" _null_ _null_ _null_	interval_justify_days - _null_ ));
DESCR("promote groups of 30 days to numbers of months");
DATA(insert OID = 1176 (  timestamptz	   PGNSP PGUID 14 1 0 f f t f s 2 1184 "1082 1083" _null_ _null_ _null_ "select cast(($1 + $2) as timestamp with time zone)" - _null_ ));
DESCR("convert date and time to timestamp with time zone");
DATA(insert OID = 1177 (  interval		   PGNSP PGUID 12 1 0 f f t f i 1 1186 "703" _null_ _null_ _null_ reltime_interval - _null_ ));
DESCR("convert reltime to interval");
DATA(insert OID = 1178 (  date			   PGNSP PGUID 12 1 0 f f t f s 1 1082 "1184" _null_ _null_ _null_	timestamptz_date - _null_ ));
DESCR("convert timestamp with time zone to date");
DATA(insert OID = 1179 (  date			   PGNSP PGUID 12 1 0 f f t f s 1 1082 "702" _null_ _null_ _null_ abstime_date - _null_ ));
DESCR("convert abstime to date");
DATA(insert OID = 1180 (  abstime		   PGNSP PGUID 12 1 0 f f t f i 1  702 "1184" _null_ _null_ _null_	timestamptz_abstime - _null_ ));
DESCR("convert timestamp with time zone to abstime");
DATA(insert OID = 1181 (  age			   PGNSP PGUID 12 1 0 f f t f s 1 23 "28" _null_ _null_ _null_	xid_age - _null_ ));
DESCR("age of a transaction ID, in transactions before current transaction");

DATA(insert OID = 1188 (  timestamptz_mi   PGNSP PGUID 12 1 0 f f t f i 2 1186 "1184 1184" _null_ _null_ _null_ timestamp_mi - _null_ ));
DESCR("subtract");
DATA(insert OID = 1189 (  timestamptz_pl_interval PGNSP PGUID 12 1 0 f f t f s 2 1184 "1184 1186" _null_ _null_ _null_	timestamptz_pl_interval - _null_ ));
DESCR("plus");
DATA(insert OID = 1190 (  timestamptz_mi_interval PGNSP PGUID 12 1 0 f f t f s 2 1184 "1184 1186" _null_ _null_ _null_	timestamptz_mi_interval - _null_ ));
DESCR("minus");
DATA(insert OID = 1194 (  reltime			PGNSP PGUID 12 1 0 f f t f i 1	703 "1186" _null_ _null_ _null_ interval_reltime - _null_ ));
DESCR("convert interval to reltime");
DATA(insert OID = 1195 (  timestamptz_smaller PGNSP PGUID 12 1 0 f f t f i 2 1184 "1184 1184" _null_ _null_ _null_	timestamp_smaller - _null_ ));
DESCR("smaller of two");
DATA(insert OID = 1196 (  timestamptz_larger  PGNSP PGUID 12 1 0 f f t f i 2 1184 "1184 1184" _null_ _null_ _null_	timestamp_larger - _null_ ));
DESCR("larger of two");
DATA(insert OID = 1197 (  interval_smaller	PGNSP PGUID 12 1 0 f f t f i 2 1186 "1186 1186" _null_ _null_ _null_	interval_smaller - _null_ ));
DESCR("smaller of two");
DATA(insert OID = 1198 (  interval_larger	PGNSP PGUID 12 1 0 f f t f i 2 1186 "1186 1186" _null_ _null_ _null_	interval_larger - _null_ ));
DESCR("larger of two");
DATA(insert OID = 1199 (  age				PGNSP PGUID 12 1 0 f f t f i 2 1186 "1184 1184" _null_ _null_ _null_	timestamptz_age - _null_ ));
DESCR("date difference preserving months and years");

/* OIDS 1200 - 1299 */

DATA(insert OID = 1200 (  interval			PGNSP PGUID 12 1 0 f f t f i 2 1186 "1186 23" _null_ _null_ _null_	interval_scale - _null_ ));
DESCR("adjust interval precision");

DATA(insert OID = 1215 (  obj_description	PGNSP PGUID 14 100 0 f f t f s 2	25 "26 19" _null_ _null_ _null_ "select description from pg_catalog.pg_description where objoid = $1 and classoid = (select oid from pg_catalog.pg_class where relname = $2 and relnamespace = PGNSP) and objsubid = 0" - _null_ ));
DESCR("get description for object id and catalog name");
DATA(insert OID = 1216 (  col_description	PGNSP PGUID 14 100 0 f f t f s 2	25 "26 23" _null_ _null_ _null_ "select description from pg_catalog.pg_description where objoid = $1 and classoid = ''pg_catalog.pg_class''::pg_catalog.regclass and objsubid = $2" - _null_ ));
DESCR("get description for table column");
DATA(insert OID = 1993 ( shobj_description	PGNSP PGUID 14 100 0 f f t f s 2	25 "26 19" _null_ _null_ _null_ "select description from pg_catalog.pg_shdescription where objoid = $1 and classoid = (select oid from pg_catalog.pg_class where relname = $2 and relnamespace = PGNSP)" - _null_ ));
DESCR("get description for object id and shared catalog name");

DATA(insert OID = 1217 (  date_trunc	   PGNSP PGUID 12 1 0 f f t f s 2 1184 "25 1184" _null_ _null_ _null_ timestamptz_trunc - _null_ ));
DESCR("truncate timestamp with time zone to specified units");
DATA(insert OID = 1218 (  date_trunc	   PGNSP PGUID 12 1 0 f f t f i 2 1186 "25 1186" _null_ _null_ _null_ interval_trunc - _null_ ));
DESCR("truncate interval to specified units");

DATA(insert OID = 1219 (  int8inc		   PGNSP PGUID 12 1 0 f f t f i 1 20 "20" _null_ _null_ _null_	int8inc - _null_ ));
DESCR("increment");
DATA(insert OID = 2804 (  int8inc_any	   PGNSP PGUID 12 1 0 f f t f i 2 20 "20 2276" _null_ _null_ _null_ int8inc_any - _null_ ));
DESCR("increment, ignores second argument");
DATA(insert OID = 1230 (  int8abs		   PGNSP PGUID 12 1 0 f f t f i 1 20 "20" _null_ _null_ _null_	int8abs - _null_ ));
DESCR("absolute value");

DATA(insert OID = 1236 (  int8larger	   PGNSP PGUID 12 1 0 f f t f i 2 20 "20 20" _null_ _null_ _null_ int8larger - _null_ ));
DESCR("larger of two");
DATA(insert OID = 1237 (  int8smaller	   PGNSP PGUID 12 1 0 f f t f i 2 20 "20 20" _null_ _null_ _null_ int8smaller - _null_ ));
DESCR("smaller of two");

DATA(insert OID = 1238 (  texticregexeq    PGNSP PGUID 12 1 0 f f t f i 2 16 "25 25" _null_ _null_ _null_ texticregexeq - _null_ ));
DESCR("matches regex., case-insensitive");
DATA(insert OID = 1239 (  texticregexne    PGNSP PGUID 12 1 0 f f t f i 2 16 "25 25" _null_ _null_ _null_ texticregexne - _null_ ));
DESCR("does not match regex., case-insensitive");
DATA(insert OID = 1240 (  nameicregexeq    PGNSP PGUID 12 1 0 f f t f i 2 16 "19 25" _null_ _null_ _null_ nameicregexeq - _null_ ));
DESCR("matches regex., case-insensitive");
DATA(insert OID = 1241 (  nameicregexne    PGNSP PGUID 12 1 0 f f t f i 2 16 "19 25" _null_ _null_ _null_ nameicregexne - _null_ ));
DESCR("does not match regex., case-insensitive");

DATA(insert OID = 1251 (  int4abs		   PGNSP PGUID 12 1 0 f f t f i 1 23 "23" _null_ _null_ _null_	int4abs - _null_ ));
DESCR("absolute value");
DATA(insert OID = 1253 (  int2abs		   PGNSP PGUID 12 1 0 f f t f i 1 21 "21" _null_ _null_ _null_	int2abs - _null_ ));
DESCR("absolute value");

DATA(insert OID = 1271 (  overlaps		   PGNSP PGUID 12 1 0 f f f f i 4 16 "1266 1266 1266 1266" _null_ _null_ _null_ overlaps_timetz - _null_ ));
DESCR("SQL92 interval comparison");
DATA(insert OID = 1272 (  datetime_pl	   PGNSP PGUID 12 1 0 f f t f i 2 1114 "1082 1083" _null_ _null_ _null_ datetime_timestamp - _null_ ));
DESCR("convert date and time to timestamp");
DATA(insert OID = 1273 (  date_part		   PGNSP PGUID 12 1 0 f f t f i 2  701 "25 1266" _null_ _null_ _null_ timetz_part - _null_ ));
DESCR("extract field from time with time zone");
DATA(insert OID = 1274 (  int84pl		   PGNSP PGUID 12 1 0 f f t f i 2 20 "20 23" _null_ _null_ _null_ int84pl - _null_ ));
DESCR("add");
DATA(insert OID = 1275 (  int84mi		   PGNSP PGUID 12 1 0 f f t f i 2 20 "20 23" _null_ _null_ _null_ int84mi - _null_ ));
DESCR("subtract");
DATA(insert OID = 1276 (  int84mul		   PGNSP PGUID 12 1 0 f f t f i 2 20 "20 23" _null_ _null_ _null_ int84mul - _null_ ));
DESCR("multiply");
DATA(insert OID = 1277 (  int84div		   PGNSP PGUID 12 1 0 f f t f i 2 20 "20 23" _null_ _null_ _null_ int84div - _null_ ));
DESCR("divide");
DATA(insert OID = 1278 (  int48pl		   PGNSP PGUID 12 1 0 f f t f i 2 20 "23 20" _null_ _null_ _null_ int48pl - _null_ ));
DESCR("add");
DATA(insert OID = 1279 (  int48mi		   PGNSP PGUID 12 1 0 f f t f i 2 20 "23 20" _null_ _null_ _null_ int48mi - _null_ ));
DESCR("subtract");
DATA(insert OID = 1280 (  int48mul		   PGNSP PGUID 12 1 0 f f t f i 2 20 "23 20" _null_ _null_ _null_ int48mul - _null_ ));
DESCR("multiply");
DATA(insert OID = 1281 (  int48div		   PGNSP PGUID 12 1 0 f f t f i 2 20 "23 20" _null_ _null_ _null_ int48div - _null_ ));
DESCR("divide");

DATA(insert OID = 1287 (  oid			   PGNSP PGUID 12 1 0 f f t f i 1 26 "20" _null_ _null_ _null_	i8tooid - _null_ ));
DESCR("convert int8 to oid");
DATA(insert OID = 1288 (  int8			   PGNSP PGUID 12 1 0 f f t f i 1 20 "26" _null_ _null_ _null_	oidtoi8 - _null_ ));
DESCR("convert oid to int8");

DATA(insert OID = 1292 ( tideq			   PGNSP PGUID 12 1 0 f f t f i 2 16 "27 27" _null_ _null_ _null_ tideq - _null_ ));
DESCR("equal");
DATA(insert OID = 1293 ( currtid		   PGNSP PGUID 12 1 0 f f t f v 2 27 "26 27" _null_ _null_ _null_ currtid_byreloid - _null_ ));
DESCR("latest tid of a tuple");
DATA(insert OID = 1294 ( currtid2		   PGNSP PGUID 12 1 0 f f t f v 2 27 "25 27" _null_ _null_ _null_ currtid_byrelname - _null_ ));
DESCR("latest tid of a tuple");
DATA(insert OID = 1265 ( tidne			   PGNSP PGUID 12 1 0 f f t f i 2 16 "27 27" _null_ _null_ _null_ tidne - _null_ ));
DESCR("not equal");
DATA(insert OID = 2790 ( tidgt			   PGNSP PGUID 12 1 0 f f t f i 2 16 "27 27" _null_ _null_ _null_ tidgt - _null_));
DESCR("greater-than");
DATA(insert OID = 2791 ( tidlt			   PGNSP PGUID 12 1 0 f f t f i 2 16 "27 27" _null_ _null_ _null_ tidlt - _null_));
DESCR("less-than");
DATA(insert OID = 2792 ( tidge			   PGNSP PGUID 12 1 0 f f t f i 2 16 "27 27" _null_ _null_ _null_ tidge - _null_));
DESCR("greater-than-or-equal");
DATA(insert OID = 2793 ( tidle			   PGNSP PGUID 12 1 0 f f t f i 2 16 "27 27" _null_ _null_ _null_ tidle - _null_));
DESCR("less-than-or-equal");
DATA(insert OID = 2794 ( bttidcmp		   PGNSP PGUID 12 1 0 f f t f i 2 23 "27 27" _null_ _null_ _null_ bttidcmp - _null_));
DESCR("btree less-equal-greater");
DATA(insert OID = 2795 ( tidlarger		   PGNSP PGUID 12 1 0 f f t f i 2 27 "27 27" _null_ _null_ _null_ tidlarger - _null_ ));
DESCR("larger of two");
DATA(insert OID = 2796 ( tidsmaller		   PGNSP PGUID 12 1 0 f f t f i 2 27 "27 27" _null_ _null_ _null_ tidsmaller - _null_ ));
DESCR("smaller of two");

DATA(insert OID = 1296 (  timedate_pl	   PGNSP PGUID 14 1 0 f f t f i 2 1114 "1083 1082" _null_ _null_ _null_ "select ($2 + $1)" - _null_ ));
DESCR("convert time and date to timestamp");
DATA(insert OID = 1297 (  datetimetz_pl    PGNSP PGUID 12 1 0 f f t f i 2 1184 "1082 1266" _null_ _null_ _null_ datetimetz_timestamptz - _null_ ));
DESCR("convert date and time with time zone to timestamp with time zone");
DATA(insert OID = 1298 (  timetzdate_pl    PGNSP PGUID 14 1 0 f f t f i 2 1184 "1266 1082" _null_ _null_ _null_ "select ($2 + $1)" - _null_ ));
DESCR("convert time with time zone and date to timestamp with time zone");
DATA(insert OID = 1299 (  now			   PGNSP PGUID 12 1 0 f f t f s 0 1184 "" _null_ _null_ _null_	now - _null_ ));
DESCR("current transaction time");
DATA(insert OID = 2647 (  transaction_timestamp PGNSP PGUID 12 1 0 f f t f s 0 1184 "" _null_ _null_ _null_ now - _null_ ));
DESCR("current transaction time");
DATA(insert OID = 2648 (  statement_timestamp	PGNSP PGUID 12 1 0 f f t f s 0 1184 "" _null_ _null_ _null_ statement_timestamp - _null_ ));
DESCR("current statement time");
DATA(insert OID = 2649 (  clock_timestamp	PGNSP PGUID 12 1 0 f f t f v 0 1184 "" _null_ _null_ _null_ clock_timestamp - _null_ ));
DESCR("current clock time");

/* OIDS 1300 - 1399 */

DATA(insert OID = 1300 (  positionsel		   PGNSP PGUID 12 1 0 f f t f s 4 701 "2281 26 2281 23" _null_ _null_ _null_	positionsel - _null_ ));
DESCR("restriction selectivity for position-comparison operators");
DATA(insert OID = 1301 (  positionjoinsel	   PGNSP PGUID 12 1 0 f f t f s 4 701 "2281 26 2281 21" _null_ _null_ _null_	positionjoinsel - _null_ ));
DESCR("join selectivity for position-comparison operators");
DATA(insert OID = 1302 (  contsel		   PGNSP PGUID 12 1 0 f f t f s 4 701 "2281 26 2281 23" _null_ _null_ _null_	contsel - _null_ ));
DESCR("restriction selectivity for containment comparison operators");
DATA(insert OID = 1303 (  contjoinsel	   PGNSP PGUID 12 1 0 f f t f s 4 701 "2281 26 2281 21" _null_ _null_ _null_	contjoinsel - _null_ ));
DESCR("join selectivity for containment comparison operators");

DATA(insert OID = 1304 ( overlaps			 PGNSP PGUID 12 1 0 f f f f i 4 16 "1184 1184 1184 1184" _null_ _null_ _null_ overlaps_timestamp - _null_ ));
DESCR("SQL92 interval comparison");
DATA(insert OID = 1305 ( overlaps			 PGNSP PGUID 14 1 0 f f f f s 4 16 "1184 1186 1184 1186" _null_ _null_ _null_ "select ($1, ($1 + $2)) overlaps ($3, ($3 + $4))" - _null_ ));
DESCR("SQL92 interval comparison");
DATA(insert OID = 1306 ( overlaps			 PGNSP PGUID 14 1 0 f f f f s 4 16 "1184 1184 1184 1186" _null_ _null_ _null_ "select ($1, $2) overlaps ($3, ($3 + $4))" - _null_ ));
DESCR("SQL92 interval comparison");
DATA(insert OID = 1307 ( overlaps			 PGNSP PGUID 14 1 0 f f f f s 4 16 "1184 1186 1184 1184" _null_ _null_ _null_ "select ($1, ($1 + $2)) overlaps ($3, $4)" - _null_ ));
DESCR("SQL92 interval comparison");

DATA(insert OID = 1308 ( overlaps			 PGNSP PGUID 12 1 0 f f f f i 4 16 "1083 1083 1083 1083" _null_ _null_ _null_ overlaps_time - _null_ ));
DESCR("SQL92 interval comparison");
DATA(insert OID = 1309 ( overlaps			 PGNSP PGUID 14 1 0 f f f f i 4 16 "1083 1186 1083 1186" _null_ _null_ _null_ "select ($1, ($1 + $2)) overlaps ($3, ($3 + $4))" - _null_ ));
DESCR("SQL92 interval comparison");
DATA(insert OID = 1310 ( overlaps			 PGNSP PGUID 14 1 0 f f f f i 4 16 "1083 1083 1083 1186" _null_ _null_ _null_ "select ($1, $2) overlaps ($3, ($3 + $4))" - _null_ ));
DESCR("SQL92 interval comparison");
DATA(insert OID = 1311 ( overlaps			 PGNSP PGUID 14 1 0 f f f f i 4 16 "1083 1186 1083 1083" _null_ _null_ _null_ "select ($1, ($1 + $2)) overlaps ($3, $4)" - _null_ ));
DESCR("SQL92 interval comparison");

DATA(insert OID = 1312 (  timestamp_in		 PGNSP PGUID 12 1 0 f f t f s 3 1114 "2275 26 23" _null_ _null_ _null_ timestamp_in - _null_ ));
DESCR("I/O");
DATA(insert OID = 1313 (  timestamp_out		 PGNSP PGUID 12 1 0 f f t f s 1 2275 "1114" _null_ _null_ _null_ timestamp_out - _null_ ));
DESCR("I/O");
DATA(insert OID = 2905 (  timestamptypmodin   	PGNSP PGUID 12 1 0 f f t f i 1 23 "1263" _null_ _null_ _null_	timestamptypmodin - _null_ ));
DESCR("I/O typmod");
DATA(insert OID = 2906 (  timestamptypmodout  	PGNSP PGUID 12 1 0 f f t f i 1 2275 "23" _null_ _null_ _null_	timestamptypmodout - _null_ ));
DESCR("I/O typmod");
DATA(insert OID = 1314 (  timestamptz_cmp	 PGNSP PGUID 12 1 0 f f t f i 2 23 "1184 1184" _null_ _null_ _null_ timestamp_cmp - _null_ ));
DESCR("less-equal-greater");
DATA(insert OID = 1315 (  interval_cmp		 PGNSP PGUID 12 1 0 f f t f i 2 23 "1186 1186" _null_ _null_ _null_ interval_cmp - _null_ ));
DESCR("less-equal-greater");
DATA(insert OID = 1316 (  time				 PGNSP PGUID 12 1 0 f f t f i 1 1083 "1114" _null_ _null_ _null_	timestamp_time - _null_ ));
DESCR("convert timestamp to time");

DATA(insert OID = 1317 (  length			 PGNSP PGUID 12 1 0 f f t f i 1 23 "25" _null_ _null_ _null_	textlen - _null_ ));
DESCR("length");
DATA(insert OID = 1318 (  length			 PGNSP PGUID 12 1 0 f f t f i 1 23 "1042" _null_ _null_ _null_	bpcharlen - _null_ ));
DESCR("character length");

DATA(insert OID = 1319 (  xideqint4			 PGNSP PGUID 12 1 0 f f t f i 2 16 "28 23" _null_ _null_ _null_ xideq - _null_ ));
DESCR("equal");

DATA(insert OID = 1326 (  interval_div		 PGNSP PGUID 12 1 0 f f t f i 2 1186 "1186 701" _null_ _null_ _null_	interval_div - _null_ ));
DESCR("divide");

DATA(insert OID = 1339 (  dlog10			 PGNSP PGUID 12 1 0 f f t f i 1 701 "701" _null_ _null_ _null_	dlog10 - _null_ ));
DESCR("base 10 logarithm");
DATA(insert OID = 1340 (  log				 PGNSP PGUID 12 1 0 f f t f i 1 701 "701" _null_ _null_ _null_	dlog10 - _null_ ));
DESCR("base 10 logarithm");
DATA(insert OID = 1341 (  ln				 PGNSP PGUID 12 1 0 f f t f i 1 701 "701" _null_ _null_ _null_	dlog1 - _null_ ));
DESCR("natural logarithm");
DATA(insert OID = 1342 (  round				 PGNSP PGUID 12 1 0 f f t f i 1 701 "701" _null_ _null_ _null_	dround - _null_ ));
DESCR("round to nearest integer");
DATA(insert OID = 1343 (  trunc				 PGNSP PGUID 12 1 0 f f t f i 1 701 "701" _null_ _null_ _null_	dtrunc - _null_ ));
DESCR("truncate to integer");
DATA(insert OID = 1344 (  sqrt				 PGNSP PGUID 12 1 0 f f t f i 1 701 "701" _null_ _null_ _null_	dsqrt - _null_ ));
DESCR("square root");
DATA(insert OID = 1345 (  cbrt				 PGNSP PGUID 12 1 0 f f t f i 1 701 "701" _null_ _null_ _null_	dcbrt - _null_ ));
DESCR("cube root");
DATA(insert OID = 1346 (  pow				 PGNSP PGUID 12 1 0 f f t f i 2 701 "701 701" _null_ _null_ _null_	dpow - _null_ ));
DESCR("exponentiation");
DATA(insert OID = 1368 (  power				 PGNSP PGUID 12 1 0 f f t f i 2 701 "701 701" _null_ _null_ _null_	dpow - _null_ ));
DESCR("exponentiation");
DATA(insert OID = 1347 (  exp				 PGNSP PGUID 12 1 0 f f t f i 1 701 "701" _null_ _null_ _null_	dexp - _null_ ));
DESCR("exponential");

/*
 * This form of obj_description is now deprecated, since it will fail if
 * OIDs are not unique across system catalogs.	Use the other forms instead.
 */
DATA(insert OID = 1348 (  obj_description	 PGNSP PGUID 14 100 0 f f t f s 1 25 "26" _null_ _null_ _null_	"select description from pg_catalog.pg_description where objoid = $1 and objsubid = 0" - _null_ ));
DESCR("get description for object id (deprecated)");
DATA(insert OID = 1349 (  oidvectortypes	 PGNSP PGUID 12 1 0 f f t f s 1 25 "30" _null_ _null_ _null_	oidvectortypes - _null_ ));
DESCR("print type names of oidvector field");


DATA(insert OID = 1350 (  timetz_in		   PGNSP PGUID 12 1 0 f f t f s 3 1266 "2275 26 23" _null_ _null_ _null_ timetz_in - _null_ ));
DESCR("I/O");
DATA(insert OID = 1351 (  timetz_out	   PGNSP PGUID 12 1 0 f f t f i 1 2275 "1266" _null_ _null_ _null_	timetz_out - _null_ ));
DESCR("I/O");
DATA(insert OID = 2911 (  timetztypmodin   	PGNSP PGUID 12 1 0 f f t f i 1 23 "1263" _null_ _null_ _null_	timetztypmodin - _null_ ));
DESCR("I/O typmod");
DATA(insert OID = 2912 (  timetztypmodout  	PGNSP PGUID 12 1 0 f f t f i 1 2275 "23" _null_ _null_ _null_	timetztypmodout - _null_ ));
DESCR("I/O typmod");
DATA(insert OID = 1352 (  timetz_eq		   PGNSP PGUID 12 1 0 f f t f i 2 16 "1266 1266" _null_ _null_ _null_ timetz_eq - _null_ ));
DESCR("equal");
DATA(insert OID = 1353 (  timetz_ne		   PGNSP PGUID 12 1 0 f f t f i 2 16 "1266 1266" _null_ _null_ _null_ timetz_ne - _null_ ));
DESCR("not equal");
DATA(insert OID = 1354 (  timetz_lt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "1266 1266" _null_ _null_ _null_ timetz_lt - _null_ ));
DESCR("less-than");
DATA(insert OID = 1355 (  timetz_le		   PGNSP PGUID 12 1 0 f f t f i 2 16 "1266 1266" _null_ _null_ _null_ timetz_le - _null_ ));
DESCR("less-than-or-equal");
DATA(insert OID = 1356 (  timetz_ge		   PGNSP PGUID 12 1 0 f f t f i 2 16 "1266 1266" _null_ _null_ _null_ timetz_ge - _null_ ));
DESCR("greater-than-or-equal");
DATA(insert OID = 1357 (  timetz_gt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "1266 1266" _null_ _null_ _null_ timetz_gt - _null_ ));
DESCR("greater-than");
DATA(insert OID = 1358 (  timetz_cmp	   PGNSP PGUID 12 1 0 f f t f i 2 23 "1266 1266" _null_ _null_ _null_ timetz_cmp - _null_ ));
DESCR("less-equal-greater");
DATA(insert OID = 1359 (  timestamptz	   PGNSP PGUID 12 1 0 f f t f i 2 1184 "1082 1266" _null_ _null_ _null_ datetimetz_timestamptz - _null_ ));
DESCR("convert date and time with time zone to timestamp with time zone");

DATA(insert OID = 1364 (  time			   PGNSP PGUID 14 1 0 f f t f s 1 1083 "702" _null_ _null_ _null_  "select cast(cast($1 as timestamp without time zone) as pg_catalog.time)" - _null_ ));
DESCR("convert abstime to time");

DATA(insert OID = 1367 (  character_length	PGNSP PGUID 12 1 0 f f t f i 1	23 "1042" _null_ _null_ _null_	bpcharlen - _null_ ));
DESCR("character length");
DATA(insert OID = 1369 (  character_length	PGNSP PGUID 12 1 0 f f t f i 1	23 "25" _null_ _null_ _null_	textlen - _null_ ));
DESCR("character length");

DATA(insert OID = 1370 (  interval			 PGNSP PGUID 12 1 0 f f t f i 1 1186 "1083" _null_ _null_ _null_	time_interval - _null_ ));
DESCR("convert time to interval");
DATA(insert OID = 1372 (  char_length		 PGNSP PGUID 12 1 0 f f t f i 1 23	 "1042" _null_ _null_ _null_	bpcharlen - _null_ ));
DESCR("character length");
DATA(insert OID = 1374 (  octet_length			 PGNSP PGUID 12 1 0 f f t f i 1 23	 "25" _null_ _null_ _null_	textoctetlen - _null_ ));
DESCR("octet length");
DATA(insert OID = 1375 (  octet_length			 PGNSP PGUID 12 1 0 f f t f i 1 23	 "1042" _null_ _null_ _null_	bpcharoctetlen - _null_ ));
DESCR("octet length");

DATA(insert OID = 1377 (  time_larger	   PGNSP PGUID 12 1 0 f f t f i 2 1083 "1083 1083" _null_ _null_ _null_ time_larger - _null_ ));
DESCR("larger of two");
DATA(insert OID = 1378 (  time_smaller	   PGNSP PGUID 12 1 0 f f t f i 2 1083 "1083 1083" _null_ _null_ _null_ time_smaller - _null_ ));
DESCR("smaller of two");
DATA(insert OID = 1379 (  timetz_larger    PGNSP PGUID 12 1 0 f f t f i 2 1266 "1266 1266" _null_ _null_ _null_ timetz_larger - _null_ ));
DESCR("larger of two");
DATA(insert OID = 1380 (  timetz_smaller   PGNSP PGUID 12 1 0 f f t f i 2 1266 "1266 1266" _null_ _null_ _null_ timetz_smaller - _null_ ));
DESCR("smaller of two");

DATA(insert OID = 1381 (  char_length	   PGNSP PGUID 12 1 0 f f t f i 1 23 "25" _null_ _null_ _null_	textlen - _null_ ));
DESCR("character length");

DATA(insert OID = 1382 (  date_part    PGNSP PGUID 14 1 0 f f t f s 2  701 "25 702" _null_ _null_ _null_	"select pg_catalog.date_part($1, cast($2 as timestamp with time zone))" - _null_ ));
DESCR("extract field from abstime");
DATA(insert OID = 1383 (  date_part    PGNSP PGUID 14 1 0 f f t f s 2  701 "25 703" _null_ _null_ _null_	"select pg_catalog.date_part($1, cast($2 as pg_catalog.interval))" - _null_ ));
DESCR("extract field from reltime");
DATA(insert OID = 1384 (  date_part    PGNSP PGUID 14 1 0 f f t f i 2  701 "25 1082" _null_ _null_ _null_ "select pg_catalog.date_part($1, cast($2 as timestamp without time zone))" - _null_ ));
DESCR("extract field from date");
DATA(insert OID = 1385 (  date_part    PGNSP PGUID 12 1 0 f f t f i 2  701 "25 1083" _null_ _null_ _null_  time_part - _null_ ));
DESCR("extract field from time");
DATA(insert OID = 1386 (  age		   PGNSP PGUID 14 1 0 f f t f s 1 1186 "1184" _null_ _null_ _null_	"select pg_catalog.age(cast(current_date as timestamp with time zone), $1)" - _null_ ));
DESCR("date difference from today preserving months and years");

DATA(insert OID = 1388 (  timetz	   PGNSP PGUID 12 1 0 f f t f s 1 1266 "1184" _null_ _null_ _null_	timestamptz_timetz - _null_ ));
DESCR("convert timestamptz to timetz");

DATA(insert OID = 1389 (  isfinite	   PGNSP PGUID 12 1 0 f f t f i 1 16 "1184" _null_ _null_ _null_	timestamp_finite - _null_ ));
DESCR("finite timestamp?");
DATA(insert OID = 1390 (  isfinite	   PGNSP PGUID 12 1 0 f f t f i 1 16 "1186" _null_ _null_ _null_	interval_finite - _null_ ));
DESCR("finite interval?");


DATA(insert OID = 1376 (  factorial		   PGNSP PGUID 12 1 0 f f t f i 1 1700 "20" _null_ _null_ _null_	numeric_fac - _null_ ));
DESCR("factorial");
DATA(insert OID = 1394 (  abs			   PGNSP PGUID 12 1 0 f f t f i 1 700 "700" _null_ _null_ _null_	float4abs - _null_ ));
DESCR("absolute value");
DATA(insert OID = 1395 (  abs			   PGNSP PGUID 12 1 0 f f t f i 1 701 "701" _null_ _null_ _null_	float8abs - _null_ ));
DESCR("absolute value");
DATA(insert OID = 1396 (  abs			   PGNSP PGUID 12 1 0 f f t f i 1 20 "20" _null_ _null_ _null_	int8abs - _null_ ));
DESCR("absolute value");
DATA(insert OID = 1397 (  abs			   PGNSP PGUID 12 1 0 f f t f i 1 23 "23" _null_ _null_ _null_	int4abs - _null_ ));
DESCR("absolute value");
DATA(insert OID = 1398 (  abs			   PGNSP PGUID 12 1 0 f f t f i 1 21 "21" _null_ _null_ _null_	int2abs - _null_ ));
DESCR("absolute value");

/* OIDS 1400 - 1499 */

DATA(insert OID = 1400 (  name		   PGNSP PGUID 12 1 0 f f t f i 1 19 "1043" _null_ _null_ _null_	text_name - _null_ ));
DESCR("convert varchar to name");
DATA(insert OID = 1401 (  varchar	   PGNSP PGUID 12 1 0 f f t f i 1 1043 "19" _null_ _null_ _null_	name_text - _null_ ));
DESCR("convert name to varchar");

DATA(insert OID = 1402 (  current_schema	PGNSP PGUID 12 1 0 f f t f s 0	  19 "" _null_ _null_ _null_ current_schema - _null_ ));
DESCR("current schema name");
DATA(insert OID = 1403 (  current_schemas	PGNSP PGUID 12 1 0 f f t f s 1	1003 "16" _null_ _null_ _null_	current_schemas - _null_ ));
DESCR("current schema search list");

DATA(insert OID = 1404 (  overlay			PGNSP PGUID 14 1 0 f f t f i 4 25 "25 25 23 23" _null_ _null_ _null_	"select pg_catalog.substring($1, 1, ($3 - 1)) || $2 || pg_catalog.substring($1, ($3 + $4))" - _null_ ));
DESCR("substitute portion of string");
DATA(insert OID = 1405 (  overlay			PGNSP PGUID 14 1 0 f f t f i 3 25 "25 25 23" _null_ _null_ _null_  "select pg_catalog.substring($1, 1, ($3 - 1)) || $2 || pg_catalog.substring($1, ($3 + pg_catalog.char_length($2)))" - _null_ ));
DESCR("substitute portion of string");

DATA(insert OID = 1406 (  isvertical		PGNSP PGUID 12 1 0 f f t f i 2	16 "600 600" _null_ _null_ _null_  point_vert - _null_ ));
DESCR("vertically aligned?");
DATA(insert OID = 1407 (  ishorizontal		PGNSP PGUID 12 1 0 f f t f i 2	16 "600 600" _null_ _null_ _null_  point_horiz - _null_ ));
DESCR("horizontally aligned?");
DATA(insert OID = 1408 (  isparallel		PGNSP PGUID 12 1 0 f f t f i 2	16 "601 601" _null_ _null_ _null_  lseg_parallel - _null_ ));
DESCR("parallel?");
DATA(insert OID = 1409 (  isperp			PGNSP PGUID 12 1 0 f f t f i 2	16 "601 601" _null_ _null_ _null_  lseg_perp - _null_ ));
DESCR("perpendicular?");
DATA(insert OID = 1410 (  isvertical		PGNSP PGUID 12 1 0 f f t f i 1	16 "601" _null_ _null_ _null_  lseg_vertical - _null_ ));
DESCR("vertical?");
DATA(insert OID = 1411 (  ishorizontal		PGNSP PGUID 12 1 0 f f t f i 1	16 "601" _null_ _null_ _null_  lseg_horizontal - _null_ ));
DESCR("horizontal?");
DATA(insert OID = 1412 (  isparallel		PGNSP PGUID 12 1 0 f f t f i 2	16 "628 628" _null_ _null_ _null_  line_parallel - _null_ ));
DESCR("parallel?");
DATA(insert OID = 1413 (  isperp			PGNSP PGUID 12 1 0 f f t f i 2	16 "628 628" _null_ _null_ _null_  line_perp - _null_ ));
DESCR("perpendicular?");
DATA(insert OID = 1414 (  isvertical		PGNSP PGUID 12 1 0 f f t f i 1	16 "628" _null_ _null_ _null_  line_vertical - _null_ ));
DESCR("vertical?");
DATA(insert OID = 1415 (  ishorizontal		PGNSP PGUID 12 1 0 f f t f i 1	16 "628" _null_ _null_ _null_  line_horizontal - _null_ ));
DESCR("horizontal?");
DATA(insert OID = 1416 (  point				PGNSP PGUID 12 1 0 f f t f i 1 600 "718" _null_ _null_ _null_ circle_center - _null_ ));
DESCR("center of");

DATA(insert OID = 1417 (  isnottrue			PGNSP PGUID 12 1 0 f f f f i 1 16 "16" _null_ _null_ _null_ isnottrue - _null_ ));
DESCR("bool is not true (ie, false or unknown)");
DATA(insert OID = 1418 (  isnotfalse		PGNSP PGUID 12 1 0 f f f f i 1 16 "16" _null_ _null_ _null_ isnotfalse - _null_ ));
DESCR("bool is not false (ie, true or unknown)");

DATA(insert OID = 1419 (  time				PGNSP PGUID 12 1 0 f f t f i 1 1083 "1186" _null_ _null_ _null_ interval_time - _null_ ));
DESCR("convert interval to time");

DATA(insert OID = 1421 (  box				PGNSP PGUID 12 1 0 f f t f i 2 603 "600 600" _null_ _null_ _null_ points_box - _null_ ));
DESCR("convert points to box");
DATA(insert OID = 1422 (  box_add			PGNSP PGUID 12 1 0 f f t f i 2 603 "603 600" _null_ _null_ _null_ box_add - _null_ ));
DESCR("add point to box (translate)");
DATA(insert OID = 1423 (  box_sub			PGNSP PGUID 12 1 0 f f t f i 2 603 "603 600" _null_ _null_ _null_ box_sub - _null_ ));
DESCR("subtract point from box (translate)");
DATA(insert OID = 1424 (  box_mul			PGNSP PGUID 12 1 0 f f t f i 2 603 "603 600" _null_ _null_ _null_ box_mul - _null_ ));
DESCR("multiply box by point (scale)");
DATA(insert OID = 1425 (  box_div			PGNSP PGUID 12 1 0 f f t f i 2 603 "603 600" _null_ _null_ _null_ box_div - _null_ ));
DESCR("divide box by point (scale)");
DATA(insert OID = 1426 (  path_contain_pt	PGNSP PGUID 14 1 0 f f t f i 2	16 "602 600" _null_ _null_ _null_  "select pg_catalog.on_ppath($2, $1)" - _null_ ));
DESCR("path contains point?");
DATA(insert OID = 1428 (  poly_contain_pt	PGNSP PGUID 12 1 0 f f t f i 2	16 "604 600" _null_ _null_ _null_  poly_contain_pt - _null_ ));
DESCR("polygon contains point?");
DATA(insert OID = 1429 (  pt_contained_poly PGNSP PGUID 12 1 0 f f t f i 2	16 "600 604" _null_ _null_ _null_  pt_contained_poly - _null_ ));
DESCR("point contained in polygon?");

DATA(insert OID = 1430 (  isclosed			PGNSP PGUID 12 1 0 f f t f i 1	16 "602" _null_ _null_ _null_  path_isclosed - _null_ ));
DESCR("path closed?");
DATA(insert OID = 1431 (  isopen			PGNSP PGUID 12 1 0 f f t f i 1	16 "602" _null_ _null_ _null_  path_isopen - _null_ ));
DESCR("path open?");
DATA(insert OID = 1432 (  path_npoints		PGNSP PGUID 12 1 0 f f t f i 1	23 "602" _null_ _null_ _null_  path_npoints - _null_ ));
DESCR("number of points in path");

/* pclose and popen might better be named close and open, but that crashes initdb.
 * - thomas 97/04/20
 */

DATA(insert OID = 1433 (  pclose			PGNSP PGUID 12 1 0 f f t f i 1 602 "602" _null_ _null_ _null_ path_close - _null_ ));
DESCR("close path");
DATA(insert OID = 1434 (  popen				PGNSP PGUID 12 1 0 f f t f i 1 602 "602" _null_ _null_ _null_ path_open - _null_ ));
DESCR("open path");
DATA(insert OID = 1435 (  path_add			PGNSP PGUID 12 1 0 f f t f i 2 602 "602 602" _null_ _null_ _null_ path_add - _null_ ));
DESCR("concatenate open paths");
DATA(insert OID = 1436 (  path_add_pt		PGNSP PGUID 12 1 0 f f t f i 2 602 "602 600" _null_ _null_ _null_ path_add_pt - _null_ ));
DESCR("add (translate path)");
DATA(insert OID = 1437 (  path_sub_pt		PGNSP PGUID 12 1 0 f f t f i 2 602 "602 600" _null_ _null_ _null_ path_sub_pt - _null_ ));
DESCR("subtract (translate path)");
DATA(insert OID = 1438 (  path_mul_pt		PGNSP PGUID 12 1 0 f f t f i 2 602 "602 600" _null_ _null_ _null_ path_mul_pt - _null_ ));
DESCR("multiply (rotate/scale path)");
DATA(insert OID = 1439 (  path_div_pt		PGNSP PGUID 12 1 0 f f t f i 2 602 "602 600" _null_ _null_ _null_ path_div_pt - _null_ ));
DESCR("divide (rotate/scale path)");

DATA(insert OID = 1440 (  point				PGNSP PGUID 12 1 0 f f t f i 2 600 "701 701" _null_ _null_ _null_ construct_point - _null_ ));
DESCR("convert x, y to point");
DATA(insert OID = 1441 (  point_add			PGNSP PGUID 12 1 0 f f t f i 2 600 "600 600" _null_ _null_ _null_ point_add - _null_ ));
DESCR("add points (translate)");
DATA(insert OID = 1442 (  point_sub			PGNSP PGUID 12 1 0 f f t f i 2 600 "600 600" _null_ _null_ _null_ point_sub - _null_ ));
DESCR("subtract points (translate)");
DATA(insert OID = 1443 (  point_mul			PGNSP PGUID 12 1 0 f f t f i 2 600 "600 600" _null_ _null_ _null_ point_mul - _null_ ));
DESCR("multiply points (scale/rotate)");
DATA(insert OID = 1444 (  point_div			PGNSP PGUID 12 1 0 f f t f i 2 600 "600 600" _null_ _null_ _null_ point_div - _null_ ));
DESCR("divide points (scale/rotate)");

DATA(insert OID = 1445 (  poly_npoints		PGNSP PGUID 12 1 0 f f t f i 1	23 "604" _null_ _null_ _null_  poly_npoints - _null_ ));
DESCR("number of points in polygon");
DATA(insert OID = 1446 (  box				PGNSP PGUID 12 1 0 f f t f i 1 603 "604" _null_ _null_ _null_ poly_box - _null_ ));
DESCR("convert polygon to bounding box");
DATA(insert OID = 1447 (  path				PGNSP PGUID 12 1 0 f f t f i 1 602 "604" _null_ _null_ _null_ poly_path - _null_ ));
DESCR("convert polygon to path");
DATA(insert OID = 1448 (  polygon			PGNSP PGUID 12 1 0 f f t f i 1 604 "603" _null_ _null_ _null_ box_poly - _null_ ));
DESCR("convert box to polygon");
DATA(insert OID = 1449 (  polygon			PGNSP PGUID 12 1 0 f f t f i 1 604 "602" _null_ _null_ _null_ path_poly - _null_ ));
DESCR("convert path to polygon");

DATA(insert OID = 1450 (  circle_in			PGNSP PGUID 12 1 0 f f t f i 1 718 "2275" _null_ _null_ _null_	circle_in - _null_ ));
DESCR("I/O");
DATA(insert OID = 1451 (  circle_out		PGNSP PGUID 12 1 0 f f t f i 1 2275 "718" _null_ _null_ _null_	circle_out - _null_ ));
DESCR("I/O");
DATA(insert OID = 1452 (  circle_same		PGNSP PGUID 12 1 0 f f t f i 2	16 "718 718" _null_ _null_ _null_  circle_same - _null_ ));
DESCR("same as?");
DATA(insert OID = 1453 (  circle_contain	PGNSP PGUID 12 1 0 f f t f i 2	16 "718 718" _null_ _null_ _null_  circle_contain - _null_ ));
DESCR("contains?");
DATA(insert OID = 1454 (  circle_left		PGNSP PGUID 12 1 0 f f t f i 2	16 "718 718" _null_ _null_ _null_  circle_left - _null_ ));
DESCR("is left of");
DATA(insert OID = 1455 (  circle_overleft	PGNSP PGUID 12 1 0 f f t f i 2	16 "718 718" _null_ _null_ _null_  circle_overleft - _null_ ));
DESCR("overlaps or is left of");
DATA(insert OID = 1456 (  circle_overright	PGNSP PGUID 12 1 0 f f t f i 2	16 "718 718" _null_ _null_ _null_  circle_overright - _null_ ));
DESCR("overlaps or is right of");
DATA(insert OID = 1457 (  circle_right		PGNSP PGUID 12 1 0 f f t f i 2	16 "718 718" _null_ _null_ _null_  circle_right - _null_ ));
DESCR("is right of");
DATA(insert OID = 1458 (  circle_contained	PGNSP PGUID 12 1 0 f f t f i 2	16 "718 718" _null_ _null_ _null_  circle_contained - _null_ ));
DESCR("is contained by?");
DATA(insert OID = 1459 (  circle_overlap	PGNSP PGUID 12 1 0 f f t f i 2	16 "718 718" _null_ _null_ _null_  circle_overlap - _null_ ));
DESCR("overlaps");
DATA(insert OID = 1460 (  circle_below		PGNSP PGUID 12 1 0 f f t f i 2	16 "718 718" _null_ _null_ _null_  circle_below - _null_ ));
DESCR("is below");
DATA(insert OID = 1461 (  circle_above		PGNSP PGUID 12 1 0 f f t f i 2	16 "718 718" _null_ _null_ _null_  circle_above - _null_ ));
DESCR("is above");
DATA(insert OID = 1462 (  circle_eq			PGNSP PGUID 12 1 0 f f t f i 2	16 "718 718" _null_ _null_ _null_  circle_eq - _null_ ));
DESCR("equal by area");
DATA(insert OID = 1463 (  circle_ne			PGNSP PGUID 12 1 0 f f t f i 2	16 "718 718" _null_ _null_ _null_  circle_ne - _null_ ));
DESCR("not equal by area");
DATA(insert OID = 1464 (  circle_lt			PGNSP PGUID 12 1 0 f f t f i 2	16 "718 718" _null_ _null_ _null_  circle_lt - _null_ ));
DESCR("less-than by area");
DATA(insert OID = 1465 (  circle_gt			PGNSP PGUID 12 1 0 f f t f i 2	16 "718 718" _null_ _null_ _null_  circle_gt - _null_ ));
DESCR("greater-than by area");
DATA(insert OID = 1466 (  circle_le			PGNSP PGUID 12 1 0 f f t f i 2	16 "718 718" _null_ _null_ _null_  circle_le - _null_ ));
DESCR("less-than-or-equal by area");
DATA(insert OID = 1467 (  circle_ge			PGNSP PGUID 12 1 0 f f t f i 2	16 "718 718" _null_ _null_ _null_  circle_ge - _null_ ));
DESCR("greater-than-or-equal by area");
DATA(insert OID = 1468 (  area				PGNSP PGUID 12 1 0 f f t f i 1 701 "718" _null_ _null_ _null_ circle_area - _null_ ));
DESCR("area of circle");
DATA(insert OID = 1469 (  diameter			PGNSP PGUID 12 1 0 f f t f i 1 701 "718" _null_ _null_ _null_ circle_diameter - _null_ ));
DESCR("diameter of circle");
DATA(insert OID = 1470 (  radius			PGNSP PGUID 12 1 0 f f t f i 1 701 "718" _null_ _null_ _null_ circle_radius - _null_ ));
DESCR("radius of circle");
DATA(insert OID = 1471 (  circle_distance	PGNSP PGUID 12 1 0 f f t f i 2 701 "718 718" _null_ _null_ _null_ circle_distance - _null_ ));
DESCR("distance between");
DATA(insert OID = 1472 (  circle_center		PGNSP PGUID 12 1 0 f f t f i 1 600 "718" _null_ _null_ _null_ circle_center - _null_ ));
DESCR("center of");
DATA(insert OID = 1473 (  circle			PGNSP PGUID 12 1 0 f f t f i 2 718 "600 701" _null_ _null_ _null_ cr_circle - _null_ ));
DESCR("convert point and radius to circle");
DATA(insert OID = 1474 (  circle			PGNSP PGUID 12 1 0 f f t f i 1 718 "604" _null_ _null_ _null_ poly_circle - _null_ ));
DESCR("convert polygon to circle");
DATA(insert OID = 1475 (  polygon			PGNSP PGUID 12 1 0 f f t f i 2 604 "23 718" _null_ _null_ _null_	circle_poly - _null_ ));
DESCR("convert vertex count and circle to polygon");
DATA(insert OID = 1476 (  dist_pc			PGNSP PGUID 12 1 0 f f t f i 2 701 "600 718" _null_ _null_ _null_ dist_pc - _null_ ));
DESCR("distance between point and circle");
DATA(insert OID = 1477 (  circle_contain_pt PGNSP PGUID 12 1 0 f f t f i 2	16 "718 600" _null_ _null_ _null_  circle_contain_pt - _null_ ));
DESCR("circle contains point?");
DATA(insert OID = 1478 (  pt_contained_circle	PGNSP PGUID 12 1 0 f f t f i 2	16 "600 718" _null_ _null_ _null_  pt_contained_circle - _null_ ));
DESCR("point contained in circle?");
DATA(insert OID = 1479 (  circle			PGNSP PGUID 12 1 0 f f t f i 1 718 "603" _null_ _null_ _null_ box_circle - _null_ ));
DESCR("convert box to circle");
DATA(insert OID = 1480 (  box				PGNSP PGUID 12 1 0 f f t f i 1 603 "718" _null_ _null_ _null_ circle_box - _null_ ));
DESCR("convert circle to box");
DATA(insert OID = 1481 (  tinterval			 PGNSP PGUID 12 1 0 f f t f i 2 704 "702 702" _null_ _null_ _null_ mktinterval - _null_ ));
DESCR("convert to tinterval");

DATA(insert OID = 1482 (  lseg_ne			PGNSP PGUID 12 1 0 f f t f i 2	16 "601 601" _null_ _null_ _null_  lseg_ne - _null_ ));
DESCR("not equal");
DATA(insert OID = 1483 (  lseg_lt			PGNSP PGUID 12 1 0 f f t f i 2	16 "601 601" _null_ _null_ _null_  lseg_lt - _null_ ));
DESCR("less-than by length");
DATA(insert OID = 1484 (  lseg_le			PGNSP PGUID 12 1 0 f f t f i 2	16 "601 601" _null_ _null_ _null_  lseg_le - _null_ ));
DESCR("less-than-or-equal by length");
DATA(insert OID = 1485 (  lseg_gt			PGNSP PGUID 12 1 0 f f t f i 2	16 "601 601" _null_ _null_ _null_  lseg_gt - _null_ ));
DESCR("greater-than by length");
DATA(insert OID = 1486 (  lseg_ge			PGNSP PGUID 12 1 0 f f t f i 2	16 "601 601" _null_ _null_ _null_  lseg_ge - _null_ ));
DESCR("greater-than-or-equal by length");
DATA(insert OID = 1487 (  lseg_length		PGNSP PGUID 12 1 0 f f t f i 1 701 "601" _null_ _null_ _null_ lseg_length - _null_ ));
DESCR("distance between endpoints");
DATA(insert OID = 1488 (  close_ls			PGNSP PGUID 12 1 0 f f t f i 2 600 "628 601" _null_ _null_ _null_ close_ls - _null_ ));
DESCR("closest point to line on line segment");
DATA(insert OID = 1489 (  close_lseg		PGNSP PGUID 12 1 0 f f t f i 2 600 "601 601" _null_ _null_ _null_ close_lseg - _null_ ));
DESCR("closest point to line segment on line segment");

DATA(insert OID = 1490 (  line_in			PGNSP PGUID 12 1 0 f f t f i 1 628 "2275" _null_ _null_ _null_	line_in - _null_ ));
DESCR("I/O");
DATA(insert OID = 1491 (  line_out			PGNSP PGUID 12 1 0 f f t f i 1 2275 "628" _null_ _null_ _null_	line_out - _null_ ));
DESCR("I/O");
DATA(insert OID = 1492 (  line_eq			PGNSP PGUID 12 1 0 f f t f i 2	16 "628 628" _null_ _null_ _null_ line_eq - _null_ ));
DESCR("lines equal?");
DATA(insert OID = 1493 (  line				PGNSP PGUID 12 1 0 f f t f i 2 628 "600 600" _null_ _null_ _null_ line_construct_pp - _null_ ));
DESCR("line from points");
DATA(insert OID = 1494 (  line_interpt		PGNSP PGUID 12 1 0 f f t f i 2 600 "628 628" _null_ _null_ _null_ line_interpt - _null_ ));
DESCR("intersection point");
DATA(insert OID = 1495 (  line_intersect	PGNSP PGUID 12 1 0 f f t f i 2	16 "628 628" _null_ _null_ _null_  line_intersect - _null_ ));
DESCR("intersect?");
DATA(insert OID = 1496 (  line_parallel		PGNSP PGUID 12 1 0 f f t f i 2	16 "628 628" _null_ _null_ _null_  line_parallel - _null_ ));
DESCR("parallel?");
DATA(insert OID = 1497 (  line_perp			PGNSP PGUID 12 1 0 f f t f i 2	16 "628 628" _null_ _null_ _null_  line_perp - _null_ ));
DESCR("perpendicular?");
DATA(insert OID = 1498 (  line_vertical		PGNSP PGUID 12 1 0 f f t f i 1	16 "628" _null_ _null_ _null_  line_vertical - _null_ ));
DESCR("vertical?");
DATA(insert OID = 1499 (  line_horizontal	PGNSP PGUID 12 1 0 f f t f i 1	16 "628" _null_ _null_ _null_  line_horizontal - _null_ ));
DESCR("horizontal?");

/* OIDS 1500 - 1599 */

DATA(insert OID = 1530 (  length			PGNSP PGUID 12 1 0 f f t f i 1 701 "601" _null_ _null_ _null_ lseg_length - _null_ ));
DESCR("distance between endpoints");
DATA(insert OID = 1531 (  length			PGNSP PGUID 12 1 0 f f t f i 1 701 "602" _null_ _null_ _null_ path_length - _null_ ));
DESCR("sum of path segments");


DATA(insert OID = 1532 (  point				PGNSP PGUID 12 1 0 f f t f i 1 600 "601" _null_ _null_ _null_ lseg_center - _null_ ));
DESCR("center of");
DATA(insert OID = 1533 (  point				PGNSP PGUID 12 1 0 f f t f i 1 600 "602" _null_ _null_ _null_ path_center - _null_ ));
DESCR("center of");
DATA(insert OID = 1534 (  point				PGNSP PGUID 12 1 0 f f t f i 1 600 "603" _null_ _null_ _null_ box_center - _null_ ));
DESCR("center of");
DATA(insert OID = 1540 (  point				PGNSP PGUID 12 1 0 f f t f i 1 600 "604" _null_ _null_ _null_ poly_center - _null_ ));
DESCR("center of");
DATA(insert OID = 1541 (  lseg				PGNSP PGUID 12 1 0 f f t f i 1 601 "603" _null_ _null_ _null_ box_diagonal - _null_ ));
DESCR("diagonal of");
DATA(insert OID = 1542 (  center			PGNSP PGUID 12 1 0 f f t f i 1 600 "603" _null_ _null_ _null_ box_center - _null_ ));
DESCR("center of");
DATA(insert OID = 1543 (  center			PGNSP PGUID 12 1 0 f f t f i 1 600 "718" _null_ _null_ _null_ circle_center - _null_ ));
DESCR("center of");
DATA(insert OID = 1544 (  polygon			PGNSP PGUID 14 1 0 f f t f i 1 604 "718" _null_ _null_ _null_ "select pg_catalog.polygon(12, $1)" - _null_ ));
DESCR("convert circle to 12-vertex polygon");
DATA(insert OID = 1545 (  npoints			PGNSP PGUID 12 1 0 f f t f i 1	23 "602" _null_ _null_ _null_  path_npoints - _null_ ));
DESCR("number of points in path");
DATA(insert OID = 1556 (  npoints			PGNSP PGUID 12 1 0 f f t f i 1	23 "604" _null_ _null_ _null_  poly_npoints - _null_ ));
DESCR("number of points in polygon");

DATA(insert OID = 1564 (  bit_in			PGNSP PGUID 12 1 0 f f t f i 3 1560 "2275 26 23" _null_ _null_ _null_ bit_in - _null_ ));
DESCR("I/O");
DATA(insert OID = 1565 (  bit_out			PGNSP PGUID 12 1 0 f f t f i 1 2275 "1560" _null_ _null_ _null_ bit_out - _null_ ));
DESCR("I/O");
DATA(insert OID = 2919 (  bittypmodin   	PGNSP PGUID 12 1 0 f f t f i 1 23 "1263" _null_ _null_ _null_	bittypmodin - _null_ ));
DESCR("I/O typmod");
DATA(insert OID = 2920 (  bittypmodout  	PGNSP PGUID 12 1 0 f f t f i 1 2275 "23" _null_ _null_ _null_	bittypmodout - _null_ ));
DESCR("I/O typmod");

DATA(insert OID = 1569 (  like				PGNSP PGUID 12 1 0 f f t f i 2 16 "25 25" _null_ _null_ _null_	textlike - _null_ ));
DESCR("matches LIKE expression");
DATA(insert OID = 1570 (  notlike			PGNSP PGUID 12 1 0 f f t f i 2 16 "25 25" _null_ _null_ _null_	textnlike - _null_ ));
DESCR("does not match LIKE expression");
DATA(insert OID = 1571 (  like				PGNSP PGUID 12 1 0 f f t f i 2 16 "19 25" _null_ _null_ _null_	namelike - _null_ ));
DESCR("matches LIKE expression");
DATA(insert OID = 1572 (  notlike			PGNSP PGUID 12 1 0 f f t f i 2 16 "19 25" _null_ _null_ _null_	namenlike - _null_ ));
DESCR("does not match LIKE expression");


/* SEQUENCE functions */
DATA(insert OID = 1574 (  nextval			PGNSP PGUID 12 1 0 f f t f v 1 20 "2205" _null_ _null_ _null_	nextval_oid - _null_ ));
DESCR("sequence next value");
DATA(insert OID = 1575 (  currval			PGNSP PGUID 12 1 0 f f t f v 1 20 "2205" _null_ _null_ _null_	currval_oid - _null_ ));
DESCR("sequence current value");
DATA(insert OID = 1576 (  setval			PGNSP PGUID 12 1 0 f f t f v 2 20 "2205 20" _null_ _null_ _null_  setval_oid - _null_ ));
DESCR("set sequence value");
DATA(insert OID = 1765 (  setval			PGNSP PGUID 12 1 0 f f t f v 3 20 "2205 20 16" _null_ _null_ _null_ setval3_oid - _null_ ));
DESCR("set sequence value and iscalled status");

DATA(insert OID = 1579 (  varbit_in			PGNSP PGUID 12 1 0 f f t f i 3 1562 "2275 26 23" _null_ _null_ _null_ varbit_in - _null_ ));
DESCR("I/O");
DATA(insert OID = 1580 (  varbit_out		PGNSP PGUID 12 1 0 f f t f i 1 2275 "1562" _null_ _null_ _null_ varbit_out - _null_ ));
DESCR("I/O");
DATA(insert OID = 2902 (  varbittypmodin   	PGNSP PGUID 12 1 0 f f t f i 1 23 "1263" _null_ _null_ _null_	varbittypmodin - _null_ ));
DESCR("I/O typmod");
DATA(insert OID = 2921 (  varbittypmodout  	PGNSP PGUID 12 1 0 f f t f i 1 2275 "23" _null_ _null_ _null_	varbittypmodout - _null_ ));
DESCR("I/O typmod");

DATA(insert OID = 1581 (  biteq				PGNSP PGUID 12 1 0 f f t f i 2 16 "1560 1560" _null_ _null_ _null_	biteq - _null_ ));
DESCR("equal");
DATA(insert OID = 1582 (  bitne				PGNSP PGUID 12 1 0 f f t f i 2 16 "1560 1560" _null_ _null_ _null_	bitne - _null_ ));
DESCR("not equal");
DATA(insert OID = 1592 (  bitge				PGNSP PGUID 12 1 0 f f t f i 2 16 "1560 1560" _null_ _null_ _null_	bitge - _null_ ));
DESCR("greater than or equal");
DATA(insert OID = 1593 (  bitgt				PGNSP PGUID 12 1 0 f f t f i 2 16 "1560 1560" _null_ _null_ _null_	bitgt - _null_ ));
DESCR("greater than");
DATA(insert OID = 1594 (  bitle				PGNSP PGUID 12 1 0 f f t f i 2 16 "1560 1560" _null_ _null_ _null_	bitle - _null_ ));
DESCR("less than or equal");
DATA(insert OID = 1595 (  bitlt				PGNSP PGUID 12 1 0 f f t f i 2 16 "1560 1560" _null_ _null_ _null_	bitlt - _null_ ));
DESCR("less than");
DATA(insert OID = 1596 (  bitcmp			PGNSP PGUID 12 1 0 f f t f i 2 23 "1560 1560" _null_ _null_ _null_	bitcmp - _null_ ));
DESCR("compare");

DATA(insert OID = 1598 (  random			PGNSP PGUID 12 1 0 f f t f v 0 701 "" _null_ _null_ _null_	drandom - _null_ ));
DESCR("random value");
DATA(insert OID = 1599 (  setseed			PGNSP PGUID 12 1 0 f f t f v 1 2278 "701" _null_ _null_ _null_ setseed - _null_ ));
DESCR("set random seed");

/* OIDS 1600 - 1699 */

DATA(insert OID = 1600 (  asin				PGNSP PGUID 12 1 0 f f t f i 1 701 "701" _null_ _null_ _null_ dasin - _null_ ));
DESCR("arcsine");
DATA(insert OID = 1601 (  acos				PGNSP PGUID 12 1 0 f f t f i 1 701 "701" _null_ _null_ _null_ dacos - _null_ ));
DESCR("arccosine");
DATA(insert OID = 1602 (  atan				PGNSP PGUID 12 1 0 f f t f i 1 701 "701" _null_ _null_ _null_ datan - _null_ ));
DESCR("arctangent");
DATA(insert OID = 1603 (  atan2				PGNSP PGUID 12 1 0 f f t f i 2 701 "701 701" _null_ _null_ _null_ datan2 - _null_ ));
DESCR("arctangent, two arguments");
DATA(insert OID = 1604 (  sin				PGNSP PGUID 12 1 0 f f t f i 1 701 "701" _null_ _null_ _null_ dsin - _null_ ));
DESCR("sine");
DATA(insert OID = 1605 (  cos				PGNSP PGUID 12 1 0 f f t f i 1 701 "701" _null_ _null_ _null_ dcos - _null_ ));
DESCR("cosine");
DATA(insert OID = 1606 (  tan				PGNSP PGUID 12 1 0 f f t f i 1 701 "701" _null_ _null_ _null_ dtan - _null_ ));
DESCR("tangent");
DATA(insert OID = 1607 (  cot				PGNSP PGUID 12 1 0 f f t f i 1 701 "701" _null_ _null_ _null_ dcot - _null_ ));
DESCR("cotangent");
DATA(insert OID = 1608 (  degrees			PGNSP PGUID 12 1 0 f f t f i 1 701 "701" _null_ _null_ _null_ degrees - _null_ ));
DESCR("radians to degrees");
DATA(insert OID = 1609 (  radians			PGNSP PGUID 12 1 0 f f t f i 1 701 "701" _null_ _null_ _null_ radians - _null_ ));
DESCR("degrees to radians");
DATA(insert OID = 1610 (  pi				PGNSP PGUID 12 1 0 f f t f i 0 701 "" _null_ _null_ _null_	dpi - _null_ ));
DESCR("PI");

DATA(insert OID = 1618 (  interval_mul		PGNSP PGUID 12 1 0 f f t f i 2 1186 "1186 701" _null_ _null_ _null_ interval_mul - _null_ ));
DESCR("multiply interval");

DATA(insert OID = 1620 (  ascii				PGNSP PGUID 12 1 0 f f t f i 1 23 "25" _null_ _null_ _null_ ascii - _null_ ));
DESCR("convert first char to int4");
DATA(insert OID = 1621 (  chr				PGNSP PGUID 12 1 0 f f t f i 1 25 "23" _null_ _null_ _null_ chr - _null_ ));
DESCR("convert int4 to char");
DATA(insert OID = 1622 (  repeat			PGNSP PGUID 12 1 0 f f t f i 2 25 "25 23" _null_ _null_ _null_	repeat - _null_ ));
DESCR("replicate string int4 times");

DATA(insert OID = 1623 (  similar_escape	PGNSP PGUID 12 1 0 f f f f i 2 25 "25 25" _null_ _null_ _null_ similar_escape - _null_ ));
DESCR("convert SQL99 regexp pattern to POSIX style");

DATA(insert OID = 1624 (  mul_d_interval	PGNSP PGUID 12 1 0 f f t f i 2 1186 "701 1186" _null_ _null_ _null_ mul_d_interval - _null_ ));

DATA(insert OID = 1631 (  bpcharlike	   PGNSP PGUID 12 1 0 f f t f i 2 16 "1042 25" _null_ _null_ _null_ textlike - _null_ ));
DESCR("matches LIKE expression");
DATA(insert OID = 1632 (  bpcharnlike	   PGNSP PGUID 12 1 0 f f t f i 2 16 "1042 25" _null_ _null_ _null_ textnlike - _null_ ));
DESCR("does not match LIKE expression");

DATA(insert OID = 1633 (  texticlike		PGNSP PGUID 12 1 0 f f t f i 2 16 "25 25" _null_ _null_ _null_ texticlike - _null_ ));
DESCR("matches LIKE expression, case-insensitive");
DATA(insert OID = 1634 (  texticnlike		PGNSP PGUID 12 1 0 f f t f i 2 16 "25 25" _null_ _null_ _null_ texticnlike - _null_ ));
DESCR("does not match LIKE expression, case-insensitive");
DATA(insert OID = 1635 (  nameiclike		PGNSP PGUID 12 1 0 f f t f i 2 16 "19 25" _null_ _null_ _null_	nameiclike - _null_ ));
DESCR("matches LIKE expression, case-insensitive");
DATA(insert OID = 1636 (  nameicnlike		PGNSP PGUID 12 1 0 f f t f i 2 16 "19 25" _null_ _null_ _null_	nameicnlike - _null_ ));
DESCR("does not match LIKE expression, case-insensitive");
DATA(insert OID = 1637 (  like_escape		PGNSP PGUID 12 1 0 f f t f i 2 25 "25 25" _null_ _null_ _null_ like_escape - _null_ ));
DESCR("convert LIKE pattern to use backslash escapes");

DATA(insert OID = 1656 (  bpcharicregexeq	 PGNSP PGUID 12 1 0 f f t f i 2 16 "1042 25" _null_ _null_ _null_ texticregexeq - _null_ ));
DESCR("matches regex., case-insensitive");
DATA(insert OID = 1657 (  bpcharicregexne	 PGNSP PGUID 12 1 0 f f t f i 2 16 "1042 25" _null_ _null_ _null_ texticregexne - _null_ ));
DESCR("does not match regex., case-insensitive");
DATA(insert OID = 1658 (  bpcharregexeq    PGNSP PGUID 12 1 0 f f t f i 2 16 "1042 25" _null_ _null_ _null_ textregexeq - _null_ ));
DESCR("matches regex., case-sensitive");
DATA(insert OID = 1659 (  bpcharregexne    PGNSP PGUID 12 1 0 f f t f i 2 16 "1042 25" _null_ _null_ _null_ textregexne - _null_ ));
DESCR("does not match regex., case-sensitive");
DATA(insert OID = 1660 (  bpchariclike		PGNSP PGUID 12 1 0 f f t f i 2 16 "1042 25" _null_ _null_ _null_ texticlike - _null_ ));
DESCR("matches LIKE expression, case-insensitive");
DATA(insert OID = 1661 (  bpcharicnlike		PGNSP PGUID 12 1 0 f f t f i 2 16 "1042 25" _null_ _null_ _null_ texticnlike - _null_ ));
DESCR("does not match LIKE expression, case-insensitive");

DATA(insert OID = 1689 (  flatfile_update_trigger  PGNSP PGUID 12 1 0 f f t f v 0 2279	"" _null_ _null_ _null_ flatfile_update_trigger - _null_ ));
DESCR("update flat-file copy of a shared catalog");

/* Oracle Compatibility Related Functions - By Edmund Mergl <E.Mergl@bawue.de> */
DATA(insert OID =  868 (  strpos	   PGNSP PGUID 12 1 0 f f t f i 2 23 "25 25" _null_ _null_ _null_ textpos - _null_ ));
DESCR("find position of substring");
DATA(insert OID =  870 (  lower		   PGNSP PGUID 12 1 0 f f t f i 1 25 "25" _null_ _null_ _null_	lower - _null_ ));
DESCR("lowercase");
DATA(insert OID =  871 (  upper		   PGNSP PGUID 12 1 0 f f t f i 1 25 "25" _null_ _null_ _null_	upper - _null_ ));
DESCR("uppercase");
DATA(insert OID =  872 (  initcap	   PGNSP PGUID 12 1 0 f f t f i 1 25 "25" _null_ _null_ _null_	initcap - _null_ ));
DESCR("capitalize each word");
DATA(insert OID =  873 (  lpad		   PGNSP PGUID 12 1 0 f f t f i 3 25 "25 23 25" _null_ _null_ _null_	lpad - _null_ ));
DESCR("left-pad string to length");
DATA(insert OID =  874 (  rpad		   PGNSP PGUID 12 1 0 f f t f i 3 25 "25 23 25" _null_ _null_ _null_	rpad - _null_ ));
DESCR("right-pad string to length");
DATA(insert OID =  875 (  ltrim		   PGNSP PGUID 12 1 0 f f t f i 2 25 "25 25" _null_ _null_ _null_ ltrim - _null_ ));
DESCR("trim selected characters from left end of string");
DATA(insert OID =  876 (  rtrim		   PGNSP PGUID 12 1 0 f f t f i 2 25 "25 25" _null_ _null_ _null_ rtrim - _null_ ));
DESCR("trim selected characters from right end of string");
DATA(insert OID =  877 (  substr	   PGNSP PGUID 12 1 0 f f t f i 3 25 "25 23 23" _null_ _null_ _null_	text_substr - _null_ ));
DESCR("return portion of string");
DATA(insert OID =  878 (  translate    PGNSP PGUID 12 1 0 f f t f i 3 25 "25 25 25" _null_ _null_ _null_	translate - _null_ ));
DESCR("map a set of character appearing in string");
DATA(insert OID =  879 (  lpad		   PGNSP PGUID 14 1 0 f f t f i 2 25 "25 23" _null_ _null_ _null_ "select pg_catalog.lpad($1, $2, '' '')" - _null_ ));
DESCR("left-pad string to length");
DATA(insert OID =  880 (  rpad		   PGNSP PGUID 14 1 0 f f t f i 2 25 "25 23" _null_ _null_ _null_ "select pg_catalog.rpad($1, $2, '' '')" - _null_ ));
DESCR("right-pad string to length");
DATA(insert OID =  881 (  ltrim		   PGNSP PGUID 12 1 0 f f t f i 1 25 "25" _null_ _null_ _null_	ltrim1 - _null_ ));
DESCR("trim spaces from left end of string");
DATA(insert OID =  882 (  rtrim		   PGNSP PGUID 12 1 0 f f t f i 1 25 "25" _null_ _null_ _null_	rtrim1 - _null_ ));
DESCR("trim spaces from right end of string");
DATA(insert OID =  883 (  substr	   PGNSP PGUID 12 1 0 f f t f i 2 25 "25 23" _null_ _null_ _null_ text_substr_no_len - _null_ ));
DESCR("return portion of string");
DATA(insert OID =  884 (  btrim		   PGNSP PGUID 12 1 0 f f t f i 2 25 "25 25" _null_ _null_ _null_ btrim - _null_ ));
DESCR("trim selected characters from both ends of string");
DATA(insert OID =  885 (  btrim		   PGNSP PGUID 12 1 0 f f t f i 1 25 "25" _null_ _null_ _null_	btrim1 - _null_ ));
DESCR("trim spaces from both ends of string");

DATA(insert OID =  936 (  substring    PGNSP PGUID 12 1 0 f f t f i 3 25 "25 23 23" _null_ _null_ _null_	text_substr - _null_ ));
DESCR("return portion of string");
DATA(insert OID =  937 (  substring    PGNSP PGUID 12 1 0 f f t f i 2 25 "25 23" _null_ _null_ _null_ text_substr_no_len - _null_ ));
DESCR("return portion of string");
DATA(insert OID =  2087 ( replace	   PGNSP PGUID 12 1 0 f f t f i 3 25 "25 25 25" _null_ _null_ _null_	replace_text - _null_ ));
DESCR("replace all occurrences of old_substr with new_substr in string");
DATA(insert OID =  2284 ( regexp_replace	   PGNSP PGUID 12 1 0 f f t f i 3 25 "25 25 25" _null_ _null_ _null_	textregexreplace_noopt - _null_ ));
DESCR("replace text using regexp");
DATA(insert OID =  2285 ( regexp_replace	   PGNSP PGUID 12 1 0 f f t f i 4 25 "25 25 25 25" _null_ _null_ _null_ textregexreplace - _null_ ));
DESCR("replace text using regexp");
DATA(insert OID =  2763 ( regexp_matches   PGNSP PGUID 12 1 1 f f t t i 2 1009 "25 25" _null_ _null_ _null_	regexp_matches_no_flags - _null_ ));
DESCR("return all match groups for regexp");
DATA(insert OID =  2764 ( regexp_matches   PGNSP PGUID 12 1 10 f f t t i 3 1009 "25 25 25" _null_ _null_ _null_	regexp_matches - _null_ ));
DESCR("return all match groups for regexp");
DATA(insert OID =  2088 ( split_part   PGNSP PGUID 12 1 0 f f t f i 3 25 "25 25 23" _null_ _null_ _null_	split_text - _null_ ));
DESCR("split string by field_sep and return field_num");
DATA(insert OID =  2765 ( regexp_split_to_table PGNSP PGUID 12 1 1000 f f t t i 2 25 "25 25" _null_ _null_ _null_	regexp_split_to_table_no_flags - _null_ ));
DESCR("split string by pattern");
DATA(insert OID =  2766 ( regexp_split_to_table PGNSP PGUID 12 1 1000 f f t t i 3 25 "25 25 25" _null_ _null_ _null_	regexp_split_to_table - _null_ ));
DESCR("split string by pattern");
DATA(insert OID =  2767 ( regexp_split_to_array PGNSP PGUID 12 1 0 f f t f i 2 1009 "25 25" _null_ _null_ _null_	regexp_split_to_array_no_flags - _null_ ));
DESCR("split string by pattern");
DATA(insert OID =  2768 ( regexp_split_to_array PGNSP PGUID 12 1 0 f f t f i 3 1009 "25 25 25" _null_ _null_ _null_	regexp_split_to_array - _null_ ));
DESCR("split string by pattern");
DATA(insert OID =  2089 ( to_hex	   PGNSP PGUID 12 1 0 f f t f i 1 25 "23" _null_ _null_ _null_	to_hex32 - _null_ ));
DESCR("convert int4 number to hex");
DATA(insert OID =  2090 ( to_hex	   PGNSP PGUID 12 1 0 f f t f i 1 25 "20" _null_ _null_ _null_	to_hex64 - _null_ ));
DESCR("convert int8 number to hex");

/* for character set encoding support */

/* return database encoding name */
DATA(insert OID = 1039 (  getdatabaseencoding	   PGNSP PGUID 12 1 0 f f t f s 0 19 "" _null_ _null_ _null_ getdatabaseencoding - _null_ ));
DESCR("encoding name of current database");

/* return client encoding name i.e. session encoding */
DATA(insert OID = 810 (  pg_client_encoding    PGNSP PGUID 12 1 0 f f t f s 0 19 "" _null_ _null_ _null_ pg_client_encoding - _null_ ));
DESCR("encoding name of current database");

DATA(insert OID = 1717 (  convert		   PGNSP PGUID 12 1 0 f f t f s 2 25 "25 19" _null_ _null_ _null_ pg_convert - _null_ ));
DESCR("convert string with specified destination encoding name");

DATA(insert OID = 1813 (  convert		   PGNSP PGUID 12 1 0 f f t f s 3 25 "25 19 19" _null_ _null_ _null_	pg_convert2 - _null_ ));
DESCR("convert string with specified encoding names");

DATA(insert OID = 1619 (  convert_using    PGNSP PGUID 12 1 0 f f t f s 2 25 "25 25" _null_ _null_ _null_  pg_convert_using - _null_ ));
DESCR("convert string with specified conversion name");

DATA(insert OID = 1264 (  pg_char_to_encoding	   PGNSP PGUID 12 1 0 f f t f s 1 23 "19" _null_ _null_ _null_	PG_char_to_encoding - _null_ ));
DESCR("convert encoding name to encoding id");

DATA(insert OID = 1597 (  pg_encoding_to_char	   PGNSP PGUID 12 1 0 f f t f s 1 19 "23" _null_ _null_ _null_	PG_encoding_to_char - _null_ ));
DESCR("convert encoding id to encoding name");

DATA(insert OID = 1638 (  oidgt				   PGNSP PGUID 12 1 0 f f t f i 2 16 "26 26" _null_ _null_ _null_ oidgt - _null_ ));
DESCR("greater-than");
DATA(insert OID = 1639 (  oidge				   PGNSP PGUID 12 1 0 f f t f i 2 16 "26 26" _null_ _null_ _null_ oidge - _null_ ));
DESCR("greater-than-or-equal");

/* System-view support functions */
DATA(insert OID = 1573 (  pg_get_ruledef	   PGNSP PGUID 12 1 0 f f t f s 1 25 "26" _null_ _null_ _null_	pg_get_ruledef - _null_ ));
DESCR("source text of a rule");
DATA(insert OID = 1640 (  pg_get_viewdef	   PGNSP PGUID 12 1 0 f f t f s 1 25 "25" _null_ _null_ _null_	pg_get_viewdef_name - _null_ ));
DESCR("select statement of a view");
DATA(insert OID = 1641 (  pg_get_viewdef	   PGNSP PGUID 12 1 0 f f t f s 1 25 "26" _null_ _null_ _null_	pg_get_viewdef - _null_ ));
DESCR("select statement of a view");
DATA(insert OID = 1642 (  pg_get_userbyid	   PGNSP PGUID 12 1 0 f f t f s 1 19 "26" _null_ _null_ _null_	pg_get_userbyid - _null_ ));
DESCR("role name by OID (with fallback)");
DATA(insert OID = 1643 (  pg_get_indexdef	   PGNSP PGUID 12 1 0 f f t f s 1 25 "26" _null_ _null_ _null_	pg_get_indexdef - _null_ ));
DESCR("index description");
DATA(insert OID = 1662 (  pg_get_triggerdef    PGNSP PGUID 12 1 0 f f t f s 1 25 "26" _null_ _null_ _null_	pg_get_triggerdef - _null_ ));
DESCR("trigger description");
DATA(insert OID = 1387 (  pg_get_constraintdef PGNSP PGUID 12 1 0 f f t f s 1 25 "26" _null_ _null_ _null_	pg_get_constraintdef - _null_ ));
DESCR("constraint description");
DATA(insert OID = 1716 (  pg_get_expr		   PGNSP PGUID 12 1 0 f f t f s 2 25 "25 26" _null_ _null_ _null_ pg_get_expr - _null_ ));
DESCR("deparse an encoded expression");
DATA(insert OID = 1665 (  pg_get_serial_sequence	PGNSP PGUID 12 1 0 f f t f s 2 25 "25 25" _null_ _null_ _null_	pg_get_serial_sequence - _null_ ));
DESCR("name of sequence for a serial column");


/* Generic referential integrity constraint triggers */
DATA(insert OID = 1644 (  RI_FKey_check_ins		PGNSP PGUID 12 1 0 f f t f v 0 2279 "" _null_ _null_ _null_ RI_FKey_check_ins - _null_ ));
DESCR("referential integrity FOREIGN KEY ... REFERENCES");
DATA(insert OID = 1645 (  RI_FKey_check_upd		PGNSP PGUID 12 1 0 f f t f v 0 2279 "" _null_ _null_ _null_ RI_FKey_check_upd - _null_ ));
DESCR("referential integrity FOREIGN KEY ... REFERENCES");
DATA(insert OID = 1646 (  RI_FKey_cascade_del	PGNSP PGUID 12 1 0 f f t f v 0 2279 "" _null_ _null_ _null_ RI_FKey_cascade_del - _null_ ));
DESCR("referential integrity ON DELETE CASCADE");
DATA(insert OID = 1647 (  RI_FKey_cascade_upd	PGNSP PGUID 12 1 0 f f t f v 0 2279 "" _null_ _null_ _null_ RI_FKey_cascade_upd - _null_ ));
DESCR("referential integrity ON UPDATE CASCADE");
DATA(insert OID = 1648 (  RI_FKey_restrict_del	PGNSP PGUID 12 1 0 f f t f v 0 2279 "" _null_ _null_ _null_ RI_FKey_restrict_del - _null_ ));
DESCR("referential integrity ON DELETE RESTRICT");
DATA(insert OID = 1649 (  RI_FKey_restrict_upd	PGNSP PGUID 12 1 0 f f t f v 0 2279 "" _null_ _null_ _null_ RI_FKey_restrict_upd - _null_ ));
DESCR("referential integrity ON UPDATE RESTRICT");
DATA(insert OID = 1650 (  RI_FKey_setnull_del	PGNSP PGUID 12 1 0 f f t f v 0 2279 "" _null_ _null_ _null_ RI_FKey_setnull_del - _null_ ));
DESCR("referential integrity ON DELETE SET NULL");
DATA(insert OID = 1651 (  RI_FKey_setnull_upd	PGNSP PGUID 12 1 0 f f t f v 0 2279 "" _null_ _null_ _null_ RI_FKey_setnull_upd - _null_ ));
DESCR("referential integrity ON UPDATE SET NULL");
DATA(insert OID = 1652 (  RI_FKey_setdefault_del PGNSP PGUID 12 1 0 f f t f v 0 2279 "" _null_ _null_ _null_ RI_FKey_setdefault_del - _null_ ));
DESCR("referential integrity ON DELETE SET DEFAULT");
DATA(insert OID = 1653 (  RI_FKey_setdefault_upd PGNSP PGUID 12 1 0 f f t f v 0 2279 "" _null_ _null_ _null_ RI_FKey_setdefault_upd - _null_ ));
DESCR("referential integrity ON UPDATE SET DEFAULT");
DATA(insert OID = 1654 (  RI_FKey_noaction_del PGNSP PGUID 12 1 0 f f t f v 0 2279 "" _null_ _null_ _null_	RI_FKey_noaction_del - _null_ ));
DESCR("referential integrity ON DELETE NO ACTION");
DATA(insert OID = 1655 (  RI_FKey_noaction_upd PGNSP PGUID 12 1 0 f f t f v 0 2279 "" _null_ _null_ _null_	RI_FKey_noaction_upd - _null_ ));
DESCR("referential integrity ON UPDATE NO ACTION");

DATA(insert OID = 1666 (  varbiteq			PGNSP PGUID 12 1 0 f f t f i 2 16 "1562 1562" _null_ _null_ _null_	biteq - _null_ ));
DESCR("equal");
DATA(insert OID = 1667 (  varbitne			PGNSP PGUID 12 1 0 f f t f i 2 16 "1562 1562" _null_ _null_ _null_	bitne - _null_ ));
DESCR("not equal");
DATA(insert OID = 1668 (  varbitge			PGNSP PGUID 12 1 0 f f t f i 2 16 "1562 1562" _null_ _null_ _null_	bitge - _null_ ));
DESCR("greater than or equal");
DATA(insert OID = 1669 (  varbitgt			PGNSP PGUID 12 1 0 f f t f i 2 16 "1562 1562" _null_ _null_ _null_	bitgt - _null_ ));
DESCR("greater than");
DATA(insert OID = 1670 (  varbitle			PGNSP PGUID 12 1 0 f f t f i 2 16 "1562 1562" _null_ _null_ _null_	bitle - _null_ ));
DESCR("less than or equal");
DATA(insert OID = 1671 (  varbitlt			PGNSP PGUID 12 1 0 f f t f i 2 16 "1562 1562" _null_ _null_ _null_	bitlt - _null_ ));
DESCR("less than");
DATA(insert OID = 1672 (  varbitcmp			PGNSP PGUID 12 1 0 f f t f i 2 23 "1562 1562" _null_ _null_ _null_	bitcmp - _null_ ));
DESCR("compare");

DATA(insert OID = 1673 (  bitand			PGNSP PGUID 12 1 0 f f t f i 2 1560 "1560 1560" _null_ _null_ _null_	bitand - _null_ ));
DESCR("bitwise and");
DATA(insert OID = 1674 (  bitor				PGNSP PGUID 12 1 0 f f t f i 2 1560 "1560 1560" _null_ _null_ _null_	bitor - _null_ ));
DESCR("bitwise or");
DATA(insert OID = 1675 (  bitxor			PGNSP PGUID 12 1 0 f f t f i 2 1560 "1560 1560" _null_ _null_ _null_	bitxor - _null_ ));
DESCR("bitwise exclusive or");
DATA(insert OID = 1676 (  bitnot			PGNSP PGUID 12 1 0 f f t f i 1 1560 "1560" _null_ _null_ _null_ bitnot - _null_ ));
DESCR("bitwise not");
DATA(insert OID = 1677 (  bitshiftleft		PGNSP PGUID 12 1 0 f f t f i 2 1560 "1560 23" _null_ _null_ _null_	bitshiftleft - _null_ ));
DESCR("bitwise left shift");
DATA(insert OID = 1678 (  bitshiftright		PGNSP PGUID 12 1 0 f f t f i 2 1560 "1560 23" _null_ _null_ _null_	bitshiftright - _null_ ));
DESCR("bitwise right shift");
DATA(insert OID = 1679 (  bitcat			PGNSP PGUID 12 1 0 f f t f i 2 1562 "1562 1562" _null_ _null_ _null_	bitcat - _null_ ));
DESCR("bitwise concatenation");
DATA(insert OID = 1680 (  substring			PGNSP PGUID 12 1 0 f f t f i 3 1560 "1560 23 23" _null_ _null_ _null_ bitsubstr - _null_ ));
DESCR("return portion of bitstring");
DATA(insert OID = 1681 (  length			PGNSP PGUID 12 1 0 f f t f i 1 23 "1560" _null_ _null_ _null_ bitlength - _null_ ));
DESCR("bitstring length");
DATA(insert OID = 1682 (  octet_length		PGNSP PGUID 12 1 0 f f t f i 1 23 "1560" _null_ _null_ _null_ bitoctetlength - _null_ ));
DESCR("octet length");
DATA(insert OID = 1683 (  bit				PGNSP PGUID 12 1 0 f f t f i 2 1560 "23 23" _null_ _null_ _null_	bitfromint4 - _null_ ));
DESCR("int4 to bitstring");
DATA(insert OID = 1684 (  int4				PGNSP PGUID 12 1 0 f f t f i 1 23 "1560" _null_ _null_ _null_ bittoint4 - _null_ ));
DESCR("bitstring to int4");

DATA(insert OID = 1685 (  bit			   PGNSP PGUID 12 1 0 f f t f i 3 1560 "1560 23 16" _null_ _null_ _null_ bit - _null_ ));
DESCR("adjust bit() to typmod length");
DATA(insert OID = 1687 (  varbit		   PGNSP PGUID 12 1 0 f f t f i 3 1562 "1562 23 16" _null_ _null_ _null_ varbit - _null_ ));
DESCR("adjust varbit() to typmod length");

DATA(insert OID = 1698 (  position		   PGNSP PGUID 12 1 0 f f t f i 2 23 "1560 1560" _null_ _null_ _null_ bitposition - _null_ ));
DESCR("return position of sub-bitstring");
DATA(insert OID = 1699 (  substring			PGNSP PGUID 14 1 0 f f t f i 2 1560 "1560 23" _null_ _null_ _null_	"select pg_catalog.substring($1, $2, -1)" - _null_ ));
DESCR("return portion of bitstring");


/* for mac type support */
DATA(insert OID = 436 (  macaddr_in			PGNSP PGUID 12 1 0 f f t f i 1 829 "2275" _null_ _null_ _null_	macaddr_in - _null_ ));
DESCR("I/O");
DATA(insert OID = 437 (  macaddr_out		PGNSP PGUID 12 1 0 f f t f i 1 2275 "829" _null_ _null_ _null_	macaddr_out - _null_ ));
DESCR("I/O");

DATA(insert OID = 753 (  trunc				PGNSP PGUID 12 1 0 f f t f i 1 829 "829" _null_ _null_ _null_ macaddr_trunc - _null_ ));
DESCR("MAC manufacturer fields");

DATA(insert OID = 830 (  macaddr_eq			PGNSP PGUID 12 1 0 f f t f i 2 16 "829 829" _null_ _null_ _null_	macaddr_eq - _null_ ));
DESCR("equal");
DATA(insert OID = 831 (  macaddr_lt			PGNSP PGUID 12 1 0 f f t f i 2 16 "829 829" _null_ _null_ _null_	macaddr_lt - _null_ ));
DESCR("less-than");
DATA(insert OID = 832 (  macaddr_le			PGNSP PGUID 12 1 0 f f t f i 2 16 "829 829" _null_ _null_ _null_	macaddr_le - _null_ ));
DESCR("less-than-or-equal");
DATA(insert OID = 833 (  macaddr_gt			PGNSP PGUID 12 1 0 f f t f i 2 16 "829 829" _null_ _null_ _null_	macaddr_gt - _null_ ));
DESCR("greater-than");
DATA(insert OID = 834 (  macaddr_ge			PGNSP PGUID 12 1 0 f f t f i 2 16 "829 829" _null_ _null_ _null_	macaddr_ge - _null_ ));
DESCR("greater-than-or-equal");
DATA(insert OID = 835 (  macaddr_ne			PGNSP PGUID 12 1 0 f f t f i 2 16 "829 829" _null_ _null_ _null_	macaddr_ne - _null_ ));
DESCR("not equal");
DATA(insert OID = 836 (  macaddr_cmp		PGNSP PGUID 12 1 0 f f t f i 2 23 "829 829" _null_ _null_ _null_	macaddr_cmp - _null_ ));
DESCR("less-equal-greater");

/* for inet type support */
DATA(insert OID = 910 (  inet_in			PGNSP PGUID 12 1 0 f f t f i 1 869 "2275" _null_ _null_ _null_	inet_in - _null_ ));
DESCR("I/O");
DATA(insert OID = 911 (  inet_out			PGNSP PGUID 12 1 0 f f t f i 1 2275 "869" _null_ _null_ _null_	inet_out - _null_ ));
DESCR("I/O");

/* for cidr type support */
DATA(insert OID = 1267 (  cidr_in			PGNSP PGUID 12 1 0 f f t f i 1 650 "2275" _null_ _null_ _null_	cidr_in - _null_ ));
DESCR("I/O");
DATA(insert OID = 1427 (  cidr_out			PGNSP PGUID 12 1 0 f f t f i 1 2275 "650" _null_ _null_ _null_	cidr_out - _null_ ));
DESCR("I/O");

/* these are used for both inet and cidr */
DATA(insert OID = 920 (  network_eq			PGNSP PGUID 12 1 0 f f t f i 2 16 "869 869" _null_ _null_ _null_	network_eq - _null_ ));
DESCR("equal");
DATA(insert OID = 921 (  network_lt			PGNSP PGUID 12 1 0 f f t f i 2 16 "869 869" _null_ _null_ _null_	network_lt - _null_ ));
DESCR("less-than");
DATA(insert OID = 922 (  network_le			PGNSP PGUID 12 1 0 f f t f i 2 16 "869 869" _null_ _null_ _null_	network_le - _null_ ));
DESCR("less-than-or-equal");
DATA(insert OID = 923 (  network_gt			PGNSP PGUID 12 1 0 f f t f i 2 16 "869 869" _null_ _null_ _null_	network_gt - _null_ ));
DESCR("greater-than");
DATA(insert OID = 924 (  network_ge			PGNSP PGUID 12 1 0 f f t f i 2 16 "869 869" _null_ _null_ _null_	network_ge - _null_ ));
DESCR("greater-than-or-equal");
DATA(insert OID = 925 (  network_ne			PGNSP PGUID 12 1 0 f f t f i 2 16 "869 869" _null_ _null_ _null_	network_ne - _null_ ));
DESCR("not equal");
DATA(insert OID = 926 (  network_cmp		PGNSP PGUID 12 1 0 f f t f i 2 23 "869 869" _null_ _null_ _null_	network_cmp - _null_ ));
DESCR("less-equal-greater");
DATA(insert OID = 927 (  network_sub		PGNSP PGUID 12 1 0 f f t f i 2 16 "869 869" _null_ _null_ _null_	network_sub - _null_ ));
DESCR("is-subnet");
DATA(insert OID = 928 (  network_subeq		PGNSP PGUID 12 1 0 f f t f i 2 16 "869 869" _null_ _null_ _null_	network_subeq - _null_ ));
DESCR("is-subnet-or-equal");
DATA(insert OID = 929 (  network_sup		PGNSP PGUID 12 1 0 f f t f i 2 16 "869 869" _null_ _null_ _null_	network_sup - _null_ ));
DESCR("is-supernet");
DATA(insert OID = 930 (  network_supeq		PGNSP PGUID 12 1 0 f f t f i 2 16 "869 869" _null_ _null_ _null_	network_supeq - _null_ ));
DESCR("is-supernet-or-equal");

/* inet/cidr functions */
DATA(insert OID = 598 (  abbrev				PGNSP PGUID 12 1 0 f f t f i 1 25 "869" _null_ _null_ _null_	inet_abbrev - _null_ ));
DESCR("abbreviated display of inet value");
DATA(insert OID = 599 (  abbrev				PGNSP PGUID 12 1 0 f f t f i 1 25 "650" _null_ _null_ _null_	cidr_abbrev - _null_ ));
DESCR("abbreviated display of cidr value");
DATA(insert OID = 605 (  set_masklen		PGNSP PGUID 12 1 0 f f t f i 2 869 "869 23" _null_ _null_ _null_	inet_set_masklen - _null_ ));
DESCR("change netmask of inet");
DATA(insert OID = 635 (  set_masklen		PGNSP PGUID 12 1 0 f f t f i 2 650 "650 23" _null_ _null_ _null_	cidr_set_masklen - _null_ ));
DESCR("change netmask of cidr");
DATA(insert OID = 711 (  family				PGNSP PGUID 12 1 0 f f t f i 1 23 "869" _null_ _null_ _null_	network_family - _null_ ));
DESCR("address family (4 for IPv4, 6 for IPv6)");
DATA(insert OID = 683 (  network			PGNSP PGUID 12 1 0 f f t f i 1 650 "869" _null_ _null_ _null_ network_network - _null_ ));
DESCR("network part of address");
DATA(insert OID = 696 (  netmask			PGNSP PGUID 12 1 0 f f t f i 1 869 "869" _null_ _null_ _null_ network_netmask - _null_ ));
DESCR("netmask of address");
DATA(insert OID = 697 (  masklen			PGNSP PGUID 12 1 0 f f t f i 1 23 "869" _null_ _null_ _null_	network_masklen - _null_ ));
DESCR("netmask length");
DATA(insert OID = 698 (  broadcast			PGNSP PGUID 12 1 0 f f t f i 1 869 "869" _null_ _null_ _null_ network_broadcast - _null_ ));
DESCR("broadcast address of network");
DATA(insert OID = 699 (  host				PGNSP PGUID 12 1 0 f f t f i 1 25 "869" _null_ _null_ _null_	network_host - _null_ ));
DESCR("show address octets only");
DATA(insert OID = 730 (  text				PGNSP PGUID 12 1 0 f f t f i 1 25 "869" _null_ _null_ _null_	network_show - _null_ ));
DESCR("show all parts of inet/cidr value");
DATA(insert OID = 1362 (  hostmask			PGNSP PGUID 12 1 0 f f t f i 1 869 "869" _null_ _null_ _null_ network_hostmask - _null_ ));
DESCR("hostmask of address");
DATA(insert OID = 1715 (  cidr				PGNSP PGUID 12 1 0 f f t f i 1 650 "869" _null_ _null_ _null_	inet_to_cidr - _null_ ));
DESCR("coerce inet to cidr");

DATA(insert OID = 2196 (  inet_client_addr		PGNSP PGUID 12 1 0 f f f f s 0 869 "" _null_ _null_ _null_	inet_client_addr - _null_ ));
DESCR("inet address of the client");
DATA(insert OID = 2197 (  inet_client_port		PGNSP PGUID 12 1 0 f f f f s 0 23 "" _null_ _null_ _null_  inet_client_port - _null_ ));
DESCR("client's port number for this connection");
DATA(insert OID = 2198 (  inet_server_addr		PGNSP PGUID 12 1 0 f f f f s 0 869 "" _null_ _null_ _null_	inet_server_addr - _null_ ));
DESCR("inet address of the server");
DATA(insert OID = 2199 (  inet_server_port		PGNSP PGUID 12 1 0 f f f f s 0 23 "" _null_ _null_ _null_  inet_server_port - _null_ ));
DESCR("server's port number for this connection");

DATA(insert OID = 2627 (  inetnot			PGNSP PGUID 12 1 0 f f t f i 1 869 "869" _null_ _null_ _null_	inetnot - _null_ ));
DESCR("bitwise not");
DATA(insert OID = 2628 (  inetand			PGNSP PGUID 12 1 0 f f t f i 2 869 "869 869" _null_ _null_ _null_	inetand - _null_ ));
DESCR("bitwise and");
DATA(insert OID = 2629 (  inetor			PGNSP PGUID 12 1 0 f f t f i 2 869 "869 869" _null_ _null_ _null_	inetor - _null_ ));
DESCR("bitwise or");
DATA(insert OID = 2630 (  inetpl			PGNSP PGUID 12 1 0 f f t f i 2 869 "869 20" _null_ _null_ _null_	inetpl - _null_ ));
DESCR("add integer to inet value");
DATA(insert OID = 2631 (  int8pl_inet		PGNSP PGUID 14 1 0 f f t f i 2 869 "20 869" _null_ _null_ _null_	"select $2 + $1" - _null_ ));
DESCR("add integer to inet value");
DATA(insert OID = 2632 (  inetmi_int8		PGNSP PGUID 12 1 0 f f t f i 2 869 "869 20" _null_ _null_ _null_	inetmi_int8 - _null_ ));
DESCR("subtract integer from inet value");
DATA(insert OID = 2633 (  inetmi			PGNSP PGUID 12 1 0 f f t f i 2 20 "869 869" _null_ _null_ _null_	inetmi - _null_ ));
DESCR("subtract inet values");

DATA(insert OID = 1690 ( time_mi_time		PGNSP PGUID 12 1 0 f f t f i 2 1186 "1083 1083" _null_ _null_ _null_	time_mi_time - _null_ ));
DESCR("minus");

DATA(insert OID =  1691 (  boolle			PGNSP PGUID 12 1 0 f f t f i 2 16 "16 16" _null_ _null_ _null_	boolle - _null_ ));
DESCR("less-than-or-equal");
DATA(insert OID =  1692 (  boolge			PGNSP PGUID 12 1 0 f f t f i 2 16 "16 16" _null_ _null_ _null_	boolge - _null_ ));
DESCR("greater-than-or-equal");
DATA(insert OID = 1693 (  btboolcmp			PGNSP PGUID 12 1 0 f f t f i 2 23 "16 16" _null_ _null_ _null_	btboolcmp - _null_ ));
DESCR("btree less-equal-greater");

DATA(insert OID = 1696 (  timetz_hash		PGNSP PGUID 12 1 0 f f t f i 1 23 "1266" _null_ _null_ _null_ timetz_hash - _null_ ));
DESCR("hash");
DATA(insert OID = 1697 (  interval_hash		PGNSP PGUID 12 1 0 f f t f i 1 23 "1186" _null_ _null_ _null_ interval_hash - _null_ ));
DESCR("hash");


/* OID's 1700 - 1799 NUMERIC data type */
DATA(insert OID = 1701 ( numeric_in				PGNSP PGUID 12 1 0 f f t f i 3 1700 "2275 26 23" _null_ _null_ _null_  numeric_in - _null_ ));
DESCR("I/O");
DATA(insert OID = 1702 ( numeric_out			PGNSP PGUID 12 1 0 f f t f i 1 2275 "1700" _null_ _null_ _null_ numeric_out - _null_ ));
DESCR("I/O");
DATA(insert OID = 2917 (  numerictypmodin   	PGNSP PGUID 12 1 0 f f t f i 1 23 "1263" _null_ _null_ _null_	numerictypmodin - _null_ ));
DESCR("I/O typmod");
DATA(insert OID = 2918 (  numerictypmodout  	PGNSP PGUID 12 1 0 f f t f i 1 2275 "23" _null_ _null_ _null_	numerictypmodout - _null_ ));
DESCR("I/O typmod");
DATA(insert OID = 1703 ( numeric				PGNSP PGUID 12 1 0 f f t f i 2 1700 "1700 23" _null_ _null_ _null_	numeric - _null_ ));
DESCR("adjust numeric to typmod precision/scale");
DATA(insert OID = 1704 ( numeric_abs			PGNSP PGUID 12 1 0 f f t f i 1 1700 "1700" _null_ _null_ _null_ numeric_abs - _null_ ));
DESCR("absolute value");
DATA(insert OID = 1705 ( abs					PGNSP PGUID 12 1 0 f f t f i 1 1700 "1700" _null_ _null_ _null_ numeric_abs - _null_ ));
DESCR("absolute value");
DATA(insert OID = 1706 ( sign					PGNSP PGUID 12 1 0 f f t f i 1 1700 "1700" _null_ _null_ _null_ numeric_sign - _null_ ));
DESCR("sign of value");
DATA(insert OID = 1707 ( round					PGNSP PGUID 12 1 0 f f t f i 2 1700 "1700 23" _null_ _null_ _null_	numeric_round - _null_ ));
DESCR("value rounded to 'scale'");
DATA(insert OID = 1708 ( round					PGNSP PGUID 14 1 0 f f t f i 1 1700 "1700" _null_ _null_ _null_ "select pg_catalog.round($1,0)" - _null_ ));
DESCR("value rounded to 'scale' of zero");
DATA(insert OID = 1709 ( trunc					PGNSP PGUID 12 1 0 f f t f i 2 1700 "1700 23" _null_ _null_ _null_	numeric_trunc - _null_ ));
DESCR("value truncated to 'scale'");
DATA(insert OID = 1710 ( trunc					PGNSP PGUID 14 1 0 f f t f i 1 1700 "1700" _null_ _null_ _null_ "select pg_catalog.trunc($1,0)" - _null_ ));
DESCR("value truncated to 'scale' of zero");
DATA(insert OID = 1711 ( ceil					PGNSP PGUID 12 1 0 f f t f i 1 1700 "1700" _null_ _null_ _null_ numeric_ceil - _null_ ));
DESCR("smallest integer >= value");
DATA(insert OID = 2167 ( ceiling				PGNSP PGUID 12 1 0 f f t f i 1 1700 "1700" _null_ _null_ _null_ numeric_ceil - _null_ ));
DESCR("smallest integer >= value");
DATA(insert OID = 1712 ( floor					PGNSP PGUID 12 1 0 f f t f i 1 1700 "1700" _null_ _null_ _null_ numeric_floor - _null_ ));
DESCR("largest integer <= value");
DATA(insert OID = 1718 ( numeric_eq				PGNSP PGUID 12 1 0 f f t f i 2 16 "1700 1700" _null_ _null_ _null_	numeric_eq - _null_ ));
DESCR("equal");
DATA(insert OID = 1719 ( numeric_ne				PGNSP PGUID 12 1 0 f f t f i 2 16 "1700 1700" _null_ _null_ _null_	numeric_ne - _null_ ));
DESCR("not equal");
DATA(insert OID = 1720 ( numeric_gt				PGNSP PGUID 12 1 0 f f t f i 2 16 "1700 1700" _null_ _null_ _null_	numeric_gt - _null_ ));
DESCR("greater-than");
DATA(insert OID = 1721 ( numeric_ge				PGNSP PGUID 12 1 0 f f t f i 2 16 "1700 1700" _null_ _null_ _null_	numeric_ge - _null_ ));
DESCR("greater-than-or-equal");
DATA(insert OID = 1722 ( numeric_lt				PGNSP PGUID 12 1 0 f f t f i 2 16 "1700 1700" _null_ _null_ _null_	numeric_lt - _null_ ));
DESCR("less-than");
DATA(insert OID = 1723 ( numeric_le				PGNSP PGUID 12 1 0 f f t f i 2 16 "1700 1700" _null_ _null_ _null_	numeric_le - _null_ ));
DESCR("less-than-or-equal");
DATA(insert OID = 1724 ( numeric_add			PGNSP PGUID 12 1 0 f f t f i 2 1700 "1700 1700" _null_ _null_ _null_	numeric_add - _null_ ));
DESCR("add");
DATA(insert OID = 1725 ( numeric_sub			PGNSP PGUID 12 1 0 f f t f i 2 1700 "1700 1700" _null_ _null_ _null_	numeric_sub - _null_ ));
DESCR("subtract");
DATA(insert OID = 1726 ( numeric_mul			PGNSP PGUID 12 1 0 f f t f i 2 1700 "1700 1700" _null_ _null_ _null_	numeric_mul - _null_ ));
DESCR("multiply");
DATA(insert OID = 1727 ( numeric_div			PGNSP PGUID 12 1 0 f f t f i 2 1700 "1700 1700" _null_ _null_ _null_	numeric_div - _null_ ));
DESCR("divide");
DATA(insert OID = 1728 ( mod					PGNSP PGUID 12 1 0 f f t f i 2 1700 "1700 1700" _null_ _null_ _null_	numeric_mod - _null_ ));
DESCR("modulus");
DATA(insert OID = 1729 ( numeric_mod			PGNSP PGUID 12 1 0 f f t f i 2 1700 "1700 1700" _null_ _null_ _null_	numeric_mod - _null_ ));
DESCR("modulus");
DATA(insert OID = 1730 ( sqrt					PGNSP PGUID 12 1 0 f f t f i 1 1700 "1700" _null_ _null_ _null_ numeric_sqrt - _null_ ));
DESCR("square root");
DATA(insert OID = 1731 ( numeric_sqrt			PGNSP PGUID 12 1 0 f f t f i 1 1700 "1700" _null_ _null_ _null_ numeric_sqrt - _null_ ));
DESCR("square root");
DATA(insert OID = 1732 ( exp					PGNSP PGUID 12 1 0 f f t f i 1 1700 "1700" _null_ _null_ _null_ numeric_exp - _null_ ));
DESCR("e raised to the power of n");
DATA(insert OID = 1733 ( numeric_exp			PGNSP PGUID 12 1 0 f f t f i 1 1700 "1700" _null_ _null_ _null_ numeric_exp - _null_ ));
DESCR("e raised to the power of n");
DATA(insert OID = 1734 ( ln						PGNSP PGUID 12 1 0 f f t f i 1 1700 "1700" _null_ _null_ _null_ numeric_ln - _null_ ));
DESCR("natural logarithm of n");
DATA(insert OID = 1735 ( numeric_ln				PGNSP PGUID 12 1 0 f f t f i 1 1700 "1700" _null_ _null_ _null_ numeric_ln - _null_ ));
DESCR("natural logarithm of n");
DATA(insert OID = 1736 ( log					PGNSP PGUID 12 1 0 f f t f i 2 1700 "1700 1700" _null_ _null_ _null_	numeric_log - _null_ ));
DESCR("logarithm base m of n");
DATA(insert OID = 1737 ( numeric_log			PGNSP PGUID 12 1 0 f f t f i 2 1700 "1700 1700" _null_ _null_ _null_	numeric_log - _null_ ));
DESCR("logarithm base m of n");
DATA(insert OID = 1738 ( pow					PGNSP PGUID 12 1 0 f f t f i 2 1700 "1700 1700" _null_ _null_ _null_	numeric_power - _null_ ));
DESCR("m raised to the power of n");
DATA(insert OID = 2169 ( power					PGNSP PGUID 12 1 0 f f t f i 2 1700 "1700 1700" _null_ _null_ _null_	numeric_power - _null_ ));
DESCR("m raised to the power of n");
DATA(insert OID = 1739 ( numeric_power			PGNSP PGUID 12 1 0 f f t f i 2 1700 "1700 1700" _null_ _null_ _null_	numeric_power - _null_ ));
DESCR("m raised to the power of n");
DATA(insert OID = 1740 ( numeric				PGNSP PGUID 12 1 0 f f t f i 1 1700 "23" _null_ _null_ _null_ int4_numeric - _null_ ));
DESCR("(internal)");
DATA(insert OID = 1741 ( log					PGNSP PGUID 14 1 0 f f t f i 1 1700 "1700" _null_ _null_ _null_ "select pg_catalog.log(10, $1)" - _null_ ));
DESCR("logarithm base 10 of n");
DATA(insert OID = 1742 ( numeric				PGNSP PGUID 12 1 0 f f t f i 1 1700 "700" _null_ _null_ _null_	float4_numeric - _null_ ));
DESCR("(internal)");
DATA(insert OID = 1743 ( numeric				PGNSP PGUID 12 1 0 f f t f i 1 1700 "701" _null_ _null_ _null_	float8_numeric - _null_ ));
DESCR("(internal)");
DATA(insert OID = 1744 ( int4					PGNSP PGUID 12 1 0 f f t f i 1 23 "1700" _null_ _null_ _null_ numeric_int4 - _null_ ));
DESCR("(internal)");
DATA(insert OID = 1745 ( float4					PGNSP PGUID 12 1 0 f f t f i 1 700 "1700" _null_ _null_ _null_	numeric_float4 - _null_ ));
DESCR("(internal)");
DATA(insert OID = 1746 ( float8					PGNSP PGUID 12 1 0 f f t f i 1 701 "1700" _null_ _null_ _null_	numeric_float8 - _null_ ));
DESCR("(internal)");
DATA(insert OID = 2170 ( width_bucket			PGNSP PGUID 12 1 0 f f t f i 4 23 "1700 1700 1700 23" _null_ _null_ _null_	width_bucket_numeric - _null_ ));
DESCR("bucket number of operand in equidepth histogram");

DATA(insert OID = 1747 ( time_pl_interval		PGNSP PGUID 12 1 0 f f t f i 2 1083 "1083 1186" _null_ _null_ _null_	time_pl_interval - _null_ ));
DESCR("plus");
DATA(insert OID = 1748 ( time_mi_interval		PGNSP PGUID 12 1 0 f f t f i 2 1083 "1083 1186" _null_ _null_ _null_	time_mi_interval - _null_ ));
DESCR("minus");
DATA(insert OID = 1749 ( timetz_pl_interval		PGNSP PGUID 12 1 0 f f t f i 2 1266 "1266 1186" _null_ _null_ _null_	timetz_pl_interval - _null_ ));
DESCR("plus");
DATA(insert OID = 1750 ( timetz_mi_interval		PGNSP PGUID 12 1 0 f f t f i 2 1266 "1266 1186" _null_ _null_ _null_	timetz_mi_interval - _null_ ));
DESCR("minus");

DATA(insert OID = 1764 ( numeric_inc			PGNSP PGUID 12 1 0 f f t f i 1 1700 "1700" _null_ _null_ _null_ numeric_inc - _null_ ));
DESCR("increment by one");
DATA(insert OID = 1766 ( numeric_smaller		PGNSP PGUID 12 1 0 f f t f i 2 1700 "1700 1700" _null_ _null_ _null_	numeric_smaller - _null_ ));
DESCR("smaller of two numbers");
DATA(insert OID = 1767 ( numeric_larger			PGNSP PGUID 12 1 0 f f t f i 2 1700 "1700 1700" _null_ _null_ _null_	numeric_larger - _null_ ));
DESCR("larger of two numbers");
DATA(insert OID = 1769 ( numeric_cmp			PGNSP PGUID 12 1 0 f f t f i 2 23 "1700 1700" _null_ _null_ _null_	numeric_cmp - _null_ ));
DESCR("compare two numbers");
DATA(insert OID = 1771 ( numeric_uminus			PGNSP PGUID 12 1 0 f f t f i 1 1700 "1700" _null_ _null_ _null_ numeric_uminus - _null_ ));
DESCR("negate");
DATA(insert OID = 1779 ( int8					PGNSP PGUID 12 1 0 f f t f i 1 20 "1700" _null_ _null_ _null_ numeric_int8 - _null_ ));
DESCR("(internal)");
DATA(insert OID = 1781 ( numeric				PGNSP PGUID 12 1 0 f f t f i 1 1700 "20" _null_ _null_ _null_ int8_numeric - _null_ ));
DESCR("(internal)");
DATA(insert OID = 1782 ( numeric				PGNSP PGUID 12 1 0 f f t f i 1 1700 "21" _null_ _null_ _null_ int2_numeric - _null_ ));
DESCR("(internal)");
DATA(insert OID = 1783 ( int2					PGNSP PGUID 12 1 0 f f t f i 1 21 "1700" _null_ _null_ _null_ numeric_int2 - _null_ ));
DESCR("(internal)");

/* formatting */
DATA(insert OID = 1770 ( to_char			PGNSP PGUID 12 1 0 f f t f s 2	25 "1184 25" _null_ _null_ _null_  timestamptz_to_char - _null_ ));
DESCR("format timestamp with time zone to text");
DATA(insert OID = 1772 ( to_char			PGNSP PGUID 12 1 0 f f t f s 2	25 "1700 25" _null_ _null_ _null_  numeric_to_char - _null_ ));
DESCR("format numeric to text");
DATA(insert OID = 1773 ( to_char			PGNSP PGUID 12 1 0 f f t f s 2	25 "23 25" _null_ _null_ _null_ int4_to_char - _null_ ));
DESCR("format int4 to text");
DATA(insert OID = 1774 ( to_char			PGNSP PGUID 12 1 0 f f t f s 2	25 "20 25" _null_ _null_ _null_ int8_to_char - _null_ ));
DESCR("format int8 to text");
DATA(insert OID = 1775 ( to_char			PGNSP PGUID 12 1 0 f f t f s 2	25 "700 25" _null_ _null_ _null_	float4_to_char - _null_ ));
DESCR("format float4 to text");
DATA(insert OID = 1776 ( to_char			PGNSP PGUID 12 1 0 f f t f s 2	25 "701 25" _null_ _null_ _null_	float8_to_char - _null_ ));
DESCR("format float8 to text");
DATA(insert OID = 1777 ( to_number			PGNSP PGUID 12 1 0 f f t f s 2	1700 "25 25" _null_ _null_ _null_  numeric_to_number - _null_ ));
DESCR("convert text to numeric");
DATA(insert OID = 1778 ( to_timestamp		PGNSP PGUID 12 1 0 f f t f s 2	1184 "25 25" _null_ _null_ _null_  to_timestamp - _null_ ));
DESCR("convert text to timestamp with time zone");
DATA(insert OID = 1780 ( to_date			PGNSP PGUID 12 1 0 f f t f s 2	1082 "25 25" _null_ _null_ _null_  to_date - _null_ ));
DESCR("convert text to date");
DATA(insert OID = 1768 ( to_char			PGNSP PGUID 12 1 0 f f t f s 2	25 "1186 25" _null_ _null_ _null_  interval_to_char - _null_ ));
DESCR("format interval to text");

DATA(insert OID =  1282 ( quote_ident	   PGNSP PGUID 12 1 0 f f t f i 1 25 "25" _null_ _null_ _null_ quote_ident - _null_ ));
DESCR("quote an identifier for usage in a querystring");
DATA(insert OID =  1283 ( quote_literal    PGNSP PGUID 12 1 0 f f t f i 1 25 "25" _null_ _null_ _null_ quote_literal - _null_ ));
DESCR("quote a literal for usage in a querystring");

DATA(insert OID = 1798 (  oidin			   PGNSP PGUID 12 1 0 f f t f i 1 26 "2275" _null_ _null_ _null_ oidin - _null_ ));
DESCR("I/O");
DATA(insert OID = 1799 (  oidout		   PGNSP PGUID 12 1 0 f f t f i 1 2275 "26" _null_ _null_ _null_ oidout - _null_ ));
DESCR("I/O");


DATA(insert OID = 1810 (  bit_length	   PGNSP PGUID 14 1 0 f f t f i 1 23 "17" _null_ _null_ _null_ "select pg_catalog.octet_length($1) * 8" - _null_ ));
DESCR("length in bits");
DATA(insert OID = 1811 (  bit_length	   PGNSP PGUID 14 1 0 f f t f i 1 23 "25" _null_ _null_ _null_ "select pg_catalog.octet_length($1) * 8" - _null_ ));
DESCR("length in bits");
DATA(insert OID = 1812 (  bit_length	   PGNSP PGUID 14 1 0 f f t f i 1 23 "1560" _null_ _null_ _null_ "select pg_catalog.length($1)" - _null_ ));
DESCR("length in bits");

/* Selectivity estimators for LIKE and related operators */
DATA(insert OID = 1814 ( iclikesel			PGNSP PGUID 12 1 0 f f t f s 4 701 "2281 26 2281 23" _null_ _null_ _null_  iclikesel - _null_ ));
DESCR("restriction selectivity of ILIKE");
DATA(insert OID = 1815 ( icnlikesel			PGNSP PGUID 12 1 0 f f t f s 4 701 "2281 26 2281 23" _null_ _null_ _null_  icnlikesel - _null_ ));
DESCR("restriction selectivity of NOT ILIKE");
DATA(insert OID = 1816 ( iclikejoinsel		PGNSP PGUID 12 1 0 f f t f s 4 701 "2281 26 2281 21" _null_ _null_ _null_  iclikejoinsel - _null_ ));
DESCR("join selectivity of ILIKE");
DATA(insert OID = 1817 ( icnlikejoinsel		PGNSP PGUID 12 1 0 f f t f s 4 701 "2281 26 2281 21" _null_ _null_ _null_  icnlikejoinsel - _null_ ));
DESCR("join selectivity of NOT ILIKE");
DATA(insert OID = 1818 ( regexeqsel			PGNSP PGUID 12 1 0 f f t f s 4 701 "2281 26 2281 23" _null_ _null_ _null_  regexeqsel - _null_ ));
DESCR("restriction selectivity of regex match");
DATA(insert OID = 1819 ( likesel			PGNSP PGUID 12 1 0 f f t f s 4 701 "2281 26 2281 23" _null_ _null_ _null_  likesel - _null_ ));
DESCR("restriction selectivity of LIKE");
DATA(insert OID = 1820 ( icregexeqsel		PGNSP PGUID 12 1 0 f f t f s 4 701 "2281 26 2281 23" _null_ _null_ _null_  icregexeqsel - _null_ ));
DESCR("restriction selectivity of case-insensitive regex match");
DATA(insert OID = 1821 ( regexnesel			PGNSP PGUID 12 1 0 f f t f s 4 701 "2281 26 2281 23" _null_ _null_ _null_  regexnesel - _null_ ));
DESCR("restriction selectivity of regex non-match");
DATA(insert OID = 1822 ( nlikesel			PGNSP PGUID 12 1 0 f f t f s 4 701 "2281 26 2281 23" _null_ _null_ _null_  nlikesel - _null_ ));
DESCR("restriction selectivity of NOT LIKE");
DATA(insert OID = 1823 ( icregexnesel		PGNSP PGUID 12 1 0 f f t f s 4 701 "2281 26 2281 23" _null_ _null_ _null_  icregexnesel - _null_ ));
DESCR("restriction selectivity of case-insensitive regex non-match");
DATA(insert OID = 1824 ( regexeqjoinsel		PGNSP PGUID 12 1 0 f f t f s 4 701 "2281 26 2281 21" _null_ _null_ _null_  regexeqjoinsel - _null_ ));
DESCR("join selectivity of regex match");
DATA(insert OID = 1825 ( likejoinsel		PGNSP PGUID 12 1 0 f f t f s 4 701 "2281 26 2281 21" _null_ _null_ _null_  likejoinsel - _null_ ));
DESCR("join selectivity of LIKE");
DATA(insert OID = 1826 ( icregexeqjoinsel	PGNSP PGUID 12 1 0 f f t f s 4 701 "2281 26 2281 21" _null_ _null_ _null_  icregexeqjoinsel - _null_ ));
DESCR("join selectivity of case-insensitive regex match");
DATA(insert OID = 1827 ( regexnejoinsel		PGNSP PGUID 12 1 0 f f t f s 4 701 "2281 26 2281 21" _null_ _null_ _null_  regexnejoinsel - _null_ ));
DESCR("join selectivity of regex non-match");
DATA(insert OID = 1828 ( nlikejoinsel		PGNSP PGUID 12 1 0 f f t f s 4 701 "2281 26 2281 21" _null_ _null_ _null_  nlikejoinsel - _null_ ));
DESCR("join selectivity of NOT LIKE");
DATA(insert OID = 1829 ( icregexnejoinsel	PGNSP PGUID 12 1 0 f f t f s 4 701 "2281 26 2281 21" _null_ _null_ _null_  icregexnejoinsel - _null_ ));
DESCR("join selectivity of case-insensitive regex non-match");

/* Aggregate-related functions */
DATA(insert OID = 1830 (  float8_avg	   PGNSP PGUID 12 1 0 f f t f i 1 701 "1022" _null_ _null_ _null_ float8_avg - _null_ ));
DESCR("AVG aggregate final function");
DATA(insert OID = 2512 (  float8_var_pop   PGNSP PGUID 12 1 0 f f t f i 1 701 "1022" _null_ _null_ _null_ float8_var_pop - _null_ ));
DESCR("VAR_POP aggregate final function");
DATA(insert OID = 1831 (  float8_var_samp  PGNSP PGUID 12 1 0 f f t f i 1 701 "1022" _null_ _null_ _null_ float8_var_samp - _null_ ));
DESCR("VAR_SAMP aggregate final function");
DATA(insert OID = 2513 (  float8_stddev_pop PGNSP PGUID 12 1 0 f f t f i 1 701 "1022" _null_ _null_ _null_ float8_stddev_pop - _null_ ));
DESCR("STDDEV_POP aggregate final function");
DATA(insert OID = 1832 (  float8_stddev_samp	PGNSP PGUID 12 1 0 f f t f i 1 701 "1022" _null_ _null_ _null_ float8_stddev_samp - _null_ ));
DESCR("STDDEV_SAMP aggregate final function");
DATA(insert OID = 1833 (  numeric_accum    PGNSP PGUID 12 1 0 f f t f i 2 1231 "1231 1700" _null_ _null_ _null_ numeric_accum - _null_ ));
DESCR("aggregate transition function");
DATA(insert OID = 2858 (  numeric_avg_accum    PGNSP PGUID 12 1 0 f f t f i 2 1231 "1231 1700" _null_ _null_ _null_ numeric_avg_accum - _null_ ));
DESCR("aggregate transition function");
DATA(insert OID = 1834 (  int2_accum	   PGNSP PGUID 12 1 0 f f t f i 2 1231 "1231 21" _null_ _null_ _null_ int2_accum - _null_ ));
DESCR("aggregate transition function");
DATA(insert OID = 1835 (  int4_accum	   PGNSP PGUID 12 1 0 f f t f i 2 1231 "1231 23" _null_ _null_ _null_ int4_accum - _null_ ));
DESCR("aggregate transition function");
DATA(insert OID = 1836 (  int8_accum	   PGNSP PGUID 12 1 0 f f t f i 2 1231 "1231 20" _null_ _null_ _null_ int8_accum - _null_ ));
DESCR("aggregate transition function");
DATA(insert OID = 2746 (  int8_avg_accum	   PGNSP PGUID 12 1 0 f f t f i 2 1231 "1231 20" _null_ _null_ _null_ int8_avg_accum - _null_ ));
DESCR("aggregate transition function");
DATA(insert OID = 1837 (  numeric_avg	   PGNSP PGUID 12 1 0 f f t f i 1 1700 "1231" _null_ _null_ _null_	numeric_avg - _null_ ));
DESCR("AVG aggregate final function");
DATA(insert OID = 2514 (  numeric_var_pop  PGNSP PGUID 12 1 0 f f t f i 1 1700 "1231" _null_ _null_ _null_	numeric_var_pop - _null_ ));
DESCR("VAR_POP aggregate final function");
DATA(insert OID = 1838 (  numeric_var_samp PGNSP PGUID 12 1 0 f f t f i 1 1700 "1231" _null_ _null_ _null_	numeric_var_samp - _null_ ));
DESCR("VAR_SAMP aggregate final function");
DATA(insert OID = 2596 (  numeric_stddev_pop PGNSP PGUID 12 1 0 f f t f i 1 1700 "1231" _null_ _null_ _null_	numeric_stddev_pop - _null_ ));
DESCR("STDDEV_POP aggregate final function");
DATA(insert OID = 1839 (  numeric_stddev_samp	PGNSP PGUID 12 1 0 f f t f i 1 1700 "1231" _null_ _null_ _null_ numeric_stddev_samp - _null_ ));
DESCR("STDDEV_SAMP aggregate final function");
DATA(insert OID = 1840 (  int2_sum		   PGNSP PGUID 12 1 0 f f f f i 2 20 "20 21" _null_ _null_ _null_ int2_sum - _null_ ));
DESCR("SUM(int2) transition function");
DATA(insert OID = 1841 (  int4_sum		   PGNSP PGUID 12 1 0 f f f f i 2 20 "20 23" _null_ _null_ _null_ int4_sum - _null_ ));
DESCR("SUM(int4) transition function");
DATA(insert OID = 1842 (  int8_sum		   PGNSP PGUID 12 1 0 f f f f i 2 1700 "1700 20" _null_ _null_ _null_ int8_sum - _null_ ));
DESCR("SUM(int8) transition function");
DATA(insert OID = 1843 (  interval_accum   PGNSP PGUID 12 1 0 f f t f i 2 1187 "1187 1186" _null_ _null_ _null_ interval_accum - _null_ ));
DESCR("aggregate transition function");
DATA(insert OID = 1844 (  interval_avg	   PGNSP PGUID 12 1 0 f f t f i 1 1186 "1187" _null_ _null_ _null_	interval_avg - _null_ ));
DESCR("AVG aggregate final function");
DATA(insert OID = 1962 (  int2_avg_accum   PGNSP PGUID 12 1 0 f f t f i 2 1016 "1016 21" _null_ _null_ _null_ int2_avg_accum - _null_ ));
DESCR("AVG(int2) transition function");
DATA(insert OID = 1963 (  int4_avg_accum   PGNSP PGUID 12 1 0 f f t f i 2 1016 "1016 23" _null_ _null_ _null_ int4_avg_accum - _null_ ));
DESCR("AVG(int4) transition function");
DATA(insert OID = 1964 (  int8_avg		   PGNSP PGUID 12 1 0 f f t f i 1 1700 "1016" _null_ _null_ _null_	int8_avg - _null_ ));
DESCR("AVG(int) aggregate final function");
DATA(insert OID = 2805 (  int8inc_float8_float8		PGNSP PGUID 12 1 0 f f t f i 3 20 "20 701 701" _null_ _null_ _null_ int8inc_float8_float8 - _null_ ));
DESCR("REGR_COUNT(double, double) transition function");
DATA(insert OID = 2806 (  float8_regr_accum			PGNSP PGUID 12 1 0 f f t f i 3 1022 "1022 701 701" _null_ _null_ _null_ float8_regr_accum - _null_ ));
DESCR("REGR_...(double, double) transition function");
DATA(insert OID = 2807 (  float8_regr_sxx			PGNSP PGUID 12 1 0 f f t f i 1 701 "1022" _null_ _null_ _null_ float8_regr_sxx - _null_ ));
DESCR("REGR_SXX(double, double) aggregate final function");
DATA(insert OID = 2808 (  float8_regr_syy			PGNSP PGUID 12 1 0 f f t f i 1 701 "1022" _null_ _null_ _null_ float8_regr_syy - _null_ ));
DESCR("REGR_SYY(double, double) aggregate final function");
DATA(insert OID = 2809 (  float8_regr_sxy			PGNSP PGUID 12 1 0 f f t f i 1 701 "1022" _null_ _null_ _null_ float8_regr_sxy - _null_ ));
DESCR("REGR_SXY(double, double) aggregate final function");
DATA(insert OID = 2810 (  float8_regr_avgx			PGNSP PGUID 12 1 0 f f t f i 1 701 "1022" _null_ _null_ _null_ float8_regr_avgx - _null_ ));
DESCR("REGR_AVGX(double, double) aggregate final function");
DATA(insert OID = 2811 (  float8_regr_avgy			PGNSP PGUID 12 1 0 f f t f i 1 701 "1022" _null_ _null_ _null_ float8_regr_avgy - _null_ ));
DESCR("REGR_AVGY(double, double) aggregate final function");
DATA(insert OID = 2812 (  float8_regr_r2			PGNSP PGUID 12 1 0 f f t f i 1 701 "1022" _null_ _null_ _null_ float8_regr_r2 - _null_ ));
DESCR("REGR_R2(double, double) aggregate final function");
DATA(insert OID = 2813 (  float8_regr_slope			PGNSP PGUID 12 1 0 f f t f i 1 701 "1022" _null_ _null_ _null_ float8_regr_slope - _null_ ));
DESCR("REGR_SLOPE(double, double) aggregate final function");
DATA(insert OID = 2814 (  float8_regr_intercept		PGNSP PGUID 12 1 0 f f t f i 1 701 "1022" _null_ _null_ _null_ float8_regr_intercept - _null_ ));
DESCR("REGR_INTERCEPT(double, double) aggregate final function");
DATA(insert OID = 2815 (  float8_covar_pop			PGNSP PGUID 12 1 0 f f t f i 1 701 "1022" _null_ _null_ _null_ float8_covar_pop - _null_ ));
DESCR("COVAR_POP(double, double) aggregate final function");
DATA(insert OID = 2816 (  float8_covar_samp			PGNSP PGUID 12 1 0 f f t f i 1 701 "1022" _null_ _null_ _null_ float8_covar_samp - _null_ ));
DESCR("COVAR_SAMP(double, double) aggregate final function");
DATA(insert OID = 2817 (  float8_corr				PGNSP PGUID 12 1 0 f f t f i 1 701 "1022" _null_ _null_ _null_ float8_corr - _null_ ));
DESCR("CORR(double, double) aggregate final function");

/* To ASCII conversion */
DATA(insert OID = 1845 ( to_ascii	PGNSP PGUID 12 1 0 f f t f i 1	25 "25" _null_ _null_ _null_	to_ascii_default - _null_ ));
DESCR("encode text from DB encoding to ASCII text");
DATA(insert OID = 1846 ( to_ascii	PGNSP PGUID 12 1 0 f f t f i 2	25 "25 23" _null_ _null_ _null_ to_ascii_enc - _null_ ));
DESCR("encode text from encoding to ASCII text");
DATA(insert OID = 1847 ( to_ascii	PGNSP PGUID 12 1 0 f f t f i 2	25 "25 19" _null_ _null_ _null_ to_ascii_encname - _null_ ));
DESCR("encode text from encoding to ASCII text");

DATA(insert OID = 1848 ( interval_pl_time	PGNSP PGUID 14 1 0 f f t f i 2 1083 "1186 1083" _null_ _null_ _null_	"select $2 + $1" - _null_ ));
DESCR("plus");

DATA(insert OID = 1850 (  int28eq		   PGNSP PGUID 12 1 0 f f t f i 2 16 "21 20" _null_ _null_ _null_ int28eq - _null_ ));
DESCR("equal");
DATA(insert OID = 1851 (  int28ne		   PGNSP PGUID 12 1 0 f f t f i 2 16 "21 20" _null_ _null_ _null_ int28ne - _null_ ));
DESCR("not equal");
DATA(insert OID = 1852 (  int28lt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "21 20" _null_ _null_ _null_ int28lt - _null_ ));
DESCR("less-than");
DATA(insert OID = 1853 (  int28gt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "21 20" _null_ _null_ _null_ int28gt - _null_ ));
DESCR("greater-than");
DATA(insert OID = 1854 (  int28le		   PGNSP PGUID 12 1 0 f f t f i 2 16 "21 20" _null_ _null_ _null_ int28le - _null_ ));
DESCR("less-than-or-equal");
DATA(insert OID = 1855 (  int28ge		   PGNSP PGUID 12 1 0 f f t f i 2 16 "21 20" _null_ _null_ _null_ int28ge - _null_ ));
DESCR("greater-than-or-equal");

DATA(insert OID = 1856 (  int82eq		   PGNSP PGUID 12 1 0 f f t f i 2 16 "20 21" _null_ _null_ _null_ int82eq - _null_ ));
DESCR("equal");
DATA(insert OID = 1857 (  int82ne		   PGNSP PGUID 12 1 0 f f t f i 2 16 "20 21" _null_ _null_ _null_ int82ne - _null_ ));
DESCR("not equal");
DATA(insert OID = 1858 (  int82lt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "20 21" _null_ _null_ _null_ int82lt - _null_ ));
DESCR("less-than");
DATA(insert OID = 1859 (  int82gt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "20 21" _null_ _null_ _null_ int82gt - _null_ ));
DESCR("greater-than");
DATA(insert OID = 1860 (  int82le		   PGNSP PGUID 12 1 0 f f t f i 2 16 "20 21" _null_ _null_ _null_ int82le - _null_ ));
DESCR("less-than-or-equal");
DATA(insert OID = 1861 (  int82ge		   PGNSP PGUID 12 1 0 f f t f i 2 16 "20 21" _null_ _null_ _null_ int82ge - _null_ ));
DESCR("greater-than-or-equal");

DATA(insert OID = 1892 (  int2and		   PGNSP PGUID 12 1 0 f f t f i 2 21 "21 21" _null_ _null_ _null_ int2and - _null_ ));
DESCR("bitwise and");
DATA(insert OID = 1893 (  int2or		   PGNSP PGUID 12 1 0 f f t f i 2 21 "21 21" _null_ _null_ _null_ int2or - _null_ ));
DESCR("bitwise or");
DATA(insert OID = 1894 (  int2xor		   PGNSP PGUID 12 1 0 f f t f i 2 21 "21 21" _null_ _null_ _null_ int2xor - _null_ ));
DESCR("bitwise xor");
DATA(insert OID = 1895 (  int2not		   PGNSP PGUID 12 1 0 f f t f i 1 21 "21" _null_ _null_ _null_	int2not - _null_ ));
DESCR("bitwise not");
DATA(insert OID = 1896 (  int2shl		   PGNSP PGUID 12 1 0 f f t f i 2 21 "21 23" _null_ _null_ _null_ int2shl - _null_ ));
DESCR("bitwise shift left");
DATA(insert OID = 1897 (  int2shr		   PGNSP PGUID 12 1 0 f f t f i 2 21 "21 23" _null_ _null_ _null_ int2shr - _null_ ));
DESCR("bitwise shift right");

DATA(insert OID = 1898 (  int4and		   PGNSP PGUID 12 1 0 f f t f i 2 23 "23 23" _null_ _null_ _null_ int4and - _null_ ));
DESCR("bitwise and");
DATA(insert OID = 1899 (  int4or		   PGNSP PGUID 12 1 0 f f t f i 2 23 "23 23" _null_ _null_ _null_ int4or - _null_ ));
DESCR("bitwise or");
DATA(insert OID = 1900 (  int4xor		   PGNSP PGUID 12 1 0 f f t f i 2 23 "23 23" _null_ _null_ _null_ int4xor - _null_ ));
DESCR("bitwise xor");
DATA(insert OID = 1901 (  int4not		   PGNSP PGUID 12 1 0 f f t f i 1 23 "23" _null_ _null_ _null_	int4not - _null_ ));
DESCR("bitwise not");
DATA(insert OID = 1902 (  int4shl		   PGNSP PGUID 12 1 0 f f t f i 2 23 "23 23" _null_ _null_ _null_ int4shl - _null_ ));
DESCR("bitwise shift left");
DATA(insert OID = 1903 (  int4shr		   PGNSP PGUID 12 1 0 f f t f i 2 23 "23 23" _null_ _null_ _null_ int4shr - _null_ ));
DESCR("bitwise shift right");

DATA(insert OID = 1904 (  int8and		   PGNSP PGUID 12 1 0 f f t f i 2 20 "20 20" _null_ _null_ _null_ int8and - _null_ ));
DESCR("bitwise and");
DATA(insert OID = 1905 (  int8or		   PGNSP PGUID 12 1 0 f f t f i 2 20 "20 20" _null_ _null_ _null_ int8or - _null_ ));
DESCR("bitwise or");
DATA(insert OID = 1906 (  int8xor		   PGNSP PGUID 12 1 0 f f t f i 2 20 "20 20" _null_ _null_ _null_ int8xor - _null_ ));
DESCR("bitwise xor");
DATA(insert OID = 1907 (  int8not		   PGNSP PGUID 12 1 0 f f t f i 1 20 "20" _null_ _null_ _null_	int8not - _null_ ));
DESCR("bitwise not");
DATA(insert OID = 1908 (  int8shl		   PGNSP PGUID 12 1 0 f f t f i 2 20 "20 23" _null_ _null_ _null_ int8shl - _null_ ));
DESCR("bitwise shift left");
DATA(insert OID = 1909 (  int8shr		   PGNSP PGUID 12 1 0 f f t f i 2 20 "20 23" _null_ _null_ _null_ int8shr - _null_ ));
DESCR("bitwise shift right");

DATA(insert OID = 1910 (  int8up		   PGNSP PGUID 12 1 0 f f t f i 1 20	"20" _null_ _null_ _null_ int8up - _null_ ));
DESCR("unary plus");
DATA(insert OID = 1911 (  int2up		   PGNSP PGUID 12 1 0 f f t f i 1 21	"21" _null_ _null_ _null_ int2up - _null_ ));
DESCR("unary plus");
DATA(insert OID = 1912 (  int4up		   PGNSP PGUID 12 1 0 f f t f i 1 23	"23" _null_ _null_ _null_ int4up - _null_ ));
DESCR("unary plus");
DATA(insert OID = 1913 (  float4up		   PGNSP PGUID 12 1 0 f f t f i 1 700 "700" _null_ _null_ _null_		float4up - _null_ ));
DESCR("unary plus");
DATA(insert OID = 1914 (  float8up		   PGNSP PGUID 12 1 0 f f t f i 1 701 "701" _null_ _null_ _null_		float8up - _null_ ));
DESCR("unary plus");
DATA(insert OID = 1915 (  numeric_uplus    PGNSP PGUID 12 1 0 f f t f i 1 1700 "1700" _null_ _null_ _null_	numeric_uplus - _null_ ));
DESCR("unary plus");

DATA(insert OID = 1922 (  has_table_privilege		   PGNSP PGUID 12 1 0 f f t f s 3 16 "19 25 25" _null_ _null_ _null_	has_table_privilege_name_name - _null_ ));
DESCR("user privilege on relation by username, rel name");
DATA(insert OID = 1923 (  has_table_privilege		   PGNSP PGUID 12 1 0 f f t f s 3 16 "19 26 25" _null_ _null_ _null_	has_table_privilege_name_id - _null_ ));
DESCR("user privilege on relation by username, rel oid");
DATA(insert OID = 1924 (  has_table_privilege		   PGNSP PGUID 12 1 0 f f t f s 3 16 "26 25 25" _null_ _null_ _null_	has_table_privilege_id_name - _null_ ));
DESCR("user privilege on relation by user oid, rel name");
DATA(insert OID = 1925 (  has_table_privilege		   PGNSP PGUID 12 1 0 f f t f s 3 16 "26 26 25" _null_ _null_ _null_	has_table_privilege_id_id - _null_ ));
DESCR("user privilege on relation by user oid, rel oid");
DATA(insert OID = 1926 (  has_table_privilege		   PGNSP PGUID 12 1 0 f f t f s 2 16 "25 25" _null_ _null_ _null_ has_table_privilege_name - _null_ ));
DESCR("current user privilege on relation by rel name");
DATA(insert OID = 1927 (  has_table_privilege		   PGNSP PGUID 12 1 0 f f t f s 2 16 "26 25" _null_ _null_ _null_ has_table_privilege_id - _null_ ));
DESCR("current user privilege on relation by rel oid");


DATA(insert OID = 1928 (  pg_stat_get_numscans			PGNSP PGUID 12 1 0 f f t f s 1 20 "26" _null_ _null_ _null_ pg_stat_get_numscans - _null_ ));
DESCR("Statistics: Number of scans done for table/index");
DATA(insert OID = 1929 (  pg_stat_get_tuples_returned	PGNSP PGUID 12 1 0 f f t f s 1 20 "26" _null_ _null_ _null_ pg_stat_get_tuples_returned - _null_ ));
DESCR("Statistics: Number of tuples read by seqscan");
DATA(insert OID = 1930 (  pg_stat_get_tuples_fetched	PGNSP PGUID 12 1 0 f f t f s 1 20 "26" _null_ _null_ _null_ pg_stat_get_tuples_fetched - _null_ ));
DESCR("Statistics: Number of tuples fetched by idxscan");
DATA(insert OID = 1931 (  pg_stat_get_tuples_inserted	PGNSP PGUID 12 1 0 f f t f s 1 20 "26" _null_ _null_ _null_ pg_stat_get_tuples_inserted - _null_ ));
DESCR("Statistics: Number of tuples inserted");
DATA(insert OID = 1932 (  pg_stat_get_tuples_updated	PGNSP PGUID 12 1 0 f f t f s 1 20 "26" _null_ _null_ _null_ pg_stat_get_tuples_updated - _null_ ));
DESCR("Statistics: Number of tuples updated");
DATA(insert OID = 1933 (  pg_stat_get_tuples_deleted	PGNSP PGUID 12 1 0 f f t f s 1 20 "26" _null_ _null_ _null_ pg_stat_get_tuples_deleted - _null_ ));
DESCR("Statistics: Number of tuples deleted");
DATA(insert OID = 2878 (  pg_stat_get_live_tuples	PGNSP PGUID 12 1 0 f f t f s 1 20 "26" _null_ _null_ _null_ pg_stat_get_live_tuples - _null_ ));
DESCR("Statistics: Number of live tuples");
DATA(insert OID = 2879 (  pg_stat_get_dead_tuples	PGNSP PGUID 12 1 0 f f t f s 1 20 "26" _null_ _null_ _null_ pg_stat_get_dead_tuples - _null_ ));
DESCR("Statistics: Number of dead tuples");
DATA(insert OID = 1934 (  pg_stat_get_blocks_fetched	PGNSP PGUID 12 1 0 f f t f s 1 20 "26" _null_ _null_ _null_ pg_stat_get_blocks_fetched - _null_ ));
DESCR("Statistics: Number of blocks fetched");
DATA(insert OID = 1935 (  pg_stat_get_blocks_hit		PGNSP PGUID 12 1 0 f f t f s 1 20 "26" _null_ _null_ _null_ pg_stat_get_blocks_hit - _null_ ));
DESCR("Statistics: Number of blocks found in cache");
DATA(insert OID = 2781 (  pg_stat_get_last_vacuum_time PGNSP PGUID 12 1 0 f f t f s 1 1184 "26" _null_ _null_ _null_	pg_stat_get_last_vacuum_time - _null_));
DESCR("Statistics: Last manual vacuum time for a table");
DATA(insert OID = 2782 (  pg_stat_get_last_autovacuum_time PGNSP PGUID 12 1 0 f f t f s 1 1184 "26" _null_ _null_ _null_	pg_stat_get_last_autovacuum_time - _null_));
DESCR("Statistics: Last auto vacuum time for a table");
DATA(insert OID = 2783 (  pg_stat_get_last_analyze_time PGNSP PGUID 12 1 0 f f t f s 1 1184 "26" _null_ _null_ _null_	pg_stat_get_last_analyze_time - _null_));
DESCR("Statistics: Last manual analyze time for a table");
DATA(insert OID = 2784 (  pg_stat_get_last_autoanalyze_time PGNSP PGUID 12 1 0 f f t f s 1 1184 "26" _null_ _null_ _null_	pg_stat_get_last_autoanalyze_time - _null_));
DESCR("Statistics: Last auto analyze time for a table");
DATA(insert OID = 1936 (  pg_stat_get_backend_idset		PGNSP PGUID 12 1 100 f f t t s 0 23 "" _null_ _null_ _null_ pg_stat_get_backend_idset - _null_ ));
DESCR("Statistics: Currently active backend IDs");
DATA(insert OID = 2026 (  pg_backend_pid				PGNSP PGUID 12 1 0 f f t f s 0 23 "" _null_ _null_ _null_ pg_backend_pid - _null_ ));
DESCR("Statistics: Current backend PID");
DATA(insert OID = 1937 (  pg_stat_get_backend_pid		PGNSP PGUID 12 1 0 f f t f s 1 23 "23" _null_ _null_ _null_ pg_stat_get_backend_pid - _null_ ));
DESCR("Statistics: PID of backend");
DATA(insert OID = 1938 (  pg_stat_get_backend_dbid		PGNSP PGUID 12 1 0 f f t f s 1 26 "23" _null_ _null_ _null_ pg_stat_get_backend_dbid - _null_ ));
DESCR("Statistics: Database ID of backend");
DATA(insert OID = 1939 (  pg_stat_get_backend_userid	PGNSP PGUID 12 1 0 f f t f s 1 26 "23" _null_ _null_ _null_ pg_stat_get_backend_userid - _null_ ));
DESCR("Statistics: User ID of backend");
DATA(insert OID = 1940 (  pg_stat_get_backend_activity	PGNSP PGUID 12 1 0 f f t f s 1 25 "23" _null_ _null_ _null_ pg_stat_get_backend_activity - _null_ ));
DESCR("Statistics: Current query of backend");
DATA(insert OID = 2853 (  pg_stat_get_backend_waiting	PGNSP PGUID 12 1 0 f f t f s 1 16 "23" _null_ _null_ _null_ pg_stat_get_backend_waiting - _null_ ));
DESCR("Statistics: Is backend currently waiting for a lock");
DATA(insert OID = 2094 (  pg_stat_get_backend_activity_start PGNSP PGUID 12 1 0 f f t f s 1 1184 "23" _null_ _null_ _null_	pg_stat_get_backend_activity_start - _null_));
DESCR("Statistics: Start time for current query of backend");
DATA(insert OID = 2857 (  pg_stat_get_backend_txn_start PGNSP PGUID 12 1 0 f f t f s 1 1184 "23" _null_ _null_ _null_	pg_stat_get_backend_txn_start - _null_));
DESCR("Statistics: Start time for backend's current transaction");
DATA(insert OID = 1391 ( pg_stat_get_backend_start PGNSP PGUID 12 1 0 f f t f s 1 1184 "23" _null_ _null_ _null_ pg_stat_get_backend_start - _null_));
DESCR("Statistics: Start time for current backend session");
DATA(insert OID = 1392 ( pg_stat_get_backend_client_addr PGNSP PGUID 12 1 0 f f t f s 1 869 "23" _null_ _null_ _null_ pg_stat_get_backend_client_addr - _null_));
DESCR("Statistics: Address of client connected to backend");
DATA(insert OID = 1393 ( pg_stat_get_backend_client_port PGNSP PGUID 12 1 0 f f t f s 1 23 "23" _null_ _null_ _null_ pg_stat_get_backend_client_port - _null_));
DESCR("Statistics: Port number of client connected to backend");
DATA(insert OID = 1941 (  pg_stat_get_db_numbackends	PGNSP PGUID 12 1 0 f f t f s 1 23 "26" _null_ _null_ _null_ pg_stat_get_db_numbackends - _null_ ));
DESCR("Statistics: Number of backends in database");
DATA(insert OID = 1942 (  pg_stat_get_db_xact_commit	PGNSP PGUID 12 1 0 f f t f s 1 20 "26" _null_ _null_ _null_ pg_stat_get_db_xact_commit - _null_ ));
DESCR("Statistics: Transactions committed");
DATA(insert OID = 1943 (  pg_stat_get_db_xact_rollback	PGNSP PGUID 12 1 0 f f t f s 1 20 "26" _null_ _null_ _null_ pg_stat_get_db_xact_rollback - _null_ ));
DESCR("Statistics: Transactions rolled back");
DATA(insert OID = 1944 (  pg_stat_get_db_blocks_fetched PGNSP PGUID 12 1 0 f f t f s 1 20 "26" _null_ _null_ _null_ pg_stat_get_db_blocks_fetched - _null_ ));
DESCR("Statistics: Blocks fetched for database");
DATA(insert OID = 1945 (  pg_stat_get_db_blocks_hit		PGNSP PGUID 12 1 0 f f t f s 1 20 "26" _null_ _null_ _null_ pg_stat_get_db_blocks_hit - _null_ ));
DESCR("Statistics: Blocks found in cache for database");
DATA(insert OID = 2758 (  pg_stat_get_db_tuples_returned PGNSP PGUID 12 1 0 f f t f s 1 20 "26" _null_ _null_ _null_ pg_stat_get_db_tuples_returned - _null_ ));
DESCR("Statistics: Tuples returned for database");
DATA(insert OID = 2759 (  pg_stat_get_db_tuples_fetched PGNSP PGUID 12 1 0 f f t f s 1 20 "26" _null_ _null_ _null_ pg_stat_get_db_tuples_fetched - _null_ ));
DESCR("Statistics: Tuples fetched for database");
DATA(insert OID = 2760 (  pg_stat_get_db_tuples_inserted PGNSP PGUID 12 1 0 f f t f s 1 20 "26" _null_ _null_ _null_ pg_stat_get_db_tuples_inserted - _null_ ));
DESCR("Statistics: Tuples inserted in database");
DATA(insert OID = 2761 (  pg_stat_get_db_tuples_updated PGNSP PGUID 12 1 0 f f t f s 1 20 "26" _null_ _null_ _null_ pg_stat_get_db_tuples_updated - _null_ ));
DESCR("Statistics: Tuples updated in database");
DATA(insert OID = 2762 (  pg_stat_get_db_tuples_deleted PGNSP PGUID 12 1 0 f f t f s 1 20 "26" _null_ _null_ _null_ pg_stat_get_db_tuples_deleted - _null_ ));
DESCR("Statistics: Tuples deleted in database");
DATA(insert OID = 2769 ( pg_stat_get_bgwriter_timed_checkpoints PGNSP PGUID 12 1 0 f f t f s 0 20 "" _null_ _null_ _null_ pg_stat_get_bgwriter_timed_checkpoints - _null_ ));
DESCR("Statistics: Number of timed checkpoints started by the bgwriter");
DATA(insert OID = 2770 ( pg_stat_get_bgwriter_requested_checkpoints PGNSP PGUID 12 1 0 f f t f s 0 20 "" _null_ _null_ _null_ pg_stat_get_bgwriter_requested_checkpoints - _null_ ));
DESCR("Statistics: Number of backend requested checkpoints started by the bgwriter");
DATA(insert OID = 2771 ( pg_stat_get_bgwriter_buf_written_checkpoints PGNSP PGUID 12 1 0 f f t f s 0 20 "" _null_ _null_ _null_ pg_stat_get_bgwriter_buf_written_checkpoints - _null_ ));
DESCR("Statistics: Number of buffers written by the bgwriter during checkpoints");
DATA(insert OID = 2772 ( pg_stat_get_bgwriter_buf_written_lru PGNSP PGUID 12 1 0 f f t f s 0 20 "" _null_ _null_ _null_ pg_stat_get_bgwriter_buf_written_lru - _null_ ));
DESCR("Statistics: Number of buffers written by the bgwriter during LRU scans");
DATA(insert OID = 2773 ( pg_stat_get_bgwriter_buf_written_all PGNSP PGUID 12 1 0 f f t f s 0 20 "" _null_ _null_ _null_ pg_stat_get_bgwriter_buf_written_all - _null_ ));
DESCR("Statistics: Number of buffers written by the bgwriter during all-buffer scans");
DATA(insert OID = 2774 ( pg_stat_get_bgwriter_maxwritten_lru PGNSP PGUID 12 1 0 f f t f s 0 20 "" _null_ _null_ _null_ pg_stat_get_bgwriter_maxwritten_lru - _null_ ));
DESCR("Statistics: Number of times the bgwriter stopped processing when it had written too many buffers during LRU scans");
DATA(insert OID = 2775 ( pg_stat_get_bgwriter_maxwritten_all PGNSP PGUID 12 1 0 f f t f s 0 20 "" _null_ _null_ _null_ pg_stat_get_bgwriter_maxwritten_all - _null_ ));
DESCR("Statistics: Number of times the bgwriter stopped processing when it had written too many buffers during all-buffer scans");
DATA(insert OID = 2230 (  pg_stat_clear_snapshot		PGNSP PGUID 12 1 0 f f f f v 0 2278  "" _null_ _null_ _null_	pg_stat_clear_snapshot - _null_ ));
DESCR("Statistics: Discard current transaction's statistics snapshot");
DATA(insert OID = 2274 (  pg_stat_reset					PGNSP PGUID 12 1 0 f f f f v 0 2278  "" _null_ _null_ _null_	pg_stat_reset - _null_ ));
DESCR("Statistics: Reset collected statistics for current database");

DATA(insert OID = 1946 (  encode						PGNSP PGUID 12 1 0 f f t f i 2 25 "17 25" _null_ _null_ _null_	binary_encode - _null_ ));
DESCR("convert bytea value into some ascii-only text string");
DATA(insert OID = 1947 (  decode						PGNSP PGUID 12 1 0 f f t f i 2 17 "25 25" _null_ _null_ _null_	binary_decode - _null_ ));
DESCR("convert ascii-encoded text string into bytea value");

DATA(insert OID = 1948 (  byteaeq		   PGNSP PGUID 12 1 0 f f t f i 2 16 "17 17" _null_ _null_ _null_ byteaeq - _null_ ));
DESCR("equal");
DATA(insert OID = 1949 (  bytealt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "17 17" _null_ _null_ _null_ bytealt - _null_ ));
DESCR("less-than");
DATA(insert OID = 1950 (  byteale		   PGNSP PGUID 12 1 0 f f t f i 2 16 "17 17" _null_ _null_ _null_ byteale - _null_ ));
DESCR("less-than-or-equal");
DATA(insert OID = 1951 (  byteagt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "17 17" _null_ _null_ _null_ byteagt - _null_ ));
DESCR("greater-than");
DATA(insert OID = 1952 (  byteage		   PGNSP PGUID 12 1 0 f f t f i 2 16 "17 17" _null_ _null_ _null_ byteage - _null_ ));
DESCR("greater-than-or-equal");
DATA(insert OID = 1953 (  byteane		   PGNSP PGUID 12 1 0 f f t f i 2 16 "17 17" _null_ _null_ _null_ byteane - _null_ ));
DESCR("not equal");
DATA(insert OID = 1954 (  byteacmp		   PGNSP PGUID 12 1 0 f f t f i 2 23 "17 17" _null_ _null_ _null_ byteacmp - _null_ ));
DESCR("less-equal-greater");

DATA(insert OID = 1961 (  timestamp		   PGNSP PGUID 12 1 0 f f t f i 2 1114 "1114 23" _null_ _null_ _null_ timestamp_scale - _null_ ));
DESCR("adjust timestamp precision");

DATA(insert OID = 1965 (  oidlarger		   PGNSP PGUID 12 1 0 f f t f i 2 26 "26 26" _null_ _null_ _null_ oidlarger - _null_ ));
DESCR("larger of two");
DATA(insert OID = 1966 (  oidsmaller	   PGNSP PGUID 12 1 0 f f t f i 2 26 "26 26" _null_ _null_ _null_ oidsmaller - _null_ ));
DESCR("smaller of two");

DATA(insert OID = 1967 (  timestamptz	   PGNSP PGUID 12 1 0 f f t f i 2 1184 "1184 23" _null_ _null_ _null_ timestamptz_scale - _null_ ));
DESCR("adjust timestamptz precision");
DATA(insert OID = 1968 (  time			   PGNSP PGUID 12 1 0 f f t f i 2 1083 "1083 23" _null_ _null_ _null_ time_scale - _null_ ));
DESCR("adjust time precision");
DATA(insert OID = 1969 (  timetz		   PGNSP PGUID 12 1 0 f f t f i 2 1266 "1266 23" _null_ _null_ _null_ timetz_scale - _null_ ));
DESCR("adjust time with time zone precision");

DATA(insert OID = 2003 (  textanycat	   PGNSP PGUID 14 1 0 f f t f i 2 25 "25 2776" _null_ _null_ _null_ "select $1 || $2::pg_catalog.text" - _null_ ));
DESCR("concatenate");
DATA(insert OID = 2004 (  anytextcat	   PGNSP PGUID 14 1 0 f f t f i 2 25 "2776 25" _null_ _null_ _null_ "select $1::pg_catalog.text || $2" - _null_ ));
DESCR("concatenate");

DATA(insert OID = 2005 (  bytealike		   PGNSP PGUID 12 1 0 f f t f i 2 16 "17 17" _null_ _null_ _null_ bytealike - _null_ ));
DESCR("matches LIKE expression");
DATA(insert OID = 2006 (  byteanlike	   PGNSP PGUID 12 1 0 f f t f i 2 16 "17 17" _null_ _null_ _null_ byteanlike - _null_ ));
DESCR("does not match LIKE expression");
DATA(insert OID = 2007 (  like			   PGNSP PGUID 12 1 0 f f t f i 2 16 "17 17" _null_ _null_ _null_ bytealike - _null_ ));
DESCR("matches LIKE expression");
DATA(insert OID = 2008 (  notlike		   PGNSP PGUID 12 1 0 f f t f i 2 16 "17 17" _null_ _null_ _null_ byteanlike - _null_ ));
DESCR("does not match LIKE expression");
DATA(insert OID = 2009 (  like_escape	   PGNSP PGUID 12 1 0 f f t f i 2 17 "17 17" _null_ _null_ _null_ like_escape_bytea - _null_ ));
DESCR("convert LIKE pattern to use backslash escapes");
DATA(insert OID = 2010 (  length		   PGNSP PGUID 12 1 0 f f t f i 1 23 "17" _null_ _null_ _null_	byteaoctetlen - _null_ ));
DESCR("octet length");
DATA(insert OID = 2011 (  byteacat		   PGNSP PGUID 12 1 0 f f t f i 2 17 "17 17" _null_ _null_ _null_ byteacat - _null_ ));
DESCR("concatenate");
DATA(insert OID = 2012 (  substring		   PGNSP PGUID 12 1 0 f f t f i 3 17 "17 23 23" _null_ _null_ _null_	bytea_substr - _null_ ));
DESCR("return portion of string");
DATA(insert OID = 2013 (  substring		   PGNSP PGUID 12 1 0 f f t f i 2 17 "17 23" _null_ _null_ _null_ bytea_substr_no_len - _null_ ));
DESCR("return portion of string");
DATA(insert OID = 2085 (  substr		   PGNSP PGUID 12 1 0 f f t f i 3 17 "17 23 23" _null_ _null_ _null_	bytea_substr - _null_ ));
DESCR("return portion of string");
DATA(insert OID = 2086 (  substr		   PGNSP PGUID 12 1 0 f f t f i 2 17 "17 23" _null_ _null_ _null_ bytea_substr_no_len - _null_ ));
DESCR("return portion of string");
DATA(insert OID = 2014 (  position		   PGNSP PGUID 12 1 0 f f t f i 2 23 "17 17" _null_ _null_ _null_ byteapos - _null_ ));
DESCR("return position of substring");
DATA(insert OID = 2015 (  btrim			   PGNSP PGUID 12 1 0 f f t f i 2 17 "17 17" _null_ _null_ _null_ byteatrim - _null_ ));
DESCR("trim both ends of string");

DATA(insert OID = 2019 (  time				PGNSP PGUID 12 1 0 f f t f s 1 1083 "1184" _null_ _null_ _null_ timestamptz_time - _null_ ));
DESCR("convert timestamptz to time");
DATA(insert OID = 2020 (  date_trunc		PGNSP PGUID 12 1 0 f f t f i 2 1114 "25 1114" _null_ _null_ _null_	timestamp_trunc - _null_ ));
DESCR("truncate timestamp to specified units");
DATA(insert OID = 2021 (  date_part			PGNSP PGUID 12 1 0 f f t f i 2	701 "25 1114" _null_ _null_ _null_	timestamp_part - _null_ ));
DESCR("extract field from timestamp");
DATA(insert OID = 2023 (  timestamp			PGNSP PGUID 12 1 0 f f t f s 1 1114 "702" _null_ _null_ _null_	abstime_timestamp - _null_ ));
DESCR("convert abstime to timestamp");
DATA(insert OID = 2024 (  timestamp			PGNSP PGUID 12 1 0 f f t f i 1 1114 "1082" _null_ _null_ _null_ date_timestamp - _null_ ));
DESCR("convert date to timestamp");
DATA(insert OID = 2025 (  timestamp			PGNSP PGUID 12 1 0 f f t f i 2 1114 "1082 1083" _null_ _null_ _null_	datetime_timestamp - _null_ ));
DESCR("convert date and time to timestamp");
DATA(insert OID = 2027 (  timestamp			PGNSP PGUID 12 1 0 f f t f s 1 1114 "1184" _null_ _null_ _null_ timestamptz_timestamp - _null_ ));
DESCR("convert timestamp with time zone to timestamp");
DATA(insert OID = 2028 (  timestamptz		PGNSP PGUID 12 1 0 f f t f s 1 1184 "1114" _null_ _null_ _null_ timestamp_timestamptz - _null_ ));
DESCR("convert timestamp to timestamp with time zone");
DATA(insert OID = 2029 (  date				PGNSP PGUID 12 1 0 f f t f i 1 1082 "1114" _null_ _null_ _null_ timestamp_date - _null_ ));
DESCR("convert timestamp to date");
DATA(insert OID = 2030 (  abstime			PGNSP PGUID 12 1 0 f f t f s 1	702 "1114" _null_ _null_ _null_ timestamp_abstime - _null_ ));
DESCR("convert timestamp to abstime");
DATA(insert OID = 2031 (  timestamp_mi		PGNSP PGUID 12 1 0 f f t f i 2 1186 "1114 1114" _null_ _null_ _null_	timestamp_mi - _null_ ));
DESCR("subtract");
DATA(insert OID = 2032 (  timestamp_pl_interval PGNSP PGUID 12 1 0 f f t f i 2 1114 "1114 1186" _null_ _null_ _null_	timestamp_pl_interval - _null_ ));
DESCR("plus");
DATA(insert OID = 2033 (  timestamp_mi_interval PGNSP PGUID 12 1 0 f f t f i 2 1114 "1114 1186" _null_ _null_ _null_	timestamp_mi_interval - _null_ ));
DESCR("minus");
DATA(insert OID = 2035 (  timestamp_smaller PGNSP PGUID 12 1 0 f f t f i 2 1114 "1114 1114" _null_ _null_ _null_	timestamp_smaller - _null_ ));
DESCR("smaller of two");
DATA(insert OID = 2036 (  timestamp_larger	PGNSP PGUID 12 1 0 f f t f i 2 1114 "1114 1114" _null_ _null_ _null_	timestamp_larger - _null_ ));
DESCR("larger of two");
DATA(insert OID = 2037 (  timezone			PGNSP PGUID 12 1 0 f f t f v 2 1266 "25 1266" _null_ _null_ _null_	timetz_zone - _null_ ));
DESCR("adjust time with time zone to new zone");
DATA(insert OID = 2038 (  timezone			PGNSP PGUID 12 1 0 f f t f i 2 1266 "1186 1266" _null_ _null_ _null_	timetz_izone - _null_ ));
DESCR("adjust time with time zone to new zone");
DATA(insert OID = 2041 ( overlaps			PGNSP PGUID 12 1 0 f f f f i 4 16 "1114 1114 1114 1114" _null_ _null_ _null_	overlaps_timestamp - _null_ ));
DESCR("SQL92 interval comparison");
DATA(insert OID = 2042 ( overlaps			PGNSP PGUID 14 1 0 f f f f i 4 16 "1114 1186 1114 1186" _null_ _null_ _null_	"select ($1, ($1 + $2)) overlaps ($3, ($3 + $4))" - _null_ ));
DESCR("SQL92 interval comparison");
DATA(insert OID = 2043 ( overlaps			PGNSP PGUID 14 1 0 f f f f i 4 16 "1114 1114 1114 1186" _null_ _null_ _null_	"select ($1, $2) overlaps ($3, ($3 + $4))" - _null_ ));
DESCR("SQL92 interval comparison");
DATA(insert OID = 2044 ( overlaps			PGNSP PGUID 14 1 0 f f f f i 4 16 "1114 1186 1114 1114" _null_ _null_ _null_	"select ($1, ($1 + $2)) overlaps ($3, $4)" - _null_ ));
DESCR("SQL92 interval comparison");
DATA(insert OID = 2045 (  timestamp_cmp		PGNSP PGUID 12 1 0 f f t f i 2	23 "1114 1114" _null_ _null_ _null_ timestamp_cmp - _null_ ));
DESCR("less-equal-greater");
DATA(insert OID = 2046 (  time				PGNSP PGUID 12 1 0 f f t f i 1 1083 "1266" _null_ _null_ _null_ timetz_time - _null_ ));
DESCR("convert time with time zone to time");
DATA(insert OID = 2047 (  timetz			PGNSP PGUID 12 1 0 f f t f s 1 1266 "1083" _null_ _null_ _null_ time_timetz - _null_ ));
DESCR("convert time to timetz");
DATA(insert OID = 2048 (  isfinite			PGNSP PGUID 12 1 0 f f t f i 1	 16 "1114" _null_ _null_ _null_ timestamp_finite - _null_ ));
DESCR("finite timestamp?");
DATA(insert OID = 2049 ( to_char			PGNSP PGUID 12 1 0 f f t f s 2	25 "1114 25" _null_ _null_ _null_  timestamp_to_char - _null_ ));
DESCR("format timestamp to text");
DATA(insert OID = 2052 (  timestamp_eq		PGNSP PGUID 12 1 0 f f t f i 2 16 "1114 1114" _null_ _null_ _null_	timestamp_eq - _null_ ));
DESCR("equal");
DATA(insert OID = 2053 (  timestamp_ne		PGNSP PGUID 12 1 0 f f t f i 2 16 "1114 1114" _null_ _null_ _null_	timestamp_ne - _null_ ));
DESCR("not equal");
DATA(insert OID = 2054 (  timestamp_lt		PGNSP PGUID 12 1 0 f f t f i 2 16 "1114 1114" _null_ _null_ _null_	timestamp_lt - _null_ ));
DESCR("less-than");
DATA(insert OID = 2055 (  timestamp_le		PGNSP PGUID 12 1 0 f f t f i 2 16 "1114 1114" _null_ _null_ _null_	timestamp_le - _null_ ));
DESCR("less-than-or-equal");
DATA(insert OID = 2056 (  timestamp_ge		PGNSP PGUID 12 1 0 f f t f i 2 16 "1114 1114" _null_ _null_ _null_	timestamp_ge - _null_ ));
DESCR("greater-than-or-equal");
DATA(insert OID = 2057 (  timestamp_gt		PGNSP PGUID 12 1 0 f f t f i 2 16 "1114 1114" _null_ _null_ _null_	timestamp_gt - _null_ ));
DESCR("greater-than");
DATA(insert OID = 2058 (  age				PGNSP PGUID 12 1 0 f f t f i 2 1186 "1114 1114" _null_ _null_ _null_	timestamp_age - _null_ ));
DESCR("date difference preserving months and years");
DATA(insert OID = 2059 (  age				PGNSP PGUID 14 1 0 f f t f s 1 1186 "1114" _null_ _null_ _null_ "select pg_catalog.age(cast(current_date as timestamp without time zone), $1)" - _null_ ));
DESCR("date difference from today preserving months and years");

DATA(insert OID = 2069 (  timezone			PGNSP PGUID 12 1 0 f f t f i 2 1184 "25 1114" _null_ _null_ _null_	timestamp_zone - _null_ ));
DESCR("adjust timestamp to new time zone");
DATA(insert OID = 2070 (  timezone			PGNSP PGUID 12 1 0 f f t f i 2 1184 "1186 1114" _null_ _null_ _null_	timestamp_izone - _null_ ));
DESCR("adjust timestamp to new time zone");
DATA(insert OID = 2071 (  date_pl_interval	PGNSP PGUID 12 1 0 f f t f i 2 1114 "1082 1186" _null_ _null_ _null_	date_pl_interval - _null_ ));
DESCR("add");
DATA(insert OID = 2072 (  date_mi_interval	PGNSP PGUID 12 1 0 f f t f i 2 1114 "1082 1186" _null_ _null_ _null_	date_mi_interval - _null_ ));
DESCR("subtract");

DATA(insert OID = 2073 (  substring			PGNSP PGUID 12 1 0 f f t f i 2 25 "25 25" _null_ _null_ _null_	textregexsubstr - _null_ ));
DESCR("extracts text matching regular expression");
DATA(insert OID = 2074 (  substring			PGNSP PGUID 14 1 0 f f t f i 3 25 "25 25 25" _null_ _null_ _null_ "select pg_catalog.substring($1, pg_catalog.similar_escape($2, $3))" - _null_ ));
DESCR("extracts text matching SQL99 regular expression");

DATA(insert OID = 2075 (  bit				PGNSP PGUID 12 1 0 f f t f i 2 1560 "20 23" _null_ _null_ _null_	bitfromint8 - _null_ ));
DESCR("int8 to bitstring");
DATA(insert OID = 2076 (  int8				PGNSP PGUID 12 1 0 f f t f i 1 20 "1560" _null_ _null_ _null_ bittoint8 - _null_ ));
DESCR("bitstring to int8");

DATA(insert OID = 2077 (  current_setting	PGNSP PGUID 12 1 0 f f t f s 1 25 "25" _null_ _null_ _null_ show_config_by_name - _null_ ));
DESCR("SHOW X as a function");
DATA(insert OID = 2078 (  set_config		PGNSP PGUID 12 1 0 f f f f v 3 25 "25 25 16" _null_ _null_ _null_ set_config_by_name - _null_ ));
DESCR("SET X as a function");
DATA(insert OID = 2084 (  pg_show_all_settings	PGNSP PGUID 12 1 1000 f f t t s 0 2249 "" _null_ _null_ _null_ show_all_settings - _null_ ));
DESCR("SHOW ALL as a function");
DATA(insert OID = 1371 (  pg_lock_status   PGNSP PGUID 12 1 1000 f f t t v 0 2249 "" _null_ _null_ _null_ pg_lock_status - _null_ ));
DESCR("view system lock information");
DATA(insert OID = 1065 (  pg_prepared_xact PGNSP PGUID 12 1 1000 f f t t v 0 2249 "" _null_ _null_ _null_ pg_prepared_xact - _null_ ));
DESCR("view two-phase transactions");

DATA(insert OID = 2079 (  pg_table_is_visible		PGNSP PGUID 12 1 0 f f t f s 1 16 "26" _null_ _null_ _null_ pg_table_is_visible - _null_ ));
DESCR("is table visible in search path?");
DATA(insert OID = 2080 (  pg_type_is_visible		PGNSP PGUID 12 1 0 f f t f s 1 16 "26" _null_ _null_ _null_ pg_type_is_visible - _null_ ));
DESCR("is type visible in search path?");
DATA(insert OID = 2081 (  pg_function_is_visible	PGNSP PGUID 12 1 0 f f t f s 1 16 "26" _null_ _null_ _null_ pg_function_is_visible - _null_ ));
DESCR("is function visible in search path?");
DATA(insert OID = 2082 (  pg_operator_is_visible	PGNSP PGUID 12 1 0 f f t f s 1 16 "26" _null_ _null_ _null_ pg_operator_is_visible - _null_ ));
DESCR("is operator visible in search path?");
DATA(insert OID = 2083 (  pg_opclass_is_visible		PGNSP PGUID 12 1 0 f f t f s 1 16 "26" _null_ _null_ _null_ pg_opclass_is_visible - _null_ ));
DESCR("is opclass visible in search path?");
DATA(insert OID = 2093 (  pg_conversion_is_visible	PGNSP PGUID 12 1 0 f f t f s 1 16 "26" _null_ _null_ _null_ pg_conversion_is_visible - _null_ ));
DESCR("is conversion visible in search path?");
DATA(insert OID = 2854 (  pg_my_temp_schema			PGNSP PGUID 12 1 0 f f t f s 0 26 "" _null_ _null_ _null_ pg_my_temp_schema - _null_ ));
DESCR("get OID of current session's temp schema, if any");
DATA(insert OID = 2855 (  pg_is_other_temp_schema	PGNSP PGUID 12 1 0 f f t f s 1 16 "26" _null_ _null_ _null_ pg_is_other_temp_schema - _null_ ));
DESCR("is schema another session's temp schema?");

DATA(insert OID = 2171 ( pg_cancel_backend		PGNSP PGUID 12 1 0 f f t f v 1 16 "23" _null_ _null_ _null_ pg_cancel_backend - _null_ ));
DESCR("cancel a server process' current query");
DATA(insert OID = 2172 ( pg_start_backup		PGNSP PGUID 12 1 0 f f t f v 1 25 "25" _null_ _null_ _null_ pg_start_backup - _null_ ));
DESCR("prepare for taking an online backup");
DATA(insert OID = 2173 ( pg_stop_backup			PGNSP PGUID 12 1 0 f f t f v 0 25 "" _null_ _null_ _null_ pg_stop_backup - _null_ ));
DESCR("finish taking an online backup");
DATA(insert OID = 2848 ( pg_switch_xlog			PGNSP PGUID 12 1 0 f f t f v 0 25 "" _null_ _null_ _null_ pg_switch_xlog - _null_ ));
DESCR("switch to new xlog file");
DATA(insert OID = 2849 ( pg_current_xlog_location	PGNSP PGUID 12 1 0 f f t f v 0 25 "" _null_ _null_ _null_ pg_current_xlog_location - _null_ ));
DESCR("current xlog write location");
DATA(insert OID = 2852 ( pg_current_xlog_insert_location	PGNSP PGUID 12 1 0 f f t f v 0 25 "" _null_ _null_ _null_ pg_current_xlog_insert_location - _null_ ));
DESCR("current xlog insert location");
DATA(insert OID = 2850 ( pg_xlogfile_name_offset	PGNSP PGUID 12 1 0 f f t f i 1 2249 "25" "{25,25,23}" "{i,o,o}" "{wal_location,file_name,file_offset}" pg_xlogfile_name_offset - _null_ ));
DESCR("xlog filename and byte offset, given an xlog location");
DATA(insert OID = 2851 ( pg_xlogfile_name			PGNSP PGUID 12 1 0 f f t f i 1 25 "25" _null_ _null_ _null_ pg_xlogfile_name - _null_ ));
DESCR("xlog filename, given an xlog location");

DATA(insert OID = 2621 ( pg_reload_conf			PGNSP PGUID 12 1 0 f f t f v 0 16 "" _null_ _null_ _null_ pg_reload_conf - _null_ ));
DESCR("reload configuration files");
DATA(insert OID = 2622 ( pg_rotate_logfile		PGNSP PGUID 12 1 0 f f t f v 0 16 "" _null_ _null_ _null_ pg_rotate_logfile - _null_ ));
DESCR("rotate log file");

DATA(insert OID = 2623 ( pg_stat_file		PGNSP PGUID 12 1 0 f f t f v 1 2249 "25" "{25,20,1184,1184,1184,1184,16}" "{i,o,o,o,o,o,o}" "{filename,size,access,modification,change,creation,isdir}" pg_stat_file - _null_ ));
DESCR("return file information");
DATA(insert OID = 2624 ( pg_read_file		PGNSP PGUID 12 1 0 f f t f v 3 25 "25 20 20" _null_ _null_ _null_ pg_read_file - _null_ ));
DESCR("read text from a file");
DATA(insert OID = 2625 ( pg_ls_dir			PGNSP PGUID 12 1 1000 f f t t v 1 25 "25" _null_ _null_ _null_ pg_ls_dir - _null_ ));
DESCR("list all files in a directory");
DATA(insert OID = 2626 ( pg_sleep			PGNSP PGUID 12 1 0 f f t f v 1 2278 "701" _null_ _null_ _null_ pg_sleep - _null_ ));
DESCR("sleep for the specified time in seconds");

DATA(insert OID = 2971 (  text				PGNSP PGUID 12 1 0 f f t f i 1 25 "16" _null_ _null_ _null_	booltext - _null_ ));
DESCR("convert boolean to text");

/* Aggregates (moved here from pg_aggregate for 7.3) */

DATA(insert OID = 2100 (  avg				PGNSP PGUID 12 1 0 t f f f i 1 1700 "20" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2101 (  avg				PGNSP PGUID 12 1 0 t f f f i 1 1700 "23" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2102 (  avg				PGNSP PGUID 12 1 0 t f f f i 1 1700 "21" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2103 (  avg				PGNSP PGUID 12 1 0 t f f f i 1 1700 "1700" _null_ _null_ _null_ aggregate_dummy - _null_ ));
DATA(insert OID = 2104 (  avg				PGNSP PGUID 12 1 0 t f f f i 1 701 "700" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2105 (  avg				PGNSP PGUID 12 1 0 t f f f i 1 701 "701" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2106 (  avg				PGNSP PGUID 12 1 0 t f f f i 1 1186 "1186" _null_ _null_ _null_ aggregate_dummy - _null_ ));

DATA(insert OID = 2107 (  sum				PGNSP PGUID 12 1 0 t f f f i 1 1700 "20" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2108 (  sum				PGNSP PGUID 12 1 0 t f f f i 1 20 "23" _null_ _null_ _null_ aggregate_dummy - _null_ ));
DATA(insert OID = 2109 (  sum				PGNSP PGUID 12 1 0 t f f f i 1 20 "21" _null_ _null_ _null_ aggregate_dummy - _null_ ));
DATA(insert OID = 2110 (  sum				PGNSP PGUID 12 1 0 t f f f i 1 700 "700" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2111 (  sum				PGNSP PGUID 12 1 0 t f f f i 1 701 "701" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2112 (  sum				PGNSP PGUID 12 1 0 t f f f i 1 790 "790" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2113 (  sum				PGNSP PGUID 12 1 0 t f f f i 1 1186 "1186" _null_ _null_ _null_ aggregate_dummy - _null_ ));
DATA(insert OID = 2114 (  sum				PGNSP PGUID 12 1 0 t f f f i 1 1700 "1700" _null_ _null_ _null_ aggregate_dummy - _null_ ));

DATA(insert OID = 2115 (  max				PGNSP PGUID 12 1 0 t f f f i 1 20 "20" _null_ _null_ _null_ aggregate_dummy - _null_ ));
DATA(insert OID = 2116 (  max				PGNSP PGUID 12 1 0 t f f f i 1 23 "23" _null_ _null_ _null_ aggregate_dummy - _null_ ));
DATA(insert OID = 2117 (  max				PGNSP PGUID 12 1 0 t f f f i 1 21 "21" _null_ _null_ _null_ aggregate_dummy - _null_ ));
DATA(insert OID = 2118 (  max				PGNSP PGUID 12 1 0 t f f f i 1 26 "26" _null_ _null_ _null_ aggregate_dummy - _null_ ));
DATA(insert OID = 2119 (  max				PGNSP PGUID 12 1 0 t f f f i 1 700 "700" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2120 (  max				PGNSP PGUID 12 1 0 t f f f i 1 701 "701" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2121 (  max				PGNSP PGUID 12 1 0 t f f f i 1 702 "702" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2122 (  max				PGNSP PGUID 12 1 0 t f f f i 1 1082 "1082" _null_ _null_ _null_ aggregate_dummy - _null_ ));
DATA(insert OID = 2123 (  max				PGNSP PGUID 12 1 0 t f f f i 1 1083 "1083" _null_ _null_ _null_ aggregate_dummy - _null_ ));
DATA(insert OID = 2124 (  max				PGNSP PGUID 12 1 0 t f f f i 1 1266 "1266" _null_ _null_ _null_ aggregate_dummy - _null_ ));
DATA(insert OID = 2125 (  max				PGNSP PGUID 12 1 0 t f f f i 1 790 "790" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2126 (  max				PGNSP PGUID 12 1 0 t f f f i 1 1114 "1114" _null_ _null_ _null_ aggregate_dummy - _null_ ));
DATA(insert OID = 2127 (  max				PGNSP PGUID 12 1 0 t f f f i 1 1184 "1184" _null_ _null_ _null_ aggregate_dummy - _null_ ));
DATA(insert OID = 2128 (  max				PGNSP PGUID 12 1 0 t f f f i 1 1186 "1186" _null_ _null_ _null_ aggregate_dummy - _null_ ));
DATA(insert OID = 2129 (  max				PGNSP PGUID 12 1 0 t f f f i 1 25 "25" _null_ _null_ _null_ aggregate_dummy - _null_ ));
DATA(insert OID = 2130 (  max				PGNSP PGUID 12 1 0 t f f f i 1 1700 "1700" _null_ _null_ _null_ aggregate_dummy - _null_ ));
DATA(insert OID = 2050 (  max				PGNSP PGUID 12 1 0 t f f f i 1 2277 "2277" _null_ _null_ _null_ aggregate_dummy - _null_ ));
DATA(insert OID = 2244 (  max				PGNSP PGUID 12 1 0 t f f f i 1 1042 "1042" _null_ _null_ _null_ aggregate_dummy - _null_ ));
DATA(insert OID = 2797 (  max				PGNSP PGUID 12 1 0 t f f f i 1 27 "27" _null_ _null_ _null_ aggregate_dummy - _null_ ));

DATA(insert OID = 2131 (  min				PGNSP PGUID 12 1 0 t f f f i 1 20 "20" _null_ _null_ _null_ aggregate_dummy - _null_ ));
DATA(insert OID = 2132 (  min				PGNSP PGUID 12 1 0 t f f f i 1 23 "23" _null_ _null_ _null_ aggregate_dummy - _null_ ));
DATA(insert OID = 2133 (  min				PGNSP PGUID 12 1 0 t f f f i 1 21 "21" _null_ _null_ _null_ aggregate_dummy - _null_ ));
DATA(insert OID = 2134 (  min				PGNSP PGUID 12 1 0 t f f f i 1 26 "26" _null_ _null_ _null_ aggregate_dummy - _null_ ));
DATA(insert OID = 2135 (  min				PGNSP PGUID 12 1 0 t f f f i 1 700 "700" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2136 (  min				PGNSP PGUID 12 1 0 t f f f i 1 701 "701" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2137 (  min				PGNSP PGUID 12 1 0 t f f f i 1 702 "702" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2138 (  min				PGNSP PGUID 12 1 0 t f f f i 1 1082 "1082" _null_ _null_ _null_ aggregate_dummy - _null_ ));
DATA(insert OID = 2139 (  min				PGNSP PGUID 12 1 0 t f f f i 1 1083 "1083" _null_ _null_ _null_ aggregate_dummy - _null_ ));
DATA(insert OID = 2140 (  min				PGNSP PGUID 12 1 0 t f f f i 1 1266 "1266" _null_ _null_ _null_ aggregate_dummy - _null_ ));
DATA(insert OID = 2141 (  min				PGNSP PGUID 12 1 0 t f f f i 1 790 "790" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2142 (  min				PGNSP PGUID 12 1 0 t f f f i 1 1114 "1114" _null_ _null_ _null_ aggregate_dummy - _null_ ));
DATA(insert OID = 2143 (  min				PGNSP PGUID 12 1 0 t f f f i 1 1184 "1184" _null_ _null_ _null_ aggregate_dummy - _null_ ));
DATA(insert OID = 2144 (  min				PGNSP PGUID 12 1 0 t f f f i 1 1186 "1186" _null_ _null_ _null_ aggregate_dummy - _null_ ));
DATA(insert OID = 2145 (  min				PGNSP PGUID 12 1 0 t f f f i 1 25 "25" _null_ _null_ _null_ aggregate_dummy - _null_ ));
DATA(insert OID = 2146 (  min				PGNSP PGUID 12 1 0 t f f f i 1 1700 "1700" _null_ _null_ _null_ aggregate_dummy - _null_ ));
DATA(insert OID = 2051 (  min				PGNSP PGUID 12 1 0 t f f f i 1 2277 "2277" _null_ _null_ _null_ aggregate_dummy - _null_ ));
DATA(insert OID = 2245 (  min				PGNSP PGUID 12 1 0 t f f f i 1 1042 "1042" _null_ _null_ _null_ aggregate_dummy - _null_ ));
DATA(insert OID = 2798 (  min				PGNSP PGUID 12 1 0 t f f f i 1 27 "27" _null_ _null_ _null_ aggregate_dummy - _null_ ));

/* count has two forms: count(any) and count(*) */
DATA(insert OID = 2147 (  count				PGNSP PGUID 12 1 0 t f f f i 1 20 "2276" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2803 (  count				PGNSP PGUID 12 1 0 t f f f i 0 20 "" _null_ _null_ _null_  aggregate_dummy - _null_ ));

DATA(insert OID = 2718 (  var_pop			PGNSP PGUID 12 1 0 t f f f i 1 1700 "20" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2719 (  var_pop			PGNSP PGUID 12 1 0 t f f f i 1 1700 "23" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2720 (  var_pop			PGNSP PGUID 12 1 0 t f f f i 1 1700 "21" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2721 (  var_pop			PGNSP PGUID 12 1 0 t f f f i 1 701 "700" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2722 (  var_pop			PGNSP PGUID 12 1 0 t f f f i 1 701 "701" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2723 (  var_pop			PGNSP PGUID 12 1 0 t f f f i 1 1700 "1700" _null_ _null_ _null_ aggregate_dummy - _null_ ));

DATA(insert OID = 2641 (  var_samp			PGNSP PGUID 12 1 0 t f f f i 1 1700 "20" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2642 (  var_samp			PGNSP PGUID 12 1 0 t f f f i 1 1700 "23" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2643 (  var_samp			PGNSP PGUID 12 1 0 t f f f i 1 1700 "21" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2644 (  var_samp			PGNSP PGUID 12 1 0 t f f f i 1 701 "700" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2645 (  var_samp			PGNSP PGUID 12 1 0 t f f f i 1 701 "701" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2646 (  var_samp			PGNSP PGUID 12 1 0 t f f f i 1 1700 "1700" _null_ _null_ _null_ aggregate_dummy - _null_ ));

DATA(insert OID = 2148 (  variance			PGNSP PGUID 12 1 0 t f f f i 1 1700 "20" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2149 (  variance			PGNSP PGUID 12 1 0 t f f f i 1 1700 "23" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2150 (  variance			PGNSP PGUID 12 1 0 t f f f i 1 1700 "21" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2151 (  variance			PGNSP PGUID 12 1 0 t f f f i 1 701 "700" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2152 (  variance			PGNSP PGUID 12 1 0 t f f f i 1 701 "701" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2153 (  variance			PGNSP PGUID 12 1 0 t f f f i 1 1700 "1700" _null_ _null_ _null_ aggregate_dummy - _null_ ));

DATA(insert OID = 2724 (  stddev_pop		PGNSP PGUID 12 1 0 t f f f i 1 1700 "20" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2725 (  stddev_pop		PGNSP PGUID 12 1 0 t f f f i 1 1700 "23" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2726 (  stddev_pop		PGNSP PGUID 12 1 0 t f f f i 1 1700 "21" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2727 (  stddev_pop		PGNSP PGUID 12 1 0 t f f f i 1 701 "700" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2728 (  stddev_pop		PGNSP PGUID 12 1 0 t f f f i 1 701 "701" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2729 (  stddev_pop		PGNSP PGUID 12 1 0 t f f f i 1 1700 "1700" _null_ _null_ _null_ aggregate_dummy - _null_ ));

DATA(insert OID = 2712 (  stddev_samp		PGNSP PGUID 12 1 0 t f f f i 1 1700 "20" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2713 (  stddev_samp		PGNSP PGUID 12 1 0 t f f f i 1 1700 "23" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2714 (  stddev_samp		PGNSP PGUID 12 1 0 t f f f i 1 1700 "21" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2715 (  stddev_samp		PGNSP PGUID 12 1 0 t f f f i 1 701 "700" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2716 (  stddev_samp		PGNSP PGUID 12 1 0 t f f f i 1 701 "701" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2717 (  stddev_samp		PGNSP PGUID 12 1 0 t f f f i 1 1700 "1700" _null_ _null_ _null_ aggregate_dummy - _null_ ));

DATA(insert OID = 2154 (  stddev			PGNSP PGUID 12 1 0 t f f f i 1 1700 "20" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2155 (  stddev			PGNSP PGUID 12 1 0 t f f f i 1 1700 "23" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2156 (  stddev			PGNSP PGUID 12 1 0 t f f f i 1 1700 "21" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2157 (  stddev			PGNSP PGUID 12 1 0 t f f f i 1 701 "700" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2158 (  stddev			PGNSP PGUID 12 1 0 t f f f i 1 701 "701" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2159 (  stddev			PGNSP PGUID 12 1 0 t f f f i 1 1700 "1700" _null_ _null_ _null_ aggregate_dummy - _null_ ));

DATA(insert OID = 2818 (  regr_count		PGNSP PGUID 12 1 0 t f f f i 2 20 "701 701" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2819 (  regr_sxx			PGNSP PGUID 12 1 0 t f f f i 2 701 "701 701" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2820 (  regr_syy			PGNSP PGUID 12 1 0 t f f f i 2 701 "701 701" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2821 (  regr_sxy			PGNSP PGUID 12 1 0 t f f f i 2 701 "701 701" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2822 (  regr_avgx			PGNSP PGUID 12 1 0 t f f f i 2 701 "701 701" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2823 (  regr_avgy			PGNSP PGUID 12 1 0 t f f f i 2 701 "701 701" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2824 (  regr_r2			PGNSP PGUID 12 1 0 t f f f i 2 701 "701 701" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2825 (  regr_slope		PGNSP PGUID 12 1 0 t f f f i 2 701 "701 701" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2826 (  regr_intercept	PGNSP PGUID 12 1 0 t f f f i 2 701 "701 701" _null_ _null_ _null_  aggregate_dummy - _null_ ));

DATA(insert OID = 2827 (  covar_pop			PGNSP PGUID 12 1 0 t f f f i 2 701 "701 701" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2828 (  covar_samp		PGNSP PGUID 12 1 0 t f f f i 2 701 "701 701" _null_ _null_ _null_  aggregate_dummy - _null_ ));
DATA(insert OID = 2829 (  corr				PGNSP PGUID 12 1 0 t f f f i 2 701 "701 701" _null_ _null_ _null_  aggregate_dummy - _null_ ));

DATA(insert OID = 2160 ( text_pattern_lt	 PGNSP PGUID 12 1 0 f f t f i 2 16 "25 25" _null_ _null_ _null_ text_pattern_lt - _null_ ));
DATA(insert OID = 2161 ( text_pattern_le	 PGNSP PGUID 12 1 0 f f t f i 2 16 "25 25" _null_ _null_ _null_ text_pattern_le - _null_ ));
DATA(insert OID = 2162 ( text_pattern_eq	 PGNSP PGUID 12 1 0 f f t f i 2 16 "25 25" _null_ _null_ _null_ text_pattern_eq - _null_ ));
DATA(insert OID = 2163 ( text_pattern_ge	 PGNSP PGUID 12 1 0 f f t f i 2 16 "25 25" _null_ _null_ _null_ text_pattern_ge - _null_ ));
DATA(insert OID = 2164 ( text_pattern_gt	 PGNSP PGUID 12 1 0 f f t f i 2 16 "25 25" _null_ _null_ _null_ text_pattern_gt - _null_ ));
DATA(insert OID = 2165 ( text_pattern_ne	 PGNSP PGUID 12 1 0 f f t f i 2 16 "25 25" _null_ _null_ _null_ text_pattern_ne - _null_ ));
DATA(insert OID = 2166 ( bttext_pattern_cmp  PGNSP PGUID 12 1 0 f f t f i 2 23 "25 25" _null_ _null_ _null_ bttext_pattern_cmp - _null_ ));

/* We use the same procedures here as above since the types are binary compatible. */
DATA(insert OID = 2174 ( bpchar_pattern_lt	  PGNSP PGUID 12 1 0 f f t f i 2 16 "1042 1042" _null_ _null_ _null_ text_pattern_lt - _null_ ));
DATA(insert OID = 2175 ( bpchar_pattern_le	  PGNSP PGUID 12 1 0 f f t f i 2 16 "1042 1042" _null_ _null_ _null_ text_pattern_le - _null_ ));
DATA(insert OID = 2176 ( bpchar_pattern_eq	  PGNSP PGUID 12 1 0 f f t f i 2 16 "1042 1042" _null_ _null_ _null_ text_pattern_eq - _null_ ));
DATA(insert OID = 2177 ( bpchar_pattern_ge	  PGNSP PGUID 12 1 0 f f t f i 2 16 "1042 1042" _null_ _null_ _null_ text_pattern_ge - _null_ ));
DATA(insert OID = 2178 ( bpchar_pattern_gt	  PGNSP PGUID 12 1 0 f f t f i 2 16 "1042 1042" _null_ _null_ _null_ text_pattern_gt - _null_ ));
DATA(insert OID = 2179 ( bpchar_pattern_ne	  PGNSP PGUID 12 1 0 f f t f i 2 16 "1042 1042" _null_ _null_ _null_ text_pattern_ne - _null_ ));
DATA(insert OID = 2180 ( btbpchar_pattern_cmp PGNSP PGUID 12 1 0 f f t f i 2 23 "1042 1042" _null_ _null_ _null_ bttext_pattern_cmp - _null_ ));

DATA(insert OID = 2181 ( name_pattern_lt	PGNSP PGUID 12 1 0 f f t f i 2 16 "19 19" _null_ _null_ _null_ name_pattern_lt - _null_ ));
DATA(insert OID = 2182 ( name_pattern_le	PGNSP PGUID 12 1 0 f f t f i 2 16 "19 19" _null_ _null_ _null_ name_pattern_le - _null_ ));
DATA(insert OID = 2183 ( name_pattern_eq	PGNSP PGUID 12 1 0 f f t f i 2 16 "19 19" _null_ _null_ _null_ name_pattern_eq - _null_ ));
DATA(insert OID = 2184 ( name_pattern_ge	PGNSP PGUID 12 1 0 f f t f i 2 16 "19 19" _null_ _null_ _null_ name_pattern_ge - _null_ ));
DATA(insert OID = 2185 ( name_pattern_gt	PGNSP PGUID 12 1 0 f f t f i 2 16 "19 19" _null_ _null_ _null_ name_pattern_gt - _null_ ));
DATA(insert OID = 2186 ( name_pattern_ne	PGNSP PGUID 12 1 0 f f t f i 2 16 "19 19" _null_ _null_ _null_ name_pattern_ne - _null_ ));
DATA(insert OID = 2187 ( btname_pattern_cmp PGNSP PGUID 12 1 0 f f t f i 2 23 "19 19" _null_ _null_ _null_ btname_pattern_cmp - _null_ ));

DATA(insert OID = 2188 ( btint48cmp			PGNSP PGUID 12 1 0 f f t f i 2 23 "23 20" _null_ _null_ _null_ btint48cmp - _null_ ));
DATA(insert OID = 2189 ( btint84cmp			PGNSP PGUID 12 1 0 f f t f i 2 23 "20 23" _null_ _null_ _null_ btint84cmp - _null_ ));
DATA(insert OID = 2190 ( btint24cmp			PGNSP PGUID 12 1 0 f f t f i 2 23 "21 23" _null_ _null_ _null_ btint24cmp - _null_ ));
DATA(insert OID = 2191 ( btint42cmp			PGNSP PGUID 12 1 0 f f t f i 2 23 "23 21" _null_ _null_ _null_ btint42cmp - _null_ ));
DATA(insert OID = 2192 ( btint28cmp			PGNSP PGUID 12 1 0 f f t f i 2 23 "21 20" _null_ _null_ _null_ btint28cmp - _null_ ));
DATA(insert OID = 2193 ( btint82cmp			PGNSP PGUID 12 1 0 f f t f i 2 23 "20 21" _null_ _null_ _null_ btint82cmp - _null_ ));
DATA(insert OID = 2194 ( btfloat48cmp		PGNSP PGUID 12 1 0 f f t f i 2 23 "700 701" _null_ _null_ _null_ btfloat48cmp - _null_ ));
DATA(insert OID = 2195 ( btfloat84cmp		PGNSP PGUID 12 1 0 f f t f i 2 23 "701 700" _null_ _null_ _null_ btfloat84cmp - _null_ ));


DATA(insert OID = 2212 (  regprocedurein	PGNSP PGUID 12 1 0 f f t f s 1 2202 "2275" _null_ _null_ _null_ regprocedurein - _null_ ));
DESCR("I/O");
DATA(insert OID = 2213 (  regprocedureout	PGNSP PGUID 12 1 0 f f t f s 1 2275 "2202" _null_ _null_ _null_ regprocedureout - _null_ ));
DESCR("I/O");
DATA(insert OID = 2214 (  regoperin			PGNSP PGUID 12 1 0 f f t f s 1 2203 "2275" _null_ _null_ _null_ regoperin - _null_ ));
DESCR("I/O");
DATA(insert OID = 2215 (  regoperout		PGNSP PGUID 12 1 0 f f t f s 1 2275 "2203" _null_ _null_ _null_ regoperout - _null_ ));
DESCR("I/O");
DATA(insert OID = 2216 (  regoperatorin		PGNSP PGUID 12 1 0 f f t f s 1 2204 "2275" _null_ _null_ _null_ regoperatorin - _null_ ));
DESCR("I/O");
DATA(insert OID = 2217 (  regoperatorout	PGNSP PGUID 12 1 0 f f t f s 1 2275 "2204" _null_ _null_ _null_ regoperatorout - _null_ ));
DESCR("I/O");
DATA(insert OID = 2218 (  regclassin		PGNSP PGUID 12 1 0 f f t f s 1 2205 "2275" _null_ _null_ _null_ regclassin - _null_ ));
DESCR("I/O");
DATA(insert OID = 2219 (  regclassout		PGNSP PGUID 12 1 0 f f t f s 1 2275 "2205" _null_ _null_ _null_ regclassout - _null_ ));
DESCR("I/O");
DATA(insert OID = 2220 (  regtypein			PGNSP PGUID 12 1 0 f f t f s 1 2206 "2275" _null_ _null_ _null_ regtypein - _null_ ));
DESCR("I/O");
DATA(insert OID = 2221 (  regtypeout		PGNSP PGUID 12 1 0 f f t f s 1 2275 "2206" _null_ _null_ _null_ regtypeout - _null_ ));
DESCR("I/O");
DATA(insert OID = 1079 (  regclass			PGNSP PGUID 12 1 0 f f t f s 1 2205 "25" _null_ _null_ _null_	text_regclass - _null_ ));
DESCR("convert text to regclass");

DATA(insert OID = 2246 ( fmgr_internal_validator PGNSP PGUID 12 1 0 f f t f s 1 2278 "26" _null_ _null_ _null_ fmgr_internal_validator - _null_ ));
DESCR("(internal)");
DATA(insert OID = 2247 ( fmgr_c_validator	PGNSP PGUID 12 1 0 f f t f s 1	 2278 "26" _null_ _null_ _null_ fmgr_c_validator - _null_ ));
DESCR("(internal)");
DATA(insert OID = 2248 ( fmgr_sql_validator PGNSP PGUID 12 1 0 f f t f s 1	 2278 "26" _null_ _null_ _null_ fmgr_sql_validator - _null_ ));
DESCR("(internal)");

DATA(insert OID = 2250 (  has_database_privilege		   PGNSP PGUID 12 1 0 f f t f s 3 16 "19 25 25" _null_ _null_ _null_	has_database_privilege_name_name - _null_ ));
DESCR("user privilege on database by username, database name");
DATA(insert OID = 2251 (  has_database_privilege		   PGNSP PGUID 12 1 0 f f t f s 3 16 "19 26 25" _null_ _null_ _null_	has_database_privilege_name_id - _null_ ));
DESCR("user privilege on database by username, database oid");
DATA(insert OID = 2252 (  has_database_privilege		   PGNSP PGUID 12 1 0 f f t f s 3 16 "26 25 25" _null_ _null_ _null_	has_database_privilege_id_name - _null_ ));
DESCR("user privilege on database by user oid, database name");
DATA(insert OID = 2253 (  has_database_privilege		   PGNSP PGUID 12 1 0 f f t f s 3 16 "26 26 25" _null_ _null_ _null_	has_database_privilege_id_id - _null_ ));
DESCR("user privilege on database by user oid, database oid");
DATA(insert OID = 2254 (  has_database_privilege		   PGNSP PGUID 12 1 0 f f t f s 2 16 "25 25" _null_ _null_ _null_ has_database_privilege_name - _null_ ));
DESCR("current user privilege on database by database name");
DATA(insert OID = 2255 (  has_database_privilege		   PGNSP PGUID 12 1 0 f f t f s 2 16 "26 25" _null_ _null_ _null_ has_database_privilege_id - _null_ ));
DESCR("current user privilege on database by database oid");

DATA(insert OID = 2256 (  has_function_privilege		   PGNSP PGUID 12 1 0 f f t f s 3 16 "19 25 25" _null_ _null_ _null_	has_function_privilege_name_name - _null_ ));
DESCR("user privilege on function by username, function name");
DATA(insert OID = 2257 (  has_function_privilege		   PGNSP PGUID 12 1 0 f f t f s 3 16 "19 26 25" _null_ _null_ _null_	has_function_privilege_name_id - _null_ ));
DESCR("user privilege on function by username, function oid");
DATA(insert OID = 2258 (  has_function_privilege		   PGNSP PGUID 12 1 0 f f t f s 3 16 "26 25 25" _null_ _null_ _null_	has_function_privilege_id_name - _null_ ));
DESCR("user privilege on function by user oid, function name");
DATA(insert OID = 2259 (  has_function_privilege		   PGNSP PGUID 12 1 0 f f t f s 3 16 "26 26 25" _null_ _null_ _null_	has_function_privilege_id_id - _null_ ));
DESCR("user privilege on function by user oid, function oid");
DATA(insert OID = 2260 (  has_function_privilege		   PGNSP PGUID 12 1 0 f f t f s 2 16 "25 25" _null_ _null_ _null_ has_function_privilege_name - _null_ ));
DESCR("current user privilege on function by function name");
DATA(insert OID = 2261 (  has_function_privilege		   PGNSP PGUID 12 1 0 f f t f s 2 16 "26 25" _null_ _null_ _null_ has_function_privilege_id - _null_ ));
DESCR("current user privilege on function by function oid");

DATA(insert OID = 2262 (  has_language_privilege		   PGNSP PGUID 12 1 0 f f t f s 3 16 "19 25 25" _null_ _null_ _null_	has_language_privilege_name_name - _null_ ));
DESCR("user privilege on language by username, language name");
DATA(insert OID = 2263 (  has_language_privilege		   PGNSP PGUID 12 1 0 f f t f s 3 16 "19 26 25" _null_ _null_ _null_	has_language_privilege_name_id - _null_ ));
DESCR("user privilege on language by username, language oid");
DATA(insert OID = 2264 (  has_language_privilege		   PGNSP PGUID 12 1 0 f f t f s 3 16 "26 25 25" _null_ _null_ _null_	has_language_privilege_id_name - _null_ ));
DESCR("user privilege on language by user oid, language name");
DATA(insert OID = 2265 (  has_language_privilege		   PGNSP PGUID 12 1 0 f f t f s 3 16 "26 26 25" _null_ _null_ _null_	has_language_privilege_id_id - _null_ ));
DESCR("user privilege on language by user oid, language oid");
DATA(insert OID = 2266 (  has_language_privilege		   PGNSP PGUID 12 1 0 f f t f s 2 16 "25 25" _null_ _null_ _null_ has_language_privilege_name - _null_ ));
DESCR("current user privilege on language by language name");
DATA(insert OID = 2267 (  has_language_privilege		   PGNSP PGUID 12 1 0 f f t f s 2 16 "26 25" _null_ _null_ _null_ has_language_privilege_id - _null_ ));
DESCR("current user privilege on language by language oid");

DATA(insert OID = 2268 (  has_schema_privilege		   PGNSP PGUID 12 1 0 f f t f s 3 16 "19 25 25" _null_ _null_ _null_	has_schema_privilege_name_name - _null_ ));
DESCR("user privilege on schema by username, schema name");
DATA(insert OID = 2269 (  has_schema_privilege		   PGNSP PGUID 12 1 0 f f t f s 3 16 "19 26 25" _null_ _null_ _null_	has_schema_privilege_name_id - _null_ ));
DESCR("user privilege on schema by username, schema oid");
DATA(insert OID = 2270 (  has_schema_privilege		   PGNSP PGUID 12 1 0 f f t f s 3 16 "26 25 25" _null_ _null_ _null_	has_schema_privilege_id_name - _null_ ));
DESCR("user privilege on schema by user oid, schema name");
DATA(insert OID = 2271 (  has_schema_privilege		   PGNSP PGUID 12 1 0 f f t f s 3 16 "26 26 25" _null_ _null_ _null_	has_schema_privilege_id_id - _null_ ));
DESCR("user privilege on schema by user oid, schema oid");
DATA(insert OID = 2272 (  has_schema_privilege		   PGNSP PGUID 12 1 0 f f t f s 2 16 "25 25" _null_ _null_ _null_ has_schema_privilege_name - _null_ ));
DESCR("current user privilege on schema by schema name");
DATA(insert OID = 2273 (  has_schema_privilege		   PGNSP PGUID 12 1 0 f f t f s 2 16 "26 25" _null_ _null_ _null_ has_schema_privilege_id - _null_ ));
DESCR("current user privilege on schema by schema oid");

DATA(insert OID = 2390 (  has_tablespace_privilege		   PGNSP PGUID 12 1 0 f f t f s 3 16 "19 25 25" _null_ _null_ _null_	has_tablespace_privilege_name_name - _null_ ));
DESCR("user privilege on tablespace by username, tablespace name");
DATA(insert OID = 2391 (  has_tablespace_privilege		   PGNSP PGUID 12 1 0 f f t f s 3 16 "19 26 25" _null_ _null_ _null_	has_tablespace_privilege_name_id - _null_ ));
DESCR("user privilege on tablespace by username, tablespace oid");
DATA(insert OID = 2392 (  has_tablespace_privilege		   PGNSP PGUID 12 1 0 f f t f s 3 16 "26 25 25" _null_ _null_ _null_	has_tablespace_privilege_id_name - _null_ ));
DESCR("user privilege on tablespace by user oid, tablespace name");
DATA(insert OID = 2393 (  has_tablespace_privilege		   PGNSP PGUID 12 1 0 f f t f s 3 16 "26 26 25" _null_ _null_ _null_	has_tablespace_privilege_id_id - _null_ ));
DESCR("user privilege on tablespace by user oid, tablespace oid");
DATA(insert OID = 2394 (  has_tablespace_privilege		   PGNSP PGUID 12 1 0 f f t f s 2 16 "25 25" _null_ _null_ _null_ has_tablespace_privilege_name - _null_ ));
DESCR("current user privilege on tablespace by tablespace name");
DATA(insert OID = 2395 (  has_tablespace_privilege		   PGNSP PGUID 12 1 0 f f t f s 2 16 "26 25" _null_ _null_ _null_ has_tablespace_privilege_id - _null_ ));
DESCR("current user privilege on tablespace by tablespace oid");

DATA(insert OID = 2705 (  pg_has_role		PGNSP PGUID 12 1 0 f f t f s 3 16 "19 19 25" _null_ _null_ _null_	pg_has_role_name_name - _null_ ));
DESCR("user privilege on role by username, role name");
DATA(insert OID = 2706 (  pg_has_role		PGNSP PGUID 12 1 0 f f t f s 3 16 "19 26 25" _null_ _null_ _null_	pg_has_role_name_id - _null_ ));
DESCR("user privilege on role by username, role oid");
DATA(insert OID = 2707 (  pg_has_role		PGNSP PGUID 12 1 0 f f t f s 3 16 "26 19 25" _null_ _null_ _null_	pg_has_role_id_name - _null_ ));
DESCR("user privilege on role by user oid, role name");
DATA(insert OID = 2708 (  pg_has_role		PGNSP PGUID 12 1 0 f f t f s 3 16 "26 26 25" _null_ _null_ _null_	pg_has_role_id_id - _null_ ));
DESCR("user privilege on role by user oid, role oid");
DATA(insert OID = 2709 (  pg_has_role		PGNSP PGUID 12 1 0 f f t f s 2 16 "19 25" _null_ _null_ _null_ pg_has_role_name - _null_ ));
DESCR("current user privilege on role by role name");
DATA(insert OID = 2710 (  pg_has_role		PGNSP PGUID 12 1 0 f f t f s 2 16 "26 25" _null_ _null_ _null_ pg_has_role_id - _null_ ));
DESCR("current user privilege on role by role oid");

DATA(insert OID = 1269 (  pg_column_size		PGNSP PGUID 12 1 0 f f t f s 1 23 "2276" _null_ _null_ _null_  pg_column_size - _null_ ));
DESCR("bytes required to store the value, perhaps with compression");
DATA(insert OID = 2322 ( pg_tablespace_size		PGNSP PGUID 12 1 0 f f t f v 1 20 "26" _null_ _null_ _null_ pg_tablespace_size_oid - _null_ ));
DESCR("total disk space usage for the specified tablespace");
DATA(insert OID = 2323 ( pg_tablespace_size		PGNSP PGUID 12 1 0 f f t f v 1 20 "19" _null_ _null_ _null_ pg_tablespace_size_name - _null_ ));
DESCR("total disk space usage for the specified tablespace");
DATA(insert OID = 2324 ( pg_database_size		PGNSP PGUID 12 1 0 f f t f v 1 20 "26" _null_ _null_ _null_ pg_database_size_oid - _null_ ));
DESCR("total disk space usage for the specified database");
DATA(insert OID = 2168 ( pg_database_size		PGNSP PGUID 12 1 0 f f t f v 1 20 "19" _null_ _null_ _null_ pg_database_size_name - _null_ ));
DESCR("total disk space usage for the specified database");
DATA(insert OID = 2325 ( pg_relation_size		PGNSP PGUID 12 1 0 f f t f v 1 20 "26" _null_ _null_ _null_ pg_relation_size_oid - _null_ ));
DESCR("disk space usage for the specified table or index");
DATA(insert OID = 2289 ( pg_relation_size		PGNSP PGUID 12 1 0 f f t f v 1 20 "25" _null_ _null_ _null_ pg_relation_size_name - _null_ ));
DESCR("disk space usage for the specified table or index");
DATA(insert OID = 2286 ( pg_total_relation_size		PGNSP PGUID 12 1 0 f f t f v 1 20 "26" _null_ _null_ _null_ pg_total_relation_size_oid - _null_ ));
DESCR("total disk space usage for the specified table and associated indexes and toast tables");
DATA(insert OID = 2287 ( pg_total_relation_size		PGNSP PGUID 12 1 0 f f t f v 1 20 "25" _null_ _null_ _null_ pg_total_relation_size_name - _null_ ));
DESCR("total disk space usage for the specified table and associated indexes and toast tables");
DATA(insert OID = 2288 ( pg_size_pretty			PGNSP PGUID 12 1 0 f f t f v 1 25 "20" _null_ _null_ _null_ pg_size_pretty - _null_ ));
DESCR("convert a long int to a human readable text using size units");

DATA(insert OID = 2290 (  record_in			PGNSP PGUID 12 1 0 f f t f v 3 2249 "2275 26 23" _null_ _null_ _null_	record_in - _null_ ));
DESCR("I/O");
DATA(insert OID = 2291 (  record_out		PGNSP PGUID 12 1 0 f f t f v 1 2275 "2249" _null_ _null_ _null_ record_out - _null_ ));
DESCR("I/O");
DATA(insert OID = 2292 (  cstring_in		PGNSP PGUID 12 1 0 f f t f i 1 2275 "2275" _null_ _null_ _null_ cstring_in - _null_ ));
DESCR("I/O");
DATA(insert OID = 2293 (  cstring_out		PGNSP PGUID 12 1 0 f f t f i 1 2275 "2275" _null_ _null_ _null_ cstring_out - _null_ ));
DESCR("I/O");
DATA(insert OID = 2294 (  any_in			PGNSP PGUID 12 1 0 f f t f i 1 2276 "2275" _null_ _null_ _null_ any_in - _null_ ));
DESCR("I/O");
DATA(insert OID = 2295 (  any_out			PGNSP PGUID 12 1 0 f f t f i 1 2275 "2276" _null_ _null_ _null_ any_out - _null_ ));
DESCR("I/O");
DATA(insert OID = 2296 (  anyarray_in		PGNSP PGUID 12 1 0 f f t f i 1 2277 "2275" _null_ _null_ _null_ anyarray_in - _null_ ));
DESCR("I/O");
DATA(insert OID = 2297 (  anyarray_out		PGNSP PGUID 12 1 0 f f t f s 1 2275 "2277" _null_ _null_ _null_ anyarray_out - _null_ ));
DESCR("I/O");
DATA(insert OID = 2298 (  void_in			PGNSP PGUID 12 1 0 f f t f i 1 2278 "2275" _null_ _null_ _null_ void_in - _null_ ));
DESCR("I/O");
DATA(insert OID = 2299 (  void_out			PGNSP PGUID 12 1 0 f f t f i 1 2275 "2278" _null_ _null_ _null_ void_out - _null_ ));
DESCR("I/O");
DATA(insert OID = 2300 (  trigger_in		PGNSP PGUID 12 1 0 f f t f i 1 2279 "2275" _null_ _null_ _null_ trigger_in - _null_ ));
DESCR("I/O");
DATA(insert OID = 2301 (  trigger_out		PGNSP PGUID 12 1 0 f f t f i 1 2275 "2279" _null_ _null_ _null_ trigger_out - _null_ ));
DESCR("I/O");
DATA(insert OID = 2302 (  language_handler_in	PGNSP PGUID 12 1 0 f f t f i 1 2280 "2275" _null_ _null_ _null_ language_handler_in - _null_ ));
DESCR("I/O");
DATA(insert OID = 2303 (  language_handler_out	PGNSP PGUID 12 1 0 f f t f i 1 2275 "2280" _null_ _null_ _null_ language_handler_out - _null_ ));
DESCR("I/O");
DATA(insert OID = 2304 (  internal_in		PGNSP PGUID 12 1 0 f f t f i 1 2281 "2275" _null_ _null_ _null_ internal_in - _null_ ));
DESCR("I/O");
DATA(insert OID = 2305 (  internal_out		PGNSP PGUID 12 1 0 f f t f i 1 2275 "2281" _null_ _null_ _null_ internal_out - _null_ ));
DESCR("I/O");
DATA(insert OID = 2306 (  opaque_in			PGNSP PGUID 12 1 0 f f t f i 1 2282 "2275" _null_ _null_ _null_ opaque_in - _null_ ));
DESCR("I/O");
DATA(insert OID = 2307 (  opaque_out		PGNSP PGUID 12 1 0 f f t f i 1 2275 "2282" _null_ _null_ _null_ opaque_out - _null_ ));
DESCR("I/O");
DATA(insert OID = 2312 (  anyelement_in		PGNSP PGUID 12 1 0 f f t f i 1 2283 "2275" _null_ _null_ _null_ anyelement_in - _null_ ));
DESCR("I/O");
DATA(insert OID = 2313 (  anyelement_out	PGNSP PGUID 12 1 0 f f t f i 1 2275 "2283" _null_ _null_ _null_ anyelement_out - _null_ ));
DESCR("I/O");
DATA(insert OID = 2398 (  shell_in			PGNSP PGUID 12 1 0 f f t f i 1 2282 "2275" _null_ _null_ _null_ shell_in - _null_ ));
DESCR("I/O");
DATA(insert OID = 2399 (  shell_out			PGNSP PGUID 12 1 0 f f t f i 1 2275 "2282" _null_ _null_ _null_ shell_out - _null_ ));
DESCR("I/O");
DATA(insert OID = 2597 (  domain_in			PGNSP PGUID 12 1 0 f f f f v 3 2276 "2275 26 23" _null_ _null_ _null_ domain_in - _null_ ));
DESCR("I/O");
DATA(insert OID = 2598 (  domain_recv		PGNSP PGUID 12 1 0 f f f f v 3 2276 "2281 26 23" _null_ _null_ _null_ domain_recv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2777 (  anynonarray_in	PGNSP PGUID 12 1 0 f f t f i 1 2776 "2275" _null_ _null_ _null_ anynonarray_in - _null_ ));
DESCR("I/O");
DATA(insert OID = 2778 (  anynonarray_out	PGNSP PGUID 12 1 0 f f t f i 1 2275 "2776" _null_ _null_ _null_ anynonarray_out - _null_ ));
DESCR("I/O");

/* cryptographic */
DATA(insert OID =  2311 (  md5	   PGNSP PGUID 12 1 0 f f t f i 1 25 "25" _null_ _null_ _null_	md5_text - _null_ ));
DESCR("calculates md5 hash");
DATA(insert OID =  2321 (  md5	   PGNSP PGUID 12 1 0 f f t f i 1 25 "17" _null_ _null_ _null_	md5_bytea - _null_ ));
DESCR("calculates md5 hash");

/* crosstype operations for date vs. timestamp and timestamptz */
DATA(insert OID = 2338 (  date_lt_timestamp		   PGNSP PGUID 12 1 0 f f t f i 2 16 "1082 1114" _null_ _null_ _null_ date_lt_timestamp - _null_ ));
DESCR("less-than");
DATA(insert OID = 2339 (  date_le_timestamp		   PGNSP PGUID 12 1 0 f f t f i 2 16 "1082 1114" _null_ _null_ _null_ date_le_timestamp - _null_ ));
DESCR("less-than-or-equal");
DATA(insert OID = 2340 (  date_eq_timestamp		   PGNSP PGUID 12 1 0 f f t f i 2 16 "1082 1114" _null_ _null_ _null_ date_eq_timestamp - _null_ ));
DESCR("equal");
DATA(insert OID = 2341 (  date_gt_timestamp		   PGNSP PGUID 12 1 0 f f t f i 2 16 "1082 1114" _null_ _null_ _null_ date_gt_timestamp - _null_ ));
DESCR("greater-than");
DATA(insert OID = 2342 (  date_ge_timestamp		   PGNSP PGUID 12 1 0 f f t f i 2 16 "1082 1114" _null_ _null_ _null_ date_ge_timestamp - _null_ ));
DESCR("greater-than-or-equal");
DATA(insert OID = 2343 (  date_ne_timestamp		   PGNSP PGUID 12 1 0 f f t f i 2 16 "1082 1114" _null_ _null_ _null_ date_ne_timestamp - _null_ ));
DESCR("not equal");
DATA(insert OID = 2344 (  date_cmp_timestamp	   PGNSP PGUID 12 1 0 f f t f i 2 23 "1082 1114" _null_ _null_ _null_ date_cmp_timestamp - _null_ ));
DESCR("less-equal-greater");

DATA(insert OID = 2351 (  date_lt_timestamptz	   PGNSP PGUID 12 1 0 f f t f s 2 16 "1082 1184" _null_ _null_ _null_ date_lt_timestamptz - _null_ ));
DESCR("less-than");
DATA(insert OID = 2352 (  date_le_timestamptz	   PGNSP PGUID 12 1 0 f f t f s 2 16 "1082 1184" _null_ _null_ _null_ date_le_timestamptz - _null_ ));
DESCR("less-than-or-equal");
DATA(insert OID = 2353 (  date_eq_timestamptz	   PGNSP PGUID 12 1 0 f f t f s 2 16 "1082 1184" _null_ _null_ _null_ date_eq_timestamptz - _null_ ));
DESCR("equal");
DATA(insert OID = 2354 (  date_gt_timestamptz	   PGNSP PGUID 12 1 0 f f t f s 2 16 "1082 1184" _null_ _null_ _null_ date_gt_timestamptz - _null_ ));
DESCR("greater-than");
DATA(insert OID = 2355 (  date_ge_timestamptz	   PGNSP PGUID 12 1 0 f f t f s 2 16 "1082 1184" _null_ _null_ _null_ date_ge_timestamptz - _null_ ));
DESCR("greater-than-or-equal");
DATA(insert OID = 2356 (  date_ne_timestamptz	   PGNSP PGUID 12 1 0 f f t f s 2 16 "1082 1184" _null_ _null_ _null_ date_ne_timestamptz - _null_ ));
DESCR("not equal");
DATA(insert OID = 2357 (  date_cmp_timestamptz	   PGNSP PGUID 12 1 0 f f t f s 2 23 "1082 1184" _null_ _null_ _null_ date_cmp_timestamptz - _null_ ));
DESCR("less-equal-greater");

DATA(insert OID = 2364 (  timestamp_lt_date		   PGNSP PGUID 12 1 0 f f t f i 2 16 "1114 1082" _null_ _null_ _null_ timestamp_lt_date - _null_ ));
DESCR("less-than");
DATA(insert OID = 2365 (  timestamp_le_date		   PGNSP PGUID 12 1 0 f f t f i 2 16 "1114 1082" _null_ _null_ _null_ timestamp_le_date - _null_ ));
DESCR("less-than-or-equal");
DATA(insert OID = 2366 (  timestamp_eq_date		   PGNSP PGUID 12 1 0 f f t f i 2 16 "1114 1082" _null_ _null_ _null_ timestamp_eq_date - _null_ ));
DESCR("equal");
DATA(insert OID = 2367 (  timestamp_gt_date		   PGNSP PGUID 12 1 0 f f t f i 2 16 "1114 1082" _null_ _null_ _null_ timestamp_gt_date - _null_ ));
DESCR("greater-than");
DATA(insert OID = 2368 (  timestamp_ge_date		   PGNSP PGUID 12 1 0 f f t f i 2 16 "1114 1082" _null_ _null_ _null_ timestamp_ge_date - _null_ ));
DESCR("greater-than-or-equal");
DATA(insert OID = 2369 (  timestamp_ne_date		   PGNSP PGUID 12 1 0 f f t f i 2 16 "1114 1082" _null_ _null_ _null_ timestamp_ne_date - _null_ ));
DESCR("not equal");
DATA(insert OID = 2370 (  timestamp_cmp_date	   PGNSP PGUID 12 1 0 f f t f i 2 23 "1114 1082" _null_ _null_ _null_ timestamp_cmp_date - _null_ ));
DESCR("less-equal-greater");

DATA(insert OID = 2377 (  timestamptz_lt_date	   PGNSP PGUID 12 1 0 f f t f s 2 16 "1184 1082" _null_ _null_ _null_ timestamptz_lt_date - _null_ ));
DESCR("less-than");
DATA(insert OID = 2378 (  timestamptz_le_date	   PGNSP PGUID 12 1 0 f f t f s 2 16 "1184 1082" _null_ _null_ _null_ timestamptz_le_date - _null_ ));
DESCR("less-than-or-equal");
DATA(insert OID = 2379 (  timestamptz_eq_date	   PGNSP PGUID 12 1 0 f f t f s 2 16 "1184 1082" _null_ _null_ _null_ timestamptz_eq_date - _null_ ));
DESCR("equal");
DATA(insert OID = 2380 (  timestamptz_gt_date	   PGNSP PGUID 12 1 0 f f t f s 2 16 "1184 1082" _null_ _null_ _null_ timestamptz_gt_date - _null_ ));
DESCR("greater-than");
DATA(insert OID = 2381 (  timestamptz_ge_date	   PGNSP PGUID 12 1 0 f f t f s 2 16 "1184 1082" _null_ _null_ _null_ timestamptz_ge_date - _null_ ));
DESCR("greater-than-or-equal");
DATA(insert OID = 2382 (  timestamptz_ne_date	   PGNSP PGUID 12 1 0 f f t f s 2 16 "1184 1082" _null_ _null_ _null_ timestamptz_ne_date - _null_ ));
DESCR("not equal");
DATA(insert OID = 2383 (  timestamptz_cmp_date	   PGNSP PGUID 12 1 0 f f t f s 2 23 "1184 1082" _null_ _null_ _null_ timestamptz_cmp_date - _null_ ));
DESCR("less-equal-greater");

/* crosstype operations for timestamp vs. timestamptz */
DATA(insert OID = 2520 (  timestamp_lt_timestamptz	PGNSP PGUID 12 1 0 f f t f s 2 16 "1114 1184" _null_ _null_ _null_	timestamp_lt_timestamptz - _null_ ));
DESCR("less-than");
DATA(insert OID = 2521 (  timestamp_le_timestamptz	PGNSP PGUID 12 1 0 f f t f s 2 16 "1114 1184" _null_ _null_ _null_	timestamp_le_timestamptz - _null_ ));
DESCR("less-than-or-equal");
DATA(insert OID = 2522 (  timestamp_eq_timestamptz	PGNSP PGUID 12 1 0 f f t f s 2 16 "1114 1184" _null_ _null_ _null_	timestamp_eq_timestamptz - _null_ ));
DESCR("equal");
DATA(insert OID = 2523 (  timestamp_gt_timestamptz	PGNSP PGUID 12 1 0 f f t f s 2 16 "1114 1184" _null_ _null_ _null_	timestamp_gt_timestamptz - _null_ ));
DESCR("greater-than");
DATA(insert OID = 2524 (  timestamp_ge_timestamptz	PGNSP PGUID 12 1 0 f f t f s 2 16 "1114 1184" _null_ _null_ _null_	timestamp_ge_timestamptz - _null_ ));
DESCR("greater-than-or-equal");
DATA(insert OID = 2525 (  timestamp_ne_timestamptz	PGNSP PGUID 12 1 0 f f t f s 2 16 "1114 1184" _null_ _null_ _null_	timestamp_ne_timestamptz - _null_ ));
DESCR("not equal");
DATA(insert OID = 2526 (  timestamp_cmp_timestamptz PGNSP PGUID 12 1 0 f f t f s 2 23 "1114 1184" _null_ _null_ _null_	timestamp_cmp_timestamptz - _null_ ));
DESCR("less-equal-greater");

DATA(insert OID = 2527 (  timestamptz_lt_timestamp	PGNSP PGUID 12 1 0 f f t f s 2 16 "1184 1114" _null_ _null_ _null_	timestamptz_lt_timestamp - _null_ ));
DESCR("less-than");
DATA(insert OID = 2528 (  timestamptz_le_timestamp	PGNSP PGUID 12 1 0 f f t f s 2 16 "1184 1114" _null_ _null_ _null_	timestamptz_le_timestamp - _null_ ));
DESCR("less-than-or-equal");
DATA(insert OID = 2529 (  timestamptz_eq_timestamp	PGNSP PGUID 12 1 0 f f t f s 2 16 "1184 1114" _null_ _null_ _null_	timestamptz_eq_timestamp - _null_ ));
DESCR("equal");
DATA(insert OID = 2530 (  timestamptz_gt_timestamp	PGNSP PGUID 12 1 0 f f t f s 2 16 "1184 1114" _null_ _null_ _null_	timestamptz_gt_timestamp - _null_ ));
DESCR("greater-than");
DATA(insert OID = 2531 (  timestamptz_ge_timestamp	PGNSP PGUID 12 1 0 f f t f s 2 16 "1184 1114" _null_ _null_ _null_	timestamptz_ge_timestamp - _null_ ));
DESCR("greater-than-or-equal");
DATA(insert OID = 2532 (  timestamptz_ne_timestamp	PGNSP PGUID 12 1 0 f f t f s 2 16 "1184 1114" _null_ _null_ _null_	timestamptz_ne_timestamp - _null_ ));
DESCR("not equal");
DATA(insert OID = 2533 (  timestamptz_cmp_timestamp PGNSP PGUID 12 1 0 f f t f s 2 23 "1184 1114" _null_ _null_ _null_	timestamptz_cmp_timestamp - _null_ ));
DESCR("less-equal-greater");


/* send/receive functions */
DATA(insert OID = 2400 (  array_recv		   PGNSP PGUID 12 1 0 f f t f s 3 2277 "2281 26 23" _null_ _null_ _null_  array_recv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2401 (  array_send		   PGNSP PGUID 12 1 0 f f t f s 1 17 "2277" _null_ _null_ _null_	array_send - _null_ ));
DESCR("I/O");
DATA(insert OID = 2402 (  record_recv		   PGNSP PGUID 12 1 0 f f t f v 3 2249 "2281 26 23" _null_ _null_ _null_  record_recv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2403 (  record_send		   PGNSP PGUID 12 1 0 f f t f v 1 17 "2249" _null_ _null_ _null_  record_send - _null_ ));
DESCR("I/O");
DATA(insert OID = 2404 (  int2recv			   PGNSP PGUID 12 1 0 f f t f i 1 21 "2281" _null_ _null_ _null_	int2recv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2405 (  int2send			   PGNSP PGUID 12 1 0 f f t f i 1 17 "21" _null_ _null_ _null_	int2send - _null_ ));
DESCR("I/O");
DATA(insert OID = 2406 (  int4recv			   PGNSP PGUID 12 1 0 f f t f i 1 23 "2281" _null_ _null_ _null_	int4recv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2407 (  int4send			   PGNSP PGUID 12 1 0 f f t f i 1 17 "23" _null_ _null_ _null_	int4send - _null_ ));
DESCR("I/O");
DATA(insert OID = 2408 (  int8recv			   PGNSP PGUID 12 1 0 f f t f i 1 20 "2281" _null_ _null_ _null_	int8recv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2409 (  int8send			   PGNSP PGUID 12 1 0 f f t f i 1 17 "20" _null_ _null_ _null_	int8send - _null_ ));
DESCR("I/O");
DATA(insert OID = 2410 (  int2vectorrecv	   PGNSP PGUID 12 1 0 f f t f i 1 22 "2281" _null_ _null_ _null_	int2vectorrecv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2411 (  int2vectorsend	   PGNSP PGUID 12 1 0 f f t f i 1 17 "22" _null_ _null_ _null_	int2vectorsend - _null_ ));
DESCR("I/O");
DATA(insert OID = 2412 (  bytearecv			   PGNSP PGUID 12 1 0 f f t f i 1 17 "2281" _null_ _null_ _null_	bytearecv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2413 (  byteasend			   PGNSP PGUID 12 1 0 f f t f i 1 17 "17" _null_ _null_ _null_	byteasend - _null_ ));
DESCR("I/O");
DATA(insert OID = 2414 (  textrecv			   PGNSP PGUID 12 1 0 f f t f s 1 25 "2281" _null_ _null_ _null_	textrecv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2415 (  textsend			   PGNSP PGUID 12 1 0 f f t f s 1 17 "25" _null_ _null_ _null_	textsend - _null_ ));
DESCR("I/O");
DATA(insert OID = 2416 (  unknownrecv		   PGNSP PGUID 12 1 0 f f t f i 1 705 "2281" _null_ _null_ _null_  unknownrecv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2417 (  unknownsend		   PGNSP PGUID 12 1 0 f f t f i 1 17 "705" _null_ _null_ _null_ unknownsend - _null_ ));
DESCR("I/O");
DATA(insert OID = 2418 (  oidrecv			   PGNSP PGUID 12 1 0 f f t f i 1 26 "2281" _null_ _null_ _null_	oidrecv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2419 (  oidsend			   PGNSP PGUID 12 1 0 f f t f i 1 17 "26" _null_ _null_ _null_	oidsend - _null_ ));
DESCR("I/O");
DATA(insert OID = 2420 (  oidvectorrecv		   PGNSP PGUID 12 1 0 f f t f i 1 30 "2281" _null_ _null_ _null_	oidvectorrecv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2421 (  oidvectorsend		   PGNSP PGUID 12 1 0 f f t f i 1 17 "30" _null_ _null_ _null_	oidvectorsend - _null_ ));
DESCR("I/O");
DATA(insert OID = 2422 (  namerecv			   PGNSP PGUID 12 1 0 f f t f s 1 19 "2281" _null_ _null_ _null_	namerecv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2423 (  namesend			   PGNSP PGUID 12 1 0 f f t f s 1 17 "19" _null_ _null_ _null_	namesend - _null_ ));
DESCR("I/O");
DATA(insert OID = 2424 (  float4recv		   PGNSP PGUID 12 1 0 f f t f i 1 700 "2281" _null_ _null_ _null_  float4recv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2425 (  float4send		   PGNSP PGUID 12 1 0 f f t f i 1 17 "700" _null_ _null_ _null_ float4send - _null_ ));
DESCR("I/O");
DATA(insert OID = 2426 (  float8recv		   PGNSP PGUID 12 1 0 f f t f i 1 701 "2281" _null_ _null_ _null_  float8recv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2427 (  float8send		   PGNSP PGUID 12 1 0 f f t f i 1 17 "701" _null_ _null_ _null_ float8send - _null_ ));
DESCR("I/O");
DATA(insert OID = 2428 (  point_recv		   PGNSP PGUID 12 1 0 f f t f i 1 600 "2281" _null_ _null_ _null_  point_recv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2429 (  point_send		   PGNSP PGUID 12 1 0 f f t f i 1 17 "600" _null_ _null_ _null_ point_send - _null_ ));
DESCR("I/O");
DATA(insert OID = 2430 (  bpcharrecv		   PGNSP PGUID 12 1 0 f f t f s 3 1042 "2281 26 23" _null_ _null_ _null_  bpcharrecv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2431 (  bpcharsend		   PGNSP PGUID 12 1 0 f f t f s 1 17 "1042" _null_ _null_ _null_	bpcharsend - _null_ ));
DESCR("I/O");
DATA(insert OID = 2432 (  varcharrecv		   PGNSP PGUID 12 1 0 f f t f s 3 1043 "2281 26 23" _null_ _null_ _null_  varcharrecv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2433 (  varcharsend		   PGNSP PGUID 12 1 0 f f t f s 1 17 "1043" _null_ _null_ _null_	varcharsend - _null_ ));
DESCR("I/O");
DATA(insert OID = 2434 (  charrecv			   PGNSP PGUID 12 1 0 f f t f i 1 18 "2281" _null_ _null_ _null_	charrecv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2435 (  charsend			   PGNSP PGUID 12 1 0 f f t f i 1 17 "18" _null_ _null_ _null_	charsend - _null_ ));
DESCR("I/O");
DATA(insert OID = 2436 (  boolrecv			   PGNSP PGUID 12 1 0 f f t f i 1 16 "2281" _null_ _null_ _null_	boolrecv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2437 (  boolsend			   PGNSP PGUID 12 1 0 f f t f i 1 17 "16" _null_ _null_ _null_	boolsend - _null_ ));
DESCR("I/O");
DATA(insert OID = 2438 (  tidrecv			   PGNSP PGUID 12 1 0 f f t f i 1 27 "2281" _null_ _null_ _null_	tidrecv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2439 (  tidsend			   PGNSP PGUID 12 1 0 f f t f i 1 17 "27" _null_ _null_ _null_	tidsend - _null_ ));
DESCR("I/O");
DATA(insert OID = 2440 (  xidrecv			   PGNSP PGUID 12 1 0 f f t f i 1 28 "2281" _null_ _null_ _null_	xidrecv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2441 (  xidsend			   PGNSP PGUID 12 1 0 f f t f i 1 17 "28" _null_ _null_ _null_	xidsend - _null_ ));
DESCR("I/O");
DATA(insert OID = 2442 (  cidrecv			   PGNSP PGUID 12 1 0 f f t f i 1 29 "2281" _null_ _null_ _null_	cidrecv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2443 (  cidsend			   PGNSP PGUID 12 1 0 f f t f i 1 17 "29" _null_ _null_ _null_	cidsend - _null_ ));
DESCR("I/O");
DATA(insert OID = 2444 (  regprocrecv		   PGNSP PGUID 12 1 0 f f t f i 1 24 "2281" _null_ _null_ _null_	regprocrecv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2445 (  regprocsend		   PGNSP PGUID 12 1 0 f f t f i 1 17 "24" _null_ _null_ _null_	regprocsend - _null_ ));
DESCR("I/O");
DATA(insert OID = 2446 (  regprocedurerecv	   PGNSP PGUID 12 1 0 f f t f i 1 2202 "2281" _null_ _null_ _null_	regprocedurerecv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2447 (  regproceduresend	   PGNSP PGUID 12 1 0 f f t f i 1 17 "2202" _null_ _null_ _null_	regproceduresend - _null_ ));
DESCR("I/O");
DATA(insert OID = 2448 (  regoperrecv		   PGNSP PGUID 12 1 0 f f t f i 1 2203 "2281" _null_ _null_ _null_	regoperrecv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2449 (  regopersend		   PGNSP PGUID 12 1 0 f f t f i 1 17 "2203" _null_ _null_ _null_	regopersend - _null_ ));
DESCR("I/O");
DATA(insert OID = 2450 (  regoperatorrecv	   PGNSP PGUID 12 1 0 f f t f i 1 2204 "2281" _null_ _null_ _null_	regoperatorrecv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2451 (  regoperatorsend	   PGNSP PGUID 12 1 0 f f t f i 1 17 "2204" _null_ _null_ _null_	regoperatorsend - _null_ ));
DESCR("I/O");
DATA(insert OID = 2452 (  regclassrecv		   PGNSP PGUID 12 1 0 f f t f i 1 2205 "2281" _null_ _null_ _null_	regclassrecv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2453 (  regclasssend		   PGNSP PGUID 12 1 0 f f t f i 1 17 "2205" _null_ _null_ _null_	regclasssend - _null_ ));
DESCR("I/O");
DATA(insert OID = 2454 (  regtyperecv		   PGNSP PGUID 12 1 0 f f t f i 1 2206 "2281" _null_ _null_ _null_	regtyperecv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2455 (  regtypesend		   PGNSP PGUID 12 1 0 f f t f i 1 17 "2206" _null_ _null_ _null_	regtypesend - _null_ ));
DESCR("I/O");
DATA(insert OID = 2456 (  bit_recv			   PGNSP PGUID 12 1 0 f f t f i 3 1560 "2281 26 23" _null_ _null_ _null_  bit_recv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2457 (  bit_send			   PGNSP PGUID 12 1 0 f f t f i 1 17 "1560" _null_ _null_ _null_	bit_send - _null_ ));
DESCR("I/O");
DATA(insert OID = 2458 (  varbit_recv		   PGNSP PGUID 12 1 0 f f t f i 3 1562 "2281 26 23" _null_ _null_ _null_  varbit_recv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2459 (  varbit_send		   PGNSP PGUID 12 1 0 f f t f i 1 17 "1562" _null_ _null_ _null_	varbit_send - _null_ ));
DESCR("I/O");
DATA(insert OID = 2460 (  numeric_recv		   PGNSP PGUID 12 1 0 f f t f i 3 1700 "2281 26 23" _null_ _null_ _null_  numeric_recv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2461 (  numeric_send		   PGNSP PGUID 12 1 0 f f t f i 1 17 "1700" _null_ _null_ _null_	numeric_send - _null_ ));
DESCR("I/O");
DATA(insert OID = 2462 (  abstimerecv		   PGNSP PGUID 12 1 0 f f t f i 1 702 "2281" _null_ _null_ _null_  abstimerecv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2463 (  abstimesend		   PGNSP PGUID 12 1 0 f f t f i 1 17 "702" _null_ _null_ _null_ abstimesend - _null_ ));
DESCR("I/O");
DATA(insert OID = 2464 (  reltimerecv		   PGNSP PGUID 12 1 0 f f t f i 1 703 "2281" _null_ _null_ _null_  reltimerecv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2465 (  reltimesend		   PGNSP PGUID 12 1 0 f f t f i 1 17 "703" _null_ _null_ _null_ reltimesend - _null_ ));
DESCR("I/O");
DATA(insert OID = 2466 (  tintervalrecv		   PGNSP PGUID 12 1 0 f f t f i 1 704 "2281" _null_ _null_ _null_  tintervalrecv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2467 (  tintervalsend		   PGNSP PGUID 12 1 0 f f t f i 1 17 "704" _null_ _null_ _null_ tintervalsend - _null_ ));
DESCR("I/O");
DATA(insert OID = 2468 (  date_recv			   PGNSP PGUID 12 1 0 f f t f i 1 1082 "2281" _null_ _null_ _null_	date_recv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2469 (  date_send			   PGNSP PGUID 12 1 0 f f t f i 1 17 "1082" _null_ _null_ _null_	date_send - _null_ ));
DESCR("I/O");
DATA(insert OID = 2470 (  time_recv			   PGNSP PGUID 12 1 0 f f t f i 3 1083 "2281 26 23" _null_ _null_ _null_  time_recv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2471 (  time_send			   PGNSP PGUID 12 1 0 f f t f i 1 17 "1083" _null_ _null_ _null_	time_send - _null_ ));
DESCR("I/O");
DATA(insert OID = 2472 (  timetz_recv		   PGNSP PGUID 12 1 0 f f t f i 3 1266 "2281 26 23" _null_ _null_ _null_  timetz_recv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2473 (  timetz_send		   PGNSP PGUID 12 1 0 f f t f i 1 17 "1266" _null_ _null_ _null_	timetz_send - _null_ ));
DESCR("I/O");
DATA(insert OID = 2474 (  timestamp_recv	   PGNSP PGUID 12 1 0 f f t f i 3 1114 "2281 26 23" _null_ _null_ _null_  timestamp_recv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2475 (  timestamp_send	   PGNSP PGUID 12 1 0 f f t f i 1 17 "1114" _null_ _null_ _null_	timestamp_send - _null_ ));
DESCR("I/O");
DATA(insert OID = 2476 (  timestamptz_recv	   PGNSP PGUID 12 1 0 f f t f i 3 1184 "2281 26 23" _null_ _null_ _null_  timestamptz_recv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2477 (  timestamptz_send	   PGNSP PGUID 12 1 0 f f t f i 1 17 "1184" _null_ _null_ _null_	timestamptz_send - _null_ ));
DESCR("I/O");
DATA(insert OID = 2478 (  interval_recv		   PGNSP PGUID 12 1 0 f f t f i 3 1186 "2281 26 23" _null_ _null_ _null_  interval_recv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2479 (  interval_send		   PGNSP PGUID 12 1 0 f f t f i 1 17 "1186" _null_ _null_ _null_	interval_send - _null_ ));
DESCR("I/O");
DATA(insert OID = 2480 (  lseg_recv			   PGNSP PGUID 12 1 0 f f t f i 1 601 "2281" _null_ _null_ _null_  lseg_recv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2481 (  lseg_send			   PGNSP PGUID 12 1 0 f f t f i 1 17 "601" _null_ _null_ _null_ lseg_send - _null_ ));
DESCR("I/O");
DATA(insert OID = 2482 (  path_recv			   PGNSP PGUID 12 1 0 f f t f i 1 602 "2281" _null_ _null_ _null_  path_recv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2483 (  path_send			   PGNSP PGUID 12 1 0 f f t f i 1 17 "602" _null_ _null_ _null_ path_send - _null_ ));
DESCR("I/O");
DATA(insert OID = 2484 (  box_recv			   PGNSP PGUID 12 1 0 f f t f i 1 603 "2281" _null_ _null_ _null_  box_recv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2485 (  box_send			   PGNSP PGUID 12 1 0 f f t f i 1 17 "603" _null_ _null_ _null_ box_send - _null_ ));
DESCR("I/O");
DATA(insert OID = 2486 (  poly_recv			   PGNSP PGUID 12 1 0 f f t f i 1 604 "2281" _null_ _null_ _null_  poly_recv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2487 (  poly_send			   PGNSP PGUID 12 1 0 f f t f i 1 17 "604" _null_ _null_ _null_ poly_send - _null_ ));
DESCR("I/O");
DATA(insert OID = 2488 (  line_recv			   PGNSP PGUID 12 1 0 f f t f i 1 628 "2281" _null_ _null_ _null_  line_recv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2489 (  line_send			   PGNSP PGUID 12 1 0 f f t f i 1 17 "628" _null_ _null_ _null_ line_send - _null_ ));
DESCR("I/O");
DATA(insert OID = 2490 (  circle_recv		   PGNSP PGUID 12 1 0 f f t f i 1 718 "2281" _null_ _null_ _null_  circle_recv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2491 (  circle_send		   PGNSP PGUID 12 1 0 f f t f i 1 17 "718" _null_ _null_ _null_ circle_send - _null_ ));
DESCR("I/O");
DATA(insert OID = 2492 (  cash_recv			   PGNSP PGUID 12 1 0 f f t f i 1 790 "2281" _null_ _null_ _null_  cash_recv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2493 (  cash_send			   PGNSP PGUID 12 1 0 f f t f i 1 17 "790" _null_ _null_ _null_ cash_send - _null_ ));
DESCR("I/O");
DATA(insert OID = 2494 (  macaddr_recv		   PGNSP PGUID 12 1 0 f f t f i 1 829 "2281" _null_ _null_ _null_  macaddr_recv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2495 (  macaddr_send		   PGNSP PGUID 12 1 0 f f t f i 1 17 "829" _null_ _null_ _null_ macaddr_send - _null_ ));
DESCR("I/O");
DATA(insert OID = 2496 (  inet_recv			   PGNSP PGUID 12 1 0 f f t f i 1 869 "2281" _null_ _null_ _null_  inet_recv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2497 (  inet_send			   PGNSP PGUID 12 1 0 f f t f i 1 17 "869" _null_ _null_ _null_ inet_send - _null_ ));
DESCR("I/O");
DATA(insert OID = 2498 (  cidr_recv			   PGNSP PGUID 12 1 0 f f t f i 1 650 "2281" _null_ _null_ _null_  cidr_recv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2499 (  cidr_send			   PGNSP PGUID 12 1 0 f f t f i 1 17 "650" _null_ _null_ _null_ cidr_send - _null_ ));
DESCR("I/O");
DATA(insert OID = 2500 (  cstring_recv		   PGNSP PGUID 12 1 0 f f t f s 1 2275 "2281" _null_ _null_ _null_	cstring_recv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2501 (  cstring_send		   PGNSP PGUID 12 1 0 f f t f s 1 17 "2275" _null_ _null_ _null_	cstring_send - _null_ ));
DESCR("I/O");
DATA(insert OID = 2502 (  anyarray_recv		   PGNSP PGUID 12 1 0 f f t f s 1 2277 "2281" _null_ _null_ _null_	anyarray_recv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2503 (  anyarray_send		   PGNSP PGUID 12 1 0 f f t f s 1 17 "2277" _null_ _null_ _null_	anyarray_send - _null_ ));
DESCR("I/O");

/* System-view support functions with pretty-print option */
DATA(insert OID = 2504 (  pg_get_ruledef	   PGNSP PGUID 12 1 0 f f t f s 2 25 "26 16" _null_ _null_ _null_  pg_get_ruledef_ext - _null_ ));
DESCR("source text of a rule with pretty-print option");
DATA(insert OID = 2505 (  pg_get_viewdef	   PGNSP PGUID 12 1 0 f f t f s 2 25 "25 16" _null_ _null_ _null_  pg_get_viewdef_name_ext - _null_ ));
DESCR("select statement of a view with pretty-print option");
DATA(insert OID = 2506 (  pg_get_viewdef	   PGNSP PGUID 12 1 0 f f t f s 2 25 "26 16" _null_ _null_ _null_  pg_get_viewdef_ext - _null_ ));
DESCR("select statement of a view with pretty-print option");
DATA(insert OID = 2507 (  pg_get_indexdef	   PGNSP PGUID 12 1 0 f f t f s 3 25 "26 23 16" _null_ _null_ _null_	pg_get_indexdef_ext - _null_ ));
DESCR("index description (full create statement or single expression) with pretty-print option");
DATA(insert OID = 2508 (  pg_get_constraintdef PGNSP PGUID 12 1 0 f f t f s 2 25 "26 16" _null_ _null_ _null_  pg_get_constraintdef_ext - _null_ ));
DESCR("constraint description with pretty-print option");
DATA(insert OID = 2509 (  pg_get_expr		   PGNSP PGUID 12 1 0 f f t f s 3 25 "25 26 16" _null_ _null_ _null_ pg_get_expr_ext - _null_ ));
DESCR("deparse an encoded expression with pretty-print option");
DATA(insert OID = 2510 (  pg_prepared_statement PGNSP PGUID 12 1 1000 f f t t s 0 2249 "" _null_ _null_ _null_ pg_prepared_statement - _null_ ));
DESCR("get the prepared statements for this session");
DATA(insert OID = 2511 (  pg_cursor PGNSP PGUID 12 1 1000 f f t t s 0 2249 "" _null_ _null_ _null_ pg_cursor - _null_ ));
DESCR("get the open cursors for this session");
DATA(insert OID = 2599 (  pg_timezone_abbrevs	PGNSP PGUID 12 1 1000 f f t t s 0 2249 "" "{25,1186,16}" "{o,o,o}" "{abbrev,utc_offset,is_dst}" pg_timezone_abbrevs - _null_ ));
DESCR("get the available time zone abbreviations");
DATA(insert OID = 2856 (  pg_timezone_names		PGNSP PGUID 12 1 1000 f f t t s 0 2249 "" "{25,25,1186,16}" "{o,o,o,o}" "{name,abbrev,utc_offset,is_dst}" pg_timezone_names - _null_ ));
DESCR("get the available time zone names");

/* non-persistent series generator */
DATA(insert OID = 1066 (  generate_series PGNSP PGUID 12 1 1000 f f t t v 3 23 "23 23 23" _null_ _null_ _null_ generate_series_step_int4 - _null_ ));
DESCR("non-persistent series generator");
DATA(insert OID = 1067 (  generate_series PGNSP PGUID 12 1 1000 f f t t v 2 23 "23 23" _null_ _null_ _null_ generate_series_int4 - _null_ ));
DESCR("non-persistent series generator");
DATA(insert OID = 1068 (  generate_series PGNSP PGUID 12 1 1000 f f t t v 3 20 "20 20 20" _null_ _null_ _null_ generate_series_step_int8 - _null_ ));
DESCR("non-persistent series generator");
DATA(insert OID = 1069 (  generate_series PGNSP PGUID 12 1 1000 f f t t v 2 20 "20 20" _null_ _null_ _null_ generate_series_int8 - _null_ ));
DESCR("non-persistent series generator");

/* boolean aggregates */
DATA(insert OID = 2515 ( booland_statefunc			   PGNSP PGUID 12 1 0 f f t f i 2 16 "16 16" _null_ _null_ _null_ booland_statefunc - _null_ ));
DESCR("boolean-and aggregate transition function");
DATA(insert OID = 2516 ( boolor_statefunc			   PGNSP PGUID 12 1 0 f f t f i 2 16 "16 16" _null_ _null_ _null_ boolor_statefunc - _null_ ));
DESCR("boolean-or aggregate transition function");
DATA(insert OID = 2517 ( bool_and					   PGNSP PGUID 12 1 0 t f f f i 1 16 "16" _null_ _null_ _null_ aggregate_dummy - _null_ ));
DESCR("boolean-and aggregate");
/* ANY, SOME? These names conflict with subquery operators. See doc. */
DATA(insert OID = 2518 ( bool_or					   PGNSP PGUID 12 1 0 t f f f i 1 16 "16" _null_ _null_ _null_ aggregate_dummy - _null_ ));
DESCR("boolean-or aggregate");
DATA(insert OID = 2519 ( every						   PGNSP PGUID 12 1 0 t f f f i 1 16 "16" _null_ _null_ _null_ aggregate_dummy - _null_ ));
DESCR("boolean-and aggregate");

/* bitwise integer aggregates */
DATA(insert OID = 2236 ( bit_and					   PGNSP PGUID 12 1 0 t f f f i 1 21 "21" _null_ _null_ _null_ aggregate_dummy - _null_));
DESCR("bitwise-and smallint aggregate");
DATA(insert OID = 2237 ( bit_or						   PGNSP PGUID 12 1 0 t f f f i 1 21 "21" _null_ _null_ _null_ aggregate_dummy - _null_));
DESCR("bitwise-or smallint aggregate");
DATA(insert OID = 2238 ( bit_and					   PGNSP PGUID 12 1 0 t f f f i 1 23 "23" _null_ _null_ _null_ aggregate_dummy - _null_));
DESCR("bitwise-and integer aggregate");
DATA(insert OID = 2239 ( bit_or						   PGNSP PGUID 12 1 0 t f f f i 1 23 "23" _null_ _null_ _null_ aggregate_dummy - _null_));
DESCR("bitwise-or integer aggregate");
DATA(insert OID = 2240 ( bit_and					   PGNSP PGUID 12 1 0 t f f f i 1 20 "20" _null_ _null_ _null_ aggregate_dummy - _null_));
DESCR("bitwise-and bigint aggregate");
DATA(insert OID = 2241 ( bit_or						   PGNSP PGUID 12 1 0 t f f f i 1 20 "20" _null_ _null_ _null_ aggregate_dummy - _null_));
DESCR("bitwise-or bigint aggregate");
DATA(insert OID = 2242 ( bit_and					   PGNSP PGUID 12 1 0 t f f f i 1 1560 "1560" _null_ _null_ _null_ aggregate_dummy - _null_));
DESCR("bitwise-and bit aggregate");
DATA(insert OID = 2243 ( bit_or						   PGNSP PGUID 12 1 0 t f f f i 1 1560 "1560" _null_ _null_ _null_ aggregate_dummy - _null_));
DESCR("bitwise-or bit aggregate");

/* formerly-missing interval + datetime operators */
DATA(insert OID = 2546 ( interval_pl_date			PGNSP PGUID 14 1 0 f f t f i 2 1114 "1186 1082" _null_ _null_ _null_	"select $2 + $1" - _null_ ));
DATA(insert OID = 2547 ( interval_pl_timetz			PGNSP PGUID 14 1 0 f f t f i 2 1266 "1186 1266" _null_ _null_ _null_	"select $2 + $1" - _null_ ));
DATA(insert OID = 2548 ( interval_pl_timestamp		PGNSP PGUID 14 1 0 f f t f i 2 1114 "1186 1114" _null_ _null_ _null_	"select $2 + $1" - _null_ ));
DATA(insert OID = 2549 ( interval_pl_timestamptz	PGNSP PGUID 14 1 0 f f t f s 2 1184 "1186 1184" _null_ _null_ _null_	"select $2 + $1" - _null_ ));
DATA(insert OID = 2550 ( integer_pl_date			PGNSP PGUID 14 1 0 f f t f i 2 1082 "23 1082" _null_ _null_ _null_	"select $2 + $1" - _null_ ));

DATA(insert OID = 2556 ( pg_tablespace_databases	PGNSP PGUID 12 1 1000 f f t t s 1 26 "26" _null_ _null_ _null_ pg_tablespace_databases - _null_));
DESCR("returns database oids in a tablespace");

DATA(insert OID = 2557 ( bool				   PGNSP PGUID 12 1 0 f f t f i 1  16 "23" _null_ _null_ _null_ int4_bool - _null_ ));
DESCR("convert int4 to boolean");
DATA(insert OID = 2558 ( int4				   PGNSP PGUID 12 1 0 f f t f i 1  23 "16" _null_ _null_ _null_ bool_int4 - _null_ ));
DESCR("convert boolean to int4");
DATA(insert OID = 2559 ( lastval			   PGNSP PGUID 12 1 0 f f t f v 0 20 "" _null_ _null_ _null_	lastval - _null_ ));
DESCR("current value from last used sequence");

/* start time function */
DATA(insert OID = 2560 (  pg_postmaster_start_time PGNSP PGUID 12 1 0 f f t f s 0 1184 "" _null_ _null_ _null_ pgsql_postmaster_start_time - _null_ ));
DESCR("postmaster start time");

/* new functions for Y-direction rtree opclasses */
DATA(insert OID = 2562 (  box_below		   PGNSP PGUID 12 1 0 f f t f i 2 16 "603 603" _null_ _null_ _null_ box_below - _null_ ));
DESCR("is below");
DATA(insert OID = 2563 (  box_overbelow    PGNSP PGUID 12 1 0 f f t f i 2 16 "603 603" _null_ _null_ _null_ box_overbelow - _null_ ));
DESCR("overlaps or is below");
DATA(insert OID = 2564 (  box_overabove    PGNSP PGUID 12 1 0 f f t f i 2 16 "603 603" _null_ _null_ _null_ box_overabove - _null_ ));
DESCR("overlaps or is above");
DATA(insert OID = 2565 (  box_above		   PGNSP PGUID 12 1 0 f f t f i 2 16 "603 603" _null_ _null_ _null_ box_above - _null_ ));
DESCR("is above");
DATA(insert OID = 2566 (  poly_below	   PGNSP PGUID 12 1 0 f f t f i 2 16 "604 604" _null_ _null_ _null_ poly_below - _null_ ));
DESCR("is below");
DATA(insert OID = 2567 (  poly_overbelow   PGNSP PGUID 12 1 0 f f t f i 2 16 "604 604" _null_ _null_ _null_ poly_overbelow - _null_ ));
DESCR("overlaps or is below");
DATA(insert OID = 2568 (  poly_overabove   PGNSP PGUID 12 1 0 f f t f i 2 16 "604 604" _null_ _null_ _null_ poly_overabove - _null_ ));
DESCR("overlaps or is above");
DATA(insert OID = 2569 (  poly_above	   PGNSP PGUID 12 1 0 f f t f i 2 16 "604 604" _null_ _null_ _null_ poly_above - _null_ ));
DESCR("is above");
DATA(insert OID = 2587 (  circle_overbelow		PGNSP PGUID 12 1 0 f f t f i 2	16 "718 718" _null_ _null_ _null_  circle_overbelow - _null_ ));
DESCR("overlaps or is below");
DATA(insert OID = 2588 (  circle_overabove		PGNSP PGUID 12 1 0 f f t f i 2	16 "718 718" _null_ _null_ _null_  circle_overabove - _null_ ));
DESCR("overlaps or is above");

/* support functions for GiST r-tree emulation */
DATA(insert OID = 2578 (  gist_box_consistent	PGNSP PGUID 12 1 0 f f t f i 3 16 "2281 603 23" _null_ _null_ _null_	gist_box_consistent - _null_ ));
DESCR("GiST support");
DATA(insert OID = 2579 (  gist_box_compress		PGNSP PGUID 12 1 0 f f t f i 1 2281 "2281" _null_ _null_ _null_ gist_box_compress - _null_ ));
DESCR("GiST support");
DATA(insert OID = 2580 (  gist_box_decompress	PGNSP PGUID 12 1 0 f f t f i 1 2281 "2281" _null_ _null_ _null_ gist_box_decompress - _null_ ));
DESCR("GiST support");
DATA(insert OID = 2581 (  gist_box_penalty		PGNSP PGUID 12 1 0 f f t f i 3 2281 "2281 2281 2281" _null_ _null_ _null_	gist_box_penalty - _null_ ));
DESCR("GiST support");
DATA(insert OID = 2582 (  gist_box_picksplit	PGNSP PGUID 12 1 0 f f t f i 2 2281 "2281 2281" _null_ _null_ _null_	gist_box_picksplit - _null_ ));
DESCR("GiST support");
DATA(insert OID = 2583 (  gist_box_union		PGNSP PGUID 12 1 0 f f t f i 2 603 "2281 2281" _null_ _null_ _null_ gist_box_union - _null_ ));
DESCR("GiST support");
DATA(insert OID = 2584 (  gist_box_same			PGNSP PGUID 12 1 0 f f t f i 3 2281 "603 603 2281" _null_ _null_ _null_ gist_box_same - _null_ ));
DESCR("GiST support");
DATA(insert OID = 2585 (  gist_poly_consistent	PGNSP PGUID 12 1 0 f f t f i 3 16 "2281 604 23" _null_ _null_ _null_	gist_poly_consistent - _null_ ));
DESCR("GiST support");
DATA(insert OID = 2586 (  gist_poly_compress	PGNSP PGUID 12 1 0 f f t f i 1 2281 "2281" _null_ _null_ _null_ gist_poly_compress - _null_ ));
DESCR("GiST support");
DATA(insert OID = 2591 (  gist_circle_consistent PGNSP PGUID 12 1 0 f f t f i 3 16 "2281 718 23" _null_ _null_ _null_	gist_circle_consistent - _null_ ));
DESCR("GiST support");
DATA(insert OID = 2592 (  gist_circle_compress	PGNSP PGUID 12 1 0 f f t f i 1 2281 "2281" _null_ _null_ _null_ gist_circle_compress - _null_ ));
DESCR("GiST support");

/* GIN */
DATA(insert OID = 2730 (  gingettuple	   PGNSP PGUID 12 1 0 f f t f v 2 16 "2281 2281" _null_ _null_ _null_  gingettuple - _null_ ));
DESCR("gin(internal)");
DATA(insert OID = 2731 (  gingetmulti	   PGNSP PGUID 12 1 0 f f t f v 4 16 "2281 2281 2281 2281" _null_ _null_ _null_  gingetmulti - _null_ ));
DESCR("gin(internal)");
DATA(insert OID = 2732 (  gininsert		   PGNSP PGUID 12 1 0 f f t f v 6 16 "2281 2281 2281 2281 2281 2281" _null_ _null_ _null_	gininsert - _null_ ));
DESCR("gin(internal)");
DATA(insert OID = 2733 (  ginbeginscan	   PGNSP PGUID 12 1 0 f f t f v 3 2281 "2281 2281 2281" _null_ _null_ _null_	ginbeginscan - _null_ ));
DESCR("gin(internal)");
DATA(insert OID = 2734 (  ginrescan		   PGNSP PGUID 12 1 0 f f t f v 2 2278 "2281 2281" _null_ _null_ _null_ ginrescan - _null_ ));
DESCR("gin(internal)");
DATA(insert OID = 2735 (  ginendscan	   PGNSP PGUID 12 1 0 f f t f v 1 2278 "2281" _null_ _null_ _null_	ginendscan - _null_ ));
DESCR("gin(internal)");
DATA(insert OID = 2736 (  ginmarkpos	   PGNSP PGUID 12 1 0 f f t f v 1 2278 "2281" _null_ _null_ _null_	ginmarkpos - _null_ ));
DESCR("gin(internal)");
DATA(insert OID = 2737 (  ginrestrpos	   PGNSP PGUID 12 1 0 f f t f v 1 2278 "2281" _null_ _null_ _null_	ginrestrpos - _null_ ));
DESCR("gin(internal)");
DATA(insert OID = 2738 (  ginbuild		   PGNSP PGUID 12 1 0 f f t f v 3 2281 "2281 2281 2281" _null_ _null_ _null_ ginbuild - _null_ ));
DESCR("gin(internal)");
DATA(insert OID = 2739 (  ginbulkdelete    PGNSP PGUID 12 1 0 f f t f v 4 2281 "2281 2281 2281 2281" _null_ _null_ _null_ ginbulkdelete - _null_ ));
DESCR("gin(internal)");
DATA(insert OID = 2740 (  ginvacuumcleanup PGNSP PGUID 12 1 0 f f t f v 2 2281 "2281 2281" _null_ _null_ _null_ ginvacuumcleanup - _null_ ));
DESCR("gin(internal)");
DATA(insert OID = 2741 (  gincostestimate  PGNSP PGUID 12 1 0 f f t f v 8 2278 "2281 2281 2281 2281 2281 2281 2281 2281" _null_ _null_ _null_  gincostestimate - _null_ ));
DESCR("gin(internal)");
DATA(insert OID = 2788 (  ginoptions	   PGNSP PGUID 12 1 0 f f t f s 2 17 "1009 16" _null_ _null_ _null_  ginoptions - _null_ ));
DESCR("gin(internal)");

/* GIN array support */
DATA(insert OID = 2743 (  ginarrayextract	 PGNSP PGUID 12 1 0 f f t f i 2 2281 "2277 2281" _null_ _null_ _null_	ginarrayextract - _null_ ));
DESCR("GIN array support");
DATA(insert OID = 2744 (  ginarrayconsistent PGNSP PGUID 12 1 0 f f t f i 3 16 "2281 21 2281" _null_ _null_ _null_	ginarrayconsistent - _null_ ));
DESCR("GIN array support");

/* overlap/contains/contained */
DATA(insert OID = 2747 (  arrayoverlap		   PGNSP PGUID 12 1 0 f f t f i 2 16 "2277 2277" _null_ _null_ _null_ arrayoverlap - _null_ ));
DESCR("overlaps");
DATA(insert OID = 2748 (  arraycontains		   PGNSP PGUID 12 1 0 f f t f i 2 16 "2277 2277" _null_ _null_ _null_ arraycontains - _null_ ));
DESCR("contains");
DATA(insert OID = 2749 (  arraycontained	   PGNSP PGUID 12 1 0 f f t f i 2 16 "2277 2277" _null_ _null_ _null_ arraycontained - _null_ ));
DESCR("is contained by");

/* userlock replacements */
DATA(insert OID = 2880 (  pg_advisory_lock				PGNSP PGUID 12 1 0 f f t f v 1 2278 "20" _null_ _null_ _null_ pg_advisory_lock_int8 - _null_ ));
DESCR("obtain exclusive advisory lock");
DATA(insert OID = 2881 (  pg_advisory_lock_shared		PGNSP PGUID 12 1 0 f f t f v 1 2278 "20" _null_ _null_ _null_ pg_advisory_lock_shared_int8 - _null_ ));
DESCR("obtain shared advisory lock");
DATA(insert OID = 2882 (  pg_try_advisory_lock			PGNSP PGUID 12 1 0 f f t f v 1 16 "20" _null_ _null_ _null_ pg_try_advisory_lock_int8 - _null_ ));
DESCR("obtain exclusive advisory lock if available");
DATA(insert OID = 2883 (  pg_try_advisory_lock_shared	PGNSP PGUID 12 1 0 f f t f v 1 16 "20" _null_ _null_ _null_ pg_try_advisory_lock_shared_int8 - _null_ ));
DESCR("obtain shared advisory lock if available");
DATA(insert OID = 2884 (  pg_advisory_unlock			PGNSP PGUID 12 1 0 f f t f v 1 16 "20" _null_ _null_ _null_ pg_advisory_unlock_int8 - _null_ ));
DESCR("release exclusive advisory lock");
DATA(insert OID = 2885 (  pg_advisory_unlock_shared		PGNSP PGUID 12 1 0 f f t f v 1 16 "20" _null_ _null_ _null_ pg_advisory_unlock_shared_int8 - _null_ ));
DESCR("release shared advisory lock");
DATA(insert OID = 2886 (  pg_advisory_lock				PGNSP PGUID 12 1 0 f f t f v 2 2278 "23 23" _null_ _null_ _null_ pg_advisory_lock_int4 - _null_ ));
DESCR("obtain exclusive advisory lock");
DATA(insert OID = 2887 (  pg_advisory_lock_shared		PGNSP PGUID 12 1 0 f f t f v 2 2278 "23 23" _null_ _null_ _null_ pg_advisory_lock_shared_int4 - _null_ ));
DESCR("obtain shared advisory lock");
DATA(insert OID = 2888 (  pg_try_advisory_lock			PGNSP PGUID 12 1 0 f f t f v 2 16 "23 23" _null_ _null_ _null_ pg_try_advisory_lock_int4 - _null_ ));
DESCR("obtain exclusive advisory lock if available");
DATA(insert OID = 2889 (  pg_try_advisory_lock_shared	PGNSP PGUID 12 1 0 f f t f v 2 16 "23 23" _null_ _null_ _null_ pg_try_advisory_lock_shared_int4 - _null_ ));
DESCR("obtain shared advisory lock if available");
DATA(insert OID = 2890 (  pg_advisory_unlock			PGNSP PGUID 12 1 0 f f t f v 2 16 "23 23" _null_ _null_ _null_ pg_advisory_unlock_int4 - _null_ ));
DESCR("release exclusive advisory lock");
DATA(insert OID = 2891 (  pg_advisory_unlock_shared		PGNSP PGUID 12 1 0 f f t f v 2 16 "23 23" _null_ _null_ _null_ pg_advisory_unlock_shared_int4 - _null_ ));
DESCR("release shared advisory lock");
DATA(insert OID = 2892 (  pg_advisory_unlock_all		PGNSP PGUID 12 1 0 f f t f v 0 2278 "" _null_ _null_ _null_ pg_advisory_unlock_all - _null_ ));
DESCR("release all advisory locks");

/* XML support */
DATA(insert OID = 2893 (  xml_in		   PGNSP PGUID 12 1 0 f f t f i 1 142 "2275" _null_ _null_ _null_ xml_in - _null_ ));
DESCR("I/O");
DATA(insert OID = 2894 (  xml_out		   PGNSP PGUID 12 1 0 f f t f i 1 2275 "142" _null_ _null_ _null_ xml_out - _null_ ));
DESCR("I/O");
DATA(insert OID = 2895 (  xmlcomment	   PGNSP PGUID 12 1 0 f f t f i 1 142 "25" _null_ _null_ _null_ xmlcomment - _null_ ));
DESCR("generate an XML comment");
DATA(insert OID = 2896 (  xml			   PGNSP PGUID 12 1 0 f f t f i 1 142 "25" _null_ _null_ _null_ texttoxml - _null_ ));
DESCR("perform a non-validating parse of a character string to produce an XML value");
DATA(insert OID = 2897 (  xmlvalidate 	   PGNSP PGUID 12 1 0 f f t f i 2 16 "142 25" _null_ _null_ _null_ xmlvalidate - _null_ ));
DESCR("validate an XML value");
DATA(insert OID = 2898 (  xml_recv		   PGNSP PGUID 12 1 0 f f t f s 1 142 "2281" _null_ _null_ _null_	xml_recv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2899 (  xml_send		   PGNSP PGUID 12 1 0 f f t f s 1 17 "142" _null_ _null_ _null_	xml_send - _null_ ));
DESCR("I/O");
DATA(insert OID = 2900 (  xmlconcat2       PGNSP PGUID 12 1 0 f f f f i 2 142 "142 142" _null_ _null_ _null_ xmlconcat2 - _null_ ));
DESCR("aggregate transition function");
DATA(insert OID = 2901 (  xmlagg           PGNSP PGUID 12 1 0 t f f f i 1 142 "142" _null_ _null_ _null_ aggregate_dummy - _null_ ));
DESCR("concatenate XML values");
DATA(insert OID = 2922 (  text             PGNSP PGUID 12 1 0 f f t f s 1 25 "142" _null_ _null_ _null_ xmltotext - _null_ ));
DESCR("serialize an XML value to a character string");

DATA(insert OID = 2923 (  table_to_xml                PGNSP PGUID 12 100 0 f f t f s 4 142 "2205 16 16 25" _null_ _null_ "{tbl,nulls,tableforest,targetns}" table_to_xml - _null_ ));
DESCR("map table contents to XML");
DATA(insert OID = 2924 (  query_to_xml                PGNSP PGUID 12 100 0 f f t f s 4 142 "25 16 16 25" _null_ _null_ "{query,nulls,tableforest,targetns}" query_to_xml - _null_ ));
DESCR("map query result to XML");
DATA(insert OID = 2925 (  cursor_to_xml               PGNSP PGUID 12 100 0 f f t f s 5 142 "1790 23 16 16 25" _null_ _null_ "{cursor,count,nulls,tableforest,targetns}" cursor_to_xml - _null_ ));
DESCR("map rows from cursor to XML");
DATA(insert OID = 2926 (  table_to_xmlschema          PGNSP PGUID 12 100 0 f f t f s 4 142 "2205 16 16 25" _null_ _null_ "{tbl,nulls,tableforest,targetns}" table_to_xmlschema - _null_ ));
DESCR("map table structure to XML Schema");
DATA(insert OID = 2927 (  query_to_xmlschema          PGNSP PGUID 12 100 0 f f t f s 4 142 "25 16 16 25" _null_ _null_ "{query,nulls,tableforest,targetns}" query_to_xmlschema - _null_ ));
DESCR("map query result structure to XML Schema");
DATA(insert OID = 2928 (  cursor_to_xmlschema         PGNSP PGUID 12 100 0 f f t f s 4 142 "1790 16 16 25" _null_ _null_ "{cursor,nulls,tableforest,targetns}" cursor_to_xmlschema - _null_ ));
DESCR("map cursor structure to XML Schema");
DATA(insert OID = 2929 (  table_to_xml_and_xmlschema  PGNSP PGUID 12 100 0 f f t f s 4 142 "2205 16 16 25" _null_ _null_ "{tbl,nulls,tableforest,targetns}" table_to_xml_and_xmlschema - _null_ ));
DESCR("map table contents and structure to XML and XML Schema");
DATA(insert OID = 2930 (  query_to_xml_and_xmlschema  PGNSP PGUID 12 100 0 f f t f s 4 142 "25 16 16 25" _null_ _null_ "{query,nulls,tableforest,targetns}" query_to_xml_and_xmlschema - _null_ ));
DESCR("map query result and structure to XML and XML Schema");

DATA(insert OID = 2933 (  schema_to_xml               PGNSP PGUID 12 100 0 f f t f s 4 142 "19 16 16 25" _null_ _null_ "{schema,nulls,tableforest,targetns}" schema_to_xml - _null_ ));
DESCR("map schema contents to XML");
DATA(insert OID = 2934 (  schema_to_xmlschema         PGNSP PGUID 12 100 0 f f t f s 4 142 "19 16 16 25" _null_ _null_ "{schema,nulls,tableforest,targetns}" schema_to_xmlschema - _null_ ));
DESCR("map schema structure to XML Schema");
DATA(insert OID = 2935 (  schema_to_xml_and_xmlschema PGNSP PGUID 12 100 0 f f t f s 4 142 "19 16 16 25" _null_ _null_ "{schema,nulls,tableforest,targetns}" schema_to_xml_and_xmlschema - _null_ ));
DESCR("map schema contents and structure to XML and XML Schema");

DATA(insert OID = 2936 (  database_to_xml             PGNSP PGUID 12 100 0 f f t f s 3 142 "16 16 25" _null_ _null_ "{nulls,tableforest,targetns}" database_to_xml - _null_ ));
DESCR("map database contents to XML");
DATA(insert OID = 2937 (  database_to_xmlschema       PGNSP PGUID 12 100 0 f f t f s 3 142 "16 16 25" _null_ _null_ "{nulls,tableforest,targetns}" database_to_xmlschema - _null_ ));
DESCR("map database structure to XML Schema");
DATA(insert OID = 2938 (  database_to_xml_and_xmlschema PGNSP PGUID 12 100 0 f f t f s 3 142 "16 16 25" _null_ _null_ "{nulls,tableforest,targetns}" database_to_xml_and_xmlschema - _null_ ));
DESCR("map database contents and structure to XML and XML Schema");

DATA(insert OID = 2931 (  xpath      PGNSP PGUID 12 1 0 f f t f i 3 143 "25 142 1009" _null_ _null_ _null_ xpath - _null_ ));
DESCR("evaluate XPath expression, with namespaces support");
DATA(insert OID = 2932 (  xpath      PGNSP PGUID 14 1 0 f f t f i 2 143 "25 142" _null_ _null_ _null_ "select pg_catalog.xpath($1, $2, ''{}''::pg_catalog.text[])" - _null_ ));
DESCR("evaluate XPath expression");

/* uuid */ 
DATA(insert OID = 2952 (  uuid_in		   PGNSP PGUID 12 1 0 f f t f i 1 2950 "2275" _null_ _null_ _null_ uuid_in - _null_ ));
DESCR("I/O");
DATA(insert OID = 2953 (  uuid_out		   PGNSP PGUID 12 1 0 f f t f i 1 2275 "2950" _null_ _null_ _null_ uuid_out - _null_ ));
DESCR("I/O");
DATA(insert OID = 2954 (  uuid_lt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "2950 2950" _null_ _null_ _null_ uuid_lt - _null_ ));
DESCR("less-than");
DATA(insert OID = 2955 (  uuid_le		   PGNSP PGUID 12 1 0 f f t f i 2 16 "2950 2950" _null_ _null_ _null_ uuid_le - _null_ ));
DESCR("less-than-or-equal");
DATA(insert OID = 2956 (  uuid_eq		   PGNSP PGUID 12 1 0 f f t f i 2 16 "2950 2950" _null_ _null_ _null_ uuid_eq - _null_ ));
DESCR("equal");
DATA(insert OID = 2957 (  uuid_ge		   PGNSP PGUID 12 1 0 f f t f i 2 16 "2950 2950" _null_ _null_ _null_ uuid_ge - _null_ ));
DESCR("greater-than-or-equal");
DATA(insert OID = 2958 (  uuid_gt		   PGNSP PGUID 12 1 0 f f t f i 2 16 "2950 2950" _null_ _null_ _null_ uuid_gt - _null_ ));
DESCR("greater-than");
DATA(insert OID = 2959 (  uuid_ne		   PGNSP PGUID 12 1 0 f f t f i 2 16 "2950 2950" _null_ _null_ _null_ uuid_ne - _null_ ));
DESCR("not-equal");
DATA(insert OID = 2960 (  uuid_cmp		   PGNSP PGUID 12 1 0 f f t f i 2 23 "2950 2950" _null_ _null_ _null_ uuid_cmp - _null_ ));
DESCR("btree less-equal-greater");
DATA(insert OID = 2961 (  uuid_recv		   PGNSP PGUID 12 1 0 f f t f i 1 2950 "2281" _null_ _null_ _null_ uuid_recv - _null_ ));
DESCR("I/O");
DATA(insert OID = 2962 (  uuid_send		   PGNSP PGUID 12 1 0 f f t f i 1 17 "2950" _null_ _null_ _null_ uuid_send - _null_ ));
DESCR("I/O");
DATA(insert OID = 2963 (  uuid_hash		   PGNSP PGUID 12 1 0 f f t f i 1 23 "2950" _null_ _null_ _null_ uuid_hash - _null_ ));
DESCR("hash");

/* enum related procs */
DATA(insert OID = 3504 (  anyenum_in	PGNSP PGUID 12 1 0 f f t f i 1 3500 "2275" _null_ _null_ _null_ anyenum_in - _null_ ));
DESCR("I/O");
DATA(insert OID = 3505 (  anyenum_out	PGNSP PGUID 12 1 0 f f t f s 1 2275 "3500" _null_ _null_ _null_ anyenum_out - _null_ ));
DESCR("I/O");
DATA(insert OID = 3506 (  enum_in		PGNSP PGUID 12 1 0 f f t f s 2 3500 "2275 26" _null_ _null_ _null_ enum_in - _null_ ));
DESCR("I/O");
DATA(insert OID = 3507 (  enum_out		PGNSP PGUID 12 1 0 f f t f s 1 2275 "3500" _null_ _null_ _null_ enum_out - _null_ ));
DESCR("I/O");
DATA(insert OID = 3508 (  enum_eq		PGNSP PGUID 12 1 0 f f t f i 2 16 "3500 3500" _null_ _null_ _null_ enum_eq - _null_ ));
DESCR("equal");
DATA(insert OID = 3509 (  enum_ne		PGNSP PGUID 12 1 0 f f t f i 2 16 "3500 3500" _null_ _null_ _null_ enum_ne - _null_ ));
DESCR("not equal");
DATA(insert OID = 3510 (  enum_lt		PGNSP PGUID 12 1 0 f f t f i 2 16 "3500 3500" _null_ _null_ _null_ enum_lt - _null_ ));
DESCR("less-than");
DATA(insert OID = 3511 (  enum_gt		PGNSP PGUID 12 1 0 f f t f i 2 16 "3500 3500" _null_ _null_ _null_ enum_gt - _null_ ));
DESCR("greater-than");
DATA(insert OID = 3512 (  enum_le		PGNSP PGUID 12 1 0 f f t f i 2 16 "3500 3500" _null_ _null_ _null_ enum_le - _null_ ));
DESCR("less-than-or-equal");
DATA(insert OID = 3513 (  enum_ge		PGNSP PGUID 12 1 0 f f t f i 2 16 "3500 3500" _null_ _null_ _null_ enum_ge - _null_ ));
DESCR("greater-than-or-equal");
DATA(insert OID = 3514 (  enum_cmp		PGNSP PGUID 12 1 0 f f t f i 2 23 "3500 3500" _null_ _null_ _null_ enum_cmp - _null_ ));
DESCR("btree-less-equal-greater");
DATA(insert OID = 3515 (  hashenum		PGNSP PGUID 12 1 0 f f t f i 1 23 "3500" _null_ _null_ _null_ hashenum - _null_ ));
DESCR("hash");
DATA(insert OID = 3524 (  enum_smaller	PGNSP PGUID 12 1 0 f f t f i 2 3500 "3500 3500" _null_ _null_ _null_ enum_smaller - _null_ ));
DESCR("smaller of two");
DATA(insert OID = 3525 (  enum_larger	PGNSP PGUID 12 1 0 f f t f i 2 3500 "3500 3500" _null_ _null_ _null_ enum_larger - _null_ ));
DESCR("larger of two");
DATA(insert OID = 3526 (  max			PGNSP PGUID 12 1 0 t f f f i 1 3500 "3500" _null_ _null_ _null_ aggregate_dummy - _null_ ));
DATA(insert OID = 3527 (  min			PGNSP PGUID 12 1 0 t f f f i 1 3500 "3500" _null_ _null_ _null_ aggregate_dummy - _null_ ));
DATA(insert OID = 3528 (  enum_first	PGNSP PGUID 12 1 0 f f f f s 1 3500 "3500" _null_ _null_ _null_ enum_first - _null_ ));
DATA(insert OID = 3529 (  enum_last		PGNSP PGUID 12 1 0 f f f f s 1 3500 "3500" _null_ _null_ _null_ enum_last - _null_ ));
DATA(insert OID = 3530 (  enum_range	PGNSP PGUID 12 1 0 f f f f s 2 2277 "3500 3500" _null_ _null_ _null_ enum_range_bounds - _null_ ));
DATA(insert OID = 3531 (  enum_range	PGNSP PGUID 12 1 0 f f f f s 1 2277 "3500" _null_ _null_ _null_ enum_range_all - _null_ ));

/*
 * Symbolic values for provolatile column: these indicate whether the result
 * of a function is dependent *only* on the values of its explicit arguments,
 * or can change due to outside factors (such as parameter variables or
 * table contents).  NOTE: functions having side-effects, such as setval(),
 * must be labeled volatile to ensure they will not get optimized away,
 * even if the actual return value is not changeable.
 */
#define PROVOLATILE_IMMUTABLE	'i'		/* never changes for given input */
#define PROVOLATILE_STABLE		's'		/* does not change within a scan */
#define PROVOLATILE_VOLATILE	'v'		/* can change even within a scan */

/*
 * Symbolic values for proargmodes column.	Note that these must agree with
 * the FunctionParameterMode enum in parsenodes.h; we declare them here to
 * be accessible from either header.
 */
#define PROARGMODE_IN		'i'
#define PROARGMODE_OUT		'o'
#define PROARGMODE_INOUT	'b'


/*
 * prototypes for functions in pg_proc.c
 */
extern Oid ProcedureCreate(const char *procedureName,
				Oid procNamespace,
				bool replace,
				bool returnsSet,
				Oid returnType,
				Oid languageObjectId,
				Oid languageValidator,
				const char *prosrc,
				const char *probin,
				bool isAgg,
				bool security_definer,
				bool isStrict,
				char volatility,
				oidvector *parameterTypes,
				Datum allParameterTypes,
				Datum parameterModes,
				Datum parameterNames,
				float4 procost,
				float4 prorows);

extern bool function_parse_error_transpose(const char *prosrc);

#endif   /* PG_PROC_H */
