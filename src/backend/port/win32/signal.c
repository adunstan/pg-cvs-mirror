/*-------------------------------------------------------------------------
 *
 * signal.c
 *	  Microsoft Windows Win32 Signal Emulation Functions
 *
 * Portions Copyright (c) 1996-2004, PostgreSQL Global Development Group
 *
 * IDENTIFICATION
 *	  $PostgreSQL: pgsql-server/src/backend/port/win32/signal.c,v 1.6 2004/08/29 04:12:46 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */

#include "postgres.h"

#include <libpq/pqsignal.h>


/* pg_signal_crit_sec is used to protect only pg_signal_queue. That is the only
 * variable that can be accessed from the signal sending threads! */
static CRITICAL_SECTION pg_signal_crit_sec;
static int	pg_signal_queue;

static pqsigfunc pg_signal_array[PG_SIGNAL_COUNT];
static pqsigfunc pg_signal_defaults[PG_SIGNAL_COUNT];
static int	pg_signal_mask;

DLLIMPORT HANDLE pgwin32_signal_event;


/* Signal handling thread function */
static DWORD WINAPI pg_signal_thread(LPVOID param);
static BOOL WINAPI pg_console_handler(DWORD dwCtrlType);

/* Sleep function that can be interrupted by signals */
void
pgwin32_backend_usleep(long microsec)
{
	if (WaitForSingleObject(pgwin32_signal_event, (microsec < 500 ? 1 : (microsec + 500) / 1000)) == WAIT_OBJECT_0)
	{
		pgwin32_dispatch_queued_signals();
		errno = EINTR;
		return;
	}
}


/* Initialization */
void
pgwin32_signal_initialize(void)
{
	int			i;
	HANDLE		signal_thread_handle;

	InitializeCriticalSection(&pg_signal_crit_sec);

	for (i = 0; i < PG_SIGNAL_COUNT; i++)
	{
		pg_signal_array[i] = SIG_DFL;
		pg_signal_defaults[i] = SIG_IGN;
	}
	pg_signal_mask = 0;
	pg_signal_queue = 0;

	/* Create the global event handle used to flag signals */
	pgwin32_signal_event = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (pgwin32_signal_event == NULL)
		ereport(FATAL,
				(errmsg_internal("failed to create signal event: %d", (int) GetLastError())));

	/* Create thread for handling signals */
	signal_thread_handle = CreateThread(NULL, 0, pg_signal_thread, NULL, 0, NULL);
	if (signal_thread_handle == NULL)
		ereport(FATAL,
			(errmsg_internal("failed to create signal handler thread")));

	/* Create console control handle to pick up Ctrl-C etc */
	if (!SetConsoleCtrlHandler(pg_console_handler, TRUE))
		ereport(FATAL,
			 (errmsg_internal("failed to set console control handler")));
}


/* Dispatch all signals currently queued and not blocked
 * Blocked signals are ignored, and will be fired at the time of
 * the sigsetmask() call. */
void
pgwin32_dispatch_queued_signals(void)
{
	int			i;

	EnterCriticalSection(&pg_signal_crit_sec);
	while (pg_signal_queue & ~pg_signal_mask)
	{
		/* One or more unblocked signals queued for execution */

		int			exec_mask = pg_signal_queue & ~pg_signal_mask;

		for (i = 0; i < PG_SIGNAL_COUNT; i++)
		{
			if (exec_mask & sigmask(i))
			{
				/* Execute this signal */
				pqsigfunc	sig = pg_signal_array[i];

				if (sig == SIG_DFL)
					sig = pg_signal_defaults[i];
				pg_signal_queue &= ~sigmask(i);
				if (sig != SIG_ERR && sig != SIG_IGN && sig != SIG_DFL)
				{
					LeaveCriticalSection(&pg_signal_crit_sec);
					sig(i);
					EnterCriticalSection(&pg_signal_crit_sec);
					break;		/* Restart outer loop, in case signal mask
								 * or queue has been modified inside
								 * signal handler */
				}
			}
		}
	}
	ResetEvent(pgwin32_signal_event);
	LeaveCriticalSection(&pg_signal_crit_sec);
}

/* signal masking. Only called on main thread, no sync required */
int
pqsigsetmask(int mask)
{
	int			prevmask;

	prevmask = pg_signal_mask;
	pg_signal_mask = mask;

	/*
	 * Dispatch any signals queued up right away, in case we have
	 * unblocked one or more signals previously queued
	 */
	pgwin32_dispatch_queued_signals();

	return prevmask;
}


/* signal manipulation. Only called on main thread, no sync required */
pqsigfunc
pqsignal(int signum, pqsigfunc handler)
{
	pqsigfunc	prevfunc;

	if (signum >= PG_SIGNAL_COUNT || signum < 0)
		return SIG_ERR;
	prevfunc = pg_signal_array[signum];
	pg_signal_array[signum] = handler;
	return prevfunc;
}

/*
 * All functions below execute on the signal handler thread
 * and must be synchronized as such!
 * NOTE! The only global variable that can be used is
 * pg_signal_queue!
 */


void
pg_queue_signal(int signum)
{
	if (signum >= PG_SIGNAL_COUNT || signum <= 0)
		return;

	EnterCriticalSection(&pg_signal_crit_sec);
	pg_signal_queue |= sigmask(signum);
	LeaveCriticalSection(&pg_signal_crit_sec);

	SetEvent(pgwin32_signal_event);
}

/* Signal dispatching thread */
static DWORD WINAPI
pg_signal_dispatch_thread(LPVOID param)
{
	HANDLE		pipe = (HANDLE) param;
	BYTE		sigNum;
	DWORD		bytes;

	if (!ReadFile(pipe, &sigNum, 1, &bytes, NULL))
	{
		/* Client died before sending */
		CloseHandle(pipe);
		return 0;
	}
	if (bytes != 1)
	{
		/* Received <bytes> bytes over signal pipe (should be 1) */
		CloseHandle(pipe);
		return 0;
	}
	WriteFile(pipe, &sigNum, 1, &bytes, NULL);	/* Don't care if it works
												 * or not.. */
	FlushFileBuffers(pipe);
	DisconnectNamedPipe(pipe);
	CloseHandle(pipe);

	pg_queue_signal(sigNum);
	return 0;
}

/* Signal handling thread */
static DWORD WINAPI
pg_signal_thread(LPVOID param)
{
	char		pipename[128];
	HANDLE		pipe = INVALID_HANDLE_VALUE;

	wsprintf(pipename, "\\\\.\\pipe\\pgsignal_%d", GetCurrentProcessId());

	for (;;)
	{
		BOOL		fConnected;
		HANDLE		hThread;

		pipe = CreateNamedPipe(pipename, PIPE_ACCESS_DUPLEX,
				   PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
						   PIPE_UNLIMITED_INSTANCES, 16, 16, 1000, NULL);
		if (pipe == INVALID_HANDLE_VALUE)
		{
			write_stderr("failed to create signal listener pipe: %d. Retrying.\n", (int) GetLastError());
			SleepEx(500, FALSE);
			continue;
		}

		fConnected = ConnectNamedPipe(pipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
		if (fConnected)
		{
			hThread = CreateThread(NULL, 0,
					  (LPTHREAD_START_ROUTINE) pg_signal_dispatch_thread,
								   (LPVOID) pipe, 0, NULL);
			if (hThread == INVALID_HANDLE_VALUE)
				write_stderr("failed to create signal dispatch thread: %d\n",
							 (int) GetLastError());
			else
				CloseHandle(hThread);
		}
		else
			/* Connection failed. Cleanup and try again */
			CloseHandle(pipe);
	}
	return 0;
}


/* Console control handler will execute on a thread created
   by the OS at the time of invocation */
static BOOL WINAPI
pg_console_handler(DWORD dwCtrlType)
{
	if (dwCtrlType == CTRL_C_EVENT ||
		dwCtrlType == CTRL_BREAK_EVENT ||
		dwCtrlType == CTRL_CLOSE_EVENT ||
		dwCtrlType == CTRL_SHUTDOWN_EVENT)
	{
		pg_queue_signal(SIGINT);
		return TRUE;
	}
	return FALSE;
}
