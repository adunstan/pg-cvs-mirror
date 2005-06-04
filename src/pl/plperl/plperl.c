/**********************************************************************
 * plperl.c - perl as a procedural language for PostgreSQL
 *
 * IDENTIFICATION
 *
 *	  This software is copyrighted by Mark Hollomon
 *	  but is shameless cribbed from pltcl.c by Jan Wieck.
 *
 *	  The author hereby grants permission  to  use,  copy,	modify,
 *	  distribute,  and	license this software and its documentation
 *	  for any purpose, provided that existing copyright notices are
 *	  retained	in	all  copies  and  that	this notice is included
 *	  verbatim in any distributions. No written agreement, license,
 *	  or  royalty  fee	is required for any of the authorized uses.
 *	  Modifications to this software may be  copyrighted  by  their
 *	  author  and  need  not  follow  the licensing terms described
 *	  here, provided that the new terms are  clearly  indicated  on
 *	  the first page of each file where they apply.
 *
 *	  IN NO EVENT SHALL THE AUTHOR OR DISTRIBUTORS BE LIABLE TO ANY
 *	  PARTY  FOR  DIRECT,	INDIRECT,	SPECIAL,   INCIDENTAL,	 OR
 *	  CONSEQUENTIAL   DAMAGES  ARISING	OUT  OF  THE  USE  OF  THIS
 *	  SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF, EVEN
 *	  IF  THE  AUTHOR  HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH
 *	  DAMAGE.
 *
 *	  THE  AUTHOR  AND	DISTRIBUTORS  SPECIFICALLY	 DISCLAIM	ANY
 *	  WARRANTIES,  INCLUDING,  BUT	NOT  LIMITED  TO,  THE	IMPLIED
 *	  WARRANTIES  OF  MERCHANTABILITY,	FITNESS  FOR  A  PARTICULAR
 *	  PURPOSE,	AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON
 *	  AN "AS IS" BASIS, AND THE AUTHOR	AND  DISTRIBUTORS  HAVE  NO
 *	  OBLIGATION   TO	PROVIDE   MAINTENANCE,	 SUPPORT,  UPDATES,
 *	  ENHANCEMENTS, OR MODIFICATIONS.
 *
 * IDENTIFICATION
 *	  $PostgreSQL: pgsql/src/pl/plperl/plperl.c,v 1.74 2005/05/23 01:57:51 neilc Exp $
 *
 **********************************************************************/

#include "postgres.h"
/* Defined by Perl */
#undef _

/* system stuff */
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>

/* postgreSQL stuff */
#include "commands/trigger.h"
#include "executor/spi.h"
#include "funcapi.h"
#include "utils/lsyscache.h"
#include "utils/memutils.h"
#include "utils/typcache.h"
#include "miscadmin.h"

/* perl stuff */
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#include "ppport.h"

/* just in case these symbols aren't provided */
#ifndef pTHX_
#define pTHX_
#define pTHX void
#endif


/**********************************************************************
 * The information we cache about loaded procedures
 **********************************************************************/
typedef struct plperl_proc_desc
{
	char	   *proname;
	TransactionId fn_xmin;
	CommandId	fn_cmin;
	bool		fn_readonly;
	bool		lanpltrusted;
	bool		fn_retistuple;	/* true, if function returns tuple */
	bool		fn_retisset;	/* true, if function returns set */
	Oid			result_oid;		/* Oid of result type */
	FmgrInfo	result_in_func;	/* I/O function and arg for result type */
	Oid			result_typioparam;
	int			nargs;
	FmgrInfo	arg_out_func[FUNC_MAX_ARGS];
	bool		arg_is_rowtype[FUNC_MAX_ARGS];
	SV		   *reference;
	FunctionCallInfo caller_info;
	Tuplestorestate *tuple_store;
	TupleDesc tuple_desc;
} plperl_proc_desc;


/**********************************************************************
 * Global data
 **********************************************************************/
static int	plperl_firstcall = 1;
static bool plperl_safe_init_done = false;
static PerlInterpreter *plperl_interp = NULL;
static HV  *plperl_proc_hash = NULL;

static bool plperl_use_strict = false;

/* this is saved and restored by plperl_call_handler */
static plperl_proc_desc *plperl_current_prodesc = NULL;

/**********************************************************************
 * Forward declarations
 **********************************************************************/
static void plperl_init_all(void);
static void plperl_init_interp(void);

Datum		plperl_call_handler(PG_FUNCTION_ARGS);
void		plperl_init(void);

HV		   *plperl_spi_exec(char *query, int limit);

static Datum plperl_func_handler(PG_FUNCTION_ARGS);

static Datum plperl_trigger_handler(PG_FUNCTION_ARGS);
static plperl_proc_desc *compile_plperl_function(Oid fn_oid, bool is_trigger);

static SV  *plperl_hash_from_tuple(HeapTuple tuple, TupleDesc tupdesc);
static void plperl_init_shared_libs(pTHX);
static HV  *plperl_spi_execute_fetch_result(SPITupleTable *, int, int);

void plperl_return_next(SV *);

/*
 * This routine is a crock, and so is everyplace that calls it.  The problem
 * is that the cached form of plperl functions/queries is allocated permanently
 * (mostly via malloc()) and never released until backend exit.  Subsidiary
 * data structures such as fmgr info records therefore must live forever
 * as well.  A better implementation would store all this stuff in a per-
 * function memory context that could be reclaimed at need.  In the meantime,
 * fmgr_info_cxt must be called specifying TopMemoryContext so that whatever
 * it might allocate, and whatever the eventual function might allocate using
 * fn_mcxt, will live forever too.
 */
static void
perm_fmgr_info(Oid functionId, FmgrInfo *finfo)
{
	fmgr_info_cxt(functionId, finfo, TopMemoryContext);
}


/* Perform initialization during postmaster startup. */

void
plperl_init(void)
{
	if (!plperl_firstcall)
		return;

	DefineCustomBoolVariable(
		"plperl.use_strict",
		"If true, will compile trusted and untrusted perl code in strict mode",
		NULL,
		&plperl_use_strict,
		PGC_USERSET,
		NULL, NULL);

	EmitWarningsOnPlaceholders("plperl");

	plperl_init_interp();
	plperl_firstcall = 0;
}


/* Perform initialization during backend startup. */

static void
plperl_init_all(void)
{
	if (plperl_firstcall)
		plperl_init();

	/* We don't need to do anything yet when a new backend starts. */
}


static void
plperl_init_interp(void)
{
	static char	   *loose_embedding[3] = {
		"", "-e",
		/* all one string follows (no commas please) */
		"SPI::bootstrap(); use vars qw(%_SHARED);"
		"sub ::mkunsafefunc {return eval(qq[ sub { $_[0] $_[1] } ]); }"
	};

	static char	   *strict_embedding[3] = {
		"", "-e",
		/* all one string follows (no commas please) */
		"SPI::bootstrap(); use vars qw(%_SHARED);"
		"sub ::mkunsafefunc {return eval("
		"qq[ sub { use strict; $_[0] $_[1] } ]); }"
	};

	plperl_interp = perl_alloc();
	if (!plperl_interp)
		elog(ERROR, "could not allocate Perl interpreter");

	perl_construct(plperl_interp);
	perl_parse(plperl_interp, plperl_init_shared_libs, 3 ,
			   (plperl_use_strict ? strict_embedding : loose_embedding), NULL);
	perl_run(plperl_interp);

	plperl_proc_hash = newHV();
}


static void
plperl_safe_init(void)
{
	static char *safe_module =
	"require Safe; $Safe::VERSION";

	static char *common_safe_ok =
	"use vars qw($PLContainer); $PLContainer = new Safe('PLPerl');"
	"$PLContainer->permit_only(':default');"
	"$PLContainer->permit(qw[:base_math !:base_io sort time]);"
	"$PLContainer->share(qw[&elog &spi_exec_query &spi_return_next "
	"&DEBUG &LOG &INFO &NOTICE &WARNING &ERROR %_SHARED ]);"
			   ;

	static char * strict_safe_ok =
		"$PLContainer->permit('require');$PLContainer->reval('use strict;');"
		"$PLContainer->deny('require');"
		"sub ::mksafefunc { return $PLContainer->reval(qq[ "
		"             sub { BEGIN { strict->import(); } $_[0] $_[1]}]); }"
		;

	static char * loose_safe_ok =
		"sub ::mksafefunc { return $PLContainer->reval(qq[ "
		"             sub { $_[0] $_[1]}]); }"
		;

	static char *safe_bad =
	"use vars qw($PLContainer); $PLContainer = new Safe('PLPerl');"
	"$PLContainer->permit_only(':default');"
	"$PLContainer->share(qw[&elog &ERROR ]);"
	"sub ::mksafefunc { return $PLContainer->reval(qq[sub { "
	"elog(ERROR,'trusted Perl functions disabled - "
	"please upgrade Perl Safe module to version 2.09 or later');}]); }"
			   ;

	SV		   *res;
	double		safe_version;

	res = eval_pv(safe_module, FALSE);	/* TRUE = croak if failure */

	safe_version = SvNV(res);

	/*
	 * We actually want to reject safe_version < 2.09, but it's risky to
	 * assume that floating-point comparisons are exact, so use a slightly
	 * smaller comparison value.
	 */
	if (safe_version < 2.0899 )
	{
		/* not safe, so disallow all trusted funcs */
		eval_pv(safe_bad, FALSE);
	}
	else
	{
		eval_pv(common_safe_ok, FALSE);
		eval_pv((plperl_use_strict ? strict_safe_ok : loose_safe_ok), FALSE);
	}

	plperl_safe_init_done = true;
}


/*
 * Perl likes to put a newline after its error messages; clean up such
 */
static char *
strip_trailing_ws(const char *msg)
{
	char   *res = pstrdup(msg);
	int		len = strlen(res);

	while (len > 0 && isspace((unsigned char) res[len-1]))
		res[--len] = '\0';
	return res;
}


/* Build a tuple from a hash. */

static HeapTuple
plperl_build_tuple_result(HV *perlhash, AttInMetadata *attinmeta)
{
	TupleDesc	td = attinmeta->tupdesc;
	char	  **values;
	SV		   *val;
	char	   *key;
	I32			klen;
	HeapTuple	tup;

	values = (char **) palloc0(td->natts * sizeof(char *));

	hv_iterinit(perlhash);
	while ((val = hv_iternextsv(perlhash, &key, &klen)))
	{
		int	attn = SPI_fnumber(td, key);

		if (attn <= 0 || td->attrs[attn - 1]->attisdropped)
			ereport(ERROR,
					(errcode(ERRCODE_UNDEFINED_COLUMN),
					 errmsg("Perl hash contains nonexistent column \"%s\"",
							key)));
		if (SvOK(val) && SvTYPE(val) != SVt_NULL)
			values[attn - 1] = SvPV(val, PL_na);
	}
	hv_iterinit(perlhash);

	tup = BuildTupleFromCStrings(attinmeta, values);
	pfree(values);
	return tup;
}


/* Set up the arguments for a trigger call. */

static SV  *
plperl_trigger_build_args(FunctionCallInfo fcinfo)
{
	TriggerData *tdata;
	TupleDesc	tupdesc;
	int			i;
	char	   *level;
	char	   *event;
	char	   *relid;
	char	   *when;
	HV		   *hv;

	hv = newHV();

	tdata = (TriggerData *) fcinfo->context;
	tupdesc = tdata->tg_relation->rd_att;

	relid = DatumGetCString(
				DirectFunctionCall1(oidout,
									ObjectIdGetDatum(tdata->tg_relation->rd_id)
				)
			);

	hv_store(hv, "name", 4, newSVpv(tdata->tg_trigger->tgname, 0), 0);
	hv_store(hv, "relid", 5, newSVpv(relid, 0), 0);

	if (TRIGGER_FIRED_BY_INSERT(tdata->tg_event))
	{
		event = "INSERT";
		if (TRIGGER_FIRED_FOR_ROW(tdata->tg_event))
			hv_store(hv, "new", 3,
					 plperl_hash_from_tuple(tdata->tg_trigtuple, tupdesc),
					 0);
	}
	else if (TRIGGER_FIRED_BY_DELETE(tdata->tg_event))
	{
		event = "DELETE";
		if (TRIGGER_FIRED_FOR_ROW(tdata->tg_event))
			hv_store(hv, "old", 3,
					 plperl_hash_from_tuple(tdata->tg_trigtuple, tupdesc),
					 0);
	}
	else if (TRIGGER_FIRED_BY_UPDATE(tdata->tg_event))
	{
		event = "UPDATE";
		if (TRIGGER_FIRED_FOR_ROW(tdata->tg_event))
		{
			hv_store(hv, "old", 3,
					 plperl_hash_from_tuple(tdata->tg_trigtuple, tupdesc),
					 0);
			hv_store(hv, "new", 3,
					 plperl_hash_from_tuple(tdata->tg_newtuple, tupdesc),
					 0);
		}
	}
	else
		event = "UNKNOWN";

	hv_store(hv, "event", 5, newSVpv(event, 0), 0);
	hv_store(hv, "argc", 4, newSViv(tdata->tg_trigger->tgnargs), 0);

	if (tdata->tg_trigger->tgnargs > 0)
	{
		AV *av = newAV();
		for (i=0; i < tdata->tg_trigger->tgnargs; i++)
			av_push(av, newSVpv(tdata->tg_trigger->tgargs[i], 0));
		hv_store(hv, "args", 4, newRV_noinc((SV *)av), 0);
	}

	hv_store(hv, "relname", 7,
			 newSVpv(SPI_getrelname(tdata->tg_relation), 0), 0);

	if (TRIGGER_FIRED_BEFORE(tdata->tg_event))
		when = "BEFORE";
	else if (TRIGGER_FIRED_AFTER(tdata->tg_event))
		when = "AFTER";
	else
		when = "UNKNOWN";
	hv_store(hv, "when", 4, newSVpv(when, 0), 0);

	if (TRIGGER_FIRED_FOR_ROW(tdata->tg_event))
		level = "ROW";
	else if (TRIGGER_FIRED_FOR_STATEMENT(tdata->tg_event))
		level = "STATEMENT";
	else
		level = "UNKNOWN";
	hv_store(hv, "level", 5, newSVpv(level, 0), 0);

	return newRV_noinc((SV*)hv);
}


/* Set up the new tuple returned from a trigger. */

static HeapTuple
plperl_modify_tuple(HV *hvTD, TriggerData *tdata, HeapTuple otup)
{
	SV		  **svp;
	HV		   *hvNew;
	HeapTuple	rtup;
	SV		   *val;
	char	   *key;
	I32			klen;
	int			slotsused;
	int		   *modattrs;
	Datum	   *modvalues;
	char	   *modnulls;

	TupleDesc	tupdesc;

	tupdesc = tdata->tg_relation->rd_att;

	svp = hv_fetch(hvTD, "new", 3, FALSE);
	if (!svp)
		ereport(ERROR,
				(errcode(ERRCODE_UNDEFINED_COLUMN),
				 errmsg("$_TD->{new} does not exist")));
	if (!SvOK(*svp) || SvTYPE(*svp) != SVt_RV || SvTYPE(SvRV(*svp)) != SVt_PVHV)
		ereport(ERROR,
				(errcode(ERRCODE_DATATYPE_MISMATCH),
				 errmsg("$_TD->{new} is not a hash reference")));
	hvNew = (HV *) SvRV(*svp);

	modattrs = palloc(tupdesc->natts * sizeof(int));
	modvalues = palloc(tupdesc->natts * sizeof(Datum));
	modnulls = palloc(tupdesc->natts * sizeof(char));
	slotsused = 0;

	hv_iterinit(hvNew);
	while ((val = hv_iternextsv(hvNew, &key, &klen)))
	{
		int			attn = SPI_fnumber(tupdesc, key);

		if (attn <= 0 || tupdesc->attrs[attn - 1]->attisdropped)
			ereport(ERROR,
					(errcode(ERRCODE_UNDEFINED_COLUMN),
					 errmsg("Perl hash contains nonexistent column \"%s\"",
							key)));
		if (SvOK(val) && SvTYPE(val) != SVt_NULL)
		{
			Oid			typinput;
			Oid			typioparam;
			FmgrInfo	finfo;

			/* XXX would be better to cache these lookups */
			getTypeInputInfo(tupdesc->attrs[attn - 1]->atttypid,
							 &typinput, &typioparam);
			fmgr_info(typinput, &finfo);
			modvalues[slotsused] = FunctionCall3(&finfo,
										 CStringGetDatum(SvPV(val, PL_na)),
										 ObjectIdGetDatum(typioparam),
						 Int32GetDatum(tupdesc->attrs[attn - 1]->atttypmod));
			modnulls[slotsused] = ' ';
		}
		else
		{
			modvalues[slotsused] = (Datum) 0;
			modnulls[slotsused] = 'n';
		}
		modattrs[slotsused] = attn;
		slotsused++;
	}
	hv_iterinit(hvNew);

	rtup = SPI_modifytuple(tdata->tg_relation, otup, slotsused,
						   modattrs, modvalues, modnulls);

	pfree(modattrs);
	pfree(modvalues);
	pfree(modnulls);

	if (rtup == NULL)
		elog(ERROR, "SPI_modifytuple failed: %s",
			 SPI_result_code_string(SPI_result));

	return rtup;
}


/* This is the only externally-visible part of the plperl interface.
 * The Postgres function and trigger managers call it to execute a
 * perl function. */

PG_FUNCTION_INFO_V1(plperl_call_handler);

Datum
plperl_call_handler(PG_FUNCTION_ARGS)
{
	Datum retval;
	plperl_proc_desc *save_prodesc;

	plperl_init_all();

	save_prodesc = plperl_current_prodesc;

	PG_TRY();
	{
		if (CALLED_AS_TRIGGER(fcinfo))
			retval = PointerGetDatum(plperl_trigger_handler(fcinfo));
		else
			retval = plperl_func_handler(fcinfo);
	}
	PG_CATCH();
	{
		plperl_current_prodesc = save_prodesc;
		PG_RE_THROW();
	}
	PG_END_TRY();

	plperl_current_prodesc = save_prodesc;

	return retval;
}


/* Uses mksafefunc/mkunsafefunc to create an anonymous sub whose text is
 * supplied in s, and returns a reference to the closure. */

static SV  *
plperl_create_sub(char *s, bool trusted)
{
	dSP;
	SV		   *subref;
	int			count;

	if (trusted && !plperl_safe_init_done)
	{
		plperl_safe_init();
		SPAGAIN;
	}

	ENTER;
	SAVETMPS;
	PUSHMARK(SP);
	XPUSHs(sv_2mortal(newSVpv("my $_TD=$_[0]; shift;", 0)));
	XPUSHs(sv_2mortal(newSVpv(s, 0)));
	PUTBACK;

	/*
	 * G_KEEPERR seems to be needed here, else we don't recognize compile
	 * errors properly.  Perhaps it's because there's another level of
	 * eval inside mksafefunc?
	 */
	count = perl_call_pv((trusted ? "mksafefunc" : "mkunsafefunc"),
						 G_SCALAR | G_EVAL | G_KEEPERR);
	SPAGAIN;

	if (count != 1)
	{
		PUTBACK;
		FREETMPS;
		LEAVE;
		elog(ERROR, "didn't get a return item from mksafefunc");
	}

	if (SvTRUE(ERRSV))
	{
		(void) POPs;
		PUTBACK;
		FREETMPS;
		LEAVE;
		ereport(ERROR,
				(errcode(ERRCODE_SYNTAX_ERROR),
				 errmsg("creation of Perl function failed: %s",
						strip_trailing_ws(SvPV(ERRSV, PL_na)))));
	}

	/*
	 * need to make a deep copy of the return. it comes off the stack as a
	 * temporary.
	 */
	subref = newSVsv(POPs);

	if (!SvROK(subref))
	{
		PUTBACK;
		FREETMPS;
		LEAVE;

		/*
		 * subref is our responsibility because it is not mortal
		 */
		SvREFCNT_dec(subref);
		elog(ERROR, "didn't get a code ref");
	}

	PUTBACK;
	FREETMPS;
	LEAVE;

	return subref;
}


/**********************************************************************
 * plperl_init_shared_libs()		-
 *
 * We cannot use the DynaLoader directly to get at the Opcode
 * module (used by Safe.pm). So, we link Opcode into ourselves
 * and do the initialization behind perl's back.
 *
 **********************************************************************/

EXTERN_C void boot_DynaLoader(pTHX_ CV *cv);
EXTERN_C void boot_SPI(pTHX_ CV *cv);

static void
plperl_init_shared_libs(pTHX)
{
	char	   *file = __FILE__;

	newXS("DynaLoader::boot_DynaLoader", boot_DynaLoader, file);
	newXS("SPI::bootstrap", boot_SPI, file);
}


static SV  *
plperl_call_perl_func(plperl_proc_desc *desc, FunctionCallInfo fcinfo)
{
	dSP;
	SV		   *retval;
	int			i;
	int			count;

	ENTER;
	SAVETMPS;

	PUSHMARK(SP);

	XPUSHs(&PL_sv_undef); /* no trigger data */

	for (i = 0; i < desc->nargs; i++)
	{
		if (fcinfo->argnull[i])
			XPUSHs(&PL_sv_undef);
		else if (desc->arg_is_rowtype[i])
		{
			HeapTupleHeader td;
			Oid			tupType;
			int32		tupTypmod;
			TupleDesc	tupdesc;
			HeapTupleData tmptup;
			SV		   *hashref;

			td = DatumGetHeapTupleHeader(fcinfo->arg[i]);
			/* Extract rowtype info and find a tupdesc */
			tupType = HeapTupleHeaderGetTypeId(td);
			tupTypmod = HeapTupleHeaderGetTypMod(td);
			tupdesc = lookup_rowtype_tupdesc(tupType, tupTypmod);
			/* Build a temporary HeapTuple control structure */
			tmptup.t_len = HeapTupleHeaderGetDatumLength(td);
			tmptup.t_data = td;

			hashref = plperl_hash_from_tuple(&tmptup, tupdesc);
			XPUSHs(sv_2mortal(hashref));
		}
		else
		{
			char	   *tmp;

			tmp = DatumGetCString(FunctionCall1(&(desc->arg_out_func[i]),
												fcinfo->arg[i]));
			XPUSHs(sv_2mortal(newSVpv(tmp, 0)));
			pfree(tmp);
		}
	}
	PUTBACK;

	/* Do NOT use G_KEEPERR here */
	count = perl_call_sv(desc->reference, G_SCALAR | G_EVAL);

	SPAGAIN;

	if (count != 1)
	{
		PUTBACK;
		FREETMPS;
		LEAVE;
		elog(ERROR, "didn't get a return item from function");
	}

	if (SvTRUE(ERRSV))
	{
		(void) POPs;
		PUTBACK;
		FREETMPS;
		LEAVE;
		/* XXX need to find a way to assign an errcode here */
		ereport(ERROR,
				(errmsg("error from Perl function: %s",
						strip_trailing_ws(SvPV(ERRSV, PL_na)))));
	}

	retval = newSVsv(POPs);

	PUTBACK;
	FREETMPS;
	LEAVE;

	return retval;
}


static SV  *
plperl_call_perl_trigger_func(plperl_proc_desc *desc, FunctionCallInfo fcinfo,
							  SV *td)
{
	dSP;
	SV		   *retval;
	Trigger    *tg_trigger;
	int			i;
	int			count;

	ENTER;
	SAVETMPS;

	PUSHMARK(sp);

	XPUSHs(td);

	tg_trigger = ((TriggerData *) fcinfo->context)->tg_trigger;
	for (i = 0; i < tg_trigger->tgnargs; i++)
		XPUSHs(sv_2mortal(newSVpv(tg_trigger->tgargs[i], 0)));
	PUTBACK;

	/* Do NOT use G_KEEPERR here */
	count = perl_call_sv(desc->reference, G_SCALAR | G_EVAL);

	SPAGAIN;

	if (count != 1)
	{
		PUTBACK;
		FREETMPS;
		LEAVE;
		elog(ERROR, "didn't get a return item from trigger function");
	}

	if (SvTRUE(ERRSV))
	{
		(void) POPs;
		PUTBACK;
		FREETMPS;
		LEAVE;
		/* XXX need to find a way to assign an errcode here */
		ereport(ERROR,
				(errmsg("error from Perl trigger function: %s",
						strip_trailing_ws(SvPV(ERRSV, PL_na)))));
	}

	retval = newSVsv(POPs);

	PUTBACK;
	FREETMPS;
	LEAVE;

	return retval;
}


static Datum
plperl_func_handler(PG_FUNCTION_ARGS)
{
	plperl_proc_desc *prodesc;
	SV		   *perlret;
	Datum		retval;
	ReturnSetInfo *rsi;

	if (SPI_connect() != SPI_OK_CONNECT)
		elog(ERROR, "could not connect to SPI manager");

	prodesc = compile_plperl_function(fcinfo->flinfo->fn_oid, false);

	plperl_current_prodesc = prodesc;
	prodesc->caller_info = fcinfo;
	prodesc->tuple_store = 0;
	prodesc->tuple_desc = 0;

	perlret = plperl_call_perl_func(prodesc, fcinfo);

	/************************************************************
	 * Disconnect from SPI manager and then create the return
	 * values datum (if the input function does a palloc for it
	 * this must not be allocated in the SPI memory context
	 * because SPI_finish would free it).
	 ************************************************************/
	if (SPI_finish() != SPI_OK_FINISH)
		elog(ERROR, "SPI_finish() failed");

	rsi = (ReturnSetInfo *)fcinfo->resultinfo;

	if (prodesc->fn_retisset) {
		if (!rsi || !IsA(rsi, ReturnSetInfo) ||
			(rsi->allowedModes & SFRM_Materialize) == 0 ||
			rsi->expectedDesc == NULL)
		{
			ereport(ERROR,
					(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
					 errmsg("set-valued function called in context that "
							"cannot accept a set")));
		}

		/* If the Perl function returned an arrayref, we pretend that it
		 * called return_next() for each element of the array, to handle
		 * old SRFs that didn't know about return_next(). Any other sort
		 * of return value is an error. */
		if (SvTYPE(perlret) == SVt_RV &&
			SvTYPE(SvRV(perlret)) == SVt_PVAV)
		{
			int i = 0;
			SV **svp = 0;
			AV *rav = (AV *)SvRV(perlret);
			while ((svp = av_fetch(rav, i, FALSE)) != NULL) {
				plperl_return_next(*svp);
				i++;
			}
		}
		else if (SvTYPE(perlret) != SVt_NULL)
		{
			ereport(ERROR,
					(errcode(ERRCODE_DATATYPE_MISMATCH),
					 errmsg("set-returning Perl function must return "
							"reference to array or use return_next")));
		}

		rsi->returnMode = SFRM_Materialize;
		if (prodesc->tuple_store) {
			rsi->setResult = prodesc->tuple_store;
			rsi->setDesc = prodesc->tuple_desc;
		}
		retval = (Datum)0;
	}
	else if (SvTYPE(perlret) == SVt_NULL)
	{
		/* Return NULL if Perl code returned undef */
		if (rsi && IsA(rsi, ReturnSetInfo))
			rsi->isDone = ExprEndResult;
		fcinfo->isnull = true;
		retval = (Datum)0;
	}
	else if (prodesc->fn_retistuple)
	{
		/* Return a perl hash converted to a Datum */
		TupleDesc td;
		AttInMetadata *attinmeta;
		HeapTuple tup;

		if (!SvOK(perlret) || SvTYPE(perlret) != SVt_RV ||
			SvTYPE(SvRV(perlret)) != SVt_PVHV)
		{
			ereport(ERROR,
					(errcode(ERRCODE_DATATYPE_MISMATCH),
					 errmsg("composite-returning Perl function "
							"must return reference to hash")));
		}

		/* XXX should cache the attinmeta data instead of recomputing */
		if (get_call_result_type(fcinfo, NULL, &td) != TYPEFUNC_COMPOSITE)
		{
			ereport(ERROR,
					(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
					 errmsg("function returning record called in context "
							"that cannot accept type record")));
		}

		attinmeta = TupleDescGetAttInMetadata(td);
		tup = plperl_build_tuple_result((HV *)SvRV(perlret), attinmeta);
		retval = HeapTupleGetDatum(tup);
	}
	else
	{
		/* Return a perl string converted to a Datum */
		char *val = SvPV(perlret, PL_na);
		retval = FunctionCall3(&prodesc->result_in_func,
							   CStringGetDatum(val),
							   ObjectIdGetDatum(prodesc->result_typioparam),
							   Int32GetDatum(-1));
	}

	SvREFCNT_dec(perlret);
	return retval;
}


static Datum
plperl_trigger_handler(PG_FUNCTION_ARGS)
{
	plperl_proc_desc *prodesc;
	SV		   *perlret;
	Datum		retval;
	SV		   *svTD;
	HV		   *hvTD;

	/* Connect to SPI manager */
	if (SPI_connect() != SPI_OK_CONNECT)
		elog(ERROR, "could not connect to SPI manager");

	/* Find or compile the function */
	prodesc = compile_plperl_function(fcinfo->flinfo->fn_oid, true);

	plperl_current_prodesc = prodesc;

	svTD = plperl_trigger_build_args(fcinfo);
	perlret = plperl_call_perl_trigger_func(prodesc, fcinfo, svTD);
	hvTD = (HV *) SvRV(svTD);

	/************************************************************
	* Disconnect from SPI manager and then create the return
	* values datum (if the input function does a palloc for it
	* this must not be allocated in the SPI memory context
	* because SPI_finish would free it).
	************************************************************/
	if (SPI_finish() != SPI_OK_FINISH)
		elog(ERROR, "SPI_finish() failed");

	if (!(perlret && SvOK(perlret) && SvTYPE(perlret) != SVt_NULL))
	{
		/* undef result means go ahead with original tuple */
		TriggerData *trigdata = ((TriggerData *) fcinfo->context);

		if (TRIGGER_FIRED_BY_INSERT(trigdata->tg_event))
			retval = (Datum) trigdata->tg_trigtuple;
		else if (TRIGGER_FIRED_BY_UPDATE(trigdata->tg_event))
			retval = (Datum) trigdata->tg_newtuple;
		else if (TRIGGER_FIRED_BY_DELETE(trigdata->tg_event))
			retval = (Datum) trigdata->tg_trigtuple;
		else
			retval = (Datum) 0;	/* can this happen? */
	}
	else
	{
		HeapTuple	trv;
		char	   *tmp;

		tmp = SvPV(perlret, PL_na);

		if (pg_strcasecmp(tmp, "SKIP") == 0)
			trv = NULL;
		else if (pg_strcasecmp(tmp, "MODIFY") == 0)
		{
			TriggerData *trigdata = (TriggerData *) fcinfo->context;

			if (TRIGGER_FIRED_BY_INSERT(trigdata->tg_event))
				trv = plperl_modify_tuple(hvTD, trigdata,
										  trigdata->tg_trigtuple);
			else if (TRIGGER_FIRED_BY_UPDATE(trigdata->tg_event))
				trv = plperl_modify_tuple(hvTD, trigdata,
										  trigdata->tg_newtuple);
			else
			{
				ereport(WARNING,
						(errcode(ERRCODE_E_R_I_E_TRIGGER_PROTOCOL_VIOLATED),
						 errmsg("ignoring modified tuple in DELETE trigger")));
				trv = NULL;
			}
		}
		else
		{
			ereport(ERROR,
					(errcode(ERRCODE_E_R_I_E_TRIGGER_PROTOCOL_VIOLATED),
					 errmsg("result of Perl trigger function must be undef, "
							"\"SKIP\" or \"MODIFY\"")));
			trv = NULL;
		}
		retval = PointerGetDatum(trv);
	}

	SvREFCNT_dec(svTD);
	if (perlret)
		SvREFCNT_dec(perlret);

	return retval;
}


static plperl_proc_desc *
compile_plperl_function(Oid fn_oid, bool is_trigger)
{
	HeapTuple	procTup;
	Form_pg_proc procStruct;
	char		internal_proname[64];
	int			proname_len;
	plperl_proc_desc *prodesc = NULL;
	int			i;
	SV			**svp;

	/* We'll need the pg_proc tuple in any case... */
	procTup = SearchSysCache(PROCOID,
							 ObjectIdGetDatum(fn_oid),
							 0, 0, 0);
	if (!HeapTupleIsValid(procTup))
		elog(ERROR, "cache lookup failed for function %u", fn_oid);
	procStruct = (Form_pg_proc) GETSTRUCT(procTup);

	/************************************************************
	 * Build our internal proc name from the functions Oid
	 ************************************************************/
	if (!is_trigger)
		sprintf(internal_proname, "__PLPerl_proc_%u", fn_oid);
	else
		sprintf(internal_proname, "__PLPerl_proc_%u_trigger", fn_oid);

	proname_len = strlen(internal_proname);

	/************************************************************
	 * Lookup the internal proc name in the hashtable
	 ************************************************************/
	svp = hv_fetch(plperl_proc_hash, internal_proname, proname_len, FALSE);
	if (svp)
	{
		bool		uptodate;

		prodesc = (plperl_proc_desc *) SvIV(*svp);

		/************************************************************
		 * If it's present, must check whether it's still up to date.
		 * This is needed because CREATE OR REPLACE FUNCTION can modify the
		 * function's pg_proc entry without changing its OID.
		 ************************************************************/
		uptodate = (prodesc->fn_xmin == HeapTupleHeaderGetXmin(procTup->t_data) &&
			prodesc->fn_cmin == HeapTupleHeaderGetCmin(procTup->t_data));

		if (!uptodate)
		{
			/* need we delete old entry? */
			prodesc = NULL;
		}
	}

	/************************************************************
	 * If we haven't found it in the hashtable, we analyze
	 * the functions arguments and returntype and store
	 * the in-/out-functions in the prodesc block and create
	 * a new hashtable entry for it.
	 *
	 * Then we load the procedure into the Perl interpreter.
	 ************************************************************/
	if (prodesc == NULL)
	{
		HeapTuple	langTup;
		HeapTuple	typeTup;
		Form_pg_language langStruct;
		Form_pg_type typeStruct;
		Datum		prosrcdatum;
		bool		isnull;
		char	   *proc_source;

		/************************************************************
		 * Allocate a new procedure description block
		 ************************************************************/
		prodesc = (plperl_proc_desc *) malloc(sizeof(plperl_proc_desc));
		if (prodesc == NULL)
			ereport(ERROR,
					(errcode(ERRCODE_OUT_OF_MEMORY),
					 errmsg("out of memory")));
		MemSet(prodesc, 0, sizeof(plperl_proc_desc));
		prodesc->proname = strdup(internal_proname);
		prodesc->fn_xmin = HeapTupleHeaderGetXmin(procTup->t_data);
		prodesc->fn_cmin = HeapTupleHeaderGetCmin(procTup->t_data);

		/* Remember if function is STABLE/IMMUTABLE */
		prodesc->fn_readonly =
			(procStruct->provolatile != PROVOLATILE_VOLATILE);

		/************************************************************
		 * Lookup the pg_language tuple by Oid
		 ************************************************************/
		langTup = SearchSysCache(LANGOID,
								 ObjectIdGetDatum(procStruct->prolang),
								 0, 0, 0);
		if (!HeapTupleIsValid(langTup))
		{
			free(prodesc->proname);
			free(prodesc);
			elog(ERROR, "cache lookup failed for language %u",
				 procStruct->prolang);
		}
		langStruct = (Form_pg_language) GETSTRUCT(langTup);
		prodesc->lanpltrusted = langStruct->lanpltrusted;
		ReleaseSysCache(langTup);

		/************************************************************
		 * Get the required information for input conversion of the
		 * return value.
		 ************************************************************/
		if (!is_trigger)
		{
			typeTup = SearchSysCache(TYPEOID,
								ObjectIdGetDatum(procStruct->prorettype),
									 0, 0, 0);
			if (!HeapTupleIsValid(typeTup))
			{
				free(prodesc->proname);
				free(prodesc);
				elog(ERROR, "cache lookup failed for type %u",
					 procStruct->prorettype);
			}
			typeStruct = (Form_pg_type) GETSTRUCT(typeTup);

			/* Disallow pseudotype result, except VOID or RECORD */
			if (typeStruct->typtype == 'p')
			{
				if (procStruct->prorettype == VOIDOID ||
					procStruct->prorettype == RECORDOID)
					 /* okay */ ;
				else if (procStruct->prorettype == TRIGGEROID)
				{
					free(prodesc->proname);
					free(prodesc);
					ereport(ERROR,
							(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
							 errmsg("trigger functions may only be called "
									"as triggers")));
				}
				else
				{
					free(prodesc->proname);
					free(prodesc);
					ereport(ERROR,
							(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
						 errmsg("plperl functions cannot return type %s",
								format_type_be(procStruct->prorettype))));
				}
			}

			prodesc->result_oid = procStruct->prorettype;
			prodesc->fn_retisset = procStruct->proretset;
			prodesc->fn_retistuple = (typeStruct->typtype == 'c' ||
									  procStruct->prorettype == RECORDOID);

			perm_fmgr_info(typeStruct->typinput, &(prodesc->result_in_func));
			prodesc->result_typioparam = getTypeIOParam(typeTup);

			ReleaseSysCache(typeTup);
		}

		/************************************************************
		 * Get the required information for output conversion
		 * of all procedure arguments
		 ************************************************************/
		if (!is_trigger)
		{
			prodesc->nargs = procStruct->pronargs;
			for (i = 0; i < prodesc->nargs; i++)
			{
				typeTup = SearchSysCache(TYPEOID,
							ObjectIdGetDatum(procStruct->proargtypes.values[i]),
										 0, 0, 0);
				if (!HeapTupleIsValid(typeTup))
				{
					free(prodesc->proname);
					free(prodesc);
					elog(ERROR, "cache lookup failed for type %u",
						 procStruct->proargtypes.values[i]);
				}
				typeStruct = (Form_pg_type) GETSTRUCT(typeTup);

				/* Disallow pseudotype argument */
				if (typeStruct->typtype == 'p')
				{
					free(prodesc->proname);
					free(prodesc);
					ereport(ERROR,
							(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
						   errmsg("plperl functions cannot take type %s",
						   format_type_be(procStruct->proargtypes.values[i]))));
				}

				if (typeStruct->typtype == 'c')
					prodesc->arg_is_rowtype[i] = true;
				else
				{
					prodesc->arg_is_rowtype[i] = false;
					perm_fmgr_info(typeStruct->typoutput,
								   &(prodesc->arg_out_func[i]));
				}

				ReleaseSysCache(typeTup);
			}
		}

		/************************************************************
		 * create the text of the anonymous subroutine.
		 * we do not use a named subroutine so that we can call directly
		 * through the reference.
		 ************************************************************/
		prosrcdatum = SysCacheGetAttr(PROCOID, procTup,
									  Anum_pg_proc_prosrc, &isnull);
		if (isnull)
			elog(ERROR, "null prosrc");
		proc_source = DatumGetCString(DirectFunctionCall1(textout,
														  prosrcdatum));

		/************************************************************
		 * Create the procedure in the interpreter
		 ************************************************************/
		prodesc->reference = plperl_create_sub(proc_source, prodesc->lanpltrusted);
		pfree(proc_source);
		if (!prodesc->reference) /* can this happen? */
		{
			free(prodesc->proname);
			free(prodesc);
			elog(ERROR, "could not create internal procedure \"%s\"",
				 internal_proname);
		}

		hv_store(plperl_proc_hash, internal_proname, proname_len,
				 newSViv((IV) prodesc), 0);
	}

	ReleaseSysCache(procTup);

	return prodesc;
}


/* Build a hash from all attributes of a given tuple. */

static SV  *
plperl_hash_from_tuple(HeapTuple tuple, TupleDesc tupdesc)
{
	HV		   *hv;
	int			i;

	hv = newHV();

	for (i = 0; i < tupdesc->natts; i++)
	{
		Datum		attr;
		bool		isnull;
		char	   *attname;
		char	   *outputstr;
		Oid			typoutput;
		bool		typisvarlena;
		int			namelen;

		if (tupdesc->attrs[i]->attisdropped)
			continue;

		attname = NameStr(tupdesc->attrs[i]->attname);
		namelen = strlen(attname);
		attr = heap_getattr(tuple, i + 1, tupdesc, &isnull);

		if (isnull) {
			/* Store (attname => undef) and move on. */
			hv_store(hv, attname, namelen, newSV(0), 0);
			continue;
		}

		/* XXX should have a way to cache these lookups */

		getTypeOutputInfo(tupdesc->attrs[i]->atttypid,
						  &typoutput, &typisvarlena);

		outputstr = DatumGetCString(OidFunctionCall1(typoutput,
													 attr));

		hv_store(hv, attname, namelen, newSVpv(outputstr, 0), 0);
	}

	return newRV_noinc((SV *) hv);
}


HV *
plperl_spi_exec(char *query, int limit)
{
	HV		   *ret_hv;

	/*
	 * Execute the query inside a sub-transaction, so we can cope with
	 * errors sanely
	 */
	MemoryContext oldcontext = CurrentMemoryContext;
	ResourceOwner oldowner = CurrentResourceOwner;

	BeginInternalSubTransaction(NULL);
	/* Want to run inside function's memory context */
	MemoryContextSwitchTo(oldcontext);

	PG_TRY();
	{
		int			spi_rv;

		spi_rv = SPI_execute(query, plperl_current_prodesc->fn_readonly,
							 limit);
		ret_hv = plperl_spi_execute_fetch_result(SPI_tuptable, SPI_processed,
												 spi_rv);

		/* Commit the inner transaction, return to outer xact context */
		ReleaseCurrentSubTransaction();
		MemoryContextSwitchTo(oldcontext);
		CurrentResourceOwner = oldowner;
		/*
		 * AtEOSubXact_SPI() should not have popped any SPI context,
		 * but just in case it did, make sure we remain connected.
		 */
		SPI_restore_connection();
	}
	PG_CATCH();
	{
		ErrorData  *edata;

		/* Save error info */
		MemoryContextSwitchTo(oldcontext);
		edata = CopyErrorData();
		FlushErrorState();

		/* Abort the inner transaction */
		RollbackAndReleaseCurrentSubTransaction();
		MemoryContextSwitchTo(oldcontext);
		CurrentResourceOwner = oldowner;

		/*
		 * If AtEOSubXact_SPI() popped any SPI context of the subxact,
		 * it will have left us in a disconnected state.  We need this
		 * hack to return to connected state.
		 */
		SPI_restore_connection();

		/* Punt the error to Perl */
		croak("%s", edata->message);

		/* Can't get here, but keep compiler quiet */
		return NULL;
	}
	PG_END_TRY();

	return ret_hv;
}


static HV  *
plperl_spi_execute_fetch_result(SPITupleTable *tuptable, int processed,
								int status)
{
	HV		   *result;

	result = newHV();

	hv_store(result, "status", strlen("status"),
			 newSVpv((char *) SPI_result_code_string(status), 0), 0);
	hv_store(result, "processed", strlen("processed"),
			 newSViv(processed), 0);

	if (status == SPI_OK_SELECT)
	{
		AV		   *rows;
		SV		   *row;
		int			i;

		rows = newAV();
		for (i = 0; i < processed; i++)
		{
			row = plperl_hash_from_tuple(tuptable->vals[i], tuptable->tupdesc);
			av_push(rows, row);
		}
		hv_store(result, "rows", strlen("rows"),
				 newRV_noinc((SV *) rows), 0);
	}

	SPI_freetuptable(tuptable);

	return result;
}


void
plperl_return_next(SV *sv)
{
	plperl_proc_desc *prodesc = plperl_current_prodesc;
	FunctionCallInfo fcinfo = prodesc->caller_info;
	ReturnSetInfo *rsi = (ReturnSetInfo *)fcinfo->resultinfo;
	MemoryContext cxt;
	HeapTuple tuple;
	TupleDesc tupdesc;

	if (!sv)
		return;

	if (!prodesc->fn_retisset)
	{
		ereport(ERROR,
				(errcode(ERRCODE_SYNTAX_ERROR),
				 errmsg("cannot use return_next in a non-SETOF function")));
	}

	if (prodesc->fn_retistuple &&
		!(SvOK(sv) && SvTYPE(sv) == SVt_RV && SvTYPE(SvRV(sv)) == SVt_PVHV))
	{
		ereport(ERROR,
				(errcode(ERRCODE_DATATYPE_MISMATCH),
				 errmsg("setof-composite-returning Perl function "
						"must call return_next with reference to hash")));
	}

	cxt = MemoryContextSwitchTo(rsi->econtext->ecxt_per_query_memory);

	if (!prodesc->tuple_store)
		prodesc->tuple_store = tuplestore_begin_heap(true, false, work_mem);

	if (prodesc->fn_retistuple)
	{
		TypeFuncClass rettype;
		AttInMetadata *attinmeta;

		rettype = get_call_result_type(fcinfo, NULL, &tupdesc);
		tupdesc = CreateTupleDescCopy(tupdesc);
		attinmeta = TupleDescGetAttInMetadata(tupdesc);
		tuple = plperl_build_tuple_result((HV *)SvRV(sv), attinmeta);
	}
	else
	{
		Datum ret;
		bool isNull;

		tupdesc = CreateTupleDescCopy(rsi->expectedDesc);

		if (SvOK(sv) && SvTYPE(sv) != SVt_NULL)
		{
			char *val = SvPV(sv, PL_na);
			ret = FunctionCall3(&prodesc->result_in_func,
								PointerGetDatum(val),
								ObjectIdGetDatum(prodesc->result_typioparam),
								Int32GetDatum(-1));
			isNull = false;
		}
		else {
			ret = (Datum)0;
			isNull = true;
		}

		tuple = heap_form_tuple(tupdesc, &ret, &isNull);
	}

	if (!prodesc->tuple_desc)
		prodesc->tuple_desc = tupdesc;

	tuplestore_puttuple(prodesc->tuple_store, tuple);
	heap_freetuple(tuple);
	MemoryContextSwitchTo(cxt);
}
