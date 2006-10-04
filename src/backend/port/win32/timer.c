/*-------------------------------------------------------------------------
 *
 * timer.c
 *	  Microsoft Windows Win32 Timer Implementation
 *
 *	  Limitations of this implementation:
 *
 *	  - Does not support interval timer (value->it_interval)
 *	  - Only supports ITIMER_REAL
 *
 * Portions Copyright (c) 1996-2006, PostgreSQL Global Development Group
 *
 * IDENTIFICATION
 *	  $PostgreSQL: pgsql/src/backend/port/win32/timer.c,v 1.12 2006/08/09 21:18:13 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */

#include "postgres.h"

#include "libpq/pqsignal.h"


/* Communication area for inter-thread communication */
typedef struct timerCA
{
	struct itimerval value;
	HANDLE		event;
	CRITICAL_SECTION crit_sec;
}	timerCA;

static timerCA timerCommArea;
static HANDLE timerThreadHandle = INVALID_HANDLE_VALUE;


/* Timer management thread */
static DWORD WINAPI
pg_timer_thread(LPVOID param)
{
	DWORD		waittime;

	Assert(param == NULL);

	waittime = INFINITE;

	for (;;)
	{
		int			r;

		r = WaitForSingleObjectEx(timerCommArea.event, waittime, FALSE);
		if (r == WAIT_OBJECT_0)
		{
			/* Event signalled from main thread, change the timer */
			EnterCriticalSection(&timerCommArea.crit_sec);
			if (timerCommArea.value.it_value.tv_sec == 0 &&
				timerCommArea.value.it_value.tv_usec == 0)
				waittime = INFINITE;	/* Cancel the interrupt */
			else
			{
				/* WaitForSingleObjectEx() uses milliseconds, round up */
				waittime = (timerCommArea.value.it_value.tv_usec + 999) / 1000 +
					timerCommArea.value.it_value.tv_sec * 1000;
			}
			ResetEvent(timerCommArea.event);
			LeaveCriticalSection(&timerCommArea.crit_sec);
		}
		else if (r == WAIT_TIMEOUT)
		{
			/* Timeout expired, signal SIGALRM and turn it off */
			pg_queue_signal(SIGALRM);
			waittime = INFINITE;
		}
		else
		{
			/* Should never happen */
			Assert(false);
		}
	}

	return 0;
}

/*
 * Win32 setitimer emulation by creating a persistent thread
 * to handle the timer setting and notification upon timeout.
 */
int
setitimer(int which, const struct itimerval * value, struct itimerval * ovalue)
{
	Assert(value != NULL);
	Assert(value->it_interval.tv_sec == 0 && value->it_interval.tv_usec == 0);
	Assert(which == ITIMER_REAL);

	if (timerThreadHandle == INVALID_HANDLE_VALUE)
	{
		/* First call in this backend, create event and the timer thread */
		timerCommArea.event = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (timerCommArea.event == NULL)
			ereport(FATAL,
					(errmsg_internal("failed to create timer event: %d",
									 (int) GetLastError())));

		MemSet(&timerCommArea.value, 0, sizeof(struct itimerval));

		InitializeCriticalSection(&timerCommArea.crit_sec);

		timerThreadHandle = CreateThread(NULL, 0, pg_timer_thread, NULL, 0, NULL);
		if (timerThreadHandle == INVALID_HANDLE_VALUE)
			ereport(FATAL,
					(errmsg_internal("failed to create timer thread: %d",
									 (int) GetLastError())));
	}

	/* Request the timer thread to change settings */
	EnterCriticalSection(&timerCommArea.crit_sec);
	if (ovalue)
		*ovalue = timerCommArea.value;
	timerCommArea.value = *value;
	LeaveCriticalSection(&timerCommArea.crit_sec);
	SetEvent(timerCommArea.event);

	return 0;
}
