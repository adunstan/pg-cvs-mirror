/*-------------------------------------------------------------------------
 *
 * libpq-int.h--
 *	  This file contains internal definitions meant to be used only by
 *	  the frontend libpq library, not by applications that call it.
 *
 *	  An application can include this file if it wants to bypass the
 *	  official API defined by libpq-fe.h, but code that does so is much
 *	  more likely to break across PostgreSQL releases than code that uses
 *	  only the official API.
 *
 * Copyright (c) 1994, Regents of the University of California
 *
 * $Id: libpq-int.h,v 1.3 1998/09/03 02:10:53 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */

#ifndef LIBPQ_INT_H
#define LIBPQ_INT_H

/* We assume libpq-fe.h has already been included. */

/* ----------------
 *		include stuff common to fe and be
 * ----------------
 */
#include "libpq/pqcomm.h"
#include "lib/dllist.h"

/* libpq supports this version of the frontend/backend protocol.
 *
 * NB: we used to use PG_PROTOCOL_LATEST from the backend pqcomm.h file,
 * but that's not really the right thing: just recompiling libpq
 * against a more recent backend isn't going to magically update it
 * for most sorts of protocol changes.	So, when you change libpq
 * to support a different protocol revision, you have to change this
 * constant too.  PG_PROTOCOL_EARLIEST and PG_PROTOCOL_LATEST in
 * pqcomm.h describe what the backend knows, not what libpq knows.
 */

#define PG_PROTOCOL_LIBPQ	PG_PROTOCOL(2,0)

/*
 * POSTGRES backend dependent Constants.
 */

/* ERROR_MSG_LENGTH should really be the same as ELOG_MAXLEN in utils/elog.h*/
#define ERROR_MSG_LENGTH 4096
#define CMDSTATUS_LEN 40

/* PGresult and the subsidiary types PGresAttDesc, PGresAttValue
 * represent the result of a query (or more precisely, of a single SQL
 * command --- a query string given to PQexec can contain multiple commands).
 * Note we assume that a single command can return at most one tuple group,
 * hence there is no need for multiple descriptor sets.
 */
	typedef struct pgresAttDesc
	{
		char	   *name;		/* type name */
		Oid			typid;		/* type id */
		int			typlen;		/* type size */
		int			atttypmod;	/* type-specific modifier info */
	} PGresAttDesc;

/* use char* for Attribute values,
   ASCII tuples are guaranteed to be null-terminated
   For binary tuples, the first four bytes of the value is the size,
   and the bytes afterwards are the value.	The binary value is
   not guaranteed to be null-terminated.  In fact, it can have embedded nulls
 */

#define NULL_LEN		(-1)	/* pg_result len for NULL value */

	typedef struct pgresAttValue
	{
		int			len;		/* length in bytes of the value */
		char	   *value;		/* actual value */
	} PGresAttValue;

 struct pg_result
	{
		int			ntups;
		int			numAttributes;
		PGresAttDesc *attDescs;
		PGresAttValue **tuples; /* each PGresTuple is an array of
								 * PGresAttValue's */
		int			tupArrSize; /* size of tuples array allocated */
		ExecStatusType resultStatus;
		char		cmdStatus[CMDSTATUS_LEN];	/* cmd status from the
												 * last insert query */
		int			binary;		/* binary tuple values if binary == 1,
								 * otherwise ASCII */
		PGconn		*conn;		/* connection we did the query on */
	};

/* PGAsyncStatusType defines the state of the query-execution state machine */
	typedef enum
	{
		PGASYNC_IDLE,			/* nothing's happening, dude */
		PGASYNC_BUSY,			/* query in progress */
		PGASYNC_READY,			/* result ready for PQgetResult */
		PGASYNC_COPY_IN,		/* Copy In data transfer in progress */
		PGASYNC_COPY_OUT		/* Copy Out data transfer in progress */
	} PGAsyncStatusType;

/* large-object-access data ... allocated only if large-object code is used. */
	typedef struct pgLobjfuncs
	{
		Oid			fn_lo_open; /* OID of backend function lo_open		*/
		Oid			fn_lo_close;/* OID of backend function lo_close		*/
		Oid			fn_lo_creat;/* OID of backend function lo_creat		*/
		Oid			fn_lo_unlink;		/* OID of backend function
										 * lo_unlink	*/
		Oid			fn_lo_lseek;/* OID of backend function lo_lseek		*/
		Oid			fn_lo_tell; /* OID of backend function lo_tell		*/
		Oid			fn_lo_read; /* OID of backend function LOread		*/
		Oid			fn_lo_write;/* OID of backend function LOwrite		*/
	} PGlobjfuncs;

/* PGconn stores all the state data associated with a single connection
 * to a backend.
 */
	struct pg_conn
	{
		/* Saved values of connection options */
		char	   *pghost;		/* the machine on which the server is
								 * running */
		char	   *pgport;		/* the server's communication port */
		char	   *pgtty;		/* tty on which the backend messages is
								 * displayed (NOT ACTUALLY USED???) */
		char	   *pgoptions;	/* options to start the backend with */
		char	   *dbName;		/* database name */
		char	   *pguser;		/* Postgres username and password, if any */
		char	   *pgpass;

		/* Optional file to write trace info to */
		FILE	   *Pfdebug;

		/* Callback procedure for notice/error message processing */
		PQnoticeProcessor	noticeHook;
		void	   *noticeArg;

		/* Status indicators */
		ConnStatusType		status;
		PGAsyncStatusType	asyncStatus;
		Dllist	   *notifyList;	/* Notify msgs not yet handed to application */

		/* Connection data */
		int			sock;		/* Unix FD for socket, -1 if not connected */
		SockAddr	laddr;		/* Local address */
		SockAddr	raddr;		/* Remote address */
		int			raddr_len;	/* Length of remote address */

		/* Miscellaneous stuff */
		int			be_pid;		/* PID of backend --- needed for cancels */
		int			be_key;		/* key of backend --- needed for cancels */
		char		salt[2];	/* password salt received from backend */
		PGlobjfuncs *lobjfuncs; /* private state for large-object access fns */

		/* Buffer for data received from backend and not yet processed */
		char		*inBuffer;	/* currently allocated buffer */
		int			inBufSize;	/* allocated size of buffer */
		int			inStart;	/* offset to first unconsumed data in buffer */
		int			inCursor;	/* next byte to tentatively consume */
		int			inEnd;		/* offset to first position after avail data */

		/* Buffer for data not yet sent to backend */
		char		*outBuffer;	/* currently allocated buffer */
		int			outBufSize;	/* allocated size of buffer */
		int			outCount;	/* number of chars waiting in buffer */

		/* Status for asynchronous result construction */
		PGresult		*result;	/* result being constructed */
		PGresAttValue	*curTuple;	/* tuple currently being read */

		/* Message space.  Placed last for code-size reasons.
		 * errorMessage is the message last returned to the application.
		 * When asyncStatus=READY, asyncErrorMessage is the pending message
		 * that will be put in errorMessage by PQgetResult. */
		char		errorMessage[ERROR_MSG_LENGTH];
		char		asyncErrorMessage[ERROR_MSG_LENGTH];
	};

/* ----------------
 * Internal functions of libpq
 * Functions declared here need to be visible across files of libpq,
 * but are not intended to be called by applications.  We use the
 * convention "pqXXX" for internal functions, vs. the "PQxxx" names
 * used for application-visible routines.
 * ----------------
 */

/* === in fe-connect.c === */

extern int	pqPacketSend(PGconn *conn, const char *buf, size_t len);

/* === in fe-exec.c === */

extern void pqClearAsyncResult(PGconn *conn);

/* === in fe-misc.c === */

 /*
  * "Get" and "Put" routines return 0 if successful, EOF if not. Note that
  * for Get, EOF merely means the buffer is exhausted, not that there is
  * necessarily any error.
  */
extern int	pqGetc(char *result, PGconn *conn);
extern int	pqGets(char *s, int maxlen, PGconn *conn);
extern int	pqPuts(const char *s, PGconn *conn);
extern int	pqGetnchar(char *s, int len, PGconn *conn);
extern int	pqPutnchar(const char *s, int len, PGconn *conn);
extern int	pqGetInt(int *result, int bytes, PGconn *conn);
extern int	pqPutInt(int value, int bytes, PGconn *conn);
extern int	pqReadData(PGconn *conn);
extern int	pqFlush(PGconn *conn);
extern int	pqWait(int forRead, int forWrite, PGconn *conn);

/* max length of message to send  */
#define MAX_MESSAGE_LEN 8193

/* maximum number of fields in a tuple */
#define MAX_FIELDS 512

/* bits in a byte */
#define BYTELEN 8

/* fall back options if they are not specified by arguments or defined
   by environment variables */
#define DefaultHost		"localhost"
#define DefaultTty		""
#define DefaultOption	""
#define DefaultAuthtype		  ""
#define DefaultPassword		  ""

/* supply an implementation of strerror() macro if system doesn't have it */
#ifndef strerror
#if defined(sun) && defined(sparc) && !defined(__SVR4)
extern char *sys_errlist[];

#define strerror(A) (sys_errlist[(A)])
#endif	 /* sunos4 */
#endif	 /* !strerror */

#endif	 /* LIBPQ_INT_H */
