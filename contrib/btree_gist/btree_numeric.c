#include "btree_gist.h"
#include "btree_utils_var.h"
#include "utils/builtins.h"
#include "utils/numeric.h"

/*
** Bytea ops
*/
PG_FUNCTION_INFO_V1(gbt_numeric_compress);
PG_FUNCTION_INFO_V1(gbt_numeric_union);
PG_FUNCTION_INFO_V1(gbt_numeric_picksplit);
PG_FUNCTION_INFO_V1(gbt_numeric_consistent);
PG_FUNCTION_INFO_V1(gbt_numeric_penalty);
PG_FUNCTION_INFO_V1(gbt_numeric_same);

Datum		gbt_numeric_compress(PG_FUNCTION_ARGS);
Datum		gbt_numeric_union(PG_FUNCTION_ARGS);
Datum		gbt_numeric_picksplit(PG_FUNCTION_ARGS);
Datum		gbt_numeric_consistent(PG_FUNCTION_ARGS);
Datum		gbt_numeric_penalty(PG_FUNCTION_ARGS);
Datum		gbt_numeric_same(PG_FUNCTION_ARGS);


/* define for comparison */

static bool
gbt_numeric_gt(const void *a, const void *b)
{
	return (DatumGetBool(DirectFunctionCall2(numeric_gt, PointerGetDatum(a), PointerGetDatum(b))));
}

static bool
gbt_numeric_ge(const void *a, const void *b)
{
	return (DatumGetBool(DirectFunctionCall2(numeric_ge, PointerGetDatum(a), PointerGetDatum(b))));
}

static bool
gbt_numeric_eq(const void *a, const void *b)
{
	return (DatumGetBool(DirectFunctionCall2(numeric_eq, PointerGetDatum(a), PointerGetDatum(b))));
}

static bool
gbt_numeric_le(const void *a, const void *b)
{
	return (DatumGetBool(DirectFunctionCall2(numeric_le, PointerGetDatum(a), PointerGetDatum(b))));
}

static bool
gbt_numeric_lt(const void *a, const void *b)
{
	return (DatumGetBool(DirectFunctionCall2(numeric_lt, PointerGetDatum(a), PointerGetDatum(b))));
}


static int32
gbt_numeric_cmp(const bytea *a, const bytea *b)
{
	return
		(DatumGetInt32(DirectFunctionCall2(numeric_cmp, PointerGetDatum(a), PointerGetDatum(b))));
}


static const gbtree_vinfo tinfo =
{
	gbt_t_numeric,
	FALSE,
	FALSE,
	gbt_numeric_gt,
	gbt_numeric_ge,
	gbt_numeric_eq,
	gbt_numeric_le,
	gbt_numeric_lt,
	gbt_numeric_cmp,
	NULL
};


/**************************************************
 * Text ops
 **************************************************/


Datum
gbt_numeric_compress(PG_FUNCTION_ARGS)
{
	GISTENTRY  *entry = (GISTENTRY *) PG_GETARG_POINTER(0);

	PG_RETURN_POINTER(gbt_var_compress(entry, &tinfo));
}



Datum
gbt_numeric_consistent(PG_FUNCTION_ARGS)
{

	GISTENTRY  *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
	GBT_VARKEY *key = (GBT_VARKEY *) DatumGetPointer(entry->key);
	void	   *qtst = (void *) DatumGetPointer(PG_GETARG_DATUM(1));
	void	   *query = (void *) DatumGetNumeric(PG_GETARG_DATUM(1));
	StrategyNumber strategy = (StrategyNumber) PG_GETARG_UINT16(2);
	bool		retval = FALSE;
	GBT_VARKEY_R r = gbt_var_key_readable(key);

	retval = gbt_var_consistent(&r, query, &strategy, GIST_LEAF(entry), &tinfo);

	if (qtst != query)
		pfree(query);
	PG_RETURN_BOOL(retval);
}



Datum
gbt_numeric_union(PG_FUNCTION_ARGS)
{
	GistEntryVector *entryvec = (GistEntryVector *) PG_GETARG_POINTER(0);
	int32	   *size = (int *) PG_GETARG_POINTER(1);

	PG_RETURN_POINTER(gbt_var_union(entryvec, size, &tinfo));
}


Datum
gbt_numeric_same(PG_FUNCTION_ARGS)
{
	Datum		d1 = PG_GETARG_DATUM(0);
	Datum		d2 = PG_GETARG_DATUM(1);
	bool	   *result = (bool *) PG_GETARG_POINTER(2);

	PG_RETURN_POINTER(gbt_var_same(result, d1, d2, &tinfo));
}


Datum
gbt_numeric_penalty(PG_FUNCTION_ARGS)
{
	GISTENTRY  *o = (GISTENTRY *) PG_GETARG_POINTER(0);
	GISTENTRY  *n = (GISTENTRY *) PG_GETARG_POINTER(1);
	float	   *result = (float *) PG_GETARG_POINTER(2);

	Numeric		us,
				os,
				ds;

	GBT_VARKEY *org = (GBT_VARKEY *) DatumGetPointer(o->key);
	GBT_VARKEY *newe = (GBT_VARKEY *) DatumGetPointer(n->key);
	Datum		uni;
	GBT_VARKEY_R rk,
				ok,
				uk;

	rk = gbt_var_key_readable(org);
	uni = PointerGetDatum(gbt_var_key_copy(&rk, TRUE));
	gbt_var_bin_union(&uni, newe, &tinfo);
	ok = gbt_var_key_readable(org);
	uk = gbt_var_key_readable((GBT_VARKEY *) DatumGetPointer(uni));

	us = DatumGetNumeric(DirectFunctionCall2(
											 numeric_sub,
											 PointerGetDatum(uk.upper),
											 PointerGetDatum(uk.lower)
											 ));

	pfree(DatumGetPointer(uni));

	os = DatumGetNumeric(DirectFunctionCall2(
											 numeric_sub,
											 PointerGetDatum(ok.upper),
											 PointerGetDatum(ok.lower)
											 ));

	ds = DatumGetNumeric(DirectFunctionCall2(
											 numeric_sub,
											 NumericGetDatum(us),
											 NumericGetDatum(os)
											 ));

	pfree(os);

	if (NUMERIC_IS_NAN(us))
	{

		if (NUMERIC_IS_NAN(os))
			*result = 0.0;
		else
			*result = 1.0;

	}
	else
	{

		Numeric		nul = DatumGetNumeric(DirectFunctionCall1(int4_numeric, Int32GetDatum(0)));

		*result = 0.0;

		if (DirectFunctionCall2(numeric_gt, NumericGetDatum(ds), NumericGetDatum(nul)))
		{

			*result += FLT_MIN;
			os = DatumGetNumeric(DirectFunctionCall2(
													 numeric_div,
													 NumericGetDatum(ds),
													 NumericGetDatum(us)
													 ));
			*result += (float4) DatumGetFloat8(DirectFunctionCall1(numeric_float8_no_overflow, NumericGetDatum(os)));
			pfree(os);

		}

		pfree(nul);
	}

	if (*result > 0)
		*result *= (FLT_MAX / (((GISTENTRY *) PG_GETARG_POINTER(0))->rel->rd_att->natts + 1));

	pfree(us);
	pfree(ds);

	PG_RETURN_POINTER(result);
}



Datum
gbt_numeric_picksplit(PG_FUNCTION_ARGS)
{
	GistEntryVector *entryvec = (GistEntryVector *) PG_GETARG_POINTER(0);
	GIST_SPLITVEC *v = (GIST_SPLITVEC *) PG_GETARG_POINTER(1);

	gbt_var_picksplit(entryvec, v, &tinfo);
	PG_RETURN_POINTER(v);
}
