/*
 * GiST support for ltree[] 
 * Teodor Sigaev <teodor@stack.net>
 */

#include "ltree.h"
#include "access/gist.h"
#include "access/rtree.h"
#include "access/nbtree.h"
#include "utils/array.h"

#include "crc32.h"

PG_FUNCTION_INFO_V1( _ltree_compress );
Datum   _ltree_compress(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1( _ltree_same );
Datum   _ltree_same(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1( _ltree_union );
Datum   _ltree_union(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1( _ltree_penalty );
Datum   _ltree_penalty(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1( _ltree_picksplit );
Datum   _ltree_picksplit(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1( _ltree_consistent );
Datum   _ltree_consistent(PG_FUNCTION_ARGS);

#define GETENTRY(vec,pos) ((ltree_gist *) DatumGetPointer(((GISTENTRY *) VARDATA(vec))[(pos)].key))
#define NEXTVAL(x) ( (ltree*)( (char*)(x) + INTALIGN( VARSIZE(x) ) ) )
#define SUMBIT(val) (        \
	GETBITBYTE(val,0) + \
	GETBITBYTE(val,1) + \
	GETBITBYTE(val,2) + \
	GETBITBYTE(val,3) + \
	GETBITBYTE(val,4) + \
	GETBITBYTE(val,5) + \
	GETBITBYTE(val,6) + \
	GETBITBYTE(val,7)   \
)
#define WISH_F(a,b,c) (double)( -(double)(((a)-(b))*((a)-(b))*((a)-(b)))*(c) )

static void
hashing(BITVECP sign, ltree *t) {
	int tlen = t->numlevel;
	ltree_level *cur = LTREE_FIRST(t);
	int  hash;

	while(tlen > 0) {
		hash = crc32_sz( cur->name, cur->len );
		AHASH( sign, hash );
		cur = LEVEL_NEXT(cur);
		tlen--;
	}
}

Datum   
_ltree_compress(PG_FUNCTION_ARGS) {
	GISTENTRY *entry = (GISTENTRY *)PG_GETARG_POINTER(0);
	GISTENTRY  *retval = entry;

	if ( entry->leafkey ) { /* ltree */
		ltree_gist	*key;
		ArrayType	*val = (ArrayType*)DatumGetPointer(PG_DETOAST_DATUM(entry->key));
		int4 len = LTG_HDRSIZE + ASIGLEN;
		int num=ArrayGetNItems( ARR_NDIM(val), ARR_DIMS(val) );
		ltree   *item = (ltree*)ARR_DATA_PTR(val);

		if ( ARR_NDIM(val) != 1 )
			 elog(ERROR,"Dimension of array != 1");

		key = (ltree_gist*)palloc( len );
		key->len = len;
		key->flag = 0;

		MemSet( LTG_SIGN(key), 0, sizeof(ASIGLEN) );
		while( num>0 ) {
			hashing(LTG_SIGN(key), item);
			num--;
			item = NEXTVAL(item);
		}

		if ( PointerGetDatum(val) != entry->key )
			pfree(val);

		retval = (GISTENTRY*)palloc( sizeof(GISTENTRY) );
		gistentryinit(*retval, PointerGetDatum(key),
			entry->rel, entry->page,
			entry->offset, key->len, FALSE);
	} else {
		int4 i,len;
		ltree_gist	*key;

		BITVECP sign = LTG_SIGN(DatumGetPointer( entry->key ) );

		ALOOPBYTE(
			if ( sign[i] != 0xff )
				PG_RETURN_POINTER(retval);
		);

		len = LTG_HDRSIZE; 
		key = (ltree_gist*)palloc( len );
		key->len = len;
		key->flag = LTG_ALLTRUE;

		retval = (GISTENTRY*)palloc( sizeof(GISTENTRY) );
		gistentryinit(*retval, PointerGetDatum(key),
			entry->rel, entry->page,
			entry->offset, key->len, FALSE);
	}
	PG_RETURN_POINTER(retval);
}

Datum   
_ltree_same(PG_FUNCTION_ARGS) {
	ltree_gist*	a=(ltree_gist*)PG_GETARG_POINTER(0);
	ltree_gist*	b=(ltree_gist*)PG_GETARG_POINTER(1);
	bool *result = (bool *)PG_GETARG_POINTER(2);

	if ( LTG_ISALLTRUE(a) && LTG_ISALLTRUE(b) ) {
		*result = true;
	} else if ( LTG_ISALLTRUE(a) ) {
		*result = false;
	} else if ( LTG_ISALLTRUE(b) ) {
		*result = false;
	} else {
		int4 i;
		BITVECP sa=LTG_SIGN(a), sb=LTG_SIGN(b);
		*result = true;
		ALOOPBYTE(
			if ( sa[i] != sb[i] ) {
				*result = false;
				break;
			}
		);
	} 
	PG_RETURN_POINTER(result); 
}

static int4 
unionkey( BITVECP sbase, ltree_gist *add ) {
	int4    i;
	BITVECP sadd = LTG_SIGN( add );

	if ( LTG_ISALLTRUE(add) )
		return 1;

	ALOOPBYTE(
		sbase[i] |= sadd[i];
	);
	return 0;
}

Datum   
_ltree_union(PG_FUNCTION_ARGS) {
	bytea *entryvec = (bytea *) PG_GETARG_POINTER(0);
	int *size = (int *) PG_GETARG_POINTER(1);
	ABITVEC base;
	int4 len = (VARSIZE(entryvec) - VARHDRSZ) / sizeof(GISTENTRY);
	int4 i;
	int4 flag = 0;
	ltree_gist	*result;

	MemSet( (void*)base, 0, sizeof(ABITVEC) );
	for(i=0;i<len;i++) {
		if ( unionkey( base, GETENTRY(entryvec, i) ) ) {
			flag = LTG_ALLTRUE;
			break;
		}
	}

	len = LTG_HDRSIZE + ( ( flag & LTG_ALLTRUE ) ? 0 : ASIGLEN );
	result = (ltree_gist*)palloc( len );
	*size = result->len = len;
	result->flag = flag;
	if ( ! LTG_ISALLTRUE(result) )
		memcpy((void*)LTG_SIGN(result), (void*)base, sizeof( ABITVEC ) );

	PG_RETURN_POINTER(result);	
}

static int4
sizebitvec( BITVECP sign ) {
	int4 size=0, i;
	ALOOPBYTE(
		size += SUMBIT(*(char*)sign);
		sign = (BITVECP) ( ((char*)sign) + 1 );
	);
	return size;
}

Datum   
_ltree_penalty(PG_FUNCTION_ARGS) {
	ltree_gist *origval = (ltree_gist*)DatumGetPointer( ( (GISTENTRY *)PG_GETARG_POINTER(0) )->key );
	ltree_gist *newval  = (ltree_gist*)DatumGetPointer( ( (GISTENTRY *)PG_GETARG_POINTER(1) )->key );
	float  *penalty = (float *)    PG_GETARG_POINTER(2);
	BITVECP orig = LTG_SIGN(origval);

	if ( LTG_ISALLTRUE(origval) ) {
		*penalty = 0.0;
		PG_RETURN_POINTER( penalty );
	}

	if ( LTG_ISALLTRUE(newval) ) {
		*penalty = (float) (ASIGLENBIT - sizebitvec( orig ) );
	} else {
		unsigned char valtmp;
		BITVECP nval = LTG_SIGN(newval);
		int4 i, unionsize=0;

		ALOOPBYTE(
			valtmp = nval[i] | orig[i];
			unionsize += SUMBIT(valtmp) - SUMBIT(orig[i]);
		);
		*penalty = (float)unionsize;
	}
	PG_RETURN_POINTER( penalty );
}

typedef struct {
	OffsetNumber    pos;
	int4           cost;
} SPLITCOST;

static int
comparecost( const void *a, const void *b ) {
	return ((SPLITCOST*)a)->cost - ((SPLITCOST*)b)->cost;
}

Datum   
_ltree_picksplit(PG_FUNCTION_ARGS) {
	bytea *entryvec = (bytea*) PG_GETARG_POINTER(0);
	GIST_SPLITVEC *v = (GIST_SPLITVEC*) PG_GETARG_POINTER(1);
	OffsetNumber k,j;
	ltree_gist	*datum_l, *datum_r;
	ABITVEC	union_l, union_r;
	bool            firsttime = true;
	int4    size_alpha,size_beta,sizeu,sizei;
	int4    size_waste, waste = 0.0;
	int4    size_l, size_r;
	int4    nbytes;
	OffsetNumber seed_1=0, seed_2=0;
	OffsetNumber    *left, *right;
	OffsetNumber maxoff;
	BITVECP ptra, ptrb, ptrc;
	int i;
	unsigned char    valtmp;
	SPLITCOST       *costvector;
	ltree_gist	*_k, *_j;

	maxoff = ((VARSIZE(entryvec) - VARHDRSZ) / sizeof(GISTENTRY)) - 2;
	nbytes = (maxoff + 2) * sizeof(OffsetNumber);
	v->spl_left = (OffsetNumber *) palloc(nbytes);
	v->spl_right = (OffsetNumber *) palloc(nbytes);

	for (k = FirstOffsetNumber; k < maxoff; k = OffsetNumberNext(k)) {
		_k = GETENTRY(entryvec,k); 
		for (j = OffsetNumberNext(k); j <= maxoff; j = OffsetNumberNext(j)) {
			_j = GETENTRY(entryvec,j);
			if ( LTG_ISALLTRUE(_k) || LTG_ISALLTRUE(_j) ) {
				sizeu = ASIGLENBIT;
				if ( LTG_ISALLTRUE(_k) && LTG_ISALLTRUE(_j) )
					sizei = ASIGLENBIT;
				else
					sizei = ( LTG_ISALLTRUE(_k) ) ? 
						sizebitvec( LTG_SIGN(_j) ) : sizebitvec( LTG_SIGN(_k) ); 
			} else {
				sizeu = sizei = 0;
				ptra = LTG_SIGN(_j);
				ptrb = LTG_SIGN(_k);
				/* critical section for bench !!! */

#define COUNT(pos) do { \
	if ( GETBITBYTE(*(char*)ptra,pos) ) { \
		sizeu++; \
		if ( GETBITBYTE(*(char*)ptrb, pos) ) \
			sizei++; \
	} else if ( GETBITBYTE(*(char*)ptrb, pos) ) \
		sizeu++; \
} while(0)

				ALOOPBYTE(
					COUNT(0);
					COUNT(1);
					COUNT(2);
					COUNT(3);
					COUNT(4);
					COUNT(5);
					COUNT(6);
					COUNT(7);
					ptra = (BITVECP) ( ((char*)ptra) + 1 );
					ptrb = (BITVECP) ( ((char*)ptrb) + 1 );
				);
			}
			size_waste = sizeu - sizei;
			if (size_waste > waste || firsttime) {
				waste = size_waste;
				seed_1 = k;
				seed_2 = j;
				firsttime = false;
			}
		}
	}

	left = v->spl_left;
	v->spl_nleft = 0;
	right = v->spl_right;
	v->spl_nright = 0;

	if ( seed_1 == 0 || seed_2 == 0 ) {
		seed_1 = 1;
		seed_2 = 2;
	}

	/* form initial .. */
	if ( LTG_ISALLTRUE(GETENTRY(entryvec,seed_1)) ) {
		datum_l = (ltree_gist*)palloc( LTG_HDRSIZE );
		datum_l->len = LTG_HDRSIZE; datum_l->flag = LTG_ALLTRUE;
		size_l = ASIGLENBIT;
	} else {
		datum_l = (ltree_gist*)palloc( LTG_HDRSIZE + ASIGLEN );
		datum_l->len = LTG_HDRSIZE + ASIGLEN; datum_l->flag = 0;
		memcpy((void*)LTG_SIGN(datum_l), (void*)LTG_SIGN(GETENTRY(entryvec,seed_1)), sizeof(ABITVEC));
		size_l = sizebitvec( LTG_SIGN(datum_l) );
	}
	if ( LTG_ISALLTRUE(GETENTRY(entryvec,seed_2)) ) {
		datum_r = (ltree_gist*)palloc( LTG_HDRSIZE );
		datum_r->len = LTG_HDRSIZE; datum_r->flag = LTG_ALLTRUE;
		size_r = ASIGLENBIT;
	} else {
		datum_r = (ltree_gist*)palloc( LTG_HDRSIZE + ASIGLEN );
		datum_r->len = LTG_HDRSIZE + ASIGLEN; datum_r->flag = 0;
		memcpy((void*)LTG_SIGN(datum_r), (void*)LTG_SIGN(GETENTRY(entryvec,seed_2)), sizeof(ABITVEC));
		size_r = sizebitvec( LTG_SIGN(datum_r) );
	}

	maxoff = OffsetNumberNext(maxoff);
	/* sort before ... */
	costvector=(SPLITCOST*)palloc( sizeof(SPLITCOST)*maxoff );
	for (j = FirstOffsetNumber; j <= maxoff; j = OffsetNumberNext(j)) {
		costvector[j-1].pos = j;
		_j = GETENTRY(entryvec,j);
		if ( LTG_ISALLTRUE(_j) ) {
			size_alpha = ASIGLENBIT - size_l;
			size_beta  = ASIGLENBIT - size_r;
		} else {
			ptra = LTG_SIGN( datum_l );
			ptrb = LTG_SIGN( datum_r );
			ptrc = LTG_SIGN( _j );
			size_beta = size_alpha = 0;
			if ( LTG_ISALLTRUE(datum_l) ) {
				if ( !LTG_ISALLTRUE(datum_r) ) {
					ALOOPBIT(
						if ( GETBIT(ptrc,i) && ! GETBIT(ptrb,i) )
							size_beta++;
					);
				}
			} else if ( LTG_ISALLTRUE(datum_r) ) {
				if ( !LTG_ISALLTRUE(datum_l) ) {
					ALOOPBIT(
						 if ( GETBIT(ptrc,i) && ! GETBIT(ptra,i) )
							size_alpha++;
					);
				}
			} else {
				ALOOPBIT(
					if ( GETBIT(ptrc,i) && ! GETBIT(ptra,i) )
						size_alpha++;
					if ( GETBIT(ptrc,i) && ! GETBIT(ptrb,i) )
						size_beta++;
				);
			}
		}
		costvector[j-1].cost = abs( size_alpha - size_beta );
	}
	qsort( (void*)costvector, maxoff, sizeof(SPLITCOST), comparecost );

	for (k = 0; k < maxoff; k++) {
		j = costvector[k].pos;
		_j = GETENTRY(entryvec,j);
		if ( j == seed_1 ) {
			*left++ = j;
			v->spl_nleft++;
			continue;
		} else if ( j == seed_2 ) {
			*right++ = j;
			v->spl_nright++;
			continue;
		}

		if ( LTG_ISALLTRUE(datum_l) || LTG_ISALLTRUE(_j) ) {
			size_alpha = ASIGLENBIT;
		} else {
			ptra = LTG_SIGN(_j);
			ptrb = LTG_SIGN(datum_l);
			size_alpha = 0;
			ALOOPBYTE(
				valtmp = union_l[i] = ptra[i] | ptrb[i];
				size_alpha += SUMBIT( valtmp );
			);
		}

		if ( LTG_ISALLTRUE(datum_r) || LTG_ISALLTRUE(_j) ) {
			size_beta = ASIGLENBIT;
		} else {
			ptra = LTG_SIGN(_j);
			ptrb = LTG_SIGN(datum_r);
			size_beta = 0;
			ALOOPBYTE(
				valtmp = union_r[i] = ptra[i] | ptrb[i];
				size_beta += SUMBIT( valtmp );
			);
		}

		if (size_alpha - size_l < size_beta - size_r + WISH_F(v->spl_nleft, v->spl_nright, 0.1)) {
			if ( ! LTG_ISALLTRUE( datum_l ) ) {
				if ( size_alpha == ASIGLENBIT ) {
					if ( size_alpha != size_l )
						MemSet( (void*)LTG_SIGN(datum_l),0xff, sizeof(ABITVEC));
				} else
					memcpy( (void*)LTG_SIGN(datum_l), (void*)union_l, sizeof(ABITVEC) );
			}
			size_l = size_alpha;
			*left++ = j;
			v->spl_nleft++;
		} else {
			if ( ! LTG_ISALLTRUE( datum_r ) ) {
				if ( size_beta == ASIGLENBIT ) {
					if ( size_beta != size_r )
						MemSet( (void*)LTG_SIGN(datum_r),0xff, sizeof(ABITVEC));
				} else
					memcpy( (void*)LTG_SIGN(datum_r), (void*)union_r, sizeof(ABITVEC) );
			}
			size_r = size_beta;
			*right++ = j;
			v->spl_nright++;
		}
	}

	*right = *left = FirstOffsetNumber;
	pfree(costvector);

	v->spl_ldatum = PointerGetDatum(datum_l);
	v->spl_rdatum = PointerGetDatum(datum_r);

	PG_RETURN_POINTER( v );
}

static bool
gist_te(ltree_gist *key, ltree* query) {
	ltree_level    *curq = LTREE_FIRST(query);
	BITVECP sign = LTG_SIGN(key);
	int     qlen = query->numlevel;
	unsigned int hv;

	if ( LTG_ISALLTRUE(key) )
		return true;

	while( qlen>0 ) {
		hv = crc32_sz(curq->name,curq->len);
		if ( ! GETBIT( sign, AHASHVAL(hv) ) )
			return false; 
		curq = LEVEL_NEXT(curq);
		qlen--;
	}

	return true;
}

static bool
checkcondition_bit(void *checkval, ITEM* val ) {
	return ( FLG_CANLOOKSIGN(val->flag) ) ? GETBIT( checkval, AHASHVAL( val->val ) ) : true;
}

static bool
gist_qtxt(ltree_gist *key, ltxtquery* query) {
	if ( LTG_ISALLTRUE(key) )
		return true;
            
	return execute(
		GETQUERY(query),
		(void*)LTG_SIGN(key), false,
		checkcondition_bit
	);
}

static bool
gist_qe(ltree_gist *key, lquery* query) {
	lquery_level    *curq = LQUERY_FIRST(query);
	BITVECP sign = LTG_SIGN(key);
	int     qlen = query->numlevel;
					 
	if ( LTG_ISALLTRUE(key) )
		return true;
			    
	while( qlen>0 ) {
		if ( curq->numvar && LQL_CANLOOKSIGN(curq) ) {
			bool isexist=false;
			int vlen = curq->numvar;
			lquery_variant *curv = LQL_FIRST(curq);
			while( vlen>0 ) {
				if ( GETBIT( sign, AHASHVAL( curv->val ) ) ) {
					isexist=true;
					break;
				}
				curv = LVAR_NEXT(curv);
				vlen--;
			}
			if ( !isexist )
				return false;
		}
 
		curq = LQL_NEXT(curq);
		qlen--;
	}

	return true;
}


Datum   
_ltree_consistent(PG_FUNCTION_ARGS) {
	GISTENTRY *entry = (GISTENTRY*)PG_GETARG_POINTER(0);
	char *query = (char*)DatumGetPointer( PG_DETOAST_DATUM(PG_GETARG_DATUM(1)) );
	ltree_gist *key = (ltree_gist*)DatumGetPointer( entry->key );
	StrategyNumber strategy = (StrategyNumber) PG_GETARG_UINT16(2);
	bool res = false;

#ifndef assert_enabled 
#define assert_enabled 0
#endif
	
	switch( strategy ) {
		case 10:
		case 11:
			res =  gist_te(key, (ltree*)query);
			break;
		case 12:
		case 13:
			res =  gist_qe(key, (lquery*)query);
			break; 
		case 14:
		case 15:
			res = gist_qtxt(key, (ltxtquery*)query);
			break;	
		default:
			elog(ERROR,"Unknown StrategyNumber: %d", strategy);
	}
	PG_RETURN_BOOL(res);
}

