/*-------------------------------------------------------------------------
 *
 * s_lock.c
 *	  Spinlock support routines
 *
 * Portions Copyright (c) 1996-2001, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  $Header: /home/cvsmirror/pg/pgsql/src/backend/storage/buffer/Attic/s_lock.c,v 1.33 2001/02/18 04:39:42 tgl Exp $
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include <sys/time.h>
#include <unistd.h>

#include "miscadmin.h"
#include "storage/s_lock.h"


/*----------
 * Each time we busy spin we select the next element of this array as the
 * number of microseconds to wait. This accomplishes pseudo random back-off.
 *
 * Note that on most platforms, specified values will be rounded up to the
 * next multiple of a clock tick, which is often ten milliseconds (10000).
 * So, we are being way overoptimistic to assume that these different values
 * are really different, other than the last.  But there are a few platforms
 * with better-than-usual timekeeping, and on these we will get pretty good
 * pseudo-random behavior.
 *
 * Total time to cycle through all 20 entries will be at least 100 msec,
 * more commonly (10 msec resolution) 220 msec, and on some platforms
 * as much as 420 msec (when the remainder of the current tick cycle is
 * ignored in deciding when to time out, as on FreeBSD and older Linuxen).
 * We use the 100msec figure to figure max_spins, so actual timeouts may
 * be as much as four times the nominal value, but will never be less.
 *----------
 */
#define S_NSPINCYCLE	20

int			s_spincycle[S_NSPINCYCLE] =
{	1,		10,		100,	1000,
	10000,	1000,	1000,	1000,
	10000,	1000,	1000,	10000,
	1000,	1000,	10000,	1000,
	10000,	1000,	10000,	30000
};

#define AVG_SPINCYCLE	5000	/* average entry in microsec: 100ms / 20 */

#define DEFAULT_TIMEOUT	(100*1000000) /* default timeout: 100 sec */


/*
 * s_lock_stuck() - complain about a stuck spinlock
 */
static void
s_lock_stuck(volatile slock_t *lock, const char *file, const int line)
{
	fprintf(stderr,
			"\nFATAL: s_lock(%p) at %s:%d, stuck spinlock. Aborting.\n",
			lock, file, line);
	fprintf(stdout,
			"\nFATAL: s_lock(%p) at %s:%d, stuck spinlock. Aborting.\n",
			lock, file, line);
	abort();
}


/*
 * s_lock_sleep() - sleep a pseudo-random amount of time, check for timeout
 *
 * The 'timeout' is given in microsec, or may be 0 for "infinity".  Note that
 * this will be a lower bound (a fairly loose lower bound, on most platforms).
 *
 * 'microsec' is the number of microsec to delay per loop.  Normally
 * 'microsec' is 0, specifying to use the next s_spincycle[] value.
 * Some callers may pass a nonzero interval, specifying to use exactly that
 * delay value rather than a pseudo-random delay.
 */
void
s_lock_sleep(unsigned spins, int timeout, int microsec,
			 volatile slock_t *lock,
			 const char *file, const int line)
{
	struct timeval delay;

	if (microsec > 0)
	{
		delay.tv_sec = 0;
		delay.tv_usec = microsec;
	}
	else
	{
		delay.tv_sec = 0;
		delay.tv_usec = s_spincycle[spins % S_NSPINCYCLE];
		microsec = AVG_SPINCYCLE; /* use average to figure timeout */
	}

	if (timeout > 0)
	{
		unsigned	max_spins = timeout / microsec;

		if (spins > max_spins)
			s_lock_stuck(lock, file, line);
	}

	(void) select(0, NULL, NULL, NULL, &delay);
}


/*
 * s_lock(lock) - take a spinlock with backoff
 */
void
s_lock(volatile slock_t *lock, const char *file, const int line)
{
	unsigned	spins = 0;

	/*
	 * If you are thinking of changing this code, be careful.  This same
	 * loop logic is used in other places that call TAS() directly.
	 *
	 * While waiting for a lock, we check for cancel/die interrupts (which
	 * is a no-op if we are inside a critical section).  The interrupt check
	 * can be omitted in places that know they are inside a critical section.
	 * Note that an interrupt must NOT be accepted after acquiring the lock.
	 */
	while (TAS(lock))
	{
		s_lock_sleep(spins++, DEFAULT_TIMEOUT, 0, lock, file, line);
		CHECK_FOR_INTERRUPTS();
	}
}

/*
 * Various TAS implementations that cannot live in s_lock.h as no inline
 * definition exists (yet).
 * In the future, get rid of tas.[cso] and fold it into this file.
 */


#if defined(__GNUC__)
/*************************************************************************
 * All the gcc flavors that are not inlined
 */


#if defined(__m68k__)
static void
tas_dummy()						/* really means: extern int tas(slock_t
								 * **lock); */
{
	__asm__ __volatile__(
"\
.global		_tas				\n\
_tas:							\n\
			movel	sp@(0x4),a0	\n\
			tas 	a0@			\n\
			beq 	_success	\n\
			moveq 	#-128,d0	\n\
			rts					\n\
_success:						\n\
			moveq 	#0,d0		\n\
			rts					\n\
");
}

#endif	 /* __m68k__ */

#if defined(__APPLE__) && defined(__ppc__)
/* used in darwin. */
/* We key off __APPLE__ here because this function differs from
 * the LinuxPPC implementation only in compiler syntax.
 */
static void
tas_dummy()
{
	   __asm__ __volatile__(
"\
			.globl 	tas			\n\
			.globl 	_tas		\n\
_tas:							\n\
tas:							\n\
			lwarx	r5,0,r3		\n\
			cmpwi 	r5,0		\n\
			bne 	fail		\n\
			addi 	r5,r5,1		\n\
			stwcx. 	r5,0,r3		\n\
			beq 	success		\n\
fail:		li 		r3,1		\n\
			blr 				\n\
success:						\n\
			li 		r3,0		\n\
			blr					\n\
");
}

#endif  /* __APPLE__ && __ppc__ */

#if defined(__powerpc__)
/* Note: need a nice gcc constrained asm version so it can be inlined */
static void
tas_dummy()
{
	__asm__ __volatile__(
"\
.global tas 					\n\
tas:							\n\
			lwarx	5,0,3		\n\
			cmpwi 	5,0 		\n\
			bne 	fail		\n\
			addi 	5,5,1		\n\
			stwcx.	5,0,3		\n\
			beq 	success 	\n\
fail:		li		3,1 		\n\
			blr 				\n\
success:						\n\
			li 		3,0			\n\
			blr					\n\
");
}

#endif	 /* __powerpc__ */

#if defined(__mips__)
static void
tas_dummy()
{
	__asm__  _volatile__(
"\
.global	tas						\n\
tas:							\n\
			.frame	$sp, 0, $31	\n\
			ll		$14, 0($4)	\n\
			or		$15, $14, 1	\n\
			sc		$15, 0($4)	\n\
			beq		$15, 0, fail\n\
			bne		$14, 0, fail\n\
			li		$2, 0		\n\
			.livereg 0x2000FF0E,0x00000FFF	\n\
			j		$31			\n\
fail:							\n\
			li		$2, 1		\n\
			j   	$31			\n\
");
}

#endif	 /* __mips__ */

#else							/* defined(__GNUC__) */
/***************************************************************************
 * All non gcc
 */



#if defined(sun3)
static void
tas_dummy()						/* really means: extern int tas(slock_t
								 * *lock); */
{
	asm("LLA0:");
	asm("   .data");
	asm("   .text");
	asm("|#PROC# 04");
	asm("   .globl  _tas");
	asm("_tas:");
	asm("|#PROLOGUE# 1");
	asm("   movel   sp@(0x4),a0");
	asm("   tas a0@");
	asm("   beq LLA1");
	asm("   moveq   #-128,d0");
	asm("   rts");
	asm("LLA1:");
	asm("   moveq   #0,d0");
	asm("   rts");
	asm("   .data");
}

#endif	 /* sun3 */



#if defined(NEED_SPARC_TAS_ASM)
/*
 * sparc machines not using gcc
 */
static void
tas_dummy()						/* really means: extern int tas(slock_t
								 * *lock); */
{
	asm(".seg \"data\"");
	asm(".seg \"text\"");
	asm("_tas:");

	/*
	 * Sparc atomic test and set (sparc calls it "atomic load-store")
	 */
	asm("ldstub [%r8], %r8");
	asm("retl");
	asm("nop");
}

#endif	 /* NEED_SPARC_TAS_ASM */




#if defined(NEED_I386_TAS_ASM)
/* non gcc i386 based things */
#endif	 /* NEED_I386_TAS_ASM */



#endif	 /* not __GNUC__ */




/*****************************************************************************/
#if defined(S_LOCK_TEST)

/*
 * test program for verifying a port.
 */

volatile slock_t test_lock;

void
main()
{
	S_INIT_LOCK(&test_lock);

	if (!S_LOCK_FREE(&test_lock))
	{
		printf("S_LOCK_TEST: failed, lock not initialized.\n");
		exit(1);
	}

	S_LOCK(&test_lock);

	if (S_LOCK_FREE(&test_lock))
	{
		printf("S_LOCK_TEST: failed, lock not locked\n");
		exit(2);
	}

	printf("S_LOCK_TEST: this will hang for a few minutes and then abort\n");
	printf("             with a 'stuck spinlock' message if S_LOCK()\n");
	printf("             and TAS() are working.\n");
	s_lock(&test_lock, __FILE__, __LINE__);

	printf("S_LOCK_TEST: failed, lock not locked~\n");
	exit(3);

}

#endif	 /* S_LOCK_TEST */
