/* 
 * default word parser 
 * Teodor Sigaev <teodor@sigaev.ru>
 */
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "postgres.h"
#include "utils/builtins.h"

#include "dict.h"
#include "wparser.h"
#include "common.h"
#include "ts_cfg.h"
#include "wordparser/parser.h"
#include "wordparser/deflex.h"

PG_FUNCTION_INFO_V1(prsd_lextype);
Datum prsd_lextype(PG_FUNCTION_ARGS);

Datum 
prsd_lextype(PG_FUNCTION_ARGS) {
	LexDescr *descr=(LexDescr*)palloc(sizeof(LexDescr)*(LASTNUM+1));
	int i;

	for(i=1;i<=LASTNUM;i++) {
		descr[i-1].lexid = i;
		descr[i-1].alias = pstrdup(tok_alias[i]);
		descr[i-1].descr = pstrdup(lex_descr[i]);
	}
	
	descr[LASTNUM].lexid=0;
		
	PG_RETURN_POINTER(descr);
}

PG_FUNCTION_INFO_V1(prsd_start);
Datum prsd_start(PG_FUNCTION_ARGS);
Datum 
prsd_start(PG_FUNCTION_ARGS) {
	start_parse_str( (char*)PG_GETARG_POINTER(0), PG_GETARG_INT32(1) );
	PG_RETURN_POINTER(NULL);
}

PG_FUNCTION_INFO_V1(prsd_getlexeme);
Datum prsd_getlexeme(PG_FUNCTION_ARGS);
Datum 
prsd_getlexeme(PG_FUNCTION_ARGS) {
	/* ParserState *p=(ParserState*)PG_GETARG_POINTER(0); */
	char **t=(char**)PG_GETARG_POINTER(1); 
	int *tlen=(int*)PG_GETARG_POINTER(2);
	int  type=tsearch2_yylex();

	*t = token;
	*tlen = tokenlen;
	PG_RETURN_INT32(type);
}

PG_FUNCTION_INFO_V1(prsd_end);
Datum prsd_end(PG_FUNCTION_ARGS);
Datum 
prsd_end(PG_FUNCTION_ARGS) {
	/* ParserState *p=(ParserState*)PG_GETARG_POINTER(0); */
	end_parse();
	PG_RETURN_VOID();
}

#define LEAVETOKEN(x)	( (x)==12 )
#define COMPLEXTOKEN(x)	( (x)==5 || (x)==15 || (x)==16 || (x)==17 )
#define ENDPUNCTOKEN(x)	( (x)==12 )


#define IDIGNORE(x) ( (x)==13 || (x)==14 || (x)==12 || (x)==23 )
#define HLIDIGNORE(x) ( (x)==5 || (x)==13 || (x)==15 || (x)==16 || (x)==17 )
#define NONWORDTOKEN(x)	( (x)==12 || HLIDIGNORE(x) )
#define NOENDTOKEN(x)	( NONWORDTOKEN(x) || (x)==7 || (x)==8 || (x)==20 || (x)==21 || (x)==22 || IDIGNORE(x) )

typedef struct {
	HLWORD	*words;
	int len;
} hlCheck;

static bool
checkcondition_HL(void *checkval, ITEM *val) {
	int i;
	for(i=0;i<((hlCheck*)checkval)->len;i++) {
		if ( ((hlCheck*)checkval)->words[i].item==val )
			return true;
	}
	return false;
}


static bool
hlCover(HLPRSTEXT *prs, QUERYTYPE *query, int *p, int *q) {
	int i,j;
	ITEM    *item=GETQUERY(query);
	int pos=*p;
	*q=0;
	*p=0x7fffffff;

	for(j=0;j<query->size;j++) {
		if ( item->type != VAL ) {
			item++;
			continue;
		}
		for(i=pos;i<prs->curwords;i++) {
			if ( prs->words[i].item == item ) {
				if ( i>*q) 
					*q = i;
				break;
			}
		}
		item++;
	}

	if ( *q==0 )
		return false;

	item=GETQUERY(query);
	for(j=0;j<query->size;j++) {
		if ( item->type != VAL ) {
			item++;
			continue;
		}
		for(i=*q;i>=pos;i--) {
			if ( prs->words[i].item == item ) {
				if ( i<*p )
					*p=i;
				break;
			}
		}
		item++;
	}	

	if ( *p<=*q ) {
		hlCheck ch={ &(prs->words[*p]), *q-*p+1 };
		if ( TS_execute(GETQUERY(query), &ch, false, checkcondition_HL) ) { 
			return true;
		} else {
			(*p)++;
			return hlCover(prs,query,p,q);
		}
	}

	return false;
}

PG_FUNCTION_INFO_V1(prsd_headline);
Datum prsd_headline(PG_FUNCTION_ARGS);
Datum 
prsd_headline(PG_FUNCTION_ARGS) {
	HLPRSTEXT	*prs=(HLPRSTEXT*)PG_GETARG_POINTER(0);
	text	*opt=(text*)PG_GETARG_POINTER(1); /* can't be toasted */
	QUERYTYPE	*query=(QUERYTYPE*)PG_GETARG_POINTER(2); /* can't be toasted */
	/* from opt + start and and tag */
	int	min_words=15;	
	int	max_words=35;	
	int	shortword=3;	

	int p=0,q=0;
	int bestb=-1,beste=-1;
	int bestlen=-1;
	int pose=0, poslen, curlen;

	int i;

	/*config*/
	prs->startsel=NULL;
	prs->stopsel=NULL;
	if ( opt ) {
		Map *map,*mptr;
		
		parse_cfgdict(opt,&map);
		mptr=map;

		while(mptr && mptr->key) {
			if ( strcasecmp(mptr->key,"MaxWords")==0 )
				max_words=pg_atoi(mptr->value,4,1);
			else if ( strcasecmp(mptr->key,"MinWords")==0 )
				min_words=pg_atoi(mptr->value,4,1);
			else if ( strcasecmp(mptr->key,"ShortWord")==0 )
				shortword=pg_atoi(mptr->value,4,1);
			else if ( strcasecmp(mptr->key,"StartSel")==0 )
				prs->startsel=pstrdup(mptr->value);
			else if ( strcasecmp(mptr->key,"StopSel")==0 )
				prs->stopsel=pstrdup(mptr->value);
				
			pfree(mptr->key);
			pfree(mptr->value);

			mptr++;
		}
		pfree(map);

		if ( min_words >= max_words )
			elog(ERROR,"Must be MinWords < MaxWords");
		if ( min_words<=0 )
			elog(ERROR,"Must be MinWords > 0");
		if ( shortword<0 )
			elog(ERROR,"Must be ShortWord >= 0");
	}

	while( hlCover(prs,query,&p,&q) ) {
		/* find cover len in words */
		curlen=0;
		poslen=0;
		for(i=p;i<=q && curlen < max_words ; i++) {
			if ( !NONWORDTOKEN(prs->words[i].type) ) 
				curlen++;
			if ( prs->words[i].item && !prs->words[i].repeated )
				poslen++; 
			pose=i;
		}

		if ( poslen<bestlen && !(NOENDTOKEN(prs->words[beste].type) || prs->words[beste].len <= shortword) ) { 
			/* best already finded, so try one more cover */
			p++;
			continue;
		}

		if ( curlen < max_words ) { /* find good end */
			for(i=i-1 ;i<prs->curwords && curlen<max_words; i++) {
				if ( i!=q ) {
					if ( !NONWORDTOKEN(prs->words[i].type) ) 
						curlen++;
					if ( prs->words[i].item && !prs->words[i].repeated )
						poslen++;
				}
				pose=i;
				if ( NOENDTOKEN(prs->words[i].type) || prs->words[i].len <= shortword ) 
					continue;
				if ( curlen>=min_words ) 	
					break;
			}
		} else { /* shorter cover :((( */
			for(;curlen>min_words;i--) {
				if ( !NONWORDTOKEN(prs->words[i].type) ) 
					curlen--;
				if ( prs->words[i].item && !prs->words[i].repeated )
					poslen--;
				pose=i;
				if ( NOENDTOKEN(prs->words[i].type) || prs->words[i].len <= shortword ) 
					continue;
				break;
			}
		}

		if ( bestlen <0 || (poslen>bestlen && !(NOENDTOKEN(prs->words[pose].type) || prs->words[pose].len <= shortword)) || 
				( bestlen>=0 && !(NOENDTOKEN(prs->words[pose].type)  || prs->words[pose].len <= shortword) && 
					(NOENDTOKEN(prs->words[beste].type) || prs->words[beste].len <= shortword) ) ) {
			bestb=p; beste=pose;
			bestlen=poslen;
		} 

		p++;
	}

	if ( bestlen<0 ) {
		curlen=0;
		poslen=0;
		for(i=0;i<prs->curwords && curlen<min_words  ; i++) {
			if ( !NONWORDTOKEN(prs->words[i].type) ) 
				curlen++;
			pose=i;
		}
		bestb=0; beste=pose;
	}

	for(i=bestb;i<=beste;i++) {
		if ( prs->words[i].item )
			prs->words[i].selected=1;
		if ( prs->words[i].repeated )
			prs->words[i].skip=1;
		if ( HLIDIGNORE(prs->words[i].type) )
			prs->words[i].replace=1;

		prs->words[i].in=1;
	}

	if (!prs->startsel)
		prs->startsel=pstrdup("<b>");
	if (!prs->stopsel)
		prs->stopsel=pstrdup("</b>");
        prs->startsellen=strlen(prs->startsel);
	prs->stopsellen=strlen(prs->stopsel);

	PG_RETURN_POINTER(prs);
}

