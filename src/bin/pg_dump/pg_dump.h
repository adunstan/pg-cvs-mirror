/*-------------------------------------------------------------------------
 *
 * pg_dump.h
 *	  header file for the pg_dump utility
 *
 * Portions Copyright (c) 1996-2000, PostgreSQL, Inc
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $Id: pg_dump.h,v 1.51 2000/08/07 12:32:54 pjw Exp $
 *
 * Modifications - 6/12/96 - dave@bensoft.com - version 1.13.dhb.2
 *
 *	 - Fixed dumpTable output to output lengths for char and varchar types!
 *	 - Added single. quote to twin single quote expansion for 'insert' string
 *	   mode.
 *
 * Modifications - 6/1/97 - igor@sba.miami.edu
 * - Added extern's for the functions that clear allocated memory
 *	 in pg_dump.c
 *-------------------------------------------------------------------------
 */

#ifndef PG_DUMP_H
#define PG_DUMP_H

#include "pqexpbuffer.h"
#include "catalog/pg_index.h"
#include "pg_backup.h"

/* The data structures used to store system catalog information */

typedef struct _typeInfo
{
	char	   *oid;
	char	   *typowner;
	char	   *typname;
	char	   *typlen;
	char	   *typprtlen;
	char	   *typinput;
	char	   *typoutput;
	char	   *typreceive;
	char	   *typsend;
	char	   *typelem;
	char	   *typdelim;
	char	   *typdefault;
	char	   *typrelid;
	char	   *usename;
	int			passedbyvalue;
	int			isArray;
} TypeInfo;

typedef struct _funcInfo
{
	char	   *oid;
	char	   *proname;
	char	   *proowner;
	int			lang;
	int			nargs;
	char	   *argtypes[FUNC_MAX_ARGS];
	char	   *prorettype;
	int			retset;			/* 1 if the function returns a set, else 0 */
	char	   *prosrc;
	char	   *probin;
	char	   *usename;
	int			iscachable;		/* Attr */
	int			dumped;			/* 1 if already dumped */
} FuncInfo;

typedef struct _trigInfo
{
	char	   *oid;
	char	   *tgname;
	char	   *tgsrc;
	char	   *tgdel;
	char	   *tgcomment;
} TrigInfo;

typedef struct _tableInfo
{
	char	   *oid;
	char	   *relname;
	char	   *relacl;
	bool		sequence;
	int			numatts;		/* number of attributes */
	int		   *inhAttrs;		/* an array of flags, one for each
								 * attribute if the value is 1, then this
								 * attribute is an inherited attribute */
	char	  **attnames;		/* the attribute names */
	char	  **attoids;		/* oids of the various attributes */
	char	  **typnames;		/* fill out attributes */
	bool	   *notnull;		/* Not null constraints of an attribute */
	char	  **adef_expr;		/* DEFAULT expressions */
	int			numParents;		/* number of (immediate) parent
								 * supertables */
	char	  **parentRels;		/* names of parent relations, NULL if
								 * numParents == 0 */
	char	  **out_attnames;	/* the attribute names, in the order they
								 * would be in, when the table is created
								 * in the target query language. this is
								 * needed because the SQL tables will not
								 * have the same order of attributes as
								 * the POSTQUEL tables */
	int		   *atttypmod;		/* type-specific type modifier */
	char	   *usename;
	int			ncheck;			/* # of CHECK expressions */
	char	  **check_expr;		/* [CONSTRAINT name] CHECK expressions */
	int			ntrig;			/* # of triggers */
	TrigInfo	*triggers;		/* Triggers on the table */
	char	   *primary_key;	/* PRIMARY KEY of the table, if any */
} TableInfo;

typedef struct _inhInfo
{
	char	   *inhrelid;
	char	   *inhparent;
} InhInfo;

typedef struct _indInfo
{
	char	   *indoid;			/* oid of the pg_class entry for the index */
	char	   *indexrelname;	/* name of the secondary index class */
	char	   *indrelname;		/* name of the indexed heap class */
	char	   *indamname;		/* name of the access method (e.g. btree,
								 * rtree, etc.) */
	char	   *indproc;		/* oid of the function to compute the
								 * index, 0 if none */
	char	   *indkey[INDEX_MAX_KEYS]; /* attribute numbers of the key
										 * attributes */
	char	   *indclass[INDEX_MAX_KEYS];		/* opclass of the keys */
	char	   *indisunique;	/* is this index unique? */
} IndInfo;

typedef struct _aggInfo
{
	char	   *oid;
	char	   *aggname;
	char	   *aggtransfn;
	char	   *aggfinalfn;
	char	   *aggtranstype;
	char	   *aggbasetype;
	char	   *agginitval;
	char	   *usename;
} AggInfo;

typedef struct _oprInfo
{
	char	   *oid;
	char	   *oprname;
	char	   *oprkind;		/* "b" = binary, "l" = left unary, "r" =
								 * right unary */
	char	   *oprcode;		/* operator function name */
	char	   *oprleft;		/* left operand type */
	char	   *oprright;		/* right operand type */
	char	   *oprcom;			/* oid of the commutator operator */
	char	   *oprnegate;		/* oid of the negator operator */
	char	   *oprrest;		/* name of the function to calculate
								 * operator restriction selectivity */
	char	   *oprjoin;		/* name of the function to calculate
								 * operator join selectivity */
	char	   *oprcanhash;		/* can we use hash join strategy ? */
	char	   *oprlsortop;		/* oid's of the left and right sort
								 * operators */
	char	   *oprrsortop;
	char	   *usename;
} OprInfo;

/* global decls */
extern bool g_force_quotes;		/* double-quotes for identifiers flag */
extern bool g_verbose;			/* verbose flag */
extern int	g_last_builtin_oid; /* value of the last builtin oid */
extern Archive *g_fout;			/* the script file */

/* placeholders for comment starting and ending delimiters */
extern char g_comment_start[10];
extern char g_comment_end[10];

extern char g_opaque_type[10];	/* name for the opaque type */

/* pg_dump is really two programs in one
	one version works with postgres v4r2
	and the other works with postgreSQL
	the common routines are declared here
*/
/*
 *	common utility functions
*/

extern TableInfo *dumpSchema(Archive *fout,
		   int *numTablesPtr,
		   const char *tablename,
		   const bool acls,
		   const bool oids,
		   const bool schemaOnly,
		   const bool dataOnly);
extern void dumpSchemaIdx(Archive *fout,
			  const char *tablename,
			  TableInfo *tblinfo,
			  int numTables);

extern char *findTypeByOid(TypeInfo *tinfo, int numTypes, const char *oid);
extern char *findOprByOid(OprInfo *oprinfo, int numOprs, const char *oid);
extern int	findFuncByName(FuncInfo *finfo, int numFuncs, const char *name);
extern int	findTableByName(TableInfo *tbinfo, int numTables, const char *relname);

extern void check_conn_and_db(void);
extern void parseNumericArray(const char *str, char **array, int arraysize);

/*
 * version specific routines
 */
extern TypeInfo *getTypes(int *numTypes);
extern FuncInfo *getFuncs(int *numFuncs);
extern AggInfo *getAggregates(int *numAggregates);

extern void clearAggInfo(AggInfo *, int);
extern void clearFuncInfo(FuncInfo *, int);
extern void clearInhInfo(InhInfo *, int);
extern void clearIndInfo(IndInfo *, int);
extern void clearOprInfo(OprInfo *, int);
extern void clearTypeInfo(TypeInfo *, int);

extern OprInfo *getOperators(int *numOperators);
extern TableInfo *getTables(int *numTables, FuncInfo *finfo, int numFuncs);
extern InhInfo *getInherits(int *numInherits);
extern void getTableAttrs(TableInfo *tbinfo, int numTables);
extern IndInfo *getIndices(int *numIndices);
extern void dumpDBComment(Archive *outfile);
extern void dumpTypes(Archive *fout, FuncInfo *finfo, int numFuncs,
		  TypeInfo *tinfo, int numTypes);
extern void dumpProcLangs(Archive *fout, FuncInfo *finfo, int numFuncs,
			  TypeInfo *tinfo, int numTypes);
extern void dumpFuncs(Archive *fout, FuncInfo *finfo, int numFuncs,
		  TypeInfo *tinfo, int numTypes);
extern void dumpAggs(Archive *fout, AggInfo *agginfo, int numAggregates,
		 TypeInfo *tinfo, int numTypes);
extern void dumpOprs(Archive *fout, OprInfo *agginfo, int numOperators,
		 TypeInfo *tinfo, int numTypes);
extern void dumpTables(Archive *fout, TableInfo *tbinfo, int numTables,
		   InhInfo *inhinfo, int numInherits,
		   TypeInfo *tinfo, int numTypes, const char *tablename,
		   const bool acls, const bool oids,
		   const bool schemaOnly, const bool dataOnly);
extern void dumpIndices(Archive *fout, IndInfo *indinfo, int numIndices,
			TableInfo *tbinfo, int numTables, const char *tablename);
extern const char *fmtId(const char *identifier, bool force_quotes);

#endif	 /* PG_DUMP_H */
