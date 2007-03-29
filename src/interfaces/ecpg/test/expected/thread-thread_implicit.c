/* Processed by ecpg (regression mode) */
/* These include files are added by the preprocessor */
#include <ecpgtype.h>
#include <ecpglib.h>
#include <ecpgerrno.h>
#include <sqlca.h>
/* End of automatic include section */
#define ECPGdebug(X,Y) ECPGdebug((X)+100,(Y))

#line 1 "thread_implicit.pgc"
/*
 *	Thread test program
 *	by Lee Kindness.
 */

#include <stdlib.h>
#include "ecpg_config.h"

#ifndef ENABLE_THREAD_SAFETY
int
main(void)
{
	printf("No threading enabled.\n");
	return 0;
}
#else
#include <pthread.h>


#line 1 "regression.h"






#line 19 "thread_implicit.pgc"


void *test_thread(void *arg);

int nthreads   = 10;
int iterations = 20;

int main(int argc, char *argv[])
{
  pthread_t *threads;
  int n;
  /* exec sql begin declare section */
   
  
#line 31 "thread_implicit.pgc"
 int  l_rows    ;
/* exec sql end declare section */
#line 32 "thread_implicit.pgc"


 /* Do not switch on debug output for regression tests. The threads get executed in
  * more or less random order */
 /* ECPGdebug(1, stderr); */

  /* setup test_thread table */
  { ECPGconnect(__LINE__, 0, "regress1" , NULL, NULL , NULL, 0); }
#line 39 "thread_implicit.pgc"

  { ECPGdo(__LINE__, 0, 1, NULL, "drop table test_thread ", ECPGt_EOIT, ECPGt_EORT);}
#line 40 "thread_implicit.pgc"
 /* DROP might fail */
  { ECPGtrans(__LINE__, NULL, "commit");}
#line 41 "thread_implicit.pgc"

  { ECPGdo(__LINE__, 0, 1, NULL, "create  table test_thread ( tstamp timestamp    not null default cast( timeofday () as timestamp   ) , thread TEXT    not null , iteration integer   not null , primary key( thread , iteration )   )    ", ECPGt_EOIT, ECPGt_EORT);}
#line 46 "thread_implicit.pgc"

  { ECPGtrans(__LINE__, NULL, "commit");}
#line 47 "thread_implicit.pgc"

  { ECPGdisconnect(__LINE__, "CURRENT");}
#line 48 "thread_implicit.pgc"


  /* create, and start, threads */
  threads = calloc(nthreads, sizeof(pthread_t));
  if( threads == NULL )
    {
      fprintf(stderr, "Cannot alloc memory\n");
      return( 1 );
    }
  for( n = 0; n < nthreads; n++ )
    {
      pthread_create(&threads[n], NULL, test_thread, (void *) (n + 1));
    }

  /* wait for thread completion */
  for( n = 0; n < nthreads; n++ )
    {
      pthread_join(threads[n], NULL);
    }
  free(threads);

  /* and check results */
  { ECPGconnect(__LINE__, 0, "regress1" , NULL, NULL , NULL, 0); }
#line 70 "thread_implicit.pgc"

  { ECPGdo(__LINE__, 0, 1, NULL, "select  count (*)  from test_thread   ", ECPGt_EOIT, 
	ECPGt_int,&(l_rows),(long)1,(long)1,sizeof(int), 
	ECPGt_NO_INDICATOR, NULL , 0L, 0L, 0L, ECPGt_EORT);}
#line 71 "thread_implicit.pgc"

  { ECPGtrans(__LINE__, NULL, "commit");}
#line 72 "thread_implicit.pgc"

  { ECPGdisconnect(__LINE__, "CURRENT");}
#line 73 "thread_implicit.pgc"

  if( l_rows == (nthreads * iterations) )
    printf("Success.\n");
  else
    printf("ERROR: Failure - expecting %d rows, got %d.\n", nthreads * iterations, l_rows);

  return( 0 );
}

void *test_thread(void *arg)
{
  long threadnum = (long)arg;
  /* exec sql begin declare section */
    
   
  
#line 86 "thread_implicit.pgc"
 int  l_i    ;
 
#line 87 "thread_implicit.pgc"
 char  l_connection [ 128 ]    ;
/* exec sql end declare section */
#line 88 "thread_implicit.pgc"


  /* build up connection name, and connect to database */
  snprintf(l_connection, sizeof(l_connection), "thread_%03ld", threadnum);
  /* exec sql whenever sqlerror  sqlprint ; */
#line 92 "thread_implicit.pgc"

  { ECPGconnect(__LINE__, 0, "regress1" , NULL, NULL , l_connection, 0); 
#line 93 "thread_implicit.pgc"

if (sqlca.sqlcode < 0) sqlprint();}
#line 93 "thread_implicit.pgc"

  if( sqlca.sqlcode != 0 )
    {
      printf("%s: ERROR: cannot connect to database!\n", l_connection);
      return( NULL );
    }
  { ECPGtrans(__LINE__, NULL, "begin transaction ");
#line 99 "thread_implicit.pgc"

if (sqlca.sqlcode < 0) sqlprint();}
#line 99 "thread_implicit.pgc"


  /* insert into test_thread table */
  for( l_i = 1; l_i <= iterations; l_i++ )
    {
      { ECPGdo(__LINE__, 0, 1, NULL, "insert into test_thread ( thread  , iteration  ) values (  ? ,  ? ) ", 
	ECPGt_char,(l_connection),(long)128,(long)1,(128)*sizeof(char), 
	ECPGt_NO_INDICATOR, NULL , 0L, 0L, 0L, 
	ECPGt_int,&(l_i),(long)1,(long)1,sizeof(int), 
	ECPGt_NO_INDICATOR, NULL , 0L, 0L, 0L, ECPGt_EOIT, ECPGt_EORT);
#line 104 "thread_implicit.pgc"

if (sqlca.sqlcode < 0) sqlprint();}
#line 104 "thread_implicit.pgc"

      if( sqlca.sqlcode != 0 )
	printf("%s: ERROR: insert failed!\n", l_connection);
    }

  /* all done */
  { ECPGtrans(__LINE__, NULL, "commit");
#line 110 "thread_implicit.pgc"

if (sqlca.sqlcode < 0) sqlprint();}
#line 110 "thread_implicit.pgc"

  { ECPGdisconnect(__LINE__, l_connection);
#line 111 "thread_implicit.pgc"

if (sqlca.sqlcode < 0) sqlprint();}
#line 111 "thread_implicit.pgc"

  return( NULL );
}
#endif /* ENABLE_THREAD_SAFETY */
