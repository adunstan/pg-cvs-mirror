/*-------------------------------------------------------------------------
 *
 * selfuncs.c
 *	  Selectivity functions and index cost estimation functions for
 *	  standard operators and index access methods.
 *
 *	  Selectivity routines are registered in the pg_operator catalog
 *	  in the "oprrest" and "oprjoin" attributes.
 *
 *	  Index cost functions are registered in the pg_am catalog
 *	  in the "amcostestimate" attribute.
 *
 * Portions Copyright (c) 1996-2005, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  $PostgreSQL: pgsql/src/backend/utils/adt/selfuncs.c,v 1.169.4.6 2007/01/03 22:39:49 tgl Exp $
 *
 *-------------------------------------------------------------------------
 */

/*----------
 * Operator selectivity estimation functions are called to estimate the
 * selectivity of WHERE clauses whose top-level operator is their operator.
 * We divide the problem into two cases:
 *		Restriction clause estimation: the clause involves vars of just
 *			one relation.
 *		Join clause estimation: the clause involves vars of multiple rels.
 * Join selectivity estimation is far more difficult and usually less accurate
 * than restriction estimation.
 *
 * When dealing with the inner scan of a nestloop join, we consider the
 * join's joinclauses as restriction clauses for the inner relation, and
 * treat vars of the outer relation as parameters (a/k/a constants of unknown
 * values).  So, restriction estimators need to be able to accept an argument
 * telling which relation is to be treated as the variable.
 *
 * The call convention for a restriction estimator (oprrest function) is
 *
 *		Selectivity oprrest (Query *root,
 *							 Oid operator,
 *							 List *args,
 *							 int varRelid);
 *
 * root: general information about the query (rtable and RelOptInfo lists
 * are particularly important for the estimator).
 * operator: OID of the specific operator in question.
 * args: argument list from the operator clause.
 * varRelid: if not zero, the relid (rtable index) of the relation to
 * be treated as the variable relation.  May be zero if the args list
 * is known to contain vars of only one relation.
 *
 * This is represented at the SQL level (in pg_proc) as
 *
 *		float8 oprrest (internal, oid, internal, int4);
 *
 * The call convention for a join estimator (oprjoin function) is similar
 * except that varRelid is not needed, and instead the join type is
 * supplied:
 *
 *		Selectivity oprjoin (Query *root,
 *							 Oid operator,
 *							 List *args,
 *							 JoinType jointype);
 *
 *		float8 oprjoin (internal, oid, internal, int2);
 *
 * (We deliberately make the SQL signature different to facilitate
 * catching errors.)
 *----------
 */

#include "postgres.h"

#include <ctype.h>
#include <math.h>

#include "access/heapam.h"
#include "access/nbtree.h"
#include "access/tuptoaster.h"
#include "catalog/catname.h"
#include "catalog/pg_namespace.h"
#include "catalog/pg_opclass.h"
#include "catalog/pg_operator.h"
#include "catalog/pg_proc.h"
#include "catalog/pg_statistic.h"
#include "catalog/pg_type.h"
#include "mb/pg_wchar.h"
#include "nodes/makefuncs.h"
#include "optimizer/clauses.h"
#include "optimizer/cost.h"
#include "optimizer/pathnode.h"
#include "optimizer/paths.h"
#include "optimizer/plancat.h"
#include "optimizer/prep.h"
#include "optimizer/restrictinfo.h"
#include "optimizer/tlist.h"
#include "optimizer/var.h"
#include "parser/parse_expr.h"
#include "parser/parse_func.h"
#include "parser/parse_oper.h"
#include "parser/parsetree.h"
#include "utils/builtins.h"
#include "utils/date.h"
#include "utils/datum.h"
#include "utils/int8.h"
#include "utils/lsyscache.h"
#include "utils/pg_locale.h"
#include "utils/selfuncs.h"
#include "utils/syscache.h"


/* Return data from examine_variable and friends */
typedef struct
{
	Node	   *var;			/* the Var or expression tree */
	RelOptInfo *rel;			/* Relation, or NULL if not identifiable */
	HeapTuple	statsTuple;		/* pg_statistic tuple, or NULL if none */
	/* NB: if statsTuple!=NULL, it must be freed when caller is done */
	Oid			vartype;		/* exposed type of expression */
	Oid			atttype;		/* type to pass to get_attstatsslot */
	int32		atttypmod;		/* typmod to pass to get_attstatsslot */
	bool		isunique;		/* true if matched to a unique index */
} VariableStatData;

#define ReleaseVariableStats(vardata)  \
	do { \
		if (HeapTupleIsValid((vardata).statsTuple)) \
			ReleaseSysCache((vardata).statsTuple); \
	} while(0)


static bool convert_to_scalar(Datum value, Oid valuetypid, double *scaledvalue,
				  Datum lobound, Datum hibound, Oid boundstypid,
				  double *scaledlobound, double *scaledhibound);
static double convert_numeric_to_scalar(Datum value, Oid typid);
static void convert_string_to_scalar(unsigned char *value,
						 double *scaledvalue,
						 unsigned char *lobound,
						 double *scaledlobound,
						 unsigned char *hibound,
						 double *scaledhibound);
static void convert_bytea_to_scalar(Datum value,
						double *scaledvalue,
						Datum lobound,
						double *scaledlobound,
						Datum hibound,
						double *scaledhibound);
static double convert_one_string_to_scalar(unsigned char *value,
							 int rangelo, int rangehi);
static double convert_one_bytea_to_scalar(unsigned char *value, int valuelen,
							int rangelo, int rangehi);
static unsigned char *convert_string_datum(Datum value, Oid typid);
static double convert_timevalue_to_scalar(Datum value, Oid typid);
static bool get_restriction_variable(Query *root, List *args, int varRelid,
						 VariableStatData *vardata, Node **other,
						 bool *varonleft);
static void get_join_variables(Query *root, List *args,
				   VariableStatData *vardata1,
				   VariableStatData *vardata2);
static void examine_variable(Query *root, Node *node, int varRelid,
				 VariableStatData *vardata);
static double get_variable_numdistinct(VariableStatData *vardata);
static bool get_variable_maximum(Query *root, VariableStatData *vardata,
					 Oid sortop, Datum *max);
static Selectivity prefix_selectivity(Query *root, Node *variable,
				   Oid opclass, Const *prefix);
static Selectivity pattern_selectivity(Const *patt, Pattern_Type ptype);
static Datum string_to_datum(const char *str, Oid datatype);
static Const *string_to_const(const char *str, Oid datatype);
static Const *string_to_bytea_const(const char *str, size_t str_len);


/*
 *		eqsel			- Selectivity of "=" for any data types.
 *
 * Note: this routine is also used to estimate selectivity for some
 * operators that are not "=" but have comparable selectivity behavior,
 * such as "~=" (geometric approximate-match).	Even for "=", we must
 * keep in mind that the left and right datatypes may differ.
 */
Datum
eqsel(PG_FUNCTION_ARGS)
{
	Query	   *root = (Query *) PG_GETARG_POINTER(0);
	Oid			operator = PG_GETARG_OID(1);
	List	   *args = (List *) PG_GETARG_POINTER(2);
	int			varRelid = PG_GETARG_INT32(3);
	VariableStatData vardata;
	Node	   *other;
	bool		varonleft;
	Datum	   *values;
	int			nvalues;
	float4	   *numbers;
	int			nnumbers;
	double		selec;

	/*
	 * If expression is not variable = something or something = variable,
	 * then punt and return a default estimate.
	 */
	if (!get_restriction_variable(root, args, varRelid,
								  &vardata, &other, &varonleft))
		PG_RETURN_FLOAT8(DEFAULT_EQ_SEL);

	/*
	 * If the something is a NULL constant, assume operator is strict and
	 * return zero, ie, operator will never return TRUE.
	 */
	if (IsA(other, Const) &&
		((Const *) other)->constisnull)
	{
		ReleaseVariableStats(vardata);
		PG_RETURN_FLOAT8(0.0);
	}

	if (HeapTupleIsValid(vardata.statsTuple))
	{
		Form_pg_statistic stats;

		stats = (Form_pg_statistic) GETSTRUCT(vardata.statsTuple);

		if (IsA(other, Const))
		{
			/* Variable is being compared to a known non-null constant */
			Datum		constval = ((Const *) other)->constvalue;
			bool		match = false;
			int			i;

			/*
			 * Is the constant "=" to any of the column's most common
			 * values?	(Although the given operator may not really be
			 * "=", we will assume that seeing whether it returns TRUE is
			 * an appropriate test.  If you don't like this, maybe you
			 * shouldn't be using eqsel for your operator...)
			 */
			if (get_attstatsslot(vardata.statsTuple,
								 vardata.atttype, vardata.atttypmod,
								 STATISTIC_KIND_MCV, InvalidOid,
								 &values, &nvalues,
								 &numbers, &nnumbers))
			{
				FmgrInfo	eqproc;

				fmgr_info(get_opcode(operator), &eqproc);

				for (i = 0; i < nvalues; i++)
				{
					/* be careful to apply operator right way 'round */
					if (varonleft)
						match = DatumGetBool(FunctionCall2(&eqproc,
														   values[i],
														   constval));
					else
						match = DatumGetBool(FunctionCall2(&eqproc,
														   constval,
														   values[i]));
					if (match)
						break;
				}
			}
			else
			{
				/* no most-common-value info available */
				values = NULL;
				numbers = NULL;
				i = nvalues = nnumbers = 0;
			}

			if (match)
			{
				/*
				 * Constant is "=" to this common value.  We know
				 * selectivity exactly (or as exactly as VACUUM could
				 * calculate it, anyway).
				 */
				selec = numbers[i];
			}
			else
			{
				/*
				 * Comparison is against a constant that is neither NULL
				 * nor any of the common values.  Its selectivity cannot
				 * be more than this:
				 */
				double		sumcommon = 0.0;
				double		otherdistinct;

				for (i = 0; i < nnumbers; i++)
					sumcommon += numbers[i];
				selec = 1.0 - sumcommon - stats->stanullfrac;
				CLAMP_PROBABILITY(selec);

				/*
				 * and in fact it's probably a good deal less. We
				 * approximate that all the not-common values share this
				 * remaining fraction equally, so we divide by the number
				 * of other distinct values.
				 */
				otherdistinct = get_variable_numdistinct(&vardata)
					- nnumbers;
				if (otherdistinct > 1)
					selec /= otherdistinct;

				/*
				 * Another cross-check: selectivity shouldn't be estimated
				 * as more than the least common "most common value".
				 */
				if (nnumbers > 0 && selec > numbers[nnumbers - 1])
					selec = numbers[nnumbers - 1];
			}

			free_attstatsslot(vardata.atttype, values, nvalues,
							  numbers, nnumbers);
		}
		else
		{
			double		ndistinct;

			/*
			 * Search is for a value that we do not know a priori, but we
			 * will assume it is not NULL.	Estimate the selectivity as
			 * non-null fraction divided by number of distinct values, so
			 * that we get a result averaged over all possible values
			 * whether common or uncommon.	(Essentially, we are assuming
			 * that the not-yet-known comparison value is equally likely
			 * to be any of the possible values, regardless of their
			 * frequency in the table.	Is that a good idea?)
			 */
			selec = 1.0 - stats->stanullfrac;
			ndistinct = get_variable_numdistinct(&vardata);
			if (ndistinct > 1)
				selec /= ndistinct;

			/*
			 * Cross-check: selectivity should never be estimated as more
			 * than the most common value's.
			 */
			if (get_attstatsslot(vardata.statsTuple,
								 vardata.atttype, vardata.atttypmod,
								 STATISTIC_KIND_MCV, InvalidOid,
								 NULL, NULL,
								 &numbers, &nnumbers))
			{
				if (nnumbers > 0 && selec > numbers[0])
					selec = numbers[0];
				free_attstatsslot(vardata.atttype, NULL, 0, numbers, nnumbers);
			}
		}
	}
	else
	{
		/*
		 * No VACUUM ANALYZE stats available, so make a guess using
		 * estimated number of distinct values and assuming they are
		 * equally common.	(The guess is unlikely to be very good, but we
		 * do know a few special cases.)
		 */
		selec = 1.0 / get_variable_numdistinct(&vardata);
	}

	ReleaseVariableStats(vardata);

	/* result should be in range, but make sure... */
	CLAMP_PROBABILITY(selec);

	PG_RETURN_FLOAT8((float8) selec);
}

/*
 *		neqsel			- Selectivity of "!=" for any data types.
 *
 * This routine is also used for some operators that are not "!="
 * but have comparable selectivity behavior.  See above comments
 * for eqsel().
 */
Datum
neqsel(PG_FUNCTION_ARGS)
{
	Query	   *root = (Query *) PG_GETARG_POINTER(0);
	Oid			operator = PG_GETARG_OID(1);
	List	   *args = (List *) PG_GETARG_POINTER(2);
	int			varRelid = PG_GETARG_INT32(3);
	Oid			eqop;
	float8		result;

	/*
	 * We want 1 - eqsel() where the equality operator is the one
	 * associated with this != operator, that is, its negator.
	 */
	eqop = get_negator(operator);
	if (eqop)
	{
		result = DatumGetFloat8(DirectFunctionCall4(eqsel,
													PointerGetDatum(root),
												  ObjectIdGetDatum(eqop),
													PointerGetDatum(args),
											   Int32GetDatum(varRelid)));
	}
	else
	{
		/* Use default selectivity (should we raise an error instead?) */
		result = DEFAULT_EQ_SEL;
	}
	result = 1.0 - result;
	PG_RETURN_FLOAT8(result);
}

/*
 *	scalarineqsel		- Selectivity of "<", "<=", ">", ">=" for scalars.
 *
 * This is the guts of both scalarltsel and scalargtsel.  The caller has
 * commuted the clause, if necessary, so that we can treat the variable as
 * being on the left.  The caller must also make sure that the other side
 * of the clause is a non-null Const, and dissect same into a value and
 * datatype.
 *
 * This routine works for any datatype (or pair of datatypes) known to
 * convert_to_scalar().  If it is applied to some other datatype,
 * it will return a default estimate.
 */
static double
scalarineqsel(Query *root, Oid operator, bool isgt,
			  VariableStatData *vardata, Datum constval, Oid consttype)
{
	Form_pg_statistic stats;
	FmgrInfo	opproc;
	Datum	   *values;
	int			nvalues;
	float4	   *numbers;
	int			nnumbers;
	double		mcv_selec,
				hist_selec,
				sumcommon;
	double		selec;
	int			i;

	if (!HeapTupleIsValid(vardata->statsTuple))
	{
		/* no stats available, so default result */
		return DEFAULT_INEQ_SEL;
	}
	stats = (Form_pg_statistic) GETSTRUCT(vardata->statsTuple);

	fmgr_info(get_opcode(operator), &opproc);

	/*
	 * If we have most-common-values info, add up the fractions of the MCV
	 * entries that satisfy MCV OP CONST.  These fractions contribute
	 * directly to the result selectivity.	Also add up the total fraction
	 * represented by MCV entries.
	 */
	mcv_selec = 0.0;
	sumcommon = 0.0;

	if (get_attstatsslot(vardata->statsTuple,
						 vardata->atttype, vardata->atttypmod,
						 STATISTIC_KIND_MCV, InvalidOid,
						 &values, &nvalues,
						 &numbers, &nnumbers))
	{
		for (i = 0; i < nvalues; i++)
		{
			if (DatumGetBool(FunctionCall2(&opproc,
										   values[i],
										   constval)))
				mcv_selec += numbers[i];
			sumcommon += numbers[i];
		}
		free_attstatsslot(vardata->atttype, values, nvalues,
						  numbers, nnumbers);
	}

	/*
	 * If there is a histogram, determine which bin the constant falls in,
	 * and compute the resulting contribution to selectivity.
	 *
	 * Someday, VACUUM might store more than one histogram per rel/att,
	 * corresponding to more than one possible sort ordering defined for
	 * the column type.  However, to make that work we will need to figure
	 * out which staop to search for --- it's not necessarily the one we
	 * have at hand!  (For example, we might have a '<=' operator rather
	 * than the '<' operator that will appear in staop.)  For now, assume
	 * that whatever appears in pg_statistic is sorted the same way our
	 * operator sorts, or the reverse way if isgt is TRUE.
	 */
	hist_selec = 0.0;

	if (get_attstatsslot(vardata->statsTuple,
						 vardata->atttype, vardata->atttypmod,
						 STATISTIC_KIND_HISTOGRAM, InvalidOid,
						 &values, &nvalues,
						 NULL, NULL))
	{
		if (nvalues > 1)
		{
			double		histfrac;
			bool		ltcmp;

			ltcmp = DatumGetBool(FunctionCall2(&opproc,
											   values[0],
											   constval));
			if (isgt)
				ltcmp = !ltcmp;
			if (!ltcmp)
			{
				/* Constant is below lower histogram boundary. */
				histfrac = 0.0;
			}
			else
			{
				/*
				 * Scan to find proper location.  This could be made
				 * faster by using a binary-search method, but it's
				 * probably not worth the trouble for typical histogram
				 * sizes.
				 */
				for (i = 1; i < nvalues; i++)
				{
					ltcmp = DatumGetBool(FunctionCall2(&opproc,
													   values[i],
													   constval));
					if (isgt)
						ltcmp = !ltcmp;
					if (!ltcmp)
						break;
				}
				if (i >= nvalues)
				{
					/* Constant is above upper histogram boundary. */
					histfrac = 1.0;
				}
				else
				{
					double		val,
								high,
								low;
					double		binfrac;

					/*
					 * We have values[i-1] < constant < values[i].
					 *
					 * Convert the constant and the two nearest bin boundary
					 * values to a uniform comparison scale, and do a
					 * linear interpolation within this bin.
					 */
					if (convert_to_scalar(constval, consttype, &val,
										  values[i - 1], values[i],
										  vardata->vartype,
										  &low, &high))
					{
						if (high <= low)
						{
							/* cope if bin boundaries appear identical */
							binfrac = 0.5;
						}
						else if (val <= low)
							binfrac = 0.0;
						else if (val >= high)
							binfrac = 1.0;
						else
						{
							binfrac = (val - low) / (high - low);

							/*
							 * Watch out for the possibility that we got a
							 * NaN or Infinity from the division.  This
							 * can happen despite the previous checks, if
							 * for example "low" is -Infinity.
							 */
							if (isnan(binfrac) ||
								binfrac < 0.0 || binfrac > 1.0)
								binfrac = 0.5;
						}
					}
					else
					{
						/*
						 * Ideally we'd produce an error here, on the
						 * grounds that the given operator shouldn't have
						 * scalarXXsel registered as its selectivity func
						 * unless we can deal with its operand types.  But
						 * currently, all manner of stuff is invoking
						 * scalarXXsel, so give a default estimate until
						 * that can be fixed.
						 */
						binfrac = 0.5;
					}

					/*
					 * Now, compute the overall selectivity across the
					 * values represented by the histogram.  We have i-1
					 * full bins and binfrac partial bin below the
					 * constant.
					 */
					histfrac = (double) (i - 1) + binfrac;
					histfrac /= (double) (nvalues - 1);
				}
			}

			/*
			 * Now histfrac = fraction of histogram entries below the
			 * constant.
			 *
			 * Account for "<" vs ">"
			 */
			hist_selec = isgt ? (1.0 - histfrac) : histfrac;

			/*
			 * The histogram boundaries are only approximate to begin
			 * with, and may well be out of date anyway.  Therefore, don't
			 * believe extremely small or large selectivity estimates.
			 */
			if (hist_selec < 0.0001)
				hist_selec = 0.0001;
			else if (hist_selec > 0.9999)
				hist_selec = 0.9999;
		}

		free_attstatsslot(vardata->atttype, values, nvalues, NULL, 0);
	}

	/*
	 * Now merge the results from the MCV and histogram calculations,
	 * realizing that the histogram covers only the non-null values that
	 * are not listed in MCV.
	 */
	selec = 1.0 - stats->stanullfrac - sumcommon;

	if (hist_selec > 0.0)
		selec *= hist_selec;
	else
	{
		/*
		 * If no histogram but there are values not accounted for by MCV,
		 * arbitrarily assume half of them will match.
		 */
		selec *= 0.5;
	}

	selec += mcv_selec;

	/* result should be in range, but make sure... */
	CLAMP_PROBABILITY(selec);

	return selec;
}

/*
 *		scalarltsel		- Selectivity of "<" (also "<=") for scalars.
 */
Datum
scalarltsel(PG_FUNCTION_ARGS)
{
	Query	   *root = (Query *) PG_GETARG_POINTER(0);
	Oid			operator = PG_GETARG_OID(1);
	List	   *args = (List *) PG_GETARG_POINTER(2);
	int			varRelid = PG_GETARG_INT32(3);
	VariableStatData vardata;
	Node	   *other;
	bool		varonleft;
	Datum		constval;
	Oid			consttype;
	bool		isgt;
	double		selec;

	/*
	 * If expression is not variable op something or something op
	 * variable, then punt and return a default estimate.
	 */
	if (!get_restriction_variable(root, args, varRelid,
								  &vardata, &other, &varonleft))
		PG_RETURN_FLOAT8(DEFAULT_INEQ_SEL);

	/*
	 * Can't do anything useful if the something is not a constant,
	 * either.
	 */
	if (!IsA(other, Const))
	{
		ReleaseVariableStats(vardata);
		PG_RETURN_FLOAT8(DEFAULT_INEQ_SEL);
	}

	/*
	 * If the constant is NULL, assume operator is strict and return zero,
	 * ie, operator will never return TRUE.
	 */
	if (((Const *) other)->constisnull)
	{
		ReleaseVariableStats(vardata);
		PG_RETURN_FLOAT8(0.0);
	}
	constval = ((Const *) other)->constvalue;
	consttype = ((Const *) other)->consttype;

	/*
	 * Force the var to be on the left to simplify logic in scalarineqsel.
	 */
	if (varonleft)
	{
		/* we have var < other */
		isgt = false;
	}
	else
	{
		/* we have other < var, commute to make var > other */
		operator = get_commutator(operator);
		if (!operator)
		{
			/* Use default selectivity (should we raise an error instead?) */
			ReleaseVariableStats(vardata);
			PG_RETURN_FLOAT8(DEFAULT_INEQ_SEL);
		}
		isgt = true;
	}

	selec = scalarineqsel(root, operator, isgt, &vardata, constval, consttype);

	ReleaseVariableStats(vardata);

	PG_RETURN_FLOAT8((float8) selec);
}

/*
 *		scalargtsel		- Selectivity of ">" (also ">=") for integers.
 */
Datum
scalargtsel(PG_FUNCTION_ARGS)
{
	Query	   *root = (Query *) PG_GETARG_POINTER(0);
	Oid			operator = PG_GETARG_OID(1);
	List	   *args = (List *) PG_GETARG_POINTER(2);
	int			varRelid = PG_GETARG_INT32(3);
	VariableStatData vardata;
	Node	   *other;
	bool		varonleft;
	Datum		constval;
	Oid			consttype;
	bool		isgt;
	double		selec;

	/*
	 * If expression is not variable op something or something op
	 * variable, then punt and return a default estimate.
	 */
	if (!get_restriction_variable(root, args, varRelid,
								  &vardata, &other, &varonleft))
		PG_RETURN_FLOAT8(DEFAULT_INEQ_SEL);

	/*
	 * Can't do anything useful if the something is not a constant,
	 * either.
	 */
	if (!IsA(other, Const))
	{
		ReleaseVariableStats(vardata);
		PG_RETURN_FLOAT8(DEFAULT_INEQ_SEL);
	}

	/*
	 * If the constant is NULL, assume operator is strict and return zero,
	 * ie, operator will never return TRUE.
	 */
	if (((Const *) other)->constisnull)
	{
		ReleaseVariableStats(vardata);
		PG_RETURN_FLOAT8(0.0);
	}
	constval = ((Const *) other)->constvalue;
	consttype = ((Const *) other)->consttype;

	/*
	 * Force the var to be on the left to simplify logic in scalarineqsel.
	 */
	if (varonleft)
	{
		/* we have var > other */
		isgt = true;
	}
	else
	{
		/* we have other > var, commute to make var < other */
		operator = get_commutator(operator);
		if (!operator)
		{
			/* Use default selectivity (should we raise an error instead?) */
			ReleaseVariableStats(vardata);
			PG_RETURN_FLOAT8(DEFAULT_INEQ_SEL);
		}
		isgt = false;
	}

	selec = scalarineqsel(root, operator, isgt, &vardata, constval, consttype);

	ReleaseVariableStats(vardata);

	PG_RETURN_FLOAT8((float8) selec);
}

/*
 * patternsel			- Generic code for pattern-match selectivity.
 */
static double
patternsel(PG_FUNCTION_ARGS, Pattern_Type ptype)
{
	Query	   *root = (Query *) PG_GETARG_POINTER(0);

#ifdef NOT_USED
	Oid			operator = PG_GETARG_OID(1);
#endif
	List	   *args = (List *) PG_GETARG_POINTER(2);
	int			varRelid = PG_GETARG_INT32(3);
	VariableStatData vardata;
	Node	   *variable;
	Node	   *other;
	bool		varonleft;
	Datum		constval;
	Oid			consttype;
	Oid			vartype;
	Oid			opclass;
	Pattern_Prefix_Status pstatus;
	Const	   *patt = NULL;
	Const	   *prefix = NULL;
	Const	   *rest = NULL;
	double		result;

	/*
	 * If expression is not variable op constant, then punt and return a
	 * default estimate.
	 */
	if (!get_restriction_variable(root, args, varRelid,
								  &vardata, &other, &varonleft))
		return DEFAULT_MATCH_SEL;
	if (!varonleft || !IsA(other, Const))
	{
		ReleaseVariableStats(vardata);
		return DEFAULT_MATCH_SEL;
	}
	variable = (Node *) linitial(args);

	/*
	 * If the constant is NULL, assume operator is strict and return zero,
	 * ie, operator will never return TRUE.
	 */
	if (((Const *) other)->constisnull)
	{
		ReleaseVariableStats(vardata);
		return 0.0;
	}
	constval = ((Const *) other)->constvalue;
	consttype = ((Const *) other)->consttype;

	/*
	 * The right-hand const is type text or bytea for all supported
	 * operators.  We do not expect to see binary-compatible types here,
	 * since const-folding should have relabeled the const to exactly
	 * match the operator's declared type.
	 */
	if (consttype != TEXTOID && consttype != BYTEAOID)
	{
		ReleaseVariableStats(vardata);
		return DEFAULT_MATCH_SEL;
	}

	/*
	 * Similarly, the exposed type of the left-hand side should be one
	 * of those we know.  (Do not look at vardata.atttype, which might be
	 * something binary-compatible but different.)  We can use it to choose
	 * the index opclass from which we must draw the comparison operators.
	 *
	 * NOTE: It would be more correct to use the PATTERN opclasses than the
	 * simple ones, but at the moment ANALYZE will not generate statistics
	 * for the PATTERN operators.  But our results are so approximate
	 * anyway that it probably hardly matters.
	 */
	vartype = vardata.vartype;

	switch (vartype)
	{
		case TEXTOID:
			opclass = TEXT_BTREE_OPS_OID;
			break;
		case VARCHAROID:
			opclass = VARCHAR_BTREE_OPS_OID;
			break;
		case BPCHAROID:
			opclass = BPCHAR_BTREE_OPS_OID;
			break;
		case NAMEOID:
			opclass = NAME_BTREE_OPS_OID;
			break;
		case BYTEAOID:
			opclass = BYTEA_BTREE_OPS_OID;
			break;
		default:
			ReleaseVariableStats(vardata);
			return DEFAULT_MATCH_SEL;
	}

	/* divide pattern into fixed prefix and remainder */
	patt = (Const *) other;
	pstatus = pattern_fixed_prefix(patt, ptype, &prefix, &rest);

	/*
	 * If necessary, coerce the prefix constant to the right type. (The
	 * "rest" constant need not be changed.)
	 */
	if (prefix && prefix->consttype != vartype)
	{
		char	   *prefixstr;

		switch (prefix->consttype)
		{
			case TEXTOID:
				prefixstr = DatumGetCString(DirectFunctionCall1(textout,
													prefix->constvalue));
				break;
			case BYTEAOID:
				prefixstr = DatumGetCString(DirectFunctionCall1(byteaout,
													prefix->constvalue));
				break;
			default:
				elog(ERROR, "unrecognized consttype: %u",
					 prefix->consttype);
				ReleaseVariableStats(vardata);
				return DEFAULT_MATCH_SEL;
		}
		prefix = string_to_const(prefixstr, vartype);
		pfree(prefixstr);
	}

	if (pstatus == Pattern_Prefix_Exact)
	{
		/*
		 * Pattern specifies an exact match, so pretend operator is '='
		 */
		Oid			eqopr = get_opclass_member(opclass, InvalidOid,
											   BTEqualStrategyNumber);
		List	   *eqargs;

		if (eqopr == InvalidOid)
			elog(ERROR, "no = operator for opclass %u", opclass);
		eqargs = list_make2(variable, prefix);
		result = DatumGetFloat8(DirectFunctionCall4(eqsel,
													PointerGetDatum(root),
												 ObjectIdGetDatum(eqopr),
												 PointerGetDatum(eqargs),
											   Int32GetDatum(varRelid)));
	}
	else
	{
		/*
		 * Not exact-match pattern.  We estimate selectivity of the fixed
		 * prefix and remainder of pattern separately, then combine the
		 * two.
		 */
		Selectivity prefixsel;
		Selectivity restsel;
		Selectivity selec;

		if (pstatus == Pattern_Prefix_Partial)
			prefixsel = prefix_selectivity(root, variable, opclass, prefix);
		else
			prefixsel = 1.0;
		restsel = pattern_selectivity(rest, ptype);
		selec = prefixsel * restsel;
		/* result should be in range, but make sure... */
		CLAMP_PROBABILITY(selec);
		result = selec;
	}

	if (prefix)
	{
		pfree(DatumGetPointer(prefix->constvalue));
		pfree(prefix);
	}

	ReleaseVariableStats(vardata);

	return result;
}

/*
 *		regexeqsel		- Selectivity of regular-expression pattern match.
 */
Datum
regexeqsel(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(patternsel(fcinfo, Pattern_Type_Regex));
}

/*
 *		icregexeqsel	- Selectivity of case-insensitive regex match.
 */
Datum
icregexeqsel(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(patternsel(fcinfo, Pattern_Type_Regex_IC));
}

/*
 *		likesel			- Selectivity of LIKE pattern match.
 */
Datum
likesel(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(patternsel(fcinfo, Pattern_Type_Like));
}

/*
 *		iclikesel			- Selectivity of ILIKE pattern match.
 */
Datum
iclikesel(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(patternsel(fcinfo, Pattern_Type_Like_IC));
}

/*
 *		regexnesel		- Selectivity of regular-expression pattern non-match.
 */
Datum
regexnesel(PG_FUNCTION_ARGS)
{
	double		result;

	result = patternsel(fcinfo, Pattern_Type_Regex);
	result = 1.0 - result;
	PG_RETURN_FLOAT8(result);
}

/*
 *		icregexnesel	- Selectivity of case-insensitive regex non-match.
 */
Datum
icregexnesel(PG_FUNCTION_ARGS)
{
	double		result;

	result = patternsel(fcinfo, Pattern_Type_Regex_IC);
	result = 1.0 - result;
	PG_RETURN_FLOAT8(result);
}

/*
 *		nlikesel		- Selectivity of LIKE pattern non-match.
 */
Datum
nlikesel(PG_FUNCTION_ARGS)
{
	double		result;

	result = patternsel(fcinfo, Pattern_Type_Like);
	result = 1.0 - result;
	PG_RETURN_FLOAT8(result);
}

/*
 *		icnlikesel		- Selectivity of ILIKE pattern non-match.
 */
Datum
icnlikesel(PG_FUNCTION_ARGS)
{
	double		result;

	result = patternsel(fcinfo, Pattern_Type_Like_IC);
	result = 1.0 - result;
	PG_RETURN_FLOAT8(result);
}

/*
 *		booltestsel		- Selectivity of BooleanTest Node.
 */
Selectivity
booltestsel(Query *root, BoolTestType booltesttype, Node *arg,
			int varRelid, JoinType jointype)
{
	VariableStatData vardata;
	double		selec;

	examine_variable(root, arg, varRelid, &vardata);

	if (HeapTupleIsValid(vardata.statsTuple))
	{
		Form_pg_statistic stats;
		double		freq_null;
		Datum	   *values;
		int			nvalues;
		float4	   *numbers;
		int			nnumbers;

		stats = (Form_pg_statistic) GETSTRUCT(vardata.statsTuple);
		freq_null = stats->stanullfrac;

		if (get_attstatsslot(vardata.statsTuple,
							 vardata.atttype, vardata.atttypmod,
							 STATISTIC_KIND_MCV, InvalidOid,
							 &values, &nvalues,
							 &numbers, &nnumbers)
			&& nnumbers > 0)
		{
			double		freq_true;
			double		freq_false;

			/*
			 * Get first MCV frequency and derive frequency for true.
			 */
			if (DatumGetBool(values[0]))
				freq_true = numbers[0];
			else
				freq_true = 1.0 - numbers[0] - freq_null;

			/*
			 * Next derive frequency for false. Then use these as
			 * appropriate to derive frequency for each case.
			 */
			freq_false = 1.0 - freq_true - freq_null;

			switch (booltesttype)
			{
				case IS_UNKNOWN:
					/* select only NULL values */
					selec = freq_null;
					break;
				case IS_NOT_UNKNOWN:
					/* select non-NULL values */
					selec = 1.0 - freq_null;
					break;
				case IS_TRUE:
					/* select only TRUE values */
					selec = freq_true;
					break;
				case IS_NOT_TRUE:
					/* select non-TRUE values */
					selec = 1.0 - freq_true;
					break;
				case IS_FALSE:
					/* select only FALSE values */
					selec = freq_false;
					break;
				case IS_NOT_FALSE:
					/* select non-FALSE values */
					selec = 1.0 - freq_false;
					break;
				default:
					elog(ERROR, "unrecognized booltesttype: %d",
						 (int) booltesttype);
					selec = 0.0;	/* Keep compiler quiet */
					break;
			}

			free_attstatsslot(vardata.atttype, values, nvalues,
							  numbers, nnumbers);
		}
		else
		{
			/*
			 * No most-common-value info available. Still have null
			 * fraction information, so use it for IS [NOT] UNKNOWN.
			 * Otherwise adjust for null fraction and assume an even split
			 * for boolean tests.
			 */
			switch (booltesttype)
			{
				case IS_UNKNOWN:

					/*
					 * Use freq_null directly.
					 */
					selec = freq_null;
					break;
				case IS_NOT_UNKNOWN:

					/*
					 * Select not unknown (not null) values. Calculate
					 * from freq_null.
					 */
					selec = 1.0 - freq_null;
					break;
				case IS_TRUE:
				case IS_NOT_TRUE:
				case IS_FALSE:
				case IS_NOT_FALSE:
					selec = (1.0 - freq_null) / 2.0;
					break;
				default:
					elog(ERROR, "unrecognized booltesttype: %d",
						 (int) booltesttype);
					selec = 0.0;	/* Keep compiler quiet */
					break;
			}
		}
	}
	else
	{
		/*
		 * If we can't get variable statistics for the argument, perhaps
		 * clause_selectivity can do something with it.  We ignore the
		 * possibility of a NULL value when using clause_selectivity, and
		 * just assume the value is either TRUE or FALSE.
		 */
		switch (booltesttype)
		{
			case IS_UNKNOWN:
				selec = DEFAULT_UNK_SEL;
				break;
			case IS_NOT_UNKNOWN:
				selec = DEFAULT_NOT_UNK_SEL;
				break;
			case IS_TRUE:
			case IS_NOT_FALSE:
				selec = (double) clause_selectivity(root, arg,
													varRelid, jointype);
				break;
			case IS_FALSE:
			case IS_NOT_TRUE:
				selec = 1.0 - (double) clause_selectivity(root, arg,
													 varRelid, jointype);
				break;
			default:
				elog(ERROR, "unrecognized booltesttype: %d",
					 (int) booltesttype);
				selec = 0.0;	/* Keep compiler quiet */
				break;
		}
	}

	ReleaseVariableStats(vardata);

	/* result should be in range, but make sure... */
	CLAMP_PROBABILITY(selec);

	return (Selectivity) selec;
}

/*
 *		nulltestsel		- Selectivity of NullTest Node.
 */
Selectivity
nulltestsel(Query *root, NullTestType nulltesttype, Node *arg, int varRelid)
{
	VariableStatData vardata;
	double		selec;

	examine_variable(root, arg, varRelid, &vardata);

	if (HeapTupleIsValid(vardata.statsTuple))
	{
		Form_pg_statistic stats;
		double		freq_null;

		stats = (Form_pg_statistic) GETSTRUCT(vardata.statsTuple);
		freq_null = stats->stanullfrac;

		switch (nulltesttype)
		{
			case IS_NULL:

				/*
				 * Use freq_null directly.
				 */
				selec = freq_null;
				break;
			case IS_NOT_NULL:

				/*
				 * Select not unknown (not null) values. Calculate from
				 * freq_null.
				 */
				selec = 1.0 - freq_null;
				break;
			default:
				elog(ERROR, "unrecognized nulltesttype: %d",
					 (int) nulltesttype);
				return (Selectivity) 0; /* keep compiler quiet */
		}
	}
	else
	{
		/*
		 * No VACUUM ANALYZE stats available, so make a guess
		 */
		switch (nulltesttype)
		{
			case IS_NULL:
				selec = DEFAULT_UNK_SEL;
				break;
			case IS_NOT_NULL:
				selec = DEFAULT_NOT_UNK_SEL;
				break;
			default:
				elog(ERROR, "unrecognized nulltesttype: %d",
					 (int) nulltesttype);
				return (Selectivity) 0; /* keep compiler quiet */
		}
	}

	ReleaseVariableStats(vardata);

	/* result should be in range, but make sure... */
	CLAMP_PROBABILITY(selec);

	return (Selectivity) selec;
}

/*
 *		eqjoinsel		- Join selectivity of "="
 */
Datum
eqjoinsel(PG_FUNCTION_ARGS)
{
	Query	   *root = (Query *) PG_GETARG_POINTER(0);
	Oid			operator = PG_GETARG_OID(1);
	List	   *args = (List *) PG_GETARG_POINTER(2);
	JoinType	jointype = (JoinType) PG_GETARG_INT16(3);
	double		selec;
	VariableStatData vardata1;
	VariableStatData vardata2;
	double		nd1;
	double		nd2;
	Form_pg_statistic stats1 = NULL;
	Form_pg_statistic stats2 = NULL;
	bool		have_mcvs1 = false;
	Datum	   *values1 = NULL;
	int			nvalues1 = 0;
	float4	   *numbers1 = NULL;
	int			nnumbers1 = 0;
	bool		have_mcvs2 = false;
	Datum	   *values2 = NULL;
	int			nvalues2 = 0;
	float4	   *numbers2 = NULL;
	int			nnumbers2 = 0;

	get_join_variables(root, args, &vardata1, &vardata2);

	nd1 = get_variable_numdistinct(&vardata1);
	nd2 = get_variable_numdistinct(&vardata2);

	if (HeapTupleIsValid(vardata1.statsTuple))
	{
		stats1 = (Form_pg_statistic) GETSTRUCT(vardata1.statsTuple);
		have_mcvs1 = get_attstatsslot(vardata1.statsTuple,
									  vardata1.atttype,
									  vardata1.atttypmod,
									  STATISTIC_KIND_MCV,
									  InvalidOid,
									  &values1, &nvalues1,
									  &numbers1, &nnumbers1);
	}

	if (HeapTupleIsValid(vardata2.statsTuple))
	{
		stats2 = (Form_pg_statistic) GETSTRUCT(vardata2.statsTuple);
		have_mcvs2 = get_attstatsslot(vardata2.statsTuple,
									  vardata2.atttype,
									  vardata2.atttypmod,
									  STATISTIC_KIND_MCV,
									  InvalidOid,
									  &values2, &nvalues2,
									  &numbers2, &nnumbers2);
	}

	if (have_mcvs1 && have_mcvs2)
	{
		/*
		 * We have most-common-value lists for both relations.	Run
		 * through the lists to see which MCVs actually join to each other
		 * with the given operator.  This allows us to determine the exact
		 * join selectivity for the portion of the relations represented
		 * by the MCV lists.  We still have to estimate for the remaining
		 * population, but in a skewed distribution this gives us a big
		 * leg up in accuracy.	For motivation see the analysis in Y.
		 * Ioannidis and S. Christodoulakis, "On the propagation of errors
		 * in the size of join results", Technical Report 1018, Computer
		 * Science Dept., University of Wisconsin, Madison, March 1991
		 * (available from ftp.cs.wisc.edu).
		 */
		FmgrInfo	eqproc;
		bool	   *hasmatch1;
		bool	   *hasmatch2;
		double		nullfrac1 = stats1->stanullfrac;
		double		nullfrac2 = stats2->stanullfrac;
		double		matchprodfreq,
					matchfreq1,
					matchfreq2,
					unmatchfreq1,
					unmatchfreq2,
					otherfreq1,
					otherfreq2,
					totalsel1,
					totalsel2;
		int			i,
					nmatches;

		fmgr_info(get_opcode(operator), &eqproc);
		hasmatch1 = (bool *) palloc0(nvalues1 * sizeof(bool));
		hasmatch2 = (bool *) palloc0(nvalues2 * sizeof(bool));

		/*
		 * If we are doing any variant of JOIN_IN, pretend all the values
		 * of the righthand relation are unique (ie, act as if it's been
		 * DISTINCT'd).
		 *
		 * NOTE: it might seem that we should unique-ify the lefthand input
		 * when considering JOIN_REVERSE_IN.  But this is not so, because
		 * the join clause we've been handed has not been commuted from
		 * the way the parser originally wrote it.	We know that the
		 * unique side of the IN clause is *always* on the right.
		 *
		 * NOTE: it would be dangerous to try to be smart about JOIN_LEFT or
		 * JOIN_RIGHT here, because we do not have enough information to
		 * determine which var is really on which side of the join.
		 * Perhaps someday we should pass in more information.
		 */
		if (jointype == JOIN_IN ||
			jointype == JOIN_REVERSE_IN ||
			jointype == JOIN_UNIQUE_INNER ||
			jointype == JOIN_UNIQUE_OUTER)
		{
			float4		oneovern = 1.0 / nd2;

			for (i = 0; i < nvalues2; i++)
				numbers2[i] = oneovern;
			nullfrac2 = oneovern;
		}

		/*
		 * Note we assume that each MCV will match at most one member of
		 * the other MCV list.	If the operator isn't really equality,
		 * there could be multiple matches --- but we don't look for them,
		 * both for speed and because the math wouldn't add up...
		 */
		matchprodfreq = 0.0;
		nmatches = 0;
		for (i = 0; i < nvalues1; i++)
		{
			int			j;

			for (j = 0; j < nvalues2; j++)
			{
				if (hasmatch2[j])
					continue;
				if (DatumGetBool(FunctionCall2(&eqproc,
											   values1[i],
											   values2[j])))
				{
					hasmatch1[i] = hasmatch2[j] = true;
					matchprodfreq += numbers1[i] * numbers2[j];
					nmatches++;
					break;
				}
			}
		}
		CLAMP_PROBABILITY(matchprodfreq);
		/* Sum up frequencies of matched and unmatched MCVs */
		matchfreq1 = unmatchfreq1 = 0.0;
		for (i = 0; i < nvalues1; i++)
		{
			if (hasmatch1[i])
				matchfreq1 += numbers1[i];
			else
				unmatchfreq1 += numbers1[i];
		}
		CLAMP_PROBABILITY(matchfreq1);
		CLAMP_PROBABILITY(unmatchfreq1);
		matchfreq2 = unmatchfreq2 = 0.0;
		for (i = 0; i < nvalues2; i++)
		{
			if (hasmatch2[i])
				matchfreq2 += numbers2[i];
			else
				unmatchfreq2 += numbers2[i];
		}
		CLAMP_PROBABILITY(matchfreq2);
		CLAMP_PROBABILITY(unmatchfreq2);
		pfree(hasmatch1);
		pfree(hasmatch2);

		/*
		 * Compute total frequency of non-null values that are not in the
		 * MCV lists.
		 */
		otherfreq1 = 1.0 - nullfrac1 - matchfreq1 - unmatchfreq1;
		otherfreq2 = 1.0 - nullfrac2 - matchfreq2 - unmatchfreq2;
		CLAMP_PROBABILITY(otherfreq1);
		CLAMP_PROBABILITY(otherfreq2);

		/*
		 * We can estimate the total selectivity from the point of view of
		 * relation 1 as: the known selectivity for matched MCVs, plus
		 * unmatched MCVs that are assumed to match against random members
		 * of relation 2's non-MCV population, plus non-MCV values that
		 * are assumed to match against random members of relation 2's
		 * unmatched MCVs plus non-MCV values.
		 */
		totalsel1 = matchprodfreq;
		if (nd2 > nvalues2)
			totalsel1 += unmatchfreq1 * otherfreq2 / (nd2 - nvalues2);
		if (nd2 > nmatches)
			totalsel1 += otherfreq1 * (otherfreq2 + unmatchfreq2) /
				(nd2 - nmatches);
		/* Same estimate from the point of view of relation 2. */
		totalsel2 = matchprodfreq;
		if (nd1 > nvalues1)
			totalsel2 += unmatchfreq2 * otherfreq1 / (nd1 - nvalues1);
		if (nd1 > nmatches)
			totalsel2 += otherfreq2 * (otherfreq1 + unmatchfreq1) /
				(nd1 - nmatches);

		/*
		 * Use the smaller of the two estimates.  This can be justified in
		 * essentially the same terms as given below for the no-stats
		 * case: to a first approximation, we are estimating from the
		 * point of view of the relation with smaller nd.
		 */
		selec = (totalsel1 < totalsel2) ? totalsel1 : totalsel2;
	}
	else
	{
		/*
		 * We do not have MCV lists for both sides.  Estimate the join
		 * selectivity as MIN(1/nd1,1/nd2)*(1-nullfrac1)*(1-nullfrac2).
		 * This is plausible if we assume that the join operator is strict
		 * and the non-null values are about equally distributed: a given
		 * non-null tuple of rel1 will join to either zero or
		 * N2*(1-nullfrac2)/nd2 rows of rel2, so total join rows are at
		 * most N1*(1-nullfrac1)*N2*(1-nullfrac2)/nd2 giving a join
		 * selectivity of not more than (1-nullfrac1)*(1-nullfrac2)/nd2.
		 * By the same logic it is not more than
		 * (1-nullfrac1)*(1-nullfrac2)/nd1, so the expression with MIN()
		 * is an upper bound.  Using the MIN() means we estimate from the
		 * point of view of the relation with smaller nd (since the larger
		 * nd is determining the MIN).	It is reasonable to assume that
		 * most tuples in this rel will have join partners, so the bound
		 * is probably reasonably tight and should be taken as-is.
		 *
		 * XXX Can we be smarter if we have an MCV list for just one side? It
		 * seems that if we assume equal distribution for the other side,
		 * we end up with the same answer anyway.
		 */
		double		nullfrac1 = stats1 ? stats1->stanullfrac : 0.0;
		double		nullfrac2 = stats2 ? stats2->stanullfrac : 0.0;

		selec = (1.0 - nullfrac1) * (1.0 - nullfrac2);
		if (nd1 > nd2)
			selec /= nd1;
		else
			selec /= nd2;
	}

	if (have_mcvs1)
		free_attstatsslot(vardata1.atttype, values1, nvalues1,
						  numbers1, nnumbers1);
	if (have_mcvs2)
		free_attstatsslot(vardata2.atttype, values2, nvalues2,
						  numbers2, nnumbers2);

	ReleaseVariableStats(vardata1);
	ReleaseVariableStats(vardata2);

	CLAMP_PROBABILITY(selec);

	PG_RETURN_FLOAT8((float8) selec);
}

/*
 *		neqjoinsel		- Join selectivity of "!="
 */
Datum
neqjoinsel(PG_FUNCTION_ARGS)
{
	Query	   *root = (Query *) PG_GETARG_POINTER(0);
	Oid			operator = PG_GETARG_OID(1);
	List	   *args = (List *) PG_GETARG_POINTER(2);
	JoinType	jointype = (JoinType) PG_GETARG_INT16(3);
	Oid			eqop;
	float8		result;

	/*
	 * We want 1 - eqjoinsel() where the equality operator is the one
	 * associated with this != operator, that is, its negator.
	 */
	eqop = get_negator(operator);
	if (eqop)
	{
		result = DatumGetFloat8(DirectFunctionCall4(eqjoinsel,
													PointerGetDatum(root),
												  ObjectIdGetDatum(eqop),
													PointerGetDatum(args),
											   Int16GetDatum(jointype)));
	}
	else
	{
		/* Use default selectivity (should we raise an error instead?) */
		result = DEFAULT_EQ_SEL;
	}
	result = 1.0 - result;
	PG_RETURN_FLOAT8(result);
}

/*
 *		scalarltjoinsel - Join selectivity of "<" and "<=" for scalars
 */
Datum
scalarltjoinsel(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(DEFAULT_INEQ_SEL);
}

/*
 *		scalargtjoinsel - Join selectivity of ">" and ">=" for scalars
 */
Datum
scalargtjoinsel(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(DEFAULT_INEQ_SEL);
}

/*
 *		regexeqjoinsel	- Join selectivity of regular-expression pattern match.
 */
Datum
regexeqjoinsel(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(DEFAULT_MATCH_SEL);
}

/*
 *		icregexeqjoinsel	- Join selectivity of case-insensitive regex match.
 */
Datum
icregexeqjoinsel(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(DEFAULT_MATCH_SEL);
}

/*
 *		likejoinsel			- Join selectivity of LIKE pattern match.
 */
Datum
likejoinsel(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(DEFAULT_MATCH_SEL);
}

/*
 *		iclikejoinsel			- Join selectivity of ILIKE pattern match.
 */
Datum
iclikejoinsel(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(DEFAULT_MATCH_SEL);
}

/*
 *		regexnejoinsel	- Join selectivity of regex non-match.
 */
Datum
regexnejoinsel(PG_FUNCTION_ARGS)
{
	float8		result;

	result = DatumGetFloat8(regexeqjoinsel(fcinfo));
	result = 1.0 - result;
	PG_RETURN_FLOAT8(result);
}

/*
 *		icregexnejoinsel	- Join selectivity of case-insensitive regex non-match.
 */
Datum
icregexnejoinsel(PG_FUNCTION_ARGS)
{
	float8		result;

	result = DatumGetFloat8(icregexeqjoinsel(fcinfo));
	result = 1.0 - result;
	PG_RETURN_FLOAT8(result);
}

/*
 *		nlikejoinsel		- Join selectivity of LIKE pattern non-match.
 */
Datum
nlikejoinsel(PG_FUNCTION_ARGS)
{
	float8		result;

	result = DatumGetFloat8(likejoinsel(fcinfo));
	result = 1.0 - result;
	PG_RETURN_FLOAT8(result);
}

/*
 *		icnlikejoinsel		- Join selectivity of ILIKE pattern non-match.
 */
Datum
icnlikejoinsel(PG_FUNCTION_ARGS)
{
	float8		result;

	result = DatumGetFloat8(iclikejoinsel(fcinfo));
	result = 1.0 - result;
	PG_RETURN_FLOAT8(result);
}

/*
 * mergejoinscansel			- Scan selectivity of merge join.
 *
 * A merge join will stop as soon as it exhausts either input stream.
 * Therefore, if we can estimate the ranges of both input variables,
 * we can estimate how much of the input will actually be read.  This
 * can have a considerable impact on the cost when using indexscans.
 *
 * clause should be a clause already known to be mergejoinable.
 *
 * *leftscan is set to the fraction of the left-hand variable expected
 * to be scanned (0 to 1), and similarly *rightscan for the right-hand
 * variable.
 */
void
mergejoinscansel(Query *root, Node *clause,
				 Selectivity *leftscan,
				 Selectivity *rightscan)
{
	Node	   *left,
			   *right;
	VariableStatData leftvar,
				rightvar;
	Oid			lefttype,
				righttype;
	Oid			opno,
				lsortop,
				rsortop,
				ltop,
				gtop,
				leop,
				revgtop,
				revleop;
	Datum		leftmax,
				rightmax;
	double		selec;

	/* Set default results if we can't figure anything out. */
	*leftscan = *rightscan = 1.0;

	/* Deconstruct the merge clause */
	if (!is_opclause(clause))
		return;					/* shouldn't happen */
	opno = ((OpExpr *) clause)->opno;
	left = get_leftop((Expr *) clause);
	right = get_rightop((Expr *) clause);
	if (!right)
		return;					/* shouldn't happen */

	/* Look for stats for the inputs */
	examine_variable(root, left, 0, &leftvar);
	examine_variable(root, right, 0, &rightvar);

	/* Get the direct input types of the operator */
	lefttype = exprType(left);
	righttype = exprType(right);

	/* Verify mergejoinability and get left and right "<" operators */
	if (!op_mergejoinable(opno,
						  &lsortop,
						  &rsortop))
		goto fail;				/* shouldn't happen */

	/* Try to get maximum values of both inputs */
	if (!get_variable_maximum(root, &leftvar, lsortop, &leftmax))
		goto fail;				/* no max available from stats */

	if (!get_variable_maximum(root, &rightvar, rsortop, &rightmax))
		goto fail;				/* no max available from stats */

	/* Look up the "left < right" and "left > right" operators */
	op_mergejoin_crossops(opno, &ltop, &gtop, NULL, NULL);

	/* Look up the "left <= right" operator */
	leop = get_negator(gtop);
	if (!OidIsValid(leop))
		goto fail;				/* insufficient info in catalogs */

	/* Look up the "right > left" operator */
	revgtop = get_commutator(ltop);
	if (!OidIsValid(revgtop))
		goto fail;				/* insufficient info in catalogs */

	/* Look up the "right <= left" operator */
	revleop = get_negator(revgtop);
	if (!OidIsValid(revleop))
		goto fail;				/* insufficient info in catalogs */

	/*
	 * Now, the fraction of the left variable that will be scanned is the
	 * fraction that's <= the right-side maximum value.  But only believe
	 * non-default estimates, else stick with our 1.0.
	 */
	selec = scalarineqsel(root, leop, false, &leftvar,
						  rightmax, righttype);
	if (selec != DEFAULT_INEQ_SEL)
		*leftscan = selec;

	/* And similarly for the right variable. */
	selec = scalarineqsel(root, revleop, false, &rightvar,
						  leftmax, lefttype);
	if (selec != DEFAULT_INEQ_SEL)
		*rightscan = selec;

	/*
	 * Only one of the two fractions can really be less than 1.0; believe
	 * the smaller estimate and reset the other one to exactly 1.0.  If we
	 * get exactly equal estimates (as can easily happen with self-joins),
	 * believe neither.
	 */
	if (*leftscan > *rightscan)
		*leftscan = 1.0;
	else if (*leftscan < *rightscan)
		*rightscan = 1.0;
	else
		*leftscan = *rightscan = 1.0;

fail:
	ReleaseVariableStats(leftvar);
	ReleaseVariableStats(rightvar);
}


/*
 * Helper routine for estimate_num_groups: add an item to a list of
 * GroupVarInfos, but only if it's not known equal to any of the existing
 * entries.
 */
typedef struct
{
	Node	   *var;		/* might be an expression, not just a Var */
	RelOptInfo *rel;		/* relation it belongs to */
	double		ndistinct;	/* # distinct values */
} GroupVarInfo;

static List *
add_unique_group_var(Query *root, List *varinfos,
					 Node *var, VariableStatData *vardata)
{
	GroupVarInfo *varinfo;
	double		ndistinct;
	ListCell   *lc;

	ndistinct = get_variable_numdistinct(vardata);

	/* cannot use foreach here because of possible list_delete */
	lc = list_head(varinfos);
	while (lc)
	{
		varinfo = (GroupVarInfo *) lfirst(lc);

		/* must advance lc before list_delete possibly pfree's it */
		lc = lnext(lc);

		/* Drop exact duplicates */
		if (equal(var, varinfo->var))
			return varinfos;

		/*
		 * Drop known-equal vars, but only if they belong to different
		 * relations (see comments for estimate_num_groups)
		 */
		if (vardata->rel != varinfo->rel &&
			exprs_known_equal(root, var, varinfo->var))
		{
			if (varinfo->ndistinct <= ndistinct)
			{
				/* Keep older item, forget new one */
				return varinfos;
			}
			else
			{
				/* Delete the older item */
				varinfos = list_delete_ptr(varinfos, varinfo);
			}
		}
	}

	varinfo = (GroupVarInfo *) palloc(sizeof(GroupVarInfo));

	varinfo->var = var;
	varinfo->rel = vardata->rel;
	varinfo->ndistinct = ndistinct;
	varinfos = lappend(varinfos, varinfo);
	return varinfos;
}

/*
 * estimate_num_groups		- Estimate number of groups in a grouped query
 *
 * Given a query having a GROUP BY clause, estimate how many groups there
 * will be --- ie, the number of distinct combinations of the GROUP BY
 * expressions.
 *
 * This routine is also used to estimate the number of rows emitted by
 * a DISTINCT filtering step; that is an isomorphic problem.  (Note:
 * actually, we only use it for DISTINCT when there's no grouping or
 * aggregation ahead of the DISTINCT.)
 *
 * Inputs:
 *	root - the query
 *	groupExprs - list of expressions being grouped by
 *	input_rows - number of rows estimated to arrive at the group/unique
 *		filter step
 *
 * Given the lack of any cross-correlation statistics in the system, it's
 * impossible to do anything really trustworthy with GROUP BY conditions
 * involving multiple Vars.  We should however avoid assuming the worst
 * case (all possible cross-product terms actually appear as groups) since
 * very often the grouped-by Vars are highly correlated.  Our current approach
 * is as follows:
 *	1.	Expressions yielding boolean are assumed to contribute two groups,
 *		independently of their content, and are ignored in the subsequent
 *		steps.  This is mainly because tests like "col IS NULL" break the
 *		heuristic used in step 2 especially badly.
 *	2.	Reduce the given expressions to a list of unique Vars used.  For
 *		example, GROUP BY a, a + b is treated the same as GROUP BY a, b.
 *		It is clearly correct not to count the same Var more than once.
 *		It is also reasonable to treat f(x) the same as x: f() cannot
 *		increase the number of distinct values (unless it is volatile,
 *		which we consider unlikely for grouping), but it probably won't
 *		reduce the number of distinct values much either.
 *		As a special case, if a GROUP BY expression can be matched to an
 *		expressional index for which we have statistics, then we treat the
 *		whole expression as though it were just a Var.
 *	3.	If the list contains Vars of different relations that are known equal
 *		due to equijoin clauses, then drop all but one of the Vars from each
 *		known-equal set, keeping the one with smallest estimated # of values
 *		(since the extra values of the others can't appear in joined rows).
 *		Note the reason we only consider Vars of different relations is that
 *		if we considered ones of the same rel, we'd be double-counting the
 *		restriction selectivity of the equality in the next step.
 *	4.	For Vars within a single source rel, we multiply together the numbers
 *		of values, clamp to the number of rows in the rel (divided by 10 if
 *		more than one Var), and then multiply by the selectivity of the
 *		restriction clauses for that rel.  When there's more than one Var,
 *		the initial product is probably too high (it's the worst case) but
 *		clamping to a fraction of the rel's rows seems to be a helpful
 *		heuristic for not letting the estimate get out of hand.  (The factor
 *		of 10 is derived from pre-Postgres-7.4 practice.)  Multiplying
 *		by the restriction selectivity is effectively assuming that the
 *		restriction clauses are independent of the grouping, which is a crummy
 *		assumption, but it's hard to do better.
 *	5.	If there are Vars from multiple rels, we repeat step 4 for each such
 *		rel, and multiply the results together.
 * Note that rels not containing grouped Vars are ignored completely, as are
 * join clauses other than the equijoin clauses used in step 3.  Such rels
 * cannot increase the number of groups, and we assume such clauses do not
 * reduce the number either (somewhat bogus, but we don't have the info to
 * do better).
 */
double
estimate_num_groups(Query *root, List *groupExprs, double input_rows)
{
	List	   *varinfos = NIL;
	double		numdistinct;
	ListCell   *l;

	/* We should not be called unless query has GROUP BY (or DISTINCT) */
	Assert(groupExprs != NIL);

	/*
	 * Count groups derived from boolean grouping expressions.  For other
	 * expressions, find the unique Vars used, treating an expression as a Var
	 * if we can find stats for it.  For each one, record the statistical
	 * estimate of number of distinct values (total in its table, without
	 * regard for filtering).
	 */
	numdistinct = 1.0;

	foreach(l, groupExprs)
	{
		Node	   *groupexpr = (Node *) lfirst(l);
		VariableStatData vardata;
		List	   *varshere;
		ListCell   *l2;

		/* Short-circuit for expressions returning boolean */
		if (exprType(groupexpr) == BOOLOID)
		{
			numdistinct *= 2.0;
			continue;
		}

		/*
		 * If examine_variable is able to deduce anything about the GROUP BY
		 * expression, treat it as a single variable even if it's really more
		 * complicated.
		 */
		examine_variable(root, groupexpr, 0, &vardata);
		if (vardata.statsTuple != NULL || vardata.isunique)
		{
			varinfos = add_unique_group_var(root, varinfos,
											groupexpr, &vardata);
			ReleaseVariableStats(vardata);
			continue;
		}
		ReleaseVariableStats(vardata);

		/*
		 * Else pull out the component Vars
		 */
		varshere = pull_var_clause(groupexpr, false);

		/*
		 * If we find any variable-free GROUP BY item, then either it is a
		 * constant (and we can ignore it) or it contains a volatile
		 * function; in the latter case we punt and assume that each input
		 * row will yield a distinct group.
		 */
		if (varshere == NIL)
		{
			if (contain_volatile_functions(groupexpr))
				return input_rows;
			continue;
		}

		/*
		 * Else add variables to varinfos list
		 */
		foreach(l2, varshere)
		{
			Node	   *var = (Node *) lfirst(l2);

			examine_variable(root, var, 0, &vardata);
			varinfos = add_unique_group_var(root, varinfos, var, &vardata);
			ReleaseVariableStats(vardata);
		}
	}

	/*
	 * If now no Vars, we must have an all-constant or all-boolean GROUP BY
	 * list.
	 */
	if (varinfos == NIL)
	{
		/* Guard against out-of-range answers */
		if (numdistinct > input_rows)
			numdistinct = input_rows;
		return numdistinct;
	}

	/*
	 * Group Vars by relation and estimate total numdistinct.
	 *
	 * For each iteration of the outer loop, we process the frontmost Var in
	 * varinfos, plus all other Vars in the same relation.	We remove
	 * these Vars from the newvarinfos list for the next iteration. This
	 * is the easiest way to group Vars of same rel together.
	 */
	do
	{
		GroupVarInfo *varinfo1 = (GroupVarInfo *) linitial(varinfos);
		RelOptInfo *rel = varinfo1->rel;
		double		reldistinct = varinfo1->ndistinct;
		double		relmaxndistinct = reldistinct;
		int			relvarcount = 1;
		List	   *newvarinfos = NIL;

		/*
		 * Get the product of numdistinct estimates of the Vars for this rel.
		 * Also, construct new varinfos list of remaining Vars.
		 */
		for_each_cell(l, lnext(list_head(varinfos)))
		{
			GroupVarInfo *varinfo2 = (GroupVarInfo *) lfirst(l);

			if (varinfo2->rel == varinfo1->rel)
			{
				reldistinct *= varinfo2->ndistinct;
				if (relmaxndistinct < varinfo2->ndistinct)
					relmaxndistinct = varinfo2->ndistinct;
				relvarcount++;
			}
			else
			{
				/* not time to process varinfo2 yet */
				newvarinfos = lcons(varinfo2, newvarinfos);
			}
		}

		/*
		 * Sanity check --- don't divide by zero if empty relation.
		 */
		Assert(rel->reloptkind == RELOPT_BASEREL);
		if (rel->tuples > 0)
		{
			/*
			 * Clamp to size of rel, or size of rel / 10 if multiple Vars.
			 * The fudge factor is because the Vars are probably correlated
			 * but we don't know by how much.  We should never clamp to less
			 * than the largest ndistinct value for any of the Vars, though,
			 * since there will surely be at least that many groups.
			 */
			double		clamp = rel->tuples;

			if (relvarcount > 1)
			{
				clamp *= 0.1;
				if (clamp < relmaxndistinct)
				{
					clamp = relmaxndistinct;
					/* for sanity in case some ndistinct is too large: */
					if (clamp > rel->tuples)
						clamp = rel->tuples;
				}
			}
			if (reldistinct > clamp)
				reldistinct = clamp;

			/*
			 * Multiply by restriction selectivity.
			 */
			reldistinct *= rel->rows / rel->tuples;

			/*
			 * Update estimate of total distinct groups.
			 */
			numdistinct *= reldistinct;
		}

		varinfos = newvarinfos;
	} while (varinfos != NIL);

	numdistinct = ceil(numdistinct);

	/* Guard against out-of-range answers */
	if (numdistinct > input_rows)
		numdistinct = input_rows;
	if (numdistinct < 1.0)
		numdistinct = 1.0;

	return numdistinct;
}

/*
 * Estimate hash bucketsize fraction (ie, number of entries in a bucket
 * divided by total tuples in relation) if the specified expression is used
 * as a hash key.
 *
 * XXX This is really pretty bogus since we're effectively assuming that the
 * distribution of hash keys will be the same after applying restriction
 * clauses as it was in the underlying relation.  However, we are not nearly
 * smart enough to figure out how the restrict clauses might change the
 * distribution, so this will have to do for now.
 *
 * We are passed the number of buckets the executor will use for the given
 * input relation.	If the data were perfectly distributed, with the same
 * number of tuples going into each available bucket, then the bucketsize
 * fraction would be 1/nbuckets.  But this happy state of affairs will occur
 * only if (a) there are at least nbuckets distinct data values, and (b)
 * we have a not-too-skewed data distribution.	Otherwise the buckets will
 * be nonuniformly occupied.  If the other relation in the join has a key
 * distribution similar to this one's, then the most-loaded buckets are
 * exactly those that will be probed most often.  Therefore, the "average"
 * bucket size for costing purposes should really be taken as something close
 * to the "worst case" bucket size.  We try to estimate this by adjusting the
 * fraction if there are too few distinct data values, and then scaling up
 * by the ratio of the most common value's frequency to the average frequency.
 *
 * If no statistics are available, use a default estimate of 0.1.  This will
 * discourage use of a hash rather strongly if the inner relation is large,
 * which is what we want.  We do not want to hash unless we know that the
 * inner rel is well-dispersed (or the alternatives seem much worse).
 */
Selectivity
estimate_hash_bucketsize(Query *root, Node *hashkey, int nbuckets)
{
	VariableStatData vardata;
	double		estfract,
				ndistinct,
				stanullfrac,
				mcvfreq,
				avgfreq;
	float4	   *numbers;
	int			nnumbers;

	examine_variable(root, hashkey, 0, &vardata);

	/* Get number of distinct values and fraction that are null */
	ndistinct = get_variable_numdistinct(&vardata);

	if (HeapTupleIsValid(vardata.statsTuple))
	{
		Form_pg_statistic stats;

		stats = (Form_pg_statistic) GETSTRUCT(vardata.statsTuple);
		stanullfrac = stats->stanullfrac;
	}
	else
	{
		/*
		 * Believe a default ndistinct only if it came from stats.
		 * Otherwise punt and return 0.1, per comments above.
		 */
		if (ndistinct == DEFAULT_NUM_DISTINCT)
		{
			ReleaseVariableStats(vardata);
			return (Selectivity) 0.1;
		}

		stanullfrac = 0.0;
	}

	/* Compute avg freq of all distinct data values in raw relation */
	avgfreq = (1.0 - stanullfrac) / ndistinct;

	/*
	 * Adjust ndistinct to account for restriction clauses.  Observe we
	 * are assuming that the data distribution is affected uniformly by
	 * the restriction clauses!
	 *
	 * XXX Possibly better way, but much more expensive: multiply by
	 * selectivity of rel's restriction clauses that mention the target
	 * Var.
	 */
	if (vardata.rel)
		ndistinct *= vardata.rel->rows / vardata.rel->tuples;

	/*
	 * Initial estimate of bucketsize fraction is 1/nbuckets as long as
	 * the number of buckets is less than the expected number of distinct
	 * values; otherwise it is 1/ndistinct.
	 */
	if (ndistinct > (double) nbuckets)
		estfract = 1.0 / (double) nbuckets;
	else
		estfract = 1.0 / ndistinct;

	/*
	 * Look up the frequency of the most common value, if available.
	 */
	mcvfreq = 0.0;

	if (HeapTupleIsValid(vardata.statsTuple))
	{
		if (get_attstatsslot(vardata.statsTuple,
							 vardata.atttype, vardata.atttypmod,
							 STATISTIC_KIND_MCV, InvalidOid,
							 NULL, NULL, &numbers, &nnumbers))
		{
			/*
			 * The first MCV stat is for the most common value.
			 */
			if (nnumbers > 0)
				mcvfreq = numbers[0];
			free_attstatsslot(vardata.atttype, NULL, 0,
							  numbers, nnumbers);
		}
	}

	/*
	 * Adjust estimated bucketsize upward to account for skewed
	 * distribution.
	 */
	if (avgfreq > 0.0 && mcvfreq > avgfreq)
		estfract *= mcvfreq / avgfreq;

	/*
	 * Clamp bucketsize to sane range (the above adjustment could easily
	 * produce an out-of-range result).  We set the lower bound a little
	 * above zero, since zero isn't a very sane result.
	 */
	if (estfract < 1.0e-6)
		estfract = 1.0e-6;
	else if (estfract > 1.0)
		estfract = 1.0;

	ReleaseVariableStats(vardata);

	return (Selectivity) estfract;
}


/*-------------------------------------------------------------------------
 *
 * Support routines
 *
 *-------------------------------------------------------------------------
 */

/*
 * convert_to_scalar
 *	  Convert non-NULL values of the indicated types to the comparison
 *	  scale needed by scalarltsel()/scalargtsel().
 *	  Returns "true" if successful.
 *
 * XXX this routine is a hack: ideally we should look up the conversion
 * subroutines in pg_type.
 *
 * All numeric datatypes are simply converted to their equivalent
 * "double" values.  (NUMERIC values that are outside the range of "double"
 * are clamped to +/- HUGE_VAL.)
 *
 * String datatypes are converted by convert_string_to_scalar(),
 * which is explained below.  The reason why this routine deals with
 * three values at a time, not just one, is that we need it for strings.
 *
 * The bytea datatype is just enough different from strings that it has
 * to be treated separately.
 *
 * The several datatypes representing absolute times are all converted
 * to Timestamp, which is actually a double, and then we just use that
 * double value.  Note this will give correct results even for the "special"
 * values of Timestamp, since those are chosen to compare correctly;
 * see timestamp_cmp.
 *
 * The several datatypes representing relative times (intervals) are all
 * converted to measurements expressed in seconds.
 */
static bool
convert_to_scalar(Datum value, Oid valuetypid, double *scaledvalue,
				  Datum lobound, Datum hibound, Oid boundstypid,
				  double *scaledlobound, double *scaledhibound)
{
	/*
	 * Both the valuetypid and the boundstypid should exactly match
	 * the declared input type(s) of the operator we are invoked for,
	 * so we just error out if either is not recognized.
	 *
	 * XXX The histogram we are interpolating between points of could belong
	 * to a column that's only binary-compatible with the declared type.
	 * In essence we are assuming that the semantics of binary-compatible
	 * types are enough alike that we can use a histogram generated with one
	 * type's operators to estimate selectivity for the other's.  This is
	 * outright wrong in some cases --- in particular signed versus unsigned
	 * interpretation could trip us up.  But it's useful enough in the
	 * majority of cases that we do it anyway.  Should think about more
	 * rigorous ways to do it.
	 */
	switch (valuetypid)
	{
			/*
			 * Built-in numeric types
			 */
		case BOOLOID:
		case INT2OID:
		case INT4OID:
		case INT8OID:
		case FLOAT4OID:
		case FLOAT8OID:
		case NUMERICOID:
		case OIDOID:
		case REGPROCOID:
		case REGPROCEDUREOID:
		case REGOPEROID:
		case REGOPERATOROID:
		case REGCLASSOID:
		case REGTYPEOID:
			*scaledvalue = convert_numeric_to_scalar(value, valuetypid);
			*scaledlobound = convert_numeric_to_scalar(lobound, boundstypid);
			*scaledhibound = convert_numeric_to_scalar(hibound, boundstypid);
			return true;

			/*
			 * Built-in string types
			 */
		case CHAROID:
		case BPCHAROID:
		case VARCHAROID:
		case TEXTOID:
		case NAMEOID:
			{
				unsigned char *valstr = convert_string_datum(value, valuetypid);
				unsigned char *lostr = convert_string_datum(lobound, boundstypid);
				unsigned char *histr = convert_string_datum(hibound, boundstypid);

				convert_string_to_scalar(valstr, scaledvalue,
										 lostr, scaledlobound,
										 histr, scaledhibound);
				pfree(valstr);
				pfree(lostr);
				pfree(histr);
				return true;
			}

			/*
			 * Built-in bytea type
			 */
		case BYTEAOID:
			{
				convert_bytea_to_scalar(value, scaledvalue,
										lobound, scaledlobound,
										hibound, scaledhibound);
				return true;
			}

			/*
			 * Built-in time types
			 */
		case TIMESTAMPOID:
		case TIMESTAMPTZOID:
		case ABSTIMEOID:
		case DATEOID:
		case INTERVALOID:
		case RELTIMEOID:
		case TINTERVALOID:
		case TIMEOID:
		case TIMETZOID:
			*scaledvalue = convert_timevalue_to_scalar(value, valuetypid);
			*scaledlobound = convert_timevalue_to_scalar(lobound, boundstypid);
			*scaledhibound = convert_timevalue_to_scalar(hibound, boundstypid);
			return true;

			/*
			 * Built-in network types
			 */
		case INETOID:
		case CIDROID:
		case MACADDROID:
			*scaledvalue = convert_network_to_scalar(value, valuetypid);
			*scaledlobound = convert_network_to_scalar(lobound, boundstypid);
			*scaledhibound = convert_network_to_scalar(hibound, boundstypid);
			return true;
	}
	/* Don't know how to convert */
	return false;
}

/*
 * Do convert_to_scalar()'s work for any numeric data type.
 */
static double
convert_numeric_to_scalar(Datum value, Oid typid)
{
	switch (typid)
	{
		case BOOLOID:
			return (double) DatumGetBool(value);
		case INT2OID:
			return (double) DatumGetInt16(value);
		case INT4OID:
			return (double) DatumGetInt32(value);
		case INT8OID:
			return (double) DatumGetInt64(value);
		case FLOAT4OID:
			return (double) DatumGetFloat4(value);
		case FLOAT8OID:
			return (double) DatumGetFloat8(value);
		case NUMERICOID:
			/* Note: out-of-range values will be clamped to +-HUGE_VAL */
			return (double)
				DatumGetFloat8(DirectFunctionCall1(numeric_float8_no_overflow,
												   value));
		case OIDOID:
		case REGPROCOID:
		case REGPROCEDUREOID:
		case REGOPEROID:
		case REGOPERATOROID:
		case REGCLASSOID:
		case REGTYPEOID:
			/* we can treat OIDs as integers... */
			return (double) DatumGetObjectId(value);
	}

	/*
	 * Can't get here unless someone tries to use scalarltsel/scalargtsel
	 * on an operator with one numeric and one non-numeric operand.
	 */
	elog(ERROR, "unsupported type: %u", typid);
	return 0;
}

/*
 * Do convert_to_scalar()'s work for any character-string data type.
 *
 * String datatypes are converted to a scale that ranges from 0 to 1,
 * where we visualize the bytes of the string as fractional digits.
 *
 * We do not want the base to be 256, however, since that tends to
 * generate inflated selectivity estimates; few databases will have
 * occurrences of all 256 possible byte values at each position.
 * Instead, use the smallest and largest byte values seen in the bounds
 * as the estimated range for each byte, after some fudging to deal with
 * the fact that we probably aren't going to see the full range that way.
 *
 * An additional refinement is that we discard any common prefix of the
 * three strings before computing the scaled values.  This allows us to
 * "zoom in" when we encounter a narrow data range.  An example is a phone
 * number database where all the values begin with the same area code.
 * (Actually, the bounds will be adjacent histogram-bin-boundary values,
 * so this is more likely to happen than you might think.)
 */
static void
convert_string_to_scalar(unsigned char *value,
						 double *scaledvalue,
						 unsigned char *lobound,
						 double *scaledlobound,
						 unsigned char *hibound,
						 double *scaledhibound)
{
	int			rangelo,
				rangehi;
	unsigned char *sptr;

	rangelo = rangehi = hibound[0];
	for (sptr = lobound; *sptr; sptr++)
	{
		if (rangelo > *sptr)
			rangelo = *sptr;
		if (rangehi < *sptr)
			rangehi = *sptr;
	}
	for (sptr = hibound; *sptr; sptr++)
	{
		if (rangelo > *sptr)
			rangelo = *sptr;
		if (rangehi < *sptr)
			rangehi = *sptr;
	}
	/* If range includes any upper-case ASCII chars, make it include all */
	if (rangelo <= 'Z' && rangehi >= 'A')
	{
		if (rangelo > 'A')
			rangelo = 'A';
		if (rangehi < 'Z')
			rangehi = 'Z';
	}
	/* Ditto lower-case */
	if (rangelo <= 'z' && rangehi >= 'a')
	{
		if (rangelo > 'a')
			rangelo = 'a';
		if (rangehi < 'z')
			rangehi = 'z';
	}
	/* Ditto digits */
	if (rangelo <= '9' && rangehi >= '0')
	{
		if (rangelo > '0')
			rangelo = '0';
		if (rangehi < '9')
			rangehi = '9';
	}

	/*
	 * If range includes less than 10 chars, assume we have not got enough
	 * data, and make it include regular ASCII set.
	 */
	if (rangehi - rangelo < 9)
	{
		rangelo = ' ';
		rangehi = 127;
	}

	/*
	 * Now strip any common prefix of the three strings.
	 */
	while (*lobound)
	{
		if (*lobound != *hibound || *lobound != *value)
			break;
		lobound++, hibound++, value++;
	}

	/*
	 * Now we can do the conversions.
	 */
	*scaledvalue = convert_one_string_to_scalar(value, rangelo, rangehi);
	*scaledlobound = convert_one_string_to_scalar(lobound, rangelo, rangehi);
	*scaledhibound = convert_one_string_to_scalar(hibound, rangelo, rangehi);
}

static double
convert_one_string_to_scalar(unsigned char *value, int rangelo, int rangehi)
{
	int			slen = strlen((char *) value);
	double		num,
				denom,
				base;

	if (slen <= 0)
		return 0.0;				/* empty string has scalar value 0 */

	/*
	 * Since base is at least 10, need not consider more than about 20
	 * chars
	 */
	if (slen > 20)
		slen = 20;

	/* Convert initial characters to fraction */
	base = rangehi - rangelo + 1;
	num = 0.0;
	denom = base;
	while (slen-- > 0)
	{
		int			ch = *value++;

		if (ch < rangelo)
			ch = rangelo - 1;
		else if (ch > rangehi)
			ch = rangehi + 1;
		num += ((double) (ch - rangelo)) / denom;
		denom *= base;
	}

	return num;
}

/*
 * Convert a string-type Datum into a palloc'd, null-terminated string.
 *
 * When using a non-C locale, we must pass the string through strxfrm()
 * before continuing, so as to generate correct locale-specific results.
 */
static unsigned char *
convert_string_datum(Datum value, Oid typid)
{
	char	   *val;

	switch (typid)
	{
		case CHAROID:
			val = (char *) palloc(2);
			val[0] = DatumGetChar(value);
			val[1] = '\0';
			break;
		case BPCHAROID:
		case VARCHAROID:
		case TEXTOID:
			{
				char	   *str = (char *) VARDATA(DatumGetPointer(value));
				int			strlength = VARSIZE(DatumGetPointer(value)) - VARHDRSZ;

				val = (char *) palloc(strlength + 1);
				memcpy(val, str, strlength);
				val[strlength] = '\0';
				break;
			}
		case NAMEOID:
			{
				NameData   *nm = (NameData *) DatumGetPointer(value);

				val = pstrdup(NameStr(*nm));
				break;
			}
		default:

			/*
			 * Can't get here unless someone tries to use scalarltsel on
			 * an operator with one string and one non-string operand.
			 */
			elog(ERROR, "unsupported type: %u", typid);
			return NULL;
	}

	if (!lc_collate_is_c())
	{
		char	   *xfrmstr;
		size_t		xfrmlen;
		size_t		xfrmlen2;

		/*
		 * Note: originally we guessed at a suitable output buffer size,
		 * and only needed to call strxfrm twice if our guess was too
		 * small. However, it seems that some versions of Solaris have
		 * buggy strxfrm that can write past the specified buffer length
		 * in that scenario.  So, do it the dumb way for portability.
		 *
		 * Yet other systems (e.g., glibc) sometimes return a smaller value
		 * from the second call than the first; thus the Assert must be <=
		 * not == as you'd expect.  Can't any of these people program
		 * their way out of a paper bag?
		 */
		xfrmlen = strxfrm(NULL, val, 0);
		xfrmstr = (char *) palloc(xfrmlen + 1);
		xfrmlen2 = strxfrm(xfrmstr, val, xfrmlen + 1);
		Assert(xfrmlen2 <= xfrmlen);
		pfree(val);
		val = xfrmstr;
	}

	return (unsigned char *) val;
}

/*
 * Do convert_to_scalar()'s work for any bytea data type.
 *
 * Very similar to convert_string_to_scalar except we can't assume
 * null-termination and therefore pass explicit lengths around.
 *
 * Also, assumptions about likely "normal" ranges of characters have been
 * removed - a data range of 0..255 is always used, for now.  (Perhaps
 * someday we will add information about actual byte data range to
 * pg_statistic.)
 */
static void
convert_bytea_to_scalar(Datum value,
						double *scaledvalue,
						Datum lobound,
						double *scaledlobound,
						Datum hibound,
						double *scaledhibound)
{
	int			rangelo,
				rangehi,
				valuelen = VARSIZE(DatumGetPointer(value)) - VARHDRSZ,
				loboundlen = VARSIZE(DatumGetPointer(lobound)) - VARHDRSZ,
				hiboundlen = VARSIZE(DatumGetPointer(hibound)) - VARHDRSZ,
				i,
				minlen;
	unsigned char *valstr = (unsigned char *) VARDATA(DatumGetPointer(value)),
			   *lostr = (unsigned char *) VARDATA(DatumGetPointer(lobound)),
			   *histr = (unsigned char *) VARDATA(DatumGetPointer(hibound));

	/*
	 * Assume bytea data is uniformly distributed across all byte values.
	 */
	rangelo = 0;
	rangehi = 255;

	/*
	 * Now strip any common prefix of the three strings.
	 */
	minlen = Min(Min(valuelen, loboundlen), hiboundlen);
	for (i = 0; i < minlen; i++)
	{
		if (*lostr != *histr || *lostr != *valstr)
			break;
		lostr++, histr++, valstr++;
		loboundlen--, hiboundlen--, valuelen--;
	}

	/*
	 * Now we can do the conversions.
	 */
	*scaledvalue = convert_one_bytea_to_scalar(valstr, valuelen, rangelo, rangehi);
	*scaledlobound = convert_one_bytea_to_scalar(lostr, loboundlen, rangelo, rangehi);
	*scaledhibound = convert_one_bytea_to_scalar(histr, hiboundlen, rangelo, rangehi);
}

static double
convert_one_bytea_to_scalar(unsigned char *value, int valuelen,
							int rangelo, int rangehi)
{
	double		num,
				denom,
				base;

	if (valuelen <= 0)
		return 0.0;				/* empty string has scalar value 0 */

	/*
	 * Since base is 256, need not consider more than about 10 chars (even
	 * this many seems like overkill)
	 */
	if (valuelen > 10)
		valuelen = 10;

	/* Convert initial characters to fraction */
	base = rangehi - rangelo + 1;
	num = 0.0;
	denom = base;
	while (valuelen-- > 0)
	{
		int			ch = *value++;

		if (ch < rangelo)
			ch = rangelo - 1;
		else if (ch > rangehi)
			ch = rangehi + 1;
		num += ((double) (ch - rangelo)) / denom;
		denom *= base;
	}

	return num;
}

/*
 * Do convert_to_scalar()'s work for any timevalue data type.
 */
static double
convert_timevalue_to_scalar(Datum value, Oid typid)
{
	switch (typid)
	{
		case TIMESTAMPOID:
			return DatumGetTimestamp(value);
		case TIMESTAMPTZOID:
			return DatumGetTimestampTz(value);
		case ABSTIMEOID:
			return DatumGetTimestamp(DirectFunctionCall1(abstime_timestamp,
														 value));
		case DATEOID:
			return DatumGetTimestamp(DirectFunctionCall1(date_timestamp,
														 value));
		case INTERVALOID:
			{
				Interval   *interval = DatumGetIntervalP(value);

				/*
				 * Convert the month part of Interval to days using
				 * assumed average month length of 365.25/12.0 days.  Not
				 * too accurate, but plenty good enough for our purposes.
				 */
#ifdef HAVE_INT64_TIMESTAMP
				return (interval->time + (interval->month * ((365.25 / 12.0) * 86400000000.0)));
#else
				return interval->time +
				interval  ->month * (365.25 / 12.0 * 24.0 * 60.0 * 60.0);
#endif
			}
		case RELTIMEOID:
#ifdef HAVE_INT64_TIMESTAMP
			return (DatumGetRelativeTime(value) * 1000000.0);
#else
			return DatumGetRelativeTime(value);
#endif
		case TINTERVALOID:
			{
				TimeInterval interval = DatumGetTimeInterval(value);

#ifdef HAVE_INT64_TIMESTAMP
				if (interval->status != 0)
					return ((interval->data[1] - interval->data[0]) *1000000.0);
#else
				if (interval->status != 0)
					return interval->data[1] - interval->data[0];
#endif
				return 0;		/* for lack of a better idea */
			}
		case TIMEOID:
			return DatumGetTimeADT(value);
		case TIMETZOID:
			{
				TimeTzADT  *timetz = DatumGetTimeTzADTP(value);

				/* use GMT-equivalent time */
#ifdef HAVE_INT64_TIMESTAMP
				return (double) (timetz->time + (timetz->zone * 1000000.0));
#else
				return (double) (timetz->time + timetz->zone);
#endif
			}
	}

	/*
	 * Can't get here unless someone tries to use scalarltsel/scalargtsel
	 * on an operator with one timevalue and one non-timevalue operand.
	 */
	elog(ERROR, "unsupported type: %u", typid);
	return 0;
}


/*
 * get_restriction_variable
 *		Examine the args of a restriction clause to see if it's of the
 *		form (variable op pseudoconstant) or (pseudoconstant op variable),
 *		where "variable" could be either a Var or an expression in vars of a
 *		single relation.  If so, extract information about the variable,
 *		and also indicate which side it was on and the other argument.
 *
 * Inputs:
 *	root: the Query
 *	args: clause argument list
 *	varRelid: see specs for restriction selectivity functions
 *
 * Outputs: (these are valid only if TRUE is returned)
 *	*vardata: gets information about variable (see examine_variable)
 *	*other: gets other clause argument, aggressively reduced to a constant
 *	*varonleft: set TRUE if variable is on the left, FALSE if on the right
 *
 * Returns TRUE if a variable is identified, otherwise FALSE.
 *
 * Note: if there are Vars on both sides of the clause, we must fail, because
 * callers are expecting that the other side will act like a pseudoconstant.
 */
static bool
get_restriction_variable(Query *root, List *args, int varRelid,
						 VariableStatData *vardata, Node **other,
						 bool *varonleft)
{
	Node	   *left,
			   *right;
	VariableStatData rdata;

	/* Fail if not a binary opclause (probably shouldn't happen) */
	if (list_length(args) != 2)
		return false;

	left = (Node *) linitial(args);
	right = (Node *) lsecond(args);

	/*
	 * Examine both sides.	Note that when varRelid is nonzero, Vars of
	 * other relations will be treated as pseudoconstants.
	 */
	examine_variable(root, left, varRelid, vardata);
	examine_variable(root, right, varRelid, &rdata);

	/*
	 * If one side is a variable and the other not, we win.
	 */
	if (vardata->rel && rdata.rel == NULL)
	{
		*varonleft = true;
		*other = estimate_expression_value(rdata.var);
		/* Assume we need no ReleaseVariableStats(rdata) here */
		return true;
	}

	if (vardata->rel == NULL && rdata.rel)
	{
		*varonleft = false;
		*other = estimate_expression_value(vardata->var);
		/* Assume we need no ReleaseVariableStats(*vardata) here */
		*vardata = rdata;
		return true;
	}

	/* Ooops, clause has wrong structure (probably var op var) */
	ReleaseVariableStats(*vardata);
	ReleaseVariableStats(rdata);

	return false;
}

/*
 * get_join_variables
 *		Apply examine_variable() to each side of a join clause.
 */
static void
get_join_variables(Query *root, List *args,
				   VariableStatData *vardata1, VariableStatData *vardata2)
{
	Node	   *left,
			   *right;

	if (list_length(args) != 2)
		elog(ERROR, "join operator should take two arguments");

	left = (Node *) linitial(args);
	right = (Node *) lsecond(args);

	examine_variable(root, left, 0, vardata1);
	examine_variable(root, right, 0, vardata2);
}

/*
 * examine_variable
 *		Try to look up statistical data about an expression.
 *		Fill in a VariableStatData struct to describe the expression.
 *
 * Inputs:
 *	root: the Query
 *	node: the expression tree to examine
 *	varRelid: see specs for restriction selectivity functions
 *
 * Outputs: *vardata is filled as follows:
 *	var: the input expression (with any binary relabeling stripped, if
 *		it is or contains a variable; but otherwise the type is preserved)
 *	rel: RelOptInfo for relation containing variable; NULL if expression
 *		contains no Vars (NOTE this could point to a RelOptInfo of a
 *		subquery, not one in the current query).
 *	statsTuple: the pg_statistic entry for the variable, if one exists;
 *		otherwise NULL.
 *	vartype: exposed type of the expression; this should always match
 *		the declared input type of the operator we are estimating for.
 *	atttype, atttypmod: type data to pass to get_attstatsslot().  This is
 *		commonly the same as the exposed type of the variable argument,
 *		but can be different in binary-compatible-type cases.
 *
 * Caller is responsible for doing ReleaseVariableStats() before exiting.
 */
static void
examine_variable(Query *root, Node *node, int varRelid,
				 VariableStatData *vardata)
{
	Node	   *basenode;
	Relids		varnos;
	RelOptInfo *onerel;

	/* Make sure we don't return dangling pointers in vardata */
	MemSet(vardata, 0, sizeof(VariableStatData));

	/* Save the exposed type of the expression */
	vardata->vartype = exprType(node);

	/* Look inside any binary-compatible relabeling */

	if (IsA(node, RelabelType))
		basenode = (Node *) ((RelabelType *) node)->arg;
	else
		basenode = node;

	/* Fast path for a simple Var */

	if (IsA(basenode, Var) &&
		(varRelid == 0 || varRelid == ((Var *) basenode)->varno))
	{
		Var		   *var = (Var *) basenode;
		Oid			relid;

		vardata->var = basenode;	/* return Var without relabeling */
		vardata->rel = find_base_rel(root, var->varno);
		vardata->atttype = var->vartype;
		vardata->atttypmod = var->vartypmod;

		relid = getrelid(var->varno, root->rtable);

		if (OidIsValid(relid))
		{
			vardata->statsTuple = SearchSysCache(STATRELATT,
												 ObjectIdGetDatum(relid),
											Int16GetDatum(var->varattno),
												 0, 0);
		}
		else
		{
			/*
			 * XXX This means the Var comes from a JOIN or sub-SELECT.
			 * Later add code to dig down into the join etc and see if we
			 * can trace the variable to something with stats.	(But
			 * beware of sub-SELECTs with DISTINCT/GROUP BY/etc.  Perhaps
			 * there are no cases where this would really be useful,
			 * because we'd have flattened the subselect if it is??)
			 */
		}

		return;
	}

	/*
	 * Okay, it's a more complicated expression.  Determine variable
	 * membership.	Note that when varRelid isn't zero, only vars of that
	 * relation are considered "real" vars.
	 */
	varnos = pull_varnos(basenode);

	onerel = NULL;

	switch (bms_membership(varnos))
	{
		case BMS_EMPTY_SET:
			/* No Vars at all ... must be pseudo-constant clause */
			break;
		case BMS_SINGLETON:
			if (varRelid == 0 || bms_is_member(varRelid, varnos))
			{
				onerel = find_base_rel(root,
				   (varRelid ? varRelid : bms_singleton_member(varnos)));
				vardata->rel = onerel;
				node = basenode; /* strip any relabeling */
			}
			/* else treat it as a constant */
			break;
		case BMS_MULTIPLE:
			if (varRelid == 0)
			{
				/* treat it as a variable of a join relation */
				vardata->rel = find_join_rel(root, varnos);
				node = basenode; /* strip any relabeling */
			}
			else if (bms_is_member(varRelid, varnos))
			{
				/* ignore the vars belonging to other relations */
				vardata->rel = find_base_rel(root, varRelid);
				node = basenode; /* strip any relabeling */
				/* note: no point in expressional-index search here */
			}
			/* else treat it as a constant */
			break;
	}

	bms_free(varnos);

	vardata->var = node;
	vardata->atttype = exprType(node);
	vardata->atttypmod = exprTypmod(node);

	if (onerel)
	{
		/*
		 * We have an expression in vars of a single relation.	Try to
		 * match it to expressional index columns, in hopes of finding
		 * some statistics.
		 *
		 * XXX it's conceivable that there are multiple matches with
		 * different index opclasses; if so, we need to pick one that
		 * matches the operator we are estimating for.	FIXME later.
		 */
		ListCell   *ilist;

		foreach(ilist, onerel->indexlist)
		{
			IndexOptInfo *index = (IndexOptInfo *) lfirst(ilist);
			ListCell   *indexpr_item;
			int			pos;

			indexpr_item = list_head(index->indexprs);
			if (indexpr_item == NULL)
				continue;		/* no expressions here... */

			/*
			 * Ignore partial indexes since they probably don't reflect
			 * whole-relation statistics.  Possibly reconsider this later.
			 */
			if (index->indpred)
				continue;

			for (pos = 0; pos < index->ncolumns; pos++)
			{
				if (index->indexkeys[pos] == 0)
				{
					Node	   *indexkey;

					if (indexpr_item == NULL)
						elog(ERROR, "too few entries in indexprs list");
					indexkey = (Node *) lfirst(indexpr_item);
					if (indexkey && IsA(indexkey, RelabelType))
						indexkey = (Node *) ((RelabelType *) indexkey)->arg;
					if (equal(node, indexkey))
					{
						/*
						 * Found a match ... is it a unique index? Tests
						 * here should match has_unique_index().
						 */
						if (index->unique &&
							index->ncolumns == 1 &&
							index->indpred == NIL)
							vardata->isunique = true;
						/* Has it got stats? */
						vardata->statsTuple = SearchSysCache(STATRELATT,
									   ObjectIdGetDatum(index->indexoid),
												  Int16GetDatum(pos + 1),
															 0, 0);
						if (vardata->statsTuple)
							break;
					}
					indexpr_item = lnext(indexpr_item);
				}
			}
			if (vardata->statsTuple)
				break;
		}
	}
}

/*
 * get_variable_numdistinct
 *	  Estimate the number of distinct values of a variable.
 *
 * vardata: results of examine_variable
 *
 * NB: be careful to produce an integral result, since callers may compare
 * the result to exact integer counts.
 */
static double
get_variable_numdistinct(VariableStatData *vardata)
{
	double		stadistinct;
	double		ntuples;

	/*
	 * Determine the stadistinct value to use.	There are cases where we
	 * can get an estimate even without a pg_statistic entry, or can get a
	 * better value than is in pg_statistic.
	 */
	if (HeapTupleIsValid(vardata->statsTuple))
	{
		/* Use the pg_statistic entry */
		Form_pg_statistic stats;

		stats = (Form_pg_statistic) GETSTRUCT(vardata->statsTuple);
		stadistinct = stats->stadistinct;
	}
	else if (vardata->vartype == BOOLOID)
	{
		/*
		 * Special-case boolean columns: presumably, two distinct values.
		 *
		 * Are there any other datatypes we should wire in special estimates
		 * for?
		 */
		stadistinct = 2.0;
	}
	else
	{
		/*
		 * We don't keep statistics for system columns, but in some cases
		 * we can infer distinctness anyway.
		 */
		if (vardata->var && IsA(vardata->var, Var))
		{
			switch (((Var *) vardata->var)->varattno)
			{
				case ObjectIdAttributeNumber:
				case SelfItemPointerAttributeNumber:
					stadistinct = -1.0; /* unique */
					break;
				case TableOidAttributeNumber:
					stadistinct = 1.0;	/* only 1 value */
					break;
				default:
					stadistinct = 0.0;	/* means "unknown" */
					break;
			}
		}
		else
			stadistinct = 0.0;	/* means "unknown" */

		/*
		 * XXX consider using estimate_num_groups on expressions?
		 */
	}

	/*
	 * If there is a unique index for the variable, assume it is unique no
	 * matter what pg_statistic says (the statistics could be out of
	 * date).  Can skip search if we already think it's unique.
	 */
	if (stadistinct != -1.0)
	{
		if (vardata->isunique)
			stadistinct = -1.0;
		else if (vardata->var && IsA(vardata->var, Var) &&
				 vardata->rel &&
				 has_unique_index(vardata->rel,
								  ((Var *) vardata->var)->varattno))
			stadistinct = -1.0;
	}

	/*
	 * If we had an absolute estimate, use that.
	 */
	if (stadistinct > 0.0)
		return stadistinct;

	/*
	 * Otherwise we need to get the relation size; punt if not available.
	 */
	if (vardata->rel == NULL)
		return DEFAULT_NUM_DISTINCT;
	ntuples = vardata->rel->tuples;
	if (ntuples <= 0.0)
		return DEFAULT_NUM_DISTINCT;

	/*
	 * If we had a relative estimate, use that.
	 */
	if (stadistinct < 0.0)
		return floor((-stadistinct * ntuples) + 0.5);

	/*
	 * With no data, estimate ndistinct = ntuples if the table is small,
	 * else use default.
	 */
	if (ntuples < DEFAULT_NUM_DISTINCT)
		return ntuples;

	return DEFAULT_NUM_DISTINCT;
}

/*
 * get_variable_maximum
 *		Estimate the maximum value of the specified variable.
 *		If successful, store value in *max and return TRUE.
 *		If no data available, return FALSE.
 *
 * sortop is the "<" comparison operator to use.  (To extract the
 * minimum instead of the maximum, just pass the ">" operator instead.)
 */
static bool
get_variable_maximum(Query *root, VariableStatData *vardata,
					 Oid sortop, Datum *max)
{
	Datum		tmax = 0;
	bool		have_max = false;
	Form_pg_statistic stats;
	int16		typLen;
	bool		typByVal;
	Datum	   *values;
	int			nvalues;
	int			i;

	if (!HeapTupleIsValid(vardata->statsTuple))
	{
		/* no stats available, so default result */
		return false;
	}
	stats = (Form_pg_statistic) GETSTRUCT(vardata->statsTuple);

	get_typlenbyval(vardata->atttype, &typLen, &typByVal);

	/*
	 * If there is a histogram, grab the last or first value as
	 * appropriate.
	 *
	 * If there is a histogram that is sorted with some other operator than
	 * the one we want, fail --- this suggests that there is data we can't
	 * use.
	 */
	if (get_attstatsslot(vardata->statsTuple,
						 vardata->atttype, vardata->atttypmod,
						 STATISTIC_KIND_HISTOGRAM, sortop,
						 &values, &nvalues,
						 NULL, NULL))
	{
		if (nvalues > 0)
		{
			tmax = datumCopy(values[nvalues - 1], typByVal, typLen);
			have_max = true;
		}
		free_attstatsslot(vardata->atttype, values, nvalues, NULL, 0);
	}
	else
	{
		Oid			rsortop = get_commutator(sortop);

		if (OidIsValid(rsortop) &&
			get_attstatsslot(vardata->statsTuple,
							 vardata->atttype, vardata->atttypmod,
							 STATISTIC_KIND_HISTOGRAM, rsortop,
							 &values, &nvalues,
							 NULL, NULL))
		{
			if (nvalues > 0)
			{
				tmax = datumCopy(values[0], typByVal, typLen);
				have_max = true;
			}
			free_attstatsslot(vardata->atttype, values, nvalues, NULL, 0);
		}
		else if (get_attstatsslot(vardata->statsTuple,
								  vardata->atttype, vardata->atttypmod,
								  STATISTIC_KIND_HISTOGRAM, InvalidOid,
								  &values, &nvalues,
								  NULL, NULL))
		{
			free_attstatsslot(vardata->atttype, values, nvalues, NULL, 0);
			return false;
		}
	}

	/*
	 * If we have most-common-values info, look for a large MCV.  This is
	 * needed even if we also have a histogram, since the histogram
	 * excludes the MCVs.  However, usually the MCVs will not be the
	 * extreme values, so avoid unnecessary data copying.
	 */
	if (get_attstatsslot(vardata->statsTuple,
						 vardata->atttype, vardata->atttypmod,
						 STATISTIC_KIND_MCV, InvalidOid,
						 &values, &nvalues,
						 NULL, NULL))
	{
		bool		large_mcv = false;
		FmgrInfo	opproc;

		fmgr_info(get_opcode(sortop), &opproc);

		for (i = 0; i < nvalues; i++)
		{
			if (!have_max)
			{
				tmax = values[i];
				large_mcv = have_max = true;
			}
			else if (DatumGetBool(FunctionCall2(&opproc, tmax, values[i])))
			{
				tmax = values[i];
				large_mcv = true;
			}
		}
		if (large_mcv)
			tmax = datumCopy(tmax, typByVal, typLen);
		free_attstatsslot(vardata->atttype, values, nvalues, NULL, 0);
	}

	*max = tmax;
	return have_max;
}


/*-------------------------------------------------------------------------
 *
 * Pattern analysis functions
 *
 * These routines support analysis of LIKE and regular-expression patterns
 * by the planner/optimizer.  It's important that they agree with the
 * regular-expression code in backend/regex/ and the LIKE code in
 * backend/utils/adt/like.c.  Also, the computation of the fixed prefix
 * must be conservative: if we report a string longer than the true fixed
 * prefix, the query may produce actually wrong answers, rather than just
 * getting a bad selectivity estimate!
 *
 * Note that the prefix-analysis functions are called from
 * backend/optimizer/path/indxpath.c as well as from routines in this file.
 *
 *-------------------------------------------------------------------------
 */

/*
 * Extract the fixed prefix, if any, for a pattern.
 *
 * *prefix is set to a palloc'd prefix string (in the form of a Const node),
 *	or to NULL if no fixed prefix exists for the pattern.
 * *rest is set to a palloc'd Const representing the remainder of the pattern
 *	after the portion describing the fixed prefix.
 * Each of these has the same type (TEXT or BYTEA) as the given pattern Const.
 *
 * The return value distinguishes no fixed prefix, a partial prefix,
 * or an exact-match-only pattern.
 */

static Pattern_Prefix_Status
like_fixed_prefix(Const *patt_const, bool case_insensitive,
				  Const **prefix_const, Const **rest_const)
{
	char	   *match;
	char	   *patt;
	int			pattlen;
	char	   *rest;
	Oid			typeid = patt_const->consttype;
	int			pos,
				match_pos;
	bool		is_multibyte = (pg_database_encoding_max_length() > 1);

	/* the right-hand const is type text or bytea */
	Assert(typeid == BYTEAOID || typeid == TEXTOID);

	if (typeid == BYTEAOID && case_insensitive)
		ereport(ERROR,
				(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
		errmsg("case insensitive matching not supported on type bytea")));

	if (typeid != BYTEAOID)
	{
		patt = DatumGetCString(DirectFunctionCall1(textout, patt_const->constvalue));
		pattlen = strlen(patt);
	}
	else
	{
		bytea	   *bstr = DatumGetByteaP(patt_const->constvalue);

		pattlen = VARSIZE(bstr) - VARHDRSZ;
		if (pattlen > 0)
		{
			patt = (char *) palloc(pattlen);
			memcpy(patt, VARDATA(bstr), pattlen);
		}
		else
			patt = NULL;

		if ((Pointer) bstr != DatumGetPointer(patt_const->constvalue))
			pfree(bstr);
	}

	match = palloc(pattlen + 1);
	match_pos = 0;
	for (pos = 0; pos < pattlen; pos++)
	{
		/* % and _ are wildcard characters in LIKE */
		if (patt[pos] == '%' ||
			patt[pos] == '_')
			break;

		/* Backslash escapes the next character */
		if (patt[pos] == '\\')
		{
			pos++;
			if (patt[pos] == '\0' && typeid != BYTEAOID)
				break;
		}

		/*
		 * XXX In multibyte character sets, we can't trust isalpha, so assume
		 * any multibyte char is potentially case-varying.
		 */
		if (case_insensitive)
		{
			if (is_multibyte && (unsigned char) patt[pos] >= 0x80)
				break;
			if (isalpha((unsigned char) patt[pos]))
				break;
		}

		/*
		 * NOTE: this code used to think that %% meant a literal %, but
		 * textlike() itself does not think that, and the SQL92 spec
		 * doesn't say any such thing either.
		 */
		match[match_pos++] = patt[pos];
	}

	match[match_pos] = '\0';
	rest = &patt[pos];

	if (typeid != BYTEAOID)
	{
		*prefix_const = string_to_const(match, typeid);
		*rest_const = string_to_const(rest, typeid);
	}
	else
	{
		*prefix_const = string_to_bytea_const(match, match_pos);
		*rest_const = string_to_bytea_const(rest, pattlen - pos);
	}

	if (patt != NULL)
		pfree(patt);
	pfree(match);

	/* in LIKE, an empty pattern is an exact match! */
	if (pos == pattlen)
		return Pattern_Prefix_Exact;	/* reached end of pattern, so
										 * exact */

	if (match_pos > 0)
		return Pattern_Prefix_Partial;

	return Pattern_Prefix_None;
}

static Pattern_Prefix_Status
regex_fixed_prefix(Const *patt_const, bool case_insensitive,
				   Const **prefix_const, Const **rest_const)
{
	char	   *match;
	int			pos,
				match_pos,
				prev_pos,
				prev_match_pos;
	bool		have_leading_paren;
	char	   *patt;
	char	   *rest;
	Oid			typeid = patt_const->consttype;
	bool		is_basic = regex_flavor_is_basic();
	bool		is_multibyte = (pg_database_encoding_max_length() > 1);

	/*
	 * Should be unnecessary, there are no bytea regex operators defined.
	 * As such, it should be noted that the rest of this function has *not*
	 * been made safe for binary (possibly NULL containing) strings.
	 */
	if (typeid == BYTEAOID)
		ereport(ERROR,
				(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
				 errmsg("regular-expression matching not supported on type bytea")));

	/* the right-hand const is type text for all of these */
	patt = DatumGetCString(DirectFunctionCall1(textout, patt_const->constvalue));

	/*
	 * Check for ARE director prefix.  It's worth our trouble to recognize
	 * this because similar_escape() uses it.
	 */
	pos = 0;
	if (strncmp(patt, "***:", 4) == 0)
	{
		pos = 4;
		is_basic = false;
	}

	/* Pattern must be anchored left */
	if (patt[pos] != '^')
	{
		rest = patt;

		*prefix_const = NULL;
		*rest_const = string_to_const(rest, typeid);

		return Pattern_Prefix_None;
	}
	pos++;

	/*
	 * If '|' is present in pattern, then there may be multiple alternatives
	 * for the start of the string.  (There are cases where this isn't so,
	 * for instance if the '|' is inside parens, but detecting that reliably
	 * is too hard.)
	 */
	if (strchr(patt + pos, '|') != NULL)
	{
		rest = patt;

		*prefix_const = NULL;
		*rest_const = string_to_const(rest, typeid);

		return Pattern_Prefix_None;
	}

	/* OK, allocate space for pattern */
	match = palloc(strlen(patt) + 1);
	prev_match_pos = match_pos = 0;

	/*
	 * We special-case the syntax '^(...)$' because psql uses it.  But beware:
	 * in BRE mode these parentheses are just ordinary characters.  Also,
	 * sequences beginning "(?" are not what they seem, unless they're "(?:".
	 * (We should recognize that, too, because of similar_escape().)
	 *
	 * Note: it's a bit bogus to be depending on the current regex_flavor
	 * setting here, because the setting could change before the pattern is
	 * used.  We minimize the risk by trusting the flavor as little as we can,
	 * but perhaps it would be a good idea to get rid of the "basic" setting.
	 */
	have_leading_paren = false;
	if (patt[pos] == '(' && !is_basic &&
		(patt[pos + 1] != '?' || patt[pos + 2] == ':'))
	{
		have_leading_paren = true;
		pos += (patt[pos + 1] != '?' ? 1 : 3);
	}

	/* Scan remainder of pattern */
	prev_pos = pos;
	while (patt[pos])
	{
		int			len;

		/*
		 * Check for characters that indicate multiple possible matches here.
		 * Also, drop out at ')' or '$' so the termination test works right.
		 */
		if (patt[pos] == '.' ||
			patt[pos] == '(' ||
			patt[pos] == ')' ||
			patt[pos] == '[' ||
			patt[pos] == '^' ||
			patt[pos] == '$')
			break;

		/*
		 * XXX In multibyte character sets, we can't trust isalpha, so assume
		 * any multibyte char is potentially case-varying.
		 */
		if (case_insensitive)
		{
			if (is_multibyte && (unsigned char) patt[pos] >= 0x80)
				break;
			if (isalpha((unsigned char) patt[pos]))
				break;
		}

		/*
		 * Check for quantifiers.  Except for +, this means the preceding
		 * character is optional, so we must remove it from the prefix too!
		 * Note: in BREs, \{ is a quantifier.
		 */
		if (patt[pos] == '*' ||
			patt[pos] == '?' ||
			patt[pos] == '{' ||
			(patt[pos] == '\\' && patt[pos + 1] == '{'))
		{
			match_pos = prev_match_pos;
			pos = prev_pos;
			break;
		}
		if (patt[pos] == '+')
		{
			pos = prev_pos;
			break;
		}

		/*
		 * Normally, backslash quotes the next character.  But in AREs,
		 * backslash followed by alphanumeric is an escape, not a quoted
		 * character.  Must treat it as having multiple possible matches.
		 * In BREs, \( is a parenthesis, so don't trust that either.
		 * Note: since only ASCII alphanumerics are escapes, we don't have
		 * to be paranoid about multibyte here.
		 */
		if (patt[pos] == '\\')
		{
			if (isalnum((unsigned char) patt[pos + 1]) || patt[pos + 1] == '(')
				break;
			pos++;
			if (patt[pos] == '\0')
				break;
		}
		/* save position in case we need to back up on next loop cycle */
		prev_match_pos = match_pos;
		prev_pos = pos;
		/* must use encoding-aware processing here */
		len = pg_mblen(&patt[pos]);
		memcpy(&match[match_pos], &patt[pos], len);
		match_pos += len;
		pos += len;
	}

	match[match_pos] = '\0';
	rest = &patt[pos];

	if (have_leading_paren && patt[pos] == ')')
		pos++;

	if (patt[pos] == '$' && patt[pos + 1] == '\0')
	{
		rest = &patt[pos + 1];

		*prefix_const = string_to_const(match, typeid);
		*rest_const = string_to_const(rest, typeid);

		pfree(patt);
		pfree(match);

		return Pattern_Prefix_Exact;	/* pattern specifies exact match */
	}

	*prefix_const = string_to_const(match, typeid);
	*rest_const = string_to_const(rest, typeid);

	pfree(patt);
	pfree(match);

	if (match_pos > 0)
		return Pattern_Prefix_Partial;

	return Pattern_Prefix_None;
}

Pattern_Prefix_Status
pattern_fixed_prefix(Const *patt, Pattern_Type ptype,
					 Const **prefix, Const **rest)
{
	Pattern_Prefix_Status result;

	switch (ptype)
	{
		case Pattern_Type_Like:
			result = like_fixed_prefix(patt, false, prefix, rest);
			break;
		case Pattern_Type_Like_IC:
			result = like_fixed_prefix(patt, true, prefix, rest);
			break;
		case Pattern_Type_Regex:
			result = regex_fixed_prefix(patt, false, prefix, rest);
			break;
		case Pattern_Type_Regex_IC:
			result = regex_fixed_prefix(patt, true, prefix, rest);
			break;
		default:
			elog(ERROR, "unrecognized ptype: %d", (int) ptype);
			result = Pattern_Prefix_None;		/* keep compiler quiet */
			break;
	}
	return result;
}

/*
 * Estimate the selectivity of a fixed prefix for a pattern match.
 *
 * A fixed prefix "foo" is estimated as the selectivity of the expression
 * "variable >= 'foo' AND variable < 'fop'" (see also indxqual.c).
 *
 * We use the >= and < operators from the specified btree opclass to do the
 * estimation.	The given variable and Const must be of the associated
 * datatype.
 *
 * XXX Note: we make use of the upper bound to estimate operator selectivity
 * even if the locale is such that we cannot rely on the upper-bound string.
 * The selectivity only needs to be approximately right anyway, so it seems
 * more useful to use the upper-bound code than not.
 */
static Selectivity
prefix_selectivity(Query *root, Node *variable,
				   Oid opclass, Const *prefixcon)
{
	Selectivity prefixsel;
	Oid			cmpopr;
	List	   *cmpargs;
	Const	   *greaterstrcon;

	cmpopr = get_opclass_member(opclass, InvalidOid,
								BTGreaterEqualStrategyNumber);
	if (cmpopr == InvalidOid)
		elog(ERROR, "no >= operator for opclass %u", opclass);
	cmpargs = list_make2(variable, prefixcon);
	/* Assume scalargtsel is appropriate for all supported types */
	prefixsel = DatumGetFloat8(DirectFunctionCall4(scalargtsel,
												   PointerGetDatum(root),
												ObjectIdGetDatum(cmpopr),
												PointerGetDatum(cmpargs),
												   Int32GetDatum(0)));

	/*-------
	 * If we can create a string larger than the prefix, say
	 *	"x < greaterstr".
	 *-------
	 */
	greaterstrcon = make_greater_string(prefixcon);
	if (greaterstrcon)
	{
		Selectivity topsel;

		cmpopr = get_opclass_member(opclass, InvalidOid,
									BTLessStrategyNumber);
		if (cmpopr == InvalidOid)
			elog(ERROR, "no < operator for opclass %u", opclass);
		cmpargs = list_make2(variable, greaterstrcon);
		/* Assume scalarltsel is appropriate for all supported types */
		topsel = DatumGetFloat8(DirectFunctionCall4(scalarltsel,
													PointerGetDatum(root),
												ObjectIdGetDatum(cmpopr),
												PointerGetDatum(cmpargs),
													Int32GetDatum(0)));

		/*
		 * Merge the two selectivities in the same way as for a range
		 * query (see clauselist_selectivity()).
		 */
		prefixsel = topsel + prefixsel - 1.0;

		/* Adjust for double-exclusion of NULLs */
		prefixsel += nulltestsel(root, IS_NULL, variable, 0);

		/*
		 * A zero or slightly negative prefixsel should be converted into
		 * a small positive value; we probably are dealing with a very
		 * tight range and got a bogus result due to roundoff errors.
		 * However, if prefixsel is very negative, then we probably have
		 * default selectivity estimates on one or both sides of the
		 * range.  In that case, insert a not-so-wildly-optimistic default
		 * estimate.
		 */
		if (prefixsel <= 0.0)
		{
			if (prefixsel < -0.01)
			{
				/*
				 * No data available --- use a default estimate that is
				 * small, but not real small.
				 */
				prefixsel = 0.005;
			}
			else
			{
				/*
				 * It's just roundoff error; use a small positive value
				 */
				prefixsel = 1.0e-10;
			}
		}
	}

	return prefixsel;
}


/*
 * Estimate the selectivity of a pattern of the specified type.
 * Note that any fixed prefix of the pattern will have been removed already.
 *
 * For now, we use a very simplistic approach: fixed characters reduce the
 * selectivity a good deal, character ranges reduce it a little,
 * wildcards (such as % for LIKE or .* for regex) increase it.
 */

#define FIXED_CHAR_SEL	0.20	/* about 1/5 */
#define CHAR_RANGE_SEL	0.25
#define ANY_CHAR_SEL	0.9		/* not 1, since it won't match
								 * end-of-string */
#define FULL_WILDCARD_SEL 5.0
#define PARTIAL_WILDCARD_SEL 2.0

static Selectivity
like_selectivity(Const *patt_const, bool case_insensitive)
{
	Selectivity sel = 1.0;
	int			pos;
	int			start;
	Oid			typeid = patt_const->consttype;
	char	   *patt;
	int			pattlen;

	/* the right-hand const is type text or bytea */
	Assert(typeid == BYTEAOID || typeid == TEXTOID);

	if (typeid == BYTEAOID && case_insensitive)
		ereport(ERROR,
				(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
		errmsg("case insensitive matching not supported on type bytea")));

	if (typeid != BYTEAOID)
	{
		patt = DatumGetCString(DirectFunctionCall1(textout, patt_const->constvalue));
		pattlen = strlen(patt);
	}
	else
	{
		bytea	   *bstr = DatumGetByteaP(patt_const->constvalue);

		pattlen = VARSIZE(bstr) - VARHDRSZ;
		if (pattlen > 0)
		{
			patt = (char *) palloc(pattlen);
			memcpy(patt, VARDATA(bstr), pattlen);
		}
		else
			patt = NULL;

		if ((Pointer) bstr != DatumGetPointer(patt_const->constvalue))
			pfree(bstr);
	}
	/* patt should never be NULL in practice */
	Assert(patt != NULL);

	/* Skip any leading %; it's already factored into initial sel */
	start = (*patt == '%') ? 1 : 0;
	for (pos = start; pos < pattlen; pos++)
	{
		/* % and _ are wildcard characters in LIKE */
		if (patt[pos] == '%')
			sel *= FULL_WILDCARD_SEL;
		else if (patt[pos] == '_')
			sel *= ANY_CHAR_SEL;
		else if (patt[pos] == '\\')
		{
			/* Backslash quotes the next character */
			pos++;
			if (patt[pos] == '\0' && typeid != BYTEAOID)
				break;
			sel *= FIXED_CHAR_SEL;
		}
		else
			sel *= FIXED_CHAR_SEL;
	}
	/* Could get sel > 1 if multiple wildcards */
	if (sel > 1.0)
		sel = 1.0;
	return sel;
}

static Selectivity
regex_selectivity_sub(char *patt, int pattlen, bool case_insensitive)
{
	Selectivity sel = 1.0;
	int			paren_depth = 0;
	int			paren_pos = 0;	/* dummy init to keep compiler quiet */
	int			pos;

	for (pos = 0; pos < pattlen; pos++)
	{
		if (patt[pos] == '(')
		{
			if (paren_depth == 0)
				paren_pos = pos;	/* remember start of parenthesized item */
			paren_depth++;
		}
		else if (patt[pos] == ')' && paren_depth > 0)
		{
			paren_depth--;
			if (paren_depth == 0)
				sel *= regex_selectivity_sub(patt + (paren_pos + 1),
											 pos - (paren_pos + 1),
											 case_insensitive);
		}
		else if (patt[pos] == '|' && paren_depth == 0)
		{
			/*
			 * If unquoted | is present at paren level 0 in pattern, we
			 * have multiple alternatives; sum their probabilities.
			 */
			sel += regex_selectivity_sub(patt + (pos + 1),
										 pattlen - (pos + 1),
										 case_insensitive);
			break;				/* rest of pattern is now processed */
		}
		else if (patt[pos] == '[')
		{
			bool		negclass = false;

			if (patt[++pos] == '^')
			{
				negclass = true;
				pos++;
			}
			if (patt[pos] == ']')		/* ']' at start of class is not
										 * special */
				pos++;
			while (pos < pattlen && patt[pos] != ']')
				pos++;
			if (paren_depth == 0)
				sel *= (negclass ? (1.0 - CHAR_RANGE_SEL) : CHAR_RANGE_SEL);
		}
		else if (patt[pos] == '.')
		{
			if (paren_depth == 0)
				sel *= ANY_CHAR_SEL;
		}
		else if (patt[pos] == '*' ||
				 patt[pos] == '?' ||
				 patt[pos] == '+')
		{
			/* Ought to be smarter about quantifiers... */
			if (paren_depth == 0)
				sel *= PARTIAL_WILDCARD_SEL;
		}
		else if (patt[pos] == '{')
		{
			while (pos < pattlen && patt[pos] != '}')
				pos++;
			if (paren_depth == 0)
				sel *= PARTIAL_WILDCARD_SEL;
		}
		else if (patt[pos] == '\\')
		{
			/* backslash quotes the next character */
			pos++;
			if (pos >= pattlen)
				break;
			if (paren_depth == 0)
				sel *= FIXED_CHAR_SEL;
		}
		else
		{
			if (paren_depth == 0)
				sel *= FIXED_CHAR_SEL;
		}
	}
	/* Could get sel > 1 if multiple wildcards */
	if (sel > 1.0)
		sel = 1.0;
	return sel;
}

static Selectivity
regex_selectivity(Const *patt_const, bool case_insensitive)
{
	Selectivity sel;
	char	   *patt;
	int			pattlen;
	Oid			typeid = patt_const->consttype;

	/*
	 * Should be unnecessary, there are no bytea regex operators defined.
	 * As such, it should be noted that the rest of this function has *not*
	 * been made safe for binary (possibly NULL containing) strings.
	 */
	if (typeid == BYTEAOID)
		ereport(ERROR,
				(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
				 errmsg("regular-expression matching not supported on type bytea")));

	/* the right-hand const is type text for all of these */
	patt = DatumGetCString(DirectFunctionCall1(textout, patt_const->constvalue));
	pattlen = strlen(patt);

	/* If patt doesn't end with $, consider it to have a trailing wildcard */
	if (pattlen > 0 && patt[pattlen - 1] == '$' &&
		(pattlen == 1 || patt[pattlen - 2] != '\\'))
	{
		/* has trailing $ */
		sel = regex_selectivity_sub(patt, pattlen - 1, case_insensitive);
	}
	else
	{
		/* no trailing $ */
		sel = regex_selectivity_sub(patt, pattlen, case_insensitive);
		sel *= FULL_WILDCARD_SEL;
		if (sel > 1.0)
			sel = 1.0;
	}
	return sel;
}

static Selectivity
pattern_selectivity(Const *patt, Pattern_Type ptype)
{
	Selectivity result;

	switch (ptype)
	{
		case Pattern_Type_Like:
			result = like_selectivity(patt, false);
			break;
		case Pattern_Type_Like_IC:
			result = like_selectivity(patt, true);
			break;
		case Pattern_Type_Regex:
			result = regex_selectivity(patt, false);
			break;
		case Pattern_Type_Regex_IC:
			result = regex_selectivity(patt, true);
			break;
		default:
			elog(ERROR, "unrecognized ptype: %d", (int) ptype);
			result = 1.0;		/* keep compiler quiet */
			break;
	}
	return result;
}


/*
 * Try to generate a string greater than the given string or any
 * string it is a prefix of.  If successful, return a palloc'd string
 * in the form of a Const pointer; else return NULL.
 *
 * The key requirement here is that given a prefix string, say "foo",
 * we must be able to generate another string "fop" that is greater
 * than all strings "foobar" starting with "foo".
 *
 * If we max out the righthand byte, truncate off the last character
 * and start incrementing the next.  For example, if "z" were the last
 * character in the sort order, then we could produce "foo" as a
 * string greater than "fonz".
 *
 * This could be rather slow in the worst case, but in most cases we
 * won't have to try more than one or two strings before succeeding.
 *
 * NOTE: at present this assumes we are in the C locale, so that simple
 * bytewise comparison applies.  However, we might be in a multibyte
 * encoding such as UTF-8, so we do have to watch out for generating
 * invalid encoding sequences.
 */
Const *
make_greater_string(const Const *str_const)
{
	Oid			datatype = str_const->consttype;
	char	   *workstr;
	int			len;

	/* Get the string and a modifiable copy */
	if (datatype == NAMEOID)
	{
		workstr = DatumGetCString(DirectFunctionCall1(nameout,
												 str_const->constvalue));
		len = strlen(workstr);
	}
	else if (datatype == BYTEAOID)
	{
		bytea	   *bstr = DatumGetByteaP(str_const->constvalue);

		len = VARSIZE(bstr) - VARHDRSZ;
		if (len > 0)
		{
			workstr = (char *) palloc(len);
			memcpy(workstr, VARDATA(bstr), len);
		}
		else
			workstr = NULL;

		if ((Pointer) bstr != DatumGetPointer(str_const->constvalue))
			pfree(bstr);
	}
	else
	{
		workstr = DatumGetCString(DirectFunctionCall1(textout,
												 str_const->constvalue));
		len = strlen(workstr);
	}

	while (len > 0)
	{
		unsigned char *lastchar = (unsigned char *) (workstr + len - 1);
		unsigned char savelastchar = *lastchar;

		/*
		 * Try to generate a larger string by incrementing the last byte.
		 */
		while (*lastchar < (unsigned char) 255)
		{
			Const	   *workstr_const;

			(*lastchar)++;

			if (datatype != BYTEAOID)
			{
				/* do not generate invalid encoding sequences */
				if (!pg_verifymbstr((const unsigned char *) workstr,
									len, true))
					continue;
				workstr_const = string_to_const(workstr, datatype);
			}
			else
				workstr_const = string_to_bytea_const(workstr, len);

			pfree(workstr);
			return workstr_const;
		}

		/* restore last byte so we don't confuse pg_mbcliplen */
		*lastchar = savelastchar;

		/*
		 * Truncate off the last character, which might be more than 1
		 * byte, depending on the character encoding.
		 */
		if (datatype != BYTEAOID && pg_database_encoding_max_length() > 1)
			len = pg_mbcliplen((const unsigned char *) workstr, len, len - 1);
		else
			len -= 1;

		if (datatype != BYTEAOID)
			workstr[len] = '\0';
	}

	/* Failed... */
	if (workstr != NULL)
		pfree(workstr);

	return NULL;
}

/*
 * Generate a Datum of the appropriate type from a C string.
 * Note that all of the supported types are pass-by-ref, so the
 * returned value should be pfree'd if no longer needed.
 */
static Datum
string_to_datum(const char *str, Oid datatype)
{
	Assert(str != NULL);

	/*
	 * We cheat a little by assuming that textin() will do for bpchar and
	 * varchar constants too...
	 */
	if (datatype == NAMEOID)
		return DirectFunctionCall1(namein, CStringGetDatum(str));
	else if (datatype == BYTEAOID)
		return DirectFunctionCall1(byteain, CStringGetDatum(str));
	else
		return DirectFunctionCall1(textin, CStringGetDatum(str));
}

/*
 * Generate a Const node of the appropriate type from a C string.
 */
static Const *
string_to_const(const char *str, Oid datatype)
{
	Datum		conval = string_to_datum(str, datatype);

	return makeConst(datatype, ((datatype == NAMEOID) ? NAMEDATALEN : -1),
					 conval, false, false);
}

/*
 * Generate a Const node of bytea type from a binary C string and a length.
 */
static Const *
string_to_bytea_const(const char *str, size_t str_len)
{
	bytea	   *bstr = palloc(VARHDRSZ + str_len);
	Datum		conval;

	memcpy(VARDATA(bstr), str, str_len);
	VARATT_SIZEP(bstr) = VARHDRSZ + str_len;
	conval = PointerGetDatum(bstr);

	return makeConst(BYTEAOID, -1, conval, false, false);
}

/*-------------------------------------------------------------------------
 *
 * Index cost estimation functions
 *
 * genericcostestimate is a general-purpose estimator for use when we
 * don't have any better idea about how to estimate.  Index-type-specific
 * knowledge can be incorporated in the type-specific routines.
 *
 *-------------------------------------------------------------------------
 */

static void
genericcostestimate(Query *root, RelOptInfo *rel,
					IndexOptInfo *index, List *indexQuals,
					Cost *indexStartupCost,
					Cost *indexTotalCost,
					Selectivity *indexSelectivity,
					double *indexCorrelation)
{
	double		numIndexTuples;
	double		numIndexPages;
	QualCost	index_qual_cost;
	double		qual_op_cost;
	double		qual_arg_cost;
	List	   *selectivityQuals;

	/*
	 * If the index is partial, AND the index predicate with the
	 * explicitly given indexquals to produce a more accurate idea of the
	 * index selectivity.  This may produce redundant clauses.	We get rid
	 * of exact duplicates in the code below.  We expect that most cases
	 * of partial redundancy (such as "x < 4" from the qual and "x < 5"
	 * from the predicate) will be recognized and handled correctly by
	 * clauselist_selectivity().  This assumption is somewhat fragile,
	 * since it depends on pred_test() and clauselist_selectivity() having
	 * similar capabilities, and there are certainly many cases where we
	 * will end up with a too-low selectivity estimate.  This will bias
	 * the system in favor of using partial indexes where possible, which
	 * is not necessarily a bad thing.	But it'd be nice to do better
	 * someday.
	 *
	 * Note that index->indpred and indexQuals are both in implicit-AND form,
	 * so ANDing them together just takes merging the lists.  However,
	 * eliminating duplicates is a bit trickier because indexQuals
	 * contains RestrictInfo nodes and the indpred does not.  It is okay
	 * to pass a mixed list to clauselist_selectivity, but we have to work
	 * a bit to generate a list without logical duplicates.  (We could
	 * just list_union indpred and strippedQuals, but then we'd not get
	 * caching of per-qual selectivity estimates.)
	 */
	if (index->indpred != NIL)
	{
		List	   *strippedQuals;
		List	   *predExtraQuals;

		strippedQuals = get_actual_clauses(indexQuals);
		predExtraQuals = list_difference(index->indpred, strippedQuals);
		selectivityQuals = list_concat(predExtraQuals, indexQuals);
	}
	else
		selectivityQuals = indexQuals;

	/* Estimate the fraction of main-table tuples that will be visited */
	*indexSelectivity = clauselist_selectivity(root, selectivityQuals,
											   rel->relid,
											   JOIN_INNER);

	/*
	 * Estimate the number of tuples that will be visited.	We do it in
	 * this rather peculiar-looking way in order to get the right answer
	 * for partial indexes.  We can bound the number of tuples by the
	 * index size, in any case.
	 */
	numIndexTuples = *indexSelectivity * rel->tuples;

	if (numIndexTuples > index->tuples)
		numIndexTuples = index->tuples;

	/*
	 * Always estimate at least one tuple is touched, even when
	 * indexSelectivity estimate is tiny.
	 */
	if (numIndexTuples < 1.0)
		numIndexTuples = 1.0;

	/*
	 * Estimate the number of index pages that will be retrieved.
	 *
	 * For all currently-supported index types, the first page of the index
	 * is a metadata page, and we should figure on fetching that plus a
	 * pro-rated fraction of the remaining pages.
	 */
	if (index->pages > 1 && index->tuples > 0)
	{
		numIndexPages = (numIndexTuples / index->tuples) * (index->pages - 1);
		numIndexPages += 1;		/* count the metapage too */
		numIndexPages = ceil(numIndexPages);
	}
	else
		numIndexPages = 1.0;

	/*
	 * Compute the index access cost.
	 *
	 * Disk cost: our generic assumption is that the index pages will be read
	 * sequentially, so they have cost 1.0 each, not random_page_cost.
	 */
	*indexTotalCost = numIndexPages;

	/*
	 * CPU cost: any complex expressions in the indexquals will need to be
	 * evaluated once at the start of the scan to reduce them to runtime
	 * keys to pass to the index AM (see nodeIndexscan.c).	We model the
	 * per-tuple CPU costs as cpu_index_tuple_cost plus one
	 * cpu_operator_cost per indexqual operator.
	 *
	 * Note: this neglects the possible costs of rechecking lossy operators
	 * and OR-clause expressions.  Detecting that that might be needed
	 * seems more expensive than it's worth, though, considering all the
	 * other inaccuracies here ...
	 */
	cost_qual_eval(&index_qual_cost, indexQuals);
	qual_op_cost = cpu_operator_cost * list_length(indexQuals);
	qual_arg_cost = index_qual_cost.startup +
		index_qual_cost.per_tuple - qual_op_cost;
	if (qual_arg_cost < 0)		/* just in case... */
		qual_arg_cost = 0;
	*indexStartupCost = qual_arg_cost;
	*indexTotalCost += qual_arg_cost;
	*indexTotalCost += numIndexTuples * (cpu_index_tuple_cost + qual_op_cost);

	/*
	 * Generic assumption about index correlation: there isn't any.
	 */
	*indexCorrelation = 0.0;
}


Datum
btcostestimate(PG_FUNCTION_ARGS)
{
	Query	   *root = (Query *) PG_GETARG_POINTER(0);
	RelOptInfo *rel = (RelOptInfo *) PG_GETARG_POINTER(1);
	IndexOptInfo *index = (IndexOptInfo *) PG_GETARG_POINTER(2);
	List	   *indexQuals = (List *) PG_GETARG_POINTER(3);
	Cost	   *indexStartupCost = (Cost *) PG_GETARG_POINTER(4);
	Cost	   *indexTotalCost = (Cost *) PG_GETARG_POINTER(5);
	Selectivity *indexSelectivity = (Selectivity *) PG_GETARG_POINTER(6);
	double	   *indexCorrelation = (double *) PG_GETARG_POINTER(7);
	Oid			relid;
	AttrNumber	colnum;
	HeapTuple	tuple;

	genericcostestimate(root, rel, index, indexQuals,
						indexStartupCost, indexTotalCost,
						indexSelectivity, indexCorrelation);

	/*
	 * If we can get an estimate of the first column's ordering
	 * correlation C from pg_statistic, estimate the index correlation as
	 * C for a single- column index, or C * 0.75 for multiple columns.
	 * (The idea here is that multiple columns dilute the importance of
	 * the first column's ordering, but don't negate it entirely.  Before
	 * 8.0 we divided the correlation by the number of columns, but that
	 * seems too strong.)
	 */
	if (index->indexkeys[0] != 0)
	{
		/* Simple variable --- look to stats for the underlying table */
		relid = getrelid(rel->relid, root->rtable);
		Assert(relid != InvalidOid);
		colnum = index->indexkeys[0];
	}
	else
	{
		/* Expression --- maybe there are stats for the index itself */
		relid = index->indexoid;
		colnum = 1;
	}

	tuple = SearchSysCache(STATRELATT,
						   ObjectIdGetDatum(relid),
						   Int16GetDatum(colnum),
						   0, 0);

	if (HeapTupleIsValid(tuple))
	{
		Oid			typid;
		int32		typmod;
		float4	   *numbers;
		int			nnumbers;

		/* XXX this code would break with different storage type */
		get_atttypetypmod(relid, colnum, &typid, &typmod);

		if (get_attstatsslot(tuple, typid, typmod,
							 STATISTIC_KIND_CORRELATION,
							 index->ordering[0],
							 NULL, NULL, &numbers, &nnumbers))
		{
			double		varCorrelation;

			Assert(nnumbers == 1);
			varCorrelation = numbers[0];

			if (index->ncolumns > 1)
				*indexCorrelation = varCorrelation * 0.75;
			else
				*indexCorrelation = varCorrelation;

			free_attstatsslot(typid, NULL, 0, numbers, nnumbers);
		}
		ReleaseSysCache(tuple);
	}

	PG_RETURN_VOID();
}

Datum
rtcostestimate(PG_FUNCTION_ARGS)
{
	Query	   *root = (Query *) PG_GETARG_POINTER(0);
	RelOptInfo *rel = (RelOptInfo *) PG_GETARG_POINTER(1);
	IndexOptInfo *index = (IndexOptInfo *) PG_GETARG_POINTER(2);
	List	   *indexQuals = (List *) PG_GETARG_POINTER(3);
	Cost	   *indexStartupCost = (Cost *) PG_GETARG_POINTER(4);
	Cost	   *indexTotalCost = (Cost *) PG_GETARG_POINTER(5);
	Selectivity *indexSelectivity = (Selectivity *) PG_GETARG_POINTER(6);
	double	   *indexCorrelation = (double *) PG_GETARG_POINTER(7);

	genericcostestimate(root, rel, index, indexQuals,
						indexStartupCost, indexTotalCost,
						indexSelectivity, indexCorrelation);

	PG_RETURN_VOID();
}

Datum
hashcostestimate(PG_FUNCTION_ARGS)
{
	Query	   *root = (Query *) PG_GETARG_POINTER(0);
	RelOptInfo *rel = (RelOptInfo *) PG_GETARG_POINTER(1);
	IndexOptInfo *index = (IndexOptInfo *) PG_GETARG_POINTER(2);
	List	   *indexQuals = (List *) PG_GETARG_POINTER(3);
	Cost	   *indexStartupCost = (Cost *) PG_GETARG_POINTER(4);
	Cost	   *indexTotalCost = (Cost *) PG_GETARG_POINTER(5);
	Selectivity *indexSelectivity = (Selectivity *) PG_GETARG_POINTER(6);
	double	   *indexCorrelation = (double *) PG_GETARG_POINTER(7);

	genericcostestimate(root, rel, index, indexQuals,
						indexStartupCost, indexTotalCost,
						indexSelectivity, indexCorrelation);

	PG_RETURN_VOID();
}

Datum
gistcostestimate(PG_FUNCTION_ARGS)
{
	Query	   *root = (Query *) PG_GETARG_POINTER(0);
	RelOptInfo *rel = (RelOptInfo *) PG_GETARG_POINTER(1);
	IndexOptInfo *index = (IndexOptInfo *) PG_GETARG_POINTER(2);
	List	   *indexQuals = (List *) PG_GETARG_POINTER(3);
	Cost	   *indexStartupCost = (Cost *) PG_GETARG_POINTER(4);
	Cost	   *indexTotalCost = (Cost *) PG_GETARG_POINTER(5);
	Selectivity *indexSelectivity = (Selectivity *) PG_GETARG_POINTER(6);
	double	   *indexCorrelation = (double *) PG_GETARG_POINTER(7);

	genericcostestimate(root, rel, index, indexQuals,
						indexStartupCost, indexTotalCost,
						indexSelectivity, indexCorrelation);

	PG_RETURN_VOID();
}
