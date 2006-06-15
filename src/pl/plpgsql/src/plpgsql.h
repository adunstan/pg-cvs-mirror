/*-------------------------------------------------------------------------
 *
 * plpgsql.h		- Definitions for the PL/pgSQL
 *			  procedural language
 *
 * Portions Copyright (c) 1996-2006, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  $PostgreSQL: pgsql/src/pl/plpgsql/src/plpgsql.h,v 1.75 2006/06/12 16:45:30 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */

#ifndef PLPGSQL_H
#define PLPGSQL_H

#include "postgres.h"

#include "fmgr.h"
#include "miscadmin.h"
#include "executor/spi.h"
#include "commands/trigger.h"
#include "utils/tuplestore.h"

/**********************************************************************
 * Definitions
 **********************************************************************/

/* ----------
 * Compiler's namestack item types
 * ----------
 */
enum
{
	PLPGSQL_NSTYPE_LABEL,
	PLPGSQL_NSTYPE_VAR,
	PLPGSQL_NSTYPE_ROW,
	PLPGSQL_NSTYPE_REC
};

/* ----------
 * Datum array node types
 * ----------
 */
enum
{
	PLPGSQL_DTYPE_VAR,
	PLPGSQL_DTYPE_ROW,
	PLPGSQL_DTYPE_REC,
	PLPGSQL_DTYPE_RECFIELD,
	PLPGSQL_DTYPE_ARRAYELEM,
	PLPGSQL_DTYPE_EXPR,
	PLPGSQL_DTYPE_TRIGARG
};

/* ----------
 * Variants distinguished in PLpgSQL_type structs
 * ----------
 */
enum
{
	PLPGSQL_TTYPE_SCALAR,		/* scalar types and domains */
	PLPGSQL_TTYPE_ROW,			/* composite types */
	PLPGSQL_TTYPE_REC,			/* RECORD pseudotype */
	PLPGSQL_TTYPE_PSEUDO		/* other pseudotypes */
};

/* ----------
 * Execution tree node types
 * ----------
 */
enum
{
	PLPGSQL_STMT_BLOCK,
	PLPGSQL_STMT_ASSIGN,
	PLPGSQL_STMT_IF,
	PLPGSQL_STMT_LOOP,
	PLPGSQL_STMT_WHILE,
	PLPGSQL_STMT_FORI,
	PLPGSQL_STMT_FORS,
	PLPGSQL_STMT_SELECT,
	PLPGSQL_STMT_EXIT,
	PLPGSQL_STMT_RETURN,
	PLPGSQL_STMT_RETURN_NEXT,
	PLPGSQL_STMT_RAISE,
	PLPGSQL_STMT_EXECSQL,
	PLPGSQL_STMT_DYNEXECUTE,
	PLPGSQL_STMT_DYNFORS,
	PLPGSQL_STMT_GETDIAG,
	PLPGSQL_STMT_OPEN,
	PLPGSQL_STMT_FETCH,
	PLPGSQL_STMT_CLOSE,
	PLPGSQL_STMT_PERFORM
};


/* ----------
 * Execution node return codes
 * ----------
 */
enum
{
	PLPGSQL_RC_OK,
	PLPGSQL_RC_EXIT,
	PLPGSQL_RC_RETURN,
	PLPGSQL_RC_CONTINUE
};

/* ----------
 * GET DIAGNOSTICS system attrs
 * ----------
 */
enum
{
	PLPGSQL_GETDIAG_ROW_COUNT,
	PLPGSQL_GETDIAG_RESULT_OID
};


/**********************************************************************
 * Node and structure definitions
 **********************************************************************/


typedef struct
{								/* Dynamic string control structure */
	int			alloc;
	int			used;			/* Including NUL terminator */
	char	   *value;
} PLpgSQL_dstring;


typedef struct
{								/* Postgres data type */
	char	   *typname;		/* (simple) name of the type */
	Oid			typoid;			/* OID of the data type */
	int			ttype;			/* PLPGSQL_TTYPE_ code */
	int16		typlen;			/* stuff copied from its pg_type entry */
	bool		typbyval;
	Oid			typrelid;
	Oid			typioparam;
	FmgrInfo	typinput;		/* lookup info for typinput function */
	int32		atttypmod;		/* typmod (taken from someplace else) */
} PLpgSQL_type;


/*
 * PLpgSQL_datum is the common supertype for PLpgSQL_expr, PLpgSQL_var,
 * PLpgSQL_row, PLpgSQL_rec, PLpgSQL_recfield, PLpgSQL_arrayelem, and
 * PLpgSQL_trigarg
 */
typedef struct
{								/* Generic datum array item		*/
	int			dtype;
	int			dno;
} PLpgSQL_datum;

/*
 * The variants PLpgSQL_var, PLpgSQL_row, and PLpgSQL_rec share these
 * fields
 */
typedef struct
{								/* Scalar or composite variable */
	int			dtype;
	int			dno;
	char	   *refname;
	int			lineno;
} PLpgSQL_variable;

typedef struct PLpgSQL_expr
{								/* SQL Query to plan and execute	*/
	int			dtype;
	int			exprno;
	char	   *query;
	void	   *plan;
	Oid		   *plan_argtypes;
	/* fields for "simple expression" fast-path execution: */
	Expr	   *expr_simple_expr;		/* NULL means not a simple expr */
	Oid			expr_simple_type;
	/*
	 * if expr is simple AND in use in current xact, expr_simple_state is
	 * valid.  Test validity by seeing if expr_simple_xid matches current XID.
	 */
	ExprState  *expr_simple_state;
	TransactionId expr_simple_xid;
	/* params to pass to expr */
	int			nparams;
	int			params[1];		/* VARIABLE SIZE ARRAY ... must be last */
} PLpgSQL_expr;


typedef struct
{								/* Scalar variable */
	int			dtype;
	int			varno;
	char	   *refname;
	int			lineno;

	PLpgSQL_type *datatype;
	int			isconst;
	int			notnull;
	PLpgSQL_expr *default_val;
	PLpgSQL_expr *cursor_explicit_expr;
	int			cursor_explicit_argrow;

	Datum		value;
	bool		isnull;
	bool		freeval;
} PLpgSQL_var;


typedef struct
{								/* Row variable */
	int			dtype;
	int			rowno;
	char	   *refname;
	int			lineno;

	TupleDesc	rowtupdesc;

	/*
	 * Note: TupleDesc is only set up for named rowtypes, else it is NULL.
	 *
	 * Note: if the underlying rowtype contains a dropped column, the
	 * corresponding fieldnames[] entry will be NULL, and there is no
	 * corresponding var (varnos[] will be -1).
	 */
	int			nfields;
	char	  **fieldnames;
	int		   *varnos;
} PLpgSQL_row;


typedef struct
{								/* Record variable (non-fixed structure) */
	int			dtype;
	int			recno;
	char	   *refname;
	int			lineno;

	HeapTuple	tup;
	TupleDesc	tupdesc;
	bool		freetup;
	bool		freetupdesc;
} PLpgSQL_rec;


typedef struct
{								/* Field in record */
	int			dtype;
	int			rfno;
	char	   *fieldname;
	int			recparentno;	/* dno of parent record */
} PLpgSQL_recfield;


typedef struct
{								/* Element of array variable */
	int			dtype;
	int			dno;
	PLpgSQL_expr *subscript;
	int			arrayparentno;	/* dno of parent array variable */
} PLpgSQL_arrayelem;


typedef struct
{								/* Positional argument to trigger	*/
	int			dtype;
	int			dno;
	PLpgSQL_expr *argnum;
} PLpgSQL_trigarg;


typedef struct
{								/* Item in the compilers namestack	*/
	int			itemtype;
	int			itemno;
	char		name[1];
} PLpgSQL_nsitem;


/* XXX: consider adapting this to use List */
typedef struct PLpgSQL_ns
{								/* Compiler namestack level		*/
	int			items_alloc;
	int			items_used;
	PLpgSQL_nsitem **items;
	struct PLpgSQL_ns *upper;
} PLpgSQL_ns;


typedef struct
{								/* Generic execution node		*/
	int			cmd_type;
	int			lineno;
} PLpgSQL_stmt;


typedef struct PLpgSQL_condition
{								/* One EXCEPTION condition name */
	int			sqlerrstate;	/* SQLSTATE code */
	char	   *condname;		/* condition name (for debugging) */
	struct PLpgSQL_condition *next;
} PLpgSQL_condition;

typedef struct
{
	int			sqlstate_varno;
	int			sqlerrm_varno;
	List	   *exc_list;		/* List of WHEN clauses */
} PLpgSQL_exception_block;

typedef struct
{								/* One EXCEPTION ... WHEN clause */
	int			lineno;
	PLpgSQL_condition *conditions;
	List	   *action;			/* List of statements */
} PLpgSQL_exception;


typedef struct
{								/* Block of statements			*/
	int			cmd_type;
	int			lineno;
	char	   *label;
	List	   *body;			/* List of statements */
	int			n_initvars;
	int		   *initvarnos;
	PLpgSQL_exception_block *exceptions;
} PLpgSQL_stmt_block;


typedef struct
{								/* Assign statement			*/
	int			cmd_type;
	int			lineno;
	int			varno;
	PLpgSQL_expr *expr;
} PLpgSQL_stmt_assign;

typedef struct
{								/* PERFORM statement		*/
	int			cmd_type;
	int			lineno;
	PLpgSQL_expr *expr;
} PLpgSQL_stmt_perform;

typedef struct
{								/* Get Diagnostics item		*/
	int			kind;			/* id for diagnostic value desired */
	int			target;			/* where to assign it */
} PLpgSQL_diag_item;

typedef struct
{								/* Get Diagnostics statement		*/
	int			cmd_type;
	int			lineno;
	List	   *diag_items;		/* List of PLpgSQL_diag_item */
} PLpgSQL_stmt_getdiag;


typedef struct
{								/* IF statement				*/
	int			cmd_type;
	int			lineno;
	PLpgSQL_expr *cond;
	List	   *true_body;		/* List of statements */
	List	   *false_body;		/* List of statements */
} PLpgSQL_stmt_if;


typedef struct
{								/* Unconditional LOOP statement		*/
	int			cmd_type;
	int			lineno;
	char	   *label;
	List	   *body;			/* List of statements */
} PLpgSQL_stmt_loop;


typedef struct
{								/* WHILE cond LOOP statement		*/
	int			cmd_type;
	int			lineno;
	char	   *label;
	PLpgSQL_expr *cond;
	List	   *body;			/* List of statements */
} PLpgSQL_stmt_while;


typedef struct
{								/* FOR statement with integer loopvar	*/
	int			cmd_type;
	int			lineno;
	char	   *label;
	PLpgSQL_var *var;
	PLpgSQL_expr *lower;
	PLpgSQL_expr *upper;
	PLpgSQL_expr *by;
	int			reverse;
	List	   *body;			/* List of statements */
} PLpgSQL_stmt_fori;


typedef struct
{								/* FOR statement running over SELECT	*/
	int			cmd_type;
	int			lineno;
	char	   *label;
	PLpgSQL_rec *rec;
	PLpgSQL_row *row;
	PLpgSQL_expr *query;
	List	   *body;			/* List of statements */
} PLpgSQL_stmt_fors;


typedef struct
{								/* FOR statement running over EXECUTE	*/
	int			cmd_type;
	int			lineno;
	char	   *label;
	PLpgSQL_rec *rec;
	PLpgSQL_row *row;
	PLpgSQL_expr *query;
	List	   *body;			/* List of statements */
} PLpgSQL_stmt_dynfors;


typedef struct
{								/* SELECT ... INTO statement		*/
	int			cmd_type;
	int			lineno;
	bool		strict;
	PLpgSQL_rec *rec;
	PLpgSQL_row *row;
	PLpgSQL_expr *query;
} PLpgSQL_stmt_select;


typedef struct
{								/* OPEN a curvar					*/
	int			cmd_type;
	int			lineno;
	int			curvar;
	PLpgSQL_row *returntype;
	PLpgSQL_expr *argquery;
	PLpgSQL_expr *query;
	PLpgSQL_expr *dynquery;
} PLpgSQL_stmt_open;


typedef struct
{								/* FETCH curvar INTO statement		*/
	int			cmd_type;
	int			lineno;
	PLpgSQL_rec *rec;
	PLpgSQL_row *row;
	int			curvar;
} PLpgSQL_stmt_fetch;


typedef struct
{								/* CLOSE curvar						*/
	int			cmd_type;
	int			lineno;
	int			curvar;
} PLpgSQL_stmt_close;


typedef struct
{								/* EXIT or CONTINUE statement			*/
	int			cmd_type;
	int			lineno;
	bool		is_exit;		/* Is this an exit or a continue? */
	char	   *label;
	PLpgSQL_expr *cond;
} PLpgSQL_stmt_exit;


typedef struct
{								/* RETURN statement			*/
	int			cmd_type;
	int			lineno;
	PLpgSQL_expr *expr;
	int			retvarno;
} PLpgSQL_stmt_return;

typedef struct
{								/* RETURN NEXT statement */
	int			cmd_type;
	int			lineno;
	PLpgSQL_expr *expr;
	int			retvarno;
} PLpgSQL_stmt_return_next;

typedef struct
{								/* RAISE statement			*/
	int			cmd_type;
	int			lineno;
	int			elog_level;
	char	   *message;
	List	   *params;			/* list of expressions */
} PLpgSQL_stmt_raise;


typedef struct
{								/* Generic SQL statement to execute */
	int			cmd_type;
	int			lineno;
	PLpgSQL_expr *sqlstmt;
} PLpgSQL_stmt_execsql;


typedef struct
{								/* Dynamic SQL string to execute */
	int			cmd_type;
	int			lineno;
	PLpgSQL_rec *rec;			/* INTO record or row variable */
	PLpgSQL_row *row;
	PLpgSQL_expr *query;
} PLpgSQL_stmt_dynexecute;


typedef struct PLpgSQL_func_hashkey
{								/* Hash lookup key for functions */
	Oid			funcOid;

	/*
	 * For a trigger function, the OID of the relation triggered on is part of
	 * the hashkey --- we want to compile the trigger separately for each
	 * relation it is used with, in case the rowtype is different.	Zero if
	 * not called as a trigger.
	 */
	Oid			trigrelOid;

	/*
	 * We include actual argument types in the hash key to support polymorphic
	 * PLpgSQL functions.  Be careful that extra positions are zeroed!
	 */
	Oid			argtypes[FUNC_MAX_ARGS];
} PLpgSQL_func_hashkey;


typedef struct PLpgSQL_function
{								/* Complete compiled function	  */
	char	   *fn_name;
	Oid			fn_oid;
	TransactionId fn_xmin;
	CommandId	fn_cmin;
	int			fn_functype;
	PLpgSQL_func_hashkey *fn_hashkey;	/* back-link to hashtable key */
	MemoryContext fn_cxt;

	Oid			fn_rettype;
	int			fn_rettyplen;
	bool		fn_retbyval;
	FmgrInfo	fn_retinput;
	Oid			fn_rettypioparam;
	bool		fn_retistuple;
	bool		fn_retset;
	bool		fn_readonly;

	int			fn_nargs;
	int			fn_argvarnos[FUNC_MAX_ARGS];
	int			out_param_varno;
	int			found_varno;
	int			new_varno;
	int			old_varno;
	int			tg_name_varno;
	int			tg_when_varno;
	int			tg_level_varno;
	int			tg_op_varno;
	int			tg_relid_varno;
	int			tg_relname_varno;
	int         tg_table_name_varno;
	int         tg_table_schema_varno;
	int			tg_nargs_varno;

	int			ndatums;
	PLpgSQL_datum **datums;
	PLpgSQL_stmt_block *action;
} PLpgSQL_function;


typedef struct
{								/* Runtime execution data	*/
	Datum		retval;
	bool		retisnull;
	Oid			rettype;		/* type of current retval */

	Oid			fn_rettype;		/* info about declared function rettype */
	bool		retistuple;
	bool		retisset;

	bool		readonly_func;

	TupleDesc	rettupdesc;
	char	   *exitlabel;		/* the "target" label of the current EXIT or
								 * CONTINUE stmt, if any */

	Tuplestorestate *tuple_store;		/* SRFs accumulate results here */
	MemoryContext tuple_store_cxt;
	ReturnSetInfo *rsi;

	int			trig_nargs;
	Datum	   *trig_argv;

	int			found_varno;
	int			ndatums;
	PLpgSQL_datum **datums;

	/* temporary state for results from evaluation of query or expr */
	SPITupleTable *eval_tuptable;
	uint32		eval_processed;
	Oid			eval_lastoid;
	ExprContext *eval_econtext;

	/* status information for error context reporting */
	PLpgSQL_function *err_func; /* current func */
	PLpgSQL_stmt *err_stmt;		/* current stmt */
	const char *err_text;		/* additional state info */
} PLpgSQL_execstate;


/**********************************************************************
 * Global variable declarations
 **********************************************************************/

extern bool plpgsql_DumpExecTree;
extern bool plpgsql_SpaceScanned;
extern int	plpgsql_nDatums;
extern PLpgSQL_datum **plpgsql_Datums;

extern int	plpgsql_error_lineno;
extern char *plpgsql_error_funcname;

/* linkage to the real yytext variable */
extern char *plpgsql_base_yytext;

#define yytext plpgsql_base_yytext

extern PLpgSQL_function *plpgsql_curr_compile;
extern bool plpgsql_check_syntax;
extern MemoryContext compile_tmp_cxt;

/**********************************************************************
 * Function declarations
 **********************************************************************/

/* ----------
 * Functions in pl_comp.c
 * ----------
 */
extern PLpgSQL_function *plpgsql_compile(FunctionCallInfo fcinfo,
				bool forValidator);
extern int	plpgsql_parse_word(char *word);
extern int	plpgsql_parse_dblword(char *word);
extern int	plpgsql_parse_tripword(char *word);
extern int	plpgsql_parse_wordtype(char *word);
extern int	plpgsql_parse_dblwordtype(char *word);
extern int	plpgsql_parse_tripwordtype(char *word);
extern int	plpgsql_parse_wordrowtype(char *word);
extern int	plpgsql_parse_dblwordrowtype(char *word);
extern PLpgSQL_type *plpgsql_parse_datatype(const char *string);
extern PLpgSQL_type *plpgsql_build_datatype(Oid typeOid, int32 typmod);
extern PLpgSQL_variable *plpgsql_build_variable(const char *refname, int lineno,
					   PLpgSQL_type *dtype,
					   bool add2namespace);
extern PLpgSQL_condition *plpgsql_parse_err_condition(char *condname);
extern void plpgsql_adddatum(PLpgSQL_datum *new);
extern int	plpgsql_add_initdatums(int **varnos);
extern void plpgsql_HashTableInit(void);
extern void plpgsql_compile_error_callback(void *arg);

/* ----------
 * Functions in pl_handler.c
 * ----------
 */
extern void plpgsql_init(void);
extern Datum plpgsql_call_handler(PG_FUNCTION_ARGS);
extern Datum plpgsql_validator(PG_FUNCTION_ARGS);

/* ----------
 * Functions in pl_exec.c
 * ----------
 */
extern Datum plpgsql_exec_function(PLpgSQL_function *func,
					  FunctionCallInfo fcinfo);
extern HeapTuple plpgsql_exec_trigger(PLpgSQL_function *func,
					 TriggerData *trigdata);
extern void plpgsql_xact_cb(XactEvent event, void *arg);

/* ----------
 * Functions for the dynamic string handling in pl_funcs.c
 * ----------
 */
extern void plpgsql_dstring_init(PLpgSQL_dstring *ds);
extern void plpgsql_dstring_free(PLpgSQL_dstring *ds);
extern void plpgsql_dstring_append(PLpgSQL_dstring *ds, const char *str);
extern void plpgsql_dstring_append_char(PLpgSQL_dstring *ds, char c);
extern char *plpgsql_dstring_get(PLpgSQL_dstring *ds);

/* ----------
 * Functions for the namestack handling in pl_funcs.c
 * ----------
 */
extern void plpgsql_ns_init(void);
extern bool plpgsql_ns_setlocal(bool flag);
extern void plpgsql_ns_push(char *label);
extern void plpgsql_ns_pop(void);
extern void plpgsql_ns_additem(int itemtype, int itemno, const char *name);
extern PLpgSQL_nsitem *plpgsql_ns_lookup(char *name, char *nsname);
extern void plpgsql_ns_rename(char *oldname, char *newname);

/* ----------
 * Other functions in pl_funcs.c
 * ----------
 */
extern void plpgsql_convert_ident(const char *s, char **output, int numidents);
extern const char *plpgsql_stmt_typename(PLpgSQL_stmt *stmt);
extern void plpgsql_dumptree(PLpgSQL_function *func);

/* ----------
 * Externs in gram.y and scan.l
 * ----------
 */
extern PLpgSQL_expr *plpgsql_read_expression(int until, const char *expected);
extern int	plpgsql_yyparse(void);
extern int	plpgsql_base_yylex(void);
extern int	plpgsql_yylex(void);
extern void plpgsql_push_back_token(int token);
extern void plpgsql_yyerror(const char *message);
extern int	plpgsql_scanner_lineno(void);
extern void plpgsql_scanner_init(const char *str, int functype);
extern void plpgsql_scanner_finish(void);
extern char *plpgsql_get_string_value(void);

#endif   /* PLPGSQL_H */
