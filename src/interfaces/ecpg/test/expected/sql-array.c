/* Processed by ecpg (4.2.1) */
/* These include files are added by the preprocessor */
#include <ecpgtype.h>
#include <ecpglib.h>
#include <ecpgerrno.h>
#include <sqlca.h>
/* End of automatic include section */

#line 1 "array.pgc"
#include <locale.h>
#include <string.h>
#include <stdlib.h>

/* exec sql whenever sqlerror  sqlprint ; */
#line 5 "array.pgc"



#line 1 "sqlca.h"
#ifndef POSTGRES_SQLCA_H
#define POSTGRES_SQLCA_H

#ifndef DLLIMPORT
#if  defined(WIN32) || defined(__CYGWIN__)
#define DLLIMPORT __declspec (dllimport)
#else
#define DLLIMPORT
#endif   /* __CYGWIN__ */
#endif   /* DLLIMPORT */

#define SQLERRMC_LEN	150

#ifdef __cplusplus
extern		"C"
{
#endif

struct sqlca_t
{
	char		sqlcaid[8];
	long		sqlabc;
	long		sqlcode;
	struct
	{
		int			sqlerrml;
		char		sqlerrmc[SQLERRMC_LEN];
	}			sqlerrm;
	char		sqlerrp[8];
	long		sqlerrd[6];
	/* Element 0: empty						*/
	/* 1: OID of processed tuple if applicable			*/
	/* 2: number of rows processed				*/
	/* after an INSERT, UPDATE or				*/
	/* DELETE statement					*/
	/* 3: empty						*/
	/* 4: empty						*/
	/* 5: empty						*/
	char		sqlwarn[8];
	/* Element 0: set to 'W' if at least one other is 'W'	*/
	/* 1: if 'W' at least one character string		*/
	/* value was truncated when it was			*/
	/* stored into a host variable.				*/

	/*
	 * 2: if 'W' a (hopefully) non-fatal notice occurred
	 */	/* 3: empty */
	/* 4: empty						*/
	/* 5: empty						*/
	/* 6: empty						*/
	/* 7: empty						*/

	char		sqlstate[5];
};

struct sqlca_t *ECPGget_sqlca(void);

#ifndef POSTGRES_ECPG_INTERNAL
#define sqlca (*ECPGget_sqlca())
#endif

#ifdef __cplusplus
}
#endif

#endif

#line 7 "array.pgc"


#line 1 "regression.h"






#line 8 "array.pgc"


int
main (void)
{
/* exec sql begin declare section */
	   
	   
	   
	   
	    
	 

#line 14 "array.pgc"
 int  i   = 1 ;
 
#line 15 "array.pgc"
 int * did   = & i ;
 
#line 16 "array.pgc"
 int  a [ 10 ]   = { 9 , 8 , 7 , 6 , 5 , 4 , 3 , 2 , 1 , 0 } ;
 
#line 17 "array.pgc"
 char  text [ 25 ]   = "klmnopqrst" ;
 
#line 18 "array.pgc"
 char * t   = ( char * ) malloc ( 11 ) ;
 
#line 19 "array.pgc"
 double  f    ;
/* exec sql end declare section */
#line 20 "array.pgc"


	strcpy(t, "0123456789");
	setlocale(LC_ALL, "C");

	ECPGdebug(1, stderr);

        { ECPGconnect(__LINE__, 0, "regress1" , NULL,NULL , NULL, 0); 
#line 27 "array.pgc"

if (sqlca.sqlcode < 0) sqlprint();}
#line 27 "array.pgc"


	{ ECPGsetcommit(__LINE__, "on", NULL);
#line 29 "array.pgc"

if (sqlca.sqlcode < 0) sqlprint();}
#line 29 "array.pgc"


 	{ ECPGtrans(__LINE__, NULL, "begin transaction ");
#line 31 "array.pgc"

if (sqlca.sqlcode < 0) sqlprint();}
#line 31 "array.pgc"


	{ ECPGdo(__LINE__, 0, 1, NULL, "create  table test ( f float    , i int   , a int [ 10 ]   , text char  ( 10 )    )    ", ECPGt_EOIT, ECPGt_EORT);
#line 33 "array.pgc"

if (sqlca.sqlcode < 0) sqlprint();}
#line 33 "array.pgc"


	{ ECPGdo(__LINE__, 0, 1, NULL, "insert into test ( f  , i  , a  , text  ) values ( 404.90 , 3 , '{0,1,2,3,4,5,6,7,8,9}' , 'abcdefghij' ) ", ECPGt_EOIT, ECPGt_EORT);
#line 35 "array.pgc"

if (sqlca.sqlcode < 0) sqlprint();}
#line 35 "array.pgc"


	{ ECPGdo(__LINE__, 0, 1, NULL, "insert into test ( f  , i  , a  , text  ) values ( 140787.0 , 2 ,  ? ,  ? ) ", 
	ECPGt_int,(a),(long)1,(long)10,sizeof(int), 
	ECPGt_NO_INDICATOR, NULL , 0L, 0L, 0L, 
	ECPGt_char,(text),(long)25,(long)1,(25)*sizeof(char), 
	ECPGt_NO_INDICATOR, NULL , 0L, 0L, 0L, ECPGt_EOIT, ECPGt_EORT);
#line 37 "array.pgc"

if (sqlca.sqlcode < 0) sqlprint();}
#line 37 "array.pgc"

	
	{ ECPGdo(__LINE__, 0, 1, NULL, "insert into test ( f  , i  , a  , text  ) values ( 14.07 ,  ? ,  ? ,  ? ) ", 
	ECPGt_int,&(did),(long)1,(long)0,sizeof(int), 
	ECPGt_NO_INDICATOR, NULL , 0L, 0L, 0L, 
	ECPGt_int,(a),(long)1,(long)10,sizeof(int), 
	ECPGt_NO_INDICATOR, NULL , 0L, 0L, 0L, 
	ECPGt_char,&(t),(long)0,(long)1,(1)*sizeof(char), 
	ECPGt_NO_INDICATOR, NULL , 0L, 0L, 0L, ECPGt_EOIT, ECPGt_EORT);
#line 39 "array.pgc"

if (sqlca.sqlcode < 0) sqlprint();}
#line 39 "array.pgc"


	{ ECPGtrans(__LINE__, NULL, "commit");
#line 41 "array.pgc"

if (sqlca.sqlcode < 0) sqlprint();}
#line 41 "array.pgc"


	{ ECPGtrans(__LINE__, NULL, "begin transaction ");
#line 43 "array.pgc"

if (sqlca.sqlcode < 0) sqlprint();}
#line 43 "array.pgc"
 

	{ ECPGdo(__LINE__, 0, 1, NULL, "select  f , text  from test where i = 1  ", ECPGt_EOIT, 
	ECPGt_double,&(f),(long)1,(long)1,sizeof(double), 
	ECPGt_NO_INDICATOR, NULL , 0L, 0L, 0L, 
	ECPGt_char,(text),(long)25,(long)1,(25)*sizeof(char), 
	ECPGt_NO_INDICATOR, NULL , 0L, 0L, 0L, ECPGt_EORT);
#line 48 "array.pgc"

if (sqlca.sqlcode < 0) sqlprint();}
#line 48 "array.pgc"


	printf("Found f=%f text=%10.10s\n", f, text);

	f=140787;
	{ ECPGdo(__LINE__, 0, 1, NULL, "select  a , text  from test where f =  ?  ", 
	ECPGt_double,&(f),(long)1,(long)1,sizeof(double), 
	ECPGt_NO_INDICATOR, NULL , 0L, 0L, 0L, ECPGt_EOIT, 
	ECPGt_int,(a),(long)1,(long)10,sizeof(int), 
	ECPGt_NO_INDICATOR, NULL , 0L, 0L, 0L, 
	ECPGt_char,&(t),(long)0,(long)1,(1)*sizeof(char), 
	ECPGt_NO_INDICATOR, NULL , 0L, 0L, 0L, ECPGt_EORT);
#line 56 "array.pgc"

if (sqlca.sqlcode < 0) sqlprint();}
#line 56 "array.pgc"


	for (i = 0; i < 10; i++)
		printf("Found a[%d] = %d\n", i, a[i]);

	printf("Found text=%10.10s\n", t);

	{ ECPGdo(__LINE__, 0, 1, NULL, "select  a  from test where f =  ?  ", 
	ECPGt_double,&(f),(long)1,(long)1,sizeof(double), 
	ECPGt_NO_INDICATOR, NULL , 0L, 0L, 0L, ECPGt_EOIT, 
	ECPGt_char,(text),(long)25,(long)1,(25)*sizeof(char), 
	ECPGt_NO_INDICATOR, NULL , 0L, 0L, 0L, ECPGt_EORT);
#line 66 "array.pgc"

if (sqlca.sqlcode < 0) sqlprint();}
#line 66 "array.pgc"


	printf("Found text=%s\n", text);

	{ ECPGdo(__LINE__, 0, 1, NULL, "drop table test ", ECPGt_EOIT, ECPGt_EORT);
#line 70 "array.pgc"

if (sqlca.sqlcode < 0) sqlprint();}
#line 70 "array.pgc"


	{ ECPGtrans(__LINE__, NULL, "commit");
#line 72 "array.pgc"

if (sqlca.sqlcode < 0) sqlprint();}
#line 72 "array.pgc"


	{ ECPGdisconnect(__LINE__, "CURRENT");
#line 74 "array.pgc"

if (sqlca.sqlcode < 0) sqlprint();}
#line 74 "array.pgc"


	return (0);
}
