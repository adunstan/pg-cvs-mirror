#include "_int.h"

#include "lib/stringinfo.h"

PG_FUNCTION_INFO_V1(_int_different);
PG_FUNCTION_INFO_V1(_int_same);
PG_FUNCTION_INFO_V1(_int_contains);
PG_FUNCTION_INFO_V1(_int_contained);
PG_FUNCTION_INFO_V1(_int_overlap);
PG_FUNCTION_INFO_V1(_int_union);
PG_FUNCTION_INFO_V1(_int_inter);

Datum		_int_different(PG_FUNCTION_ARGS);
Datum		_int_same(PG_FUNCTION_ARGS);
Datum		_int_contains(PG_FUNCTION_ARGS);
Datum		_int_contained(PG_FUNCTION_ARGS);
Datum		_int_overlap(PG_FUNCTION_ARGS);
Datum		_int_union(PG_FUNCTION_ARGS);
Datum		_int_inter(PG_FUNCTION_ARGS);

Datum
_int_contained(PG_FUNCTION_ARGS)
{
	PG_RETURN_BOOL(DatumGetBool(
								DirectFunctionCall2(
													_int_contains,
								   PointerGetDatum(PG_GETARG_POINTER(1)),
									PointerGetDatum(PG_GETARG_POINTER(0))
													)
								));
}

Datum
_int_contains(PG_FUNCTION_ARGS)
{
	ArrayType  *a = (ArrayType *) DatumGetPointer(PG_DETOAST_DATUM_COPY(PG_GETARG_DATUM(0)));
	ArrayType  *b = (ArrayType *) DatumGetPointer(PG_DETOAST_DATUM_COPY(PG_GETARG_DATUM(1)));
	bool		res;

	if (ARRISVOID(a) || ARRISVOID(b))
		return FALSE;

	PREPAREARR(a);
	PREPAREARR(b);
	res = inner_int_contains(a, b);
	pfree(a);
	pfree(b);
	PG_RETURN_BOOL(res);
}

Datum
_int_different(PG_FUNCTION_ARGS)
{
	PG_RETURN_BOOL(!DatumGetBool(
								 DirectFunctionCall2(
													 _int_same,
								   PointerGetDatum(PG_GETARG_POINTER(0)),
									PointerGetDatum(PG_GETARG_POINTER(1))
													 )
								 ));
}

Datum
_int_same(PG_FUNCTION_ARGS)
{
	ArrayType  *a = (ArrayType *) DatumGetPointer(PG_DETOAST_DATUM_COPY(PG_GETARG_DATUM(0)));
	ArrayType  *b = (ArrayType *) DatumGetPointer(PG_DETOAST_DATUM_COPY(PG_GETARG_DATUM(1)));
	int			na,
				nb;
	int			n;
	int		   *da,
			   *db;
	bool		result;
	bool		avoid = ARRISVOID(a);
	bool		bvoid = ARRISVOID(b);

	if (avoid || bvoid)
		return (avoid && bvoid) ? TRUE : FALSE;

	SORT(a);
	SORT(b);
	na = ARRNELEMS(a);
	nb = ARRNELEMS(b);
	da = ARRPTR(a);
	db = ARRPTR(b);

	result = FALSE;

	if (na == nb)
	{
		result = TRUE;
		for (n = 0; n < na; n++)
			if (da[n] != db[n])
			{
				result = FALSE;
				break;
			}
	}

	pfree(a);
	pfree(b);

	PG_RETURN_BOOL(result);
}

/*	_int_overlap -- does a overlap b?
 */
Datum
_int_overlap(PG_FUNCTION_ARGS)
{
	ArrayType  *a = (ArrayType *) DatumGetPointer(PG_DETOAST_DATUM_COPY(PG_GETARG_DATUM(0)));
	ArrayType  *b = (ArrayType *) DatumGetPointer(PG_DETOAST_DATUM_COPY(PG_GETARG_DATUM(1)));
	bool		result;

	if (ARRISVOID(a) || ARRISVOID(b))
		return FALSE;

	SORT(a);
	SORT(b);

	result = inner_int_overlap(a, b);

	pfree(a);
	pfree(b);

	PG_RETURN_BOOL(result);
}

Datum
_int_union(PG_FUNCTION_ARGS)
{
	ArrayType  *a = (ArrayType *) DatumGetPointer(PG_DETOAST_DATUM_COPY(PG_GETARG_DATUM(0)));
	ArrayType  *b = (ArrayType *) DatumGetPointer(PG_DETOAST_DATUM_COPY(PG_GETARG_DATUM(1)));
	ArrayType  *result;

	if (!ARRISVOID(a))
		SORT(a);
	if (!ARRISVOID(b))
		SORT(b);

	result = inner_int_union(a, b);

	if (a)
		pfree(a);
	if (b)
		pfree(b);

	PG_RETURN_POINTER(result);
}

Datum
_int_inter(PG_FUNCTION_ARGS)
{
	ArrayType  *a = (ArrayType *) DatumGetPointer(PG_DETOAST_DATUM_COPY(PG_GETARG_DATUM(0)));
	ArrayType  *b = (ArrayType *) DatumGetPointer(PG_DETOAST_DATUM_COPY(PG_GETARG_DATUM(1)));
	ArrayType  *result;

	if (ARRISVOID(a) || ARRISVOID(b))
		PG_RETURN_POINTER(new_intArrayType(0));

	SORT(a);
	SORT(b);

	result = inner_int_inter(a, b);

	pfree(a);
	pfree(b);

	PG_RETURN_POINTER(result);
}


PG_FUNCTION_INFO_V1(intset);
PG_FUNCTION_INFO_V1(icount);
PG_FUNCTION_INFO_V1(sort);
PG_FUNCTION_INFO_V1(sort_asc);
PG_FUNCTION_INFO_V1(sort_desc);
PG_FUNCTION_INFO_V1(uniq);
PG_FUNCTION_INFO_V1(idx);
PG_FUNCTION_INFO_V1(subarray);
PG_FUNCTION_INFO_V1(intarray_push_elem);
PG_FUNCTION_INFO_V1(intarray_push_array);
PG_FUNCTION_INFO_V1(intarray_del_elem);
PG_FUNCTION_INFO_V1(intset_union_elem);
PG_FUNCTION_INFO_V1(intset_subtract);
Datum		intset(PG_FUNCTION_ARGS);
Datum		icount(PG_FUNCTION_ARGS);
Datum		sort(PG_FUNCTION_ARGS);
Datum		sort_asc(PG_FUNCTION_ARGS);
Datum		sort_desc(PG_FUNCTION_ARGS);
Datum		uniq(PG_FUNCTION_ARGS);
Datum		idx(PG_FUNCTION_ARGS);
Datum		subarray(PG_FUNCTION_ARGS);
Datum		intarray_push_elem(PG_FUNCTION_ARGS);
Datum		intarray_push_array(PG_FUNCTION_ARGS);
Datum		intarray_del_elem(PG_FUNCTION_ARGS);
Datum		intset_union_elem(PG_FUNCTION_ARGS);
Datum		intset_subtract(PG_FUNCTION_ARGS);

#define QSORT(a, direction)						\
if (ARRNELEMS(a) > 1)						\
	qsort((void*)ARRPTR(a), ARRNELEMS(a),sizeof(int4),	\
		(direction) ? compASC : compDESC )


Datum
intset(PG_FUNCTION_ARGS)
{
	PG_RETURN_POINTER(int_to_intset(PG_GETARG_INT32(0)));
}

Datum
icount(PG_FUNCTION_ARGS)
{
	ArrayType  *a = (ArrayType *) DatumGetPointer(PG_DETOAST_DATUM(PG_GETARG_DATUM(0)));
	int32		count = (ARRISVOID(a)) ? 0 : ARRNELEMS(a);

	PG_FREE_IF_COPY(a, 0);
	PG_RETURN_INT32(count);
}

Datum
sort(PG_FUNCTION_ARGS)
{
	ArrayType  *a = (ArrayType *) DatumGetPointer(PG_DETOAST_DATUM_COPY(PG_GETARG_DATUM(0)));
	text	   *dirstr = (fcinfo->nargs == 2) ? PG_GETARG_TEXT_P(1) : NULL;
	int32		dc = (dirstr) ? VARSIZE(dirstr) - VARHDRSZ : 0;
	char	   *d = (dirstr) ? VARDATA(dirstr) : NULL;
	int			dir = -1;

	if (ARRISVOID(a) || ARRNELEMS(a) < 2)
		PG_RETURN_POINTER(a);

	if (dirstr == NULL || (dc == 3
						   && (d[0] == 'A' || d[0] == 'a')
						   && (d[1] == 'S' || d[1] == 's')
						   && (d[2] == 'C' || d[2] == 'c')))
		dir = 1;
	else if (dc == 4
			 && (d[0] == 'D' || d[0] == 'd')
			 && (d[1] == 'E' || d[1] == 'e')
			 && (d[2] == 'S' || d[2] == 's')
			 && (d[3] == 'C' || d[3] == 'c'))
		dir = 0;
	if (dir == -1)
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				 errmsg("second parameter must be \"ASC\" or \"DESC\"")));
	QSORT(a, dir);
	PG_RETURN_POINTER(a);
}

Datum
sort_asc(PG_FUNCTION_ARGS)
{
	ArrayType  *a = (ArrayType *) DatumGetPointer(PG_DETOAST_DATUM_COPY(PG_GETARG_DATUM(0)));

	if (ARRISVOID(a))
		PG_RETURN_POINTER(a);
	QSORT(a, 1);
	PG_RETURN_POINTER(a);
}

Datum
sort_desc(PG_FUNCTION_ARGS)
{
	ArrayType  *a = (ArrayType *) DatumGetPointer(PG_DETOAST_DATUM_COPY(PG_GETARG_DATUM(0)));

	if (ARRISVOID(a))
		PG_RETURN_POINTER(a);
	QSORT(a, 0);
	PG_RETURN_POINTER(a);
}

Datum
uniq(PG_FUNCTION_ARGS)
{
	ArrayType  *a = (ArrayType *) DatumGetPointer(PG_DETOAST_DATUM_COPY(PG_GETARG_DATUM(0)));

	if (ARRISVOID(a) || ARRNELEMS(a) < 2)
		PG_RETURN_POINTER(a);
	a = _int_unique(a);
	PG_RETURN_POINTER(a);
}

Datum
idx(PG_FUNCTION_ARGS)
{
	ArrayType  *a = (ArrayType *) DatumGetPointer(PG_DETOAST_DATUM(PG_GETARG_DATUM(0)));
	int32		result = (ARRISVOID(a)) ? 0 : ARRNELEMS(a);

	if (result)
		result = intarray_match_first(a, PG_GETARG_INT32(1));
	PG_FREE_IF_COPY(a, 0);
	PG_RETURN_INT32(result);
}

Datum
subarray(PG_FUNCTION_ARGS)
{
	ArrayType  *a = (ArrayType *) DatumGetPointer(PG_DETOAST_DATUM(PG_GETARG_DATUM(0)));
	ArrayType  *result;
	int32		start = (PG_GETARG_INT32(1) > 0) ? PG_GETARG_INT32(1) - 1 : PG_GETARG_INT32(1);
	int32		len = (fcinfo->nargs == 3) ? PG_GETARG_INT32(2) : 0;
	int32		end = 0;
	int32		c;

	if (ARRISVOID(a))
	{
		PG_FREE_IF_COPY(a, 0);
		PG_RETURN_POINTER(new_intArrayType(0));
	}

	c = ARRNELEMS(a);

	if (start < 0)
		start = c + start;

	if (len < 0)
		end = c + len;
	else if (len == 0)
		end = c;
	else
		end = start + len;

	if (end > c)
		end = c;

	if (start < 0)
		start = 0;

	if (start >= end || end <= 0)
	{
		PG_FREE_IF_COPY(a, 0);
		PG_RETURN_POINTER(new_intArrayType(0));
	}


	result = new_intArrayType(end - start);
	if (end - start > 0)
		memcpy(ARRPTR(result), ARRPTR(a) + start, (end - start) * sizeof(int32));
	PG_FREE_IF_COPY(a, 0);
	PG_RETURN_POINTER(result);
}

Datum
intarray_push_elem(PG_FUNCTION_ARGS)
{
	ArrayType  *a = (ArrayType *) DatumGetPointer(PG_DETOAST_DATUM(PG_GETARG_DATUM(0)));
	ArrayType  *result;

	result = intarray_add_elem(a, PG_GETARG_INT32(1));
	PG_FREE_IF_COPY(a, 0);
	PG_RETURN_POINTER(result);
}

Datum
intarray_push_array(PG_FUNCTION_ARGS)
{
	ArrayType  *a = (ArrayType *) DatumGetPointer(PG_DETOAST_DATUM(PG_GETARG_DATUM(0)));
	ArrayType  *b = (ArrayType *) DatumGetPointer(PG_DETOAST_DATUM(PG_GETARG_DATUM(1)));
	ArrayType  *result;

	result = intarray_concat_arrays(a, b);
	PG_FREE_IF_COPY(a, 0);
	PG_FREE_IF_COPY(b, 1);
	PG_RETURN_POINTER(result);
}

Datum
intarray_del_elem(PG_FUNCTION_ARGS)
{
	ArrayType  *a = (ArrayType *) DatumGetPointer(PG_DETOAST_DATUM_COPY(PG_GETARG_DATUM(0)));
	int32		c = (ARRISVOID(a)) ? 0 : ARRNELEMS(a);
	int32	   *aa = ARRPTR(a);
	int32		n = 0,
				i;
	int32		elem = PG_GETARG_INT32(1);

	for (i = 0; i < c; i++)
		if (aa[i] != elem)
		{
			if (i > n)
				aa[n++] = aa[i];
			else
				n++;
		}
	if (c > 0)
		a = resize_intArrayType(a, n);
	PG_RETURN_POINTER(a);
}

Datum
intset_union_elem(PG_FUNCTION_ARGS)
{
	ArrayType  *a = (ArrayType *) DatumGetPointer(PG_DETOAST_DATUM(PG_GETARG_DATUM(0)));
	ArrayType  *result;

	result = intarray_add_elem(a, PG_GETARG_INT32(1));
	PG_FREE_IF_COPY(a, 0);
	QSORT(result, 1);
	PG_RETURN_POINTER(_int_unique(result));
}

Datum
intset_subtract(PG_FUNCTION_ARGS)
{
	ArrayType  *a = (ArrayType *) DatumGetPointer(PG_DETOAST_DATUM_COPY(PG_GETARG_DATUM(0)));
	ArrayType  *b = (ArrayType *) DatumGetPointer(PG_DETOAST_DATUM_COPY(PG_GETARG_DATUM(1)));
	ArrayType  *result;
	int32		ca = ARRISVOID(a);
	int32		cb = ARRISVOID(b);
	int32	   *aa,
			   *bb,
			   *r;
	int32		n = 0,
				i = 0,
				k = 0;

	QSORT(a, 1);
	a = _int_unique(a);
	ca = ARRNELEMS(a);
	QSORT(b, 1);
	b = _int_unique(b);
	cb = ARRNELEMS(b);
	result = new_intArrayType(ca);
	aa = ARRPTR(a);
	bb = ARRPTR(b);
	r = ARRPTR(result);
	while (i < ca)
	{
		if (k == cb || aa[i] < bb[k])
			r[n++] = aa[i++];
		else if (aa[i] == bb[k])
		{
			i++;
			k++;
		}
		else
			k++;
	}
	result = resize_intArrayType(result, n);
	pfree(a);
	pfree(b);
	PG_RETURN_POINTER(result);
}
