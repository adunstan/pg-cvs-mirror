/*-------------------------------------------------------------------------
 *
 * postmaster.c--
 *	  This program acts as a clearing house for requests to the
 *	  POSTGRES system.	Frontend programs send a startup message
 *	  to the Postmaster and the postmaster uses the info in the
 *	  message to setup a backend process.
 *
 * Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  $Header: /home/cvsmirror/pg/pgsql/src/backend/postmaster/postmaster.c,v 1.95 1998/08/25 21:33:59 scrappy Exp $
 *
 * NOTES
 *
 * Initialization:
 *		The Postmaster sets up a few shared memory data structures
 *		for the backends.  It should at the very least initialize the
 *		lock manager.
 *
 * Synchronization:
 *		The Postmaster shares memory with the backends and will have to lock
 *		the shared memory it accesses.	The Postmaster should never block
 *		on messages from clients.
 *
 * Garbage Collection:
 *		The Postmaster cleans up after backends if they have an emergency
 *		exit and/or core dump.
 *
 * Communication:
 *
 *-------------------------------------------------------------------------
 */
 /* moved here to prevent double define */
#include <sys/param.h>			/* for MAXHOSTNAMELEN on most */
#ifdef HAVE_NETDB_H
#include <netdb.h>				/* for MAXHOSTNAMELEN on some */
#endif

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 256
#endif

#include "postgres.h"

#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#if !defined(NO_UNISTD_H)
#include <unistd.h>
#endif							/* !NO_UNISTD_H */

#include <ctype.h>
#include <sys/types.h>			/* for fd_set stuff */
#include <sys/stat.h>			/* for umask */
#include <sys/time.h>
#include <sys/socket.h>
#ifdef HAVE_LIMITS_H
#include <limits.h>
#else
#include <values.h>
#endif
#include <sys/wait.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#include "storage/ipc.h"
#include "libpq/libpq.h"
#include "libpq/auth.h"
#include "libpq/pqcomm.h"
#include "libpq/pqsignal.h"
#include "libpq/crypt.h"
#include "miscadmin.h"
#include "version.h"
#include "lib/dllist.h"
#include "tcop/tcopprot.h"
#include "commands/async.h"
#include "nodes/nodes.h"
#include "utils/mcxt.h"
#include "storage/proc.h"
#include "utils/elog.h"
#ifndef HAVE_GETHOSTNAME
#include "port-protos.h"		/* For gethostname() */
#endif
#include "storage/fd.h"
#include "utils/trace.h"

#if !defined(MAXINT)
#define MAXINT		   INT_MAX
#endif

#define INVALID_SOCK	(-1)
#define ARGV_SIZE	64

 /*
  * Max time in seconds for socket to linger (close() to block) waiting
  * for frontend to retrieve its message from us.
  */

/*
 * Info for garbage collection.  Whenever a process dies, the Postmaster
 * cleans up after it.	Currently, NO information is required for cleanup,
 * but I left this structure around in case that changed.
 */
typedef struct bkend
{
	int			pid;			/* process id of backend */
	long		cancel_key;		/* cancel key for cancels for this backend */
} Backend;

Port *MyBackendPort = NULL;

/* list of active backends.  For garbage collection only now. */

static Dllist *BackendList;

/* list of ports associated with still open, but incomplete connections */
static Dllist *PortList;

static short PostPortName = -1;
static short ActiveBackends = FALSE;

 /*
  * This is a boolean indicating that there is at least one backend that
  * is accessing the current shared memory and semaphores. Between the
  * time that we start up, or throw away shared memory segments and start
  * over, and the time we generate the next backend (because we received a
  * connection request), it is false. Other times, it is true.
  */
static short shmem_seq = 0;

 /*
  * This is a sequence number that indicates how many times we've had to
  * throw away the shared memory and start over because we doubted its
  * integrity.	It starts off at zero and is incremented every time we
  * start over.  We use this to ensure that we use a new IPC shared memory
  * key for the new shared memory segment in case the old segment isn't
  * entirely gone yet.
  *
  * The sequence actually cycles back to 0 after 9, so pathologically there
  * could be an IPC failure if 10 sets of backends are all stuck and won't
  * release IPC resources.
  */

static IpcMemoryKey ipc_key;

 /*
  * This is the base IPC shared memory key.  Other keys are generated by
  * adding to this.
  */


static int	NextBackendId = MAXINT;		/* XXX why? */
static char *progname = (char *) NULL;
static char **real_argv;
static int  real_argc;

/*
 * Default Values
 */
static char Execfile[MAXPATHLEN] = "";

static int	ServerSock_INET = INVALID_SOCK;		/* stream socket server */
static int	ServerSock_UNIX = INVALID_SOCK;		/* stream socket server */

/*
 * Set by the -o option
 */
static char ExtraOptions[ARGV_SIZE] = "";

/*
 * These globals control the behavior of the postmaster in case some
 * backend dumps core.	Normally, it kills all peers of the dead backend
 * and reinitializes shared memory.  By specifying -s or -n, we can have
 * the postmaster stop (rather than kill) peers and not reinitialize
 * shared data structures.
 */
static bool	Reinit = true;
static int	SendStop = false;

static bool	NetServer = false;		/* if not zero, postmaster listen for
								 * non-local connections */


/*
 * GH: For !HAVE_SIGPROCMASK (NEXTSTEP), TRH implemented an
 * alternative interface.
 */
#ifdef HAVE_SIGPROCMASK
static	sigset_t	oldsigmask,
				newsigmask;
#else
static	int			orgsigmask = sigblock(0);
#endif

/*
 * State for assigning random salts and cancel keys.
 * Also, the global MyCancelKey passes the cancel key assigned to a given
 * backend from the postmaster to that backend (via fork).
 */

static unsigned int random_seed = 0;

extern char *optarg;
extern int	optind,
			opterr;

								 
/*
 * postmaster.c - function prototypes
 */
static void pmdaemonize(void);
static Port *ConnCreate(int serverFd);
static void reset_shared(short port);
static void pmdie(SIGNAL_ARGS);
static void reaper(SIGNAL_ARGS);
static void dumpstatus(SIGNAL_ARGS);
static void CleanupProc(int pid, int exitstatus);
static int DoBackend(Port *port);
static void ExitPostmaster(int status);
static void usage(const char *);
static int ServerLoop(void);
static int BackendStartup(Port *port);
static int readStartupPacket(void *arg, PacketLen len, void *pkt);
static int processCancelRequest(Port *port, PacketLen len, void *pkt);
static int initMasks(fd_set *rmask, fd_set *wmask);
static long PostmasterRandom(void);
static void RandomSalt(char *salt);
static void SignalChildren(SIGNAL_ARGS);

#ifdef CYR_RECODE
void GetCharSetByHost(char *, int, char *);

#endif

#ifdef USE_ASSERT_CHECKING
int assert_enabled = 1;
#endif

static void
checkDataDir(const char *DataDir, bool *DataDirOK)
{
	if (DataDir == NULL)
	{
		fprintf(stderr, "%s does not know where to find the database system "
				"data.  You must specify the directory that contains the "
				"database system either by specifying the -D invocation "
			 "option or by setting the PGDATA environment variable.\n\n",
				progname);
		*DataDirOK = false;
	}
	else
	{
		char		path[MAXPATHLEN];
		FILE	   *fp;

		sprintf(path, "%s%cbase%ctemplate1%cpg_class",
				DataDir, SEP_CHAR, SEP_CHAR, SEP_CHAR);
		fp = AllocateFile(path, "r");
		if (fp == NULL)
		{
			fprintf(stderr, "%s does not find the database system.  "
					"Expected to find it "
			   "in the PGDATA directory \"%s\", but unable to open file "
					"with pathname \"%s\".\n\n",
					progname, DataDir, path);
			*DataDirOK = false;
		}
		else
		{
			char	   *reason;

			/* reason ValidatePgVersion failed.  NULL if didn't */

			FreeFile(fp);

			ValidatePgVersion(DataDir, &reason);
			if (reason)
			{
				fprintf(stderr,
						"Database system in directory %s "
						"is not compatible with this version of "
						"Postgres, or we are unable to read the "
						"PG_VERSION file.  "
						"Explanation from ValidatePgVersion: %s\n\n",
						DataDir, reason);
				free(reason);
				*DataDirOK = false;
			}
			else
				*DataDirOK = true;
		}
	}
}

int
PostmasterMain(int argc, char *argv[])
{
	extern int	NBuffers;		/* from buffer/bufmgr.c */
	int			opt;
	char	   *hostName;
	int			status;
	int			silentflag = 0;
	bool		DataDirOK;		/* We have a usable PGDATA value */
	char		hostbuf[MAXHOSTNAMELEN];
	int			nonblank_argc;
	
	/*
	 *	We need three params so we can display status.  If we don't
	 *	get them from the user, let's make them ourselves.
	 */
	if (argc < 5)
	{
		int i;
		char *new_argv[6];

		for (i=0; i < argc; i++)
			new_argv[i] = argv[i];
		for (; i < 5; i++)
			new_argv[i] = "";
		new_argv[5] = NULL;

		if (!Execfile[0] && FindExec(Execfile, argv[0], "postmaster") < 0)
		{
			fprintf(stderr, "%s: could not find postmaster to execute...\n",
					argv[0]);
			exit(1);
		}
		new_argv[0] = Execfile;
		
		execv(new_argv[0], new_argv);

		/* How did we get here, error! */
		perror(new_argv[0]);
		fprintf(stderr, "PostmasterMain execv failed on %s\n", argv[0]);
		exit(1);
	}
	    
	progname = argv[0];
	real_argv = argv;
	real_argc = argc;

	/* don't process any dummy args we placed at the end for status display */
	for (nonblank_argc = argc; nonblank_argc > 0; nonblank_argc--)
		if (argv[nonblank_argc-1] != NULL && argv[nonblank_argc-1][0] != '\0')
			break;

	/*
	 * for security, no dir or file created can be group or other
	 * accessible
	 */
	umask((mode_t) 0077);

	if (!(hostName = getenv("PGHOST")))
	{
		if (gethostname(hostbuf, MAXHOSTNAMELEN) < 0)
			strcpy(hostbuf, "localhost");
		hostName = hostbuf;
	}

	MyProcPid = getpid();
	DataDir = getenv("PGDATA"); /* default value */

	opterr = 0;
	while ((opt = getopt(nonblank_argc, argv,"A:a:B:b:D:dim:Mno:p:Ss")) != EOF)
	{
		switch (opt)
		{
			case 'A':
#ifndef USE_ASSERT_CHECKING
				fprintf(stderr, "Assert checking is not enabled\n");
#else
				/*
				 * Pass this option also to each backend.
				 */
				assert_enabled = atoi(optarg);
				strcat(ExtraOptions, " -A ");
				strcat(ExtraOptions, optarg);
#endif
				break;
			case 'a':
				/* Can no longer set authentication method. */
				break;
			case 'B':

				/*
				 * The number of buffers to create.  Setting this option
				 * means we have to start each backend with a -B # to make
				 * sure they know how many buffers were allocated.
				 */
				NBuffers = atol(optarg);
				strcat(ExtraOptions, " -B ");
				strcat(ExtraOptions, optarg);
				break;
			case 'b':
				/* Set the backend executable file to use. */
				if (!ValidateBinary(optarg))
					strcpy(Execfile, optarg);
				else
				{
					fprintf(stderr, "%s: invalid backend \"%s\"\n",
							progname, optarg);
					exit(2);
				}
				break;
			case 'D':
				/* Set PGDATA from the command line. */
				DataDir = optarg;
				break;
			case 'd':

				/*
				 * Turn on debugging for the postmaster and the backend
				 * servers descended from it.
				 */
				if ((optind < nonblank_argc) && *argv[optind] != '-')
				{
					DebugLvl = atoi(argv[optind]);
					optind++;
				}
				else
					DebugLvl = 1;
				pg_options[TRACE_VERBOSE] = DebugLvl;
				break;
			case 'i':
				NetServer = true;
				break;
			case 'm':
				/* Multiplexed backends no longer supported. */
				break;
			case 'M':

				/*
				 * ignore this flag.  This may be passed in because the
				 * program was run as 'postgres -M' instead of
				 * 'postmaster'
				 */
				break;
			case 'n':
				/* Don't reinit shared mem after abnormal exit */
				Reinit = false;
				break;
			case 'o':

				/*
				 * Other options to pass to the backend on the command
				 * line -- useful only for debugging.
				 */
				strcat(ExtraOptions, " ");
				strcat(ExtraOptions, optarg);
				break;
			case 'p':
				/* Set PGPORT by hand. */
				PostPortName = (short) atoi(optarg);
				break;
			case 'S':

				/*
				 * Start in 'S'ilent mode (disassociate from controlling
				 * tty). You may also think of this as 'S'ysV mode since
				 * it's most badly needed on SysV-derived systems like
				 * SVR4 and HP-UX.
				 */
				silentflag = 1;
				break;
			case 's':

				/*
				 * In the event that some backend dumps core, send
				 * SIGSTOP, rather than SIGUSR1, to all its peers.	This
				 * lets the wily post_hacker collect core dumps from
				 * everyone.
				 */
				SendStop = true;
				break;
			default:
				/* usage() never returns */
				usage(progname);
				break;
		}
	}
	if (PostPortName == -1)
		PostPortName = pq_getport();

	checkDataDir(DataDir, &DataDirOK);	/* issues error messages */
	if (!DataDirOK)
	{
		fprintf(stderr, "No data directory -- can't proceed.\n");
		exit(2);
	}

	if (!Execfile[0] && FindExec(Execfile, argv[0], "postgres") < 0)
	{
		fprintf(stderr, "%s: could not find backend to execute...\n",
				argv[0]);
		exit(1);
	}

	if (NetServer)
	{
		status = StreamServerPort(hostName, PostPortName, &ServerSock_INET);
		if (status != STATUS_OK)
		{
			fprintf(stderr, "%s: cannot create INET stream port\n",
					progname);
			exit(1);
		}
	}
	status = StreamServerPort(NULL, PostPortName, &ServerSock_UNIX);
	if (status != STATUS_OK)
	{
		fprintf(stderr, "%s: cannot create UNIX stream port\n",
				progname);
		exit(1);
	}

	/* set up shared memory and semaphores */
	EnableMemoryContext(TRUE);
	reset_shared(PostPortName);

	/*
	 * Initialize the list of active backends.	This list is only used for
	 * garbage collecting the backend processes.
	 */
	BackendList = DLNewList();
	PortList = DLNewList();

	if (silentflag)
		pmdaemonize();

	/*
	 * Set up signal handlers for the postmaster process.
	 */

	pqsignal(SIGHUP,   pmdie);		/* send SIGHUP, don't die */
	pqsignal(SIGINT,   pmdie);		/* die */
	pqsignal(SIGQUIT,  pmdie);		/* send SIGTERM and die */
	pqsignal(SIGTERM,  pmdie);		/* send SIGTERM,SIGKILL and die */
	pqsignal(SIGPIPE,  SIG_IGN);	/* ignored */
	pqsignal(SIGUSR1,  pmdie);		/* send SIGUSR1 and die */
	pqsignal(SIGUSR2,  pmdie);		/* send SIGUSR2, don't die */
	pqsignal(SIGCHLD,  reaper);		/* handle child termination */
	pqsignal(SIGTTIN,  SIG_IGN);	/* ignored */
	pqsignal(SIGTTOU,  SIG_IGN);	/* ignored */
	pqsignal(SIGWINCH, dumpstatus);	/* dump port status */

	status = ServerLoop();

	ExitPostmaster(status != STATUS_OK);
	return 0;					/* not reached */
}

static void
pmdaemonize(void)
{
	int			i;

	if (fork())
		_exit(0);
/* GH: If there's no setsid(), we hopefully don't need silent mode.
 * Until there's a better solution.
 */
#ifdef HAVE_SETSID
	if (setsid() < 0)
	{
		fprintf(stderr, "%s: ", progname);
		perror("cannot disassociate from controlling TTY");
		exit(1);
	}
#endif
	i = open(NULL_DEV, O_RDWR);
	dup2(i, 0);
	dup2(i, 1);
	dup2(i, 2);
	close(i);
}

static void
usage(const char *progname)
{
	fprintf(stderr, "usage: %s [options]\n", progname);
#ifdef USE_ASSERT_CHECKING
	fprintf(stderr, "\t-A [1|0]\tenable/disable runtime assert checking\n");
#endif
	fprintf(stderr, "\t-B nbufs\tset number of shared buffers\n");
	fprintf(stderr, "\t-D datadir\tset data directory\n");
	fprintf(stderr, "\t-S \t\tsilent mode (disassociate from tty)\n");
	fprintf(stderr, "\t-a system\tuse this authentication system\n");
	fprintf(stderr, "\t-b backend\tuse a specific backend server executable\n");
	fprintf(stderr, "\t-d [1|2|3]\tset debugging level\n");
	fprintf(stderr, "\t-i \t\tlisten on TCP/IP sockets as well as Unix domain socket\n");
	fprintf(stderr, "\t-n \t\tdon't reinitialize shared memory after abnormal exit\n");
	fprintf(stderr, "\t-o option\tpass 'option' to each backend servers\n");
	fprintf(stderr, "\t-p port\tspecify port for postmaster to listen on\n");
	fprintf(stderr, "\t-s \t\tsend SIGSTOP to all backend servers if one dies\n");
	exit(1);
}

static int
ServerLoop(void)
{
	fd_set		readmask,
				writemask;
	int			nSockets;
	Dlelem	   *curr;
	struct timeval now, later;
	struct timezone tz;

	gettimeofday(&now, &tz);

	nSockets = initMasks(&readmask, &writemask);

#ifdef HAVE_SIGPROCMASK
	sigprocmask(0, NULL, &oldsigmask);
	sigemptyset(&newsigmask);
	sigaddset(&newsigmask, SIGCHLD);
#endif

	for (;;)
	{
		Port	   *port;
		fd_set		rmask,
					wmask;

#ifdef HAVE_SIGPROCMASK
		sigprocmask(SIG_SETMASK, &oldsigmask, 0);
#else
		sigsetmask(orgsigmask);
#endif

		memmove((char *) &rmask, (char *) &readmask, sizeof(fd_set));
		memmove((char *) &wmask, (char *) &writemask, sizeof(fd_set));
		if (select(nSockets, &rmask, &wmask, (fd_set *) NULL,
				   (struct timeval *) NULL) < 0)
		{
			if (errno == EINTR)
				continue;
			fprintf(stderr, "%s: ServerLoop: select failed\n",
					progname);
			return (STATUS_ERROR);
		}

		/*
		 * Select a random seed at the time of first receiving a request.
		 */
		while (random_seed == 0)
		{
			gettimeofday(&later, &tz);
	
			/*
			 *	We are not sure how much precision is in tv_usec, so we
			 *	swap the nibbles of 'later' and XOR them with 'now'.
			 *  On the off chance that the result is 0, we loop until
			 *  it isn't.
			 */
			random_seed = now.tv_usec ^
					((later.tv_usec << 16) |
					((later.tv_usec >> 16) & 0xffff));
		}
				
		/*
		 * [TRH] To avoid race conditions, block SIGCHLD signals while we
		 * are handling the request. (both reaper() and ConnCreate()
		 * manipulate the BackEnd list, and reaper() calls free() which is
		 * usually non-reentrant.)
		 */
#ifdef HAVE_SIGPROCMASK
		sigprocmask(SIG_BLOCK, &newsigmask, &oldsigmask);
#else
		sigblock(sigmask(SIGCHLD));		/* XXX[TRH] portability */
#endif

		/* new connection pending on our well-known port's socket */

		if (ServerSock_UNIX != INVALID_SOCK &&
			FD_ISSET(ServerSock_UNIX, &rmask) &&
			(port = ConnCreate(ServerSock_UNIX)) != NULL)
			PacketReceiveSetup(&port->pktInfo,
							   readStartupPacket,
							   (void *) port);

		if (ServerSock_INET != INVALID_SOCK &&
			FD_ISSET(ServerSock_INET, &rmask) &&
			(port = ConnCreate(ServerSock_INET)) != NULL)
			PacketReceiveSetup(&port->pktInfo,
							   readStartupPacket,
							   (void *) port);

		/* Build up new masks for select(). */

		nSockets = initMasks(&readmask, &writemask);

		curr = DLGetHead(PortList);

		while (curr)
		{
			Port	   *port = (Port *) DLE_VAL(curr);
			int			status = STATUS_OK;
			Dlelem	   *next;

			if (FD_ISSET(port->sock, &rmask))
			{
				if (DebugLvl > 1)
					fprintf(stderr, "%s: ServerLoop:\t\thandling reading %d\n",
							progname, port->sock);

				if (PacketReceiveFragment(&port->pktInfo, port->sock) != STATUS_OK)
					status = STATUS_ERROR;
			}

			if (FD_ISSET(port->sock, &wmask))
			{
				if (DebugLvl > 1)
					fprintf(stderr, "%s: ServerLoop:\t\thandling writing %d\n",
							progname, port->sock);

				if (PacketSendFragment(&port->pktInfo, port->sock) != STATUS_OK)
					status = STATUS_ERROR;
			}

			/* Get this before the connection might be closed. */

			next = DLGetSucc(curr);

			/*
			 * If there is no error and no outstanding data transfer going
			 * on, then the authentication handshake must be complete to
			 * the postmaster's satisfaction.  So, start the backend.
			 */

			if (status == STATUS_OK && port->pktInfo.state == Idle)
			{

				/*
				 * If the backend start fails then keep the connection
				 * open to report it.  Otherwise, pretend there is an
				 * error to close the connection which will now be managed
				 * by the backend.
				 */

				if (BackendStartup(port) != STATUS_OK)
					PacketSendError(&port->pktInfo,
									"Backend startup failed");
				else
					status = STATUS_ERROR;
			}

			/* Close the connection if required. */

			if (status != STATUS_OK)
			{
				StreamClose(port->sock);
				DLRemove(curr);
				free(port);
				DLFreeElem(curr);
			}
			else
			{
				/* Set the masks for this connection. */

				if (nSockets <= port->sock)
					nSockets = port->sock + 1;

				if (port->pktInfo.state == WritingPacket)
					FD_SET(port->sock, &writemask);
				else
					FD_SET(port->sock, &readmask);
			}

			curr = next;
		}
	}
}


/*
 * Initialise the read and write masks for select() for the well-known ports
 * we are listening on.  Return the number of sockets to listen on.
 */

static int
initMasks(fd_set *rmask, fd_set *wmask)
{
	int			nsocks = -1;

	FD_ZERO(rmask);
	FD_ZERO(wmask);

	if (ServerSock_UNIX != INVALID_SOCK)
	{
		FD_SET(ServerSock_UNIX, rmask);

		if (ServerSock_UNIX > nsocks)
			nsocks = ServerSock_UNIX;
	}

	if (ServerSock_INET != INVALID_SOCK)
	{
		FD_SET(ServerSock_INET, rmask);

		if (ServerSock_INET > nsocks)
			nsocks = ServerSock_INET;
	}

	return (nsocks + 1);
}


/*
 * Called when the startup packet has been read.
 */

static int
readStartupPacket(void *arg, PacketLen len, void *pkt)
{
	Port	   *port;
	StartupPacket *si;

	port = (Port *) arg;
	si = (StartupPacket *) pkt;

	/* The first field is either a protocol version number or
	 * a special request code.
	 */

	port->proto = ntohl(si->protoVersion);

	if (port->proto == CANCEL_REQUEST_CODE)
		return processCancelRequest(port, len, pkt);

	/* Could add additional special packet types here */

	/* Check we can handle the protocol the frontend is using. */

	if (PG_PROTOCOL_MAJOR(port->proto) < PG_PROTOCOL_MAJOR(PG_PROTOCOL_EARLIEST) ||
		PG_PROTOCOL_MAJOR(port->proto) > PG_PROTOCOL_MAJOR(PG_PROTOCOL_LATEST) ||
		(PG_PROTOCOL_MAJOR(port->proto) == PG_PROTOCOL_MAJOR(PG_PROTOCOL_LATEST) &&
		 PG_PROTOCOL_MINOR(port->proto) > PG_PROTOCOL_MINOR(PG_PROTOCOL_LATEST)))
	{
		PacketSendError(&port->pktInfo, "Unsupported frontend protocol.");
		return STATUS_OK;		/* don't close the connection yet */
	}

	/*
	 * Get the parameters from the startup packet as C strings.  The
	 * packet destination was cleared first so a short packet has zeros
	 * silently added and a long packet is silently truncated.
	 */

	StrNCpy(port->database, si->database, sizeof(port->database) - 1);
	StrNCpy(port->user, si->user, sizeof(port->user) - 1);
	StrNCpy(port->options, si->options, sizeof(port->options) - 1);
	StrNCpy(port->tty, si->tty, sizeof(port->tty) - 1);

	/* The database defaults to the user name. */

	if (port->database[0] == '\0')
		StrNCpy(port->database, si->user, sizeof(port->database) - 1);

	/* Check a user name was given. */

	if (port->user[0] == '\0')
	{
		PacketSendError(&port->pktInfo,
					"No Postgres username specified in startup packet.");
		return STATUS_OK;		/* don't close the connection yet */
	}

	/* Start the authentication itself. */

	be_recvauth(port);

	return STATUS_OK;			/* don't close the connection yet */
}


/*
 * The client has sent a cancel request packet, not a normal
 * start-a-new-backend packet.  Perform the necessary processing.
 * Note that in any case, we return STATUS_ERROR to close the
 * connection immediately.  Nothing is sent back to the client.
 */

static int
processCancelRequest(Port *port, PacketLen len, void *pkt)
{
	CancelRequestPacket	*canc = (CancelRequestPacket *) pkt;
	int			backendPID;
	long		cancelAuthCode;
	Dlelem	   *curr;
	Backend    *bp;

	backendPID = (int) ntohl(canc->backendPID);
	cancelAuthCode = (long) ntohl(canc->cancelAuthCode);

	/* See if we have a matching backend */

	for (curr = DLGetHead(BackendList); curr; curr = DLGetSucc(curr))
	{
		bp = (Backend *) DLE_VAL(curr);
		if (bp->pid == backendPID)
		{
			if (bp->cancel_key == cancelAuthCode)
			{
				/* Found a match; signal that backend to cancel current op */
				if (DebugLvl)
					fprintf(stderr, "%s: processCancelRequest: sending SIGINT to process %d\n",
							progname, bp->pid);
				kill(bp->pid, SIGINT);
			}
			else
			{
				/* Right PID, wrong key: no way, Jose */
				if (DebugLvl)
					fprintf(stderr, "%s: processCancelRequest: bad key in cancel request for process %d\n",
							progname, bp->pid);
			}
			return STATUS_ERROR;
		}
	}

	/* No matching backend */
	if (DebugLvl)
		fprintf(stderr, "%s: processCancelRequest: bad PID in cancel request for process %d\n",
				progname, backendPID);

	return STATUS_ERROR;
}


/*
 * ConnCreate -- create a local connection data structure
 */
static Port *
ConnCreate(int serverFd)
{
	Port	   *port;


	if (!(port = (Port *) calloc(1, sizeof(Port))))
	{
		fprintf(stderr, "%s: ConnCreate: malloc failed\n",
				progname);
		ExitPostmaster(1);
	}

	if (StreamConnection(serverFd, port) != STATUS_OK)
	{
		StreamClose(port->sock);
		free(port);
		port = NULL;
	}
	else
	{
		DLAddHead(PortList, DLNewElem(port));
		RandomSalt(port->salt);
		port->pktInfo.state = Idle;
	}

	return port;
}

/*
 * reset_shared -- reset shared memory and semaphores
 */
static void
reset_shared(short port)
{
	ipc_key = port * 1000 + shmem_seq * 100;
	CreateSharedMemoryAndSemaphores(ipc_key);
	ActiveBackends = FALSE;
	shmem_seq += 1;
	if (shmem_seq >= 10)
		shmem_seq -= 10;
}

/*
 * pmdie -- signal handler for cleaning up after a kill signal.
 */
static void
pmdie(SIGNAL_ARGS)
{
	int i;

	TPRINTF(TRACE_VERBOSE, "pmdie %d", postgres_signal_arg);

	/*
	 * Kill self and/or children processes depending on signal number.
	 */
	switch (postgres_signal_arg) {
		case SIGHUP:
			/* Send SIGHUP to all children (update options flags) */
			SignalChildren(SIGHUP);
			/* Don't die */
			return;
		case SIGINT:
			/* Die without killing children */
			break;
		case SIGQUIT:
			/* Shutdown all children with SIGTERM */
			SignalChildren(SIGTERM);
			/* Don't die */
			return;
		case SIGTERM:
			/* Shutdown all children with SIGTERM and SIGKILL, then die */
			SignalChildren(SIGTERM);
			for (i=0; i<10; i++) {
				if (!DLGetHead(BackendList)) {
					break;
				}
				sleep(1);
			}
			if (DLGetHead(BackendList)) {
				SignalChildren(SIGKILL);
			}
			break;
		case SIGUSR1:
			/* Quick die all children with SIGUSR1 and die */
			SignalChildren(SIGUSR1);
			break;
		case SIGUSR2:
			/* Send SIGUSR2 to all children (AsyncNotifyHandler) */
			SignalChildren(SIGUSR2);
			/* Don't die */
			return;
	}

	/* exit postmaster */
	proc_exit(0);
}

/*
 * Reaper -- signal handler to cleanup after a backend (child) dies.
 */
static void
reaper(SIGNAL_ARGS)
{
/* GH: replace waitpid for !HAVE_WAITPID. Does this work ? */
#ifdef HAVE_WAITPID
	int			status;			/* backend exit status */

#else
	union wait	statusp;		/* backend exit status */

#endif
	int			pid;			/* process id of dead backend */

	if (DebugLvl)
		fprintf(stderr, "%s: reaping dead processes...\n",
				progname);
#ifdef HAVE_WAITPID
	while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
	{
		CleanupProc(pid, status);
		pqsignal(SIGCHLD, reaper);
	}
#else
	while ((pid = wait3(&statusp, WNOHANG, NULL)) > 0)
	{
		CleanupProc(pid, statusp.w_status);
		pqsignal(SIGCHLD, reaper);
	}
#endif
}

/*
 * CleanupProc -- cleanup after terminated backend.
 *
 * Remove all local state associated with backend.
 *
 * Dillon's note: should log child's exit status in the system log.
 */
static void
CleanupProc(int pid,
			int exitstatus)		/* child's exit status. */
{
	Dlelem	   *prev,
			   *curr;
	Backend    *bp;
	int			sig;

	if (DebugLvl)
	{
		fprintf(stderr, "%s: CleanupProc: pid %d exited with status %d\n",
				progname, pid, exitstatus);
	}

	/*
	 * If a backend dies in an ugly way (i.e.
	 * exit status not 0) then we must signal all other backends to
	 * quickdie.  If exit status is zero we assume everything is hunky
	 * dory and simply remove the backend from the active backend list.
	 */
	if (!exitstatus)
	{
		curr = DLGetHead(BackendList);
		while (curr)
		{
			bp = (Backend *) DLE_VAL(curr);
			if (bp->pid == pid)
			{
				DLRemove(curr);
				free(bp);
				DLFreeElem(curr);
				break;
			}
			curr = DLGetSucc(curr);
		}

		ProcRemove(pid);

		return;
	}

	curr = DLGetHead(BackendList);
	while (curr)
	{
		bp = (Backend *) DLE_VAL(curr);

		/* -----------------
		 * SIGUSR1 is the special signal that says exit
		 * without proc_exit and let the user know what's going on.
		 * ProcSemaphoreKill() cleans up the backends semaphore.  If
		 * SendStop is set (-s on command line), then we send a SIGSTOP so
		 * that we can core dumps from all backends by hand.
		 * -----------------
		 */
		sig = (SendStop) ? SIGSTOP : SIGUSR1;
		if (bp->pid != pid)
		{
			if (DebugLvl)
				fprintf(stderr, "%s: CleanupProc: sending %s to process %d\n",
						progname,
						(sig == SIGUSR1)
						? "SIGUSR1" : "SIGSTOP",
						bp->pid);
			kill(bp->pid, sig);
		}
		ProcRemove(bp->pid);

		prev = DLGetPred(curr);
		DLRemove(curr);
		free(bp);
		DLFreeElem(curr);
		if (!prev)
		{						/* removed head */
			curr = DLGetHead(BackendList);
			continue;
		}
		curr = DLGetSucc(prev);
	}

	/*
	 * Nothing up my sleeve here, ActiveBackends means that since the last
	 * time we recreated shared memory and sems another frontend has
	 * requested and received a connection and I have forked off another
	 * backend.  This prevents me from reinitializing shared stuff more
	 * than once for the set of backends that caused the failure and were
	 * killed off.
	 */
	if (ActiveBackends == TRUE && Reinit)
	{
		if (DebugLvl)
			fprintf(stderr, "%s: CleanupProc: reinitializing shared memory and semaphores\n",
					progname);
		shmem_exit(0);
		reset_shared(PostPortName);
	}
}

/*
 * Send a signal to all chidren processes.
 */
static void
SignalChildren(int signal)
{
	Dlelem		*curr,
				*next;
	Backend 	*bp;
	int			mypid = getpid();

	curr = DLGetHead(BackendList);
	while (curr)
	{
		next = DLGetSucc(curr);
		bp = (Backend *) DLE_VAL(curr);

		if (bp->pid != mypid)
		{
			TPRINTF(TRACE_VERBOSE,
					"SignalChildren: sending signal %d to process %d",
					signal, bp->pid);
			kill(bp->pid, signal);
		}

		curr = next;
	}
}

/*
 * BackendStartup -- start backend process
 *
 * returns: STATUS_ERROR if the fork/exec failed, STATUS_OK
 *		otherwise.
 *
 */
static int
BackendStartup(Port *port)
{
	Backend    *bn;				/* for backend cleanup */
	int			pid,
				i;

#ifdef CYR_RECODE
#define NR_ENVIRONMENT_VBL 6
	char		ChTable[80];

#else
#define NR_ENVIRONMENT_VBL 5
#endif

	static char envEntry[NR_ENVIRONMENT_VBL][2 * ARGV_SIZE];

	for (i = 0; i < NR_ENVIRONMENT_VBL; ++i)
		MemSet(envEntry[i], 0, 2 * ARGV_SIZE);

	/*
	 * Set up the necessary environment variables for the backend This
	 * should really be some sort of message....
	 */
	sprintf(envEntry[0], "POSTPORT=%d", PostPortName);
	putenv(envEntry[0]);
	sprintf(envEntry[1], "POSTID=%d", NextBackendId);
	putenv(envEntry[1]);
	sprintf(envEntry[2], "PG_USER=%s", port->user);
	putenv(envEntry[2]);
	if (!getenv("PGDATA"))
	{
		sprintf(envEntry[3], "PGDATA=%s", DataDir);
		putenv(envEntry[3]);
	}
	sprintf(envEntry[4], "IPC_KEY=%d", ipc_key);
	putenv(envEntry[4]);

#ifdef CYR_RECODE
	GetCharSetByHost(ChTable, port->raddr.in.sin_addr.s_addr, DataDir);
	if (*ChTable != '\0')
	{
		sprintf(envEntry[5], "PG_RECODETABLE=%s", ChTable);
		putenv(envEntry[5]);
	}
#endif

	/*
	 * Compute the cancel key that will be assigned to this backend.
	 * The backend will have its own copy in the forked-off process'
	 * value of MyCancelKey, so that it can transmit the key to the
	 * frontend.
	 */
	MyCancelKey = PostmasterRandom();

	if (DebugLvl > 2)
	{
		char	  **p;
		extern char **environ;

		fprintf(stderr, "%s: BackendStartup: environ dump:\n",
				progname);
		fprintf(stderr, "-----------------------------------------\n");
		for (p = environ; *p; ++p)
			fprintf(stderr, "\t%s\n", *p);
		fprintf(stderr, "-----------------------------------------\n");
	}

	/* Flush all stdio channels just before fork,
	 * to avoid double-output problems.
	 */
	fflush(NULL);

    if ((pid = fork()) == 0)
	{  /* child */
        if (DoBackend(port))
		{
            fprintf(stderr, "%s child[%d]: BackendStartup: backend startup failed\n",
                    progname, (int) getpid());
	 		exit(1);
		}
		else
	    	exit(0);
	}

	/* in parent */
	if (pid < 0)
	{
		fprintf(stderr, "%s: BackendStartup: fork failed\n",
				progname);
		return (STATUS_ERROR);
	}

	if (DebugLvl)
		fprintf(stderr, "%s: BackendStartup: pid %d user %s db %s socket %d\n",
				progname, pid, port->user, port->database,
				port->sock);

	/* adjust backend counter */
	/* XXX Don't know why this is done, but for now backend needs it */
	NextBackendId -= 1;

	/*
	 * Everything's been successful, it's safe to add this backend to our
	 * list of backends.
	 */
	if (!(bn = (Backend *) calloc(1, sizeof(Backend))))
	{
		fprintf(stderr, "%s: BackendStartup: malloc failed\n",
				progname);
		ExitPostmaster(1);
	}

	bn->pid = pid;
	bn->cancel_key = MyCancelKey;
	DLAddHead(BackendList, DLNewElem(bn));

	ActiveBackends = TRUE;

	return (STATUS_OK);
}

/*
 * split_opts -- destructively load a string into an argv array
 *
 * Since no current POSTGRES arguments require any quoting characters,
 * we can use the simple-minded tactic of assuming each set of space-
 * delimited characters is a separate argv element.
 *
 * If you don't like that, well, we *used* to pass the whole option string
 * as ONE argument to execl(), which was even less intelligent...
 */
static void
split_opts(char **argv, int *argcp, char *s)
{
	int			i = *argcp;

	while (s && *s)
	{
		while (isspace(*s))
			++s;
		if (*s)
			argv[i++] = s;
		while (*s && !isspace(*s))
			++s;
		if (isspace(*s))
			*s++ = '\0';
	}
	*argcp = i;
}

/*
 * DoBackend -- set up the argument list and perform an execv system call
 *
 * returns: 
 *      Shouldn't return at all.
 *      If execv() fails, return status.
 */
static int
DoBackend(Port *port)
{
	char		execbuf[MAXPATHLEN];
	char		portbuf[ARGV_SIZE];
	char		debugbuf[ARGV_SIZE];
	char		ttybuf[ARGV_SIZE + 1];
	char		protobuf[ARGV_SIZE + 1];
	char		argbuf[(2 * ARGV_SIZE) + 1];

	/*
	 * each argument takes at least three chars, so we can't have more
	 * than ARGV_SIZE arguments in (2 * ARGV_SIZE) chars (i.e.,
	 * port->options plus ExtraOptions)...
	 */
	char	   *av[ARGV_SIZE];
	char		dbbuf[ARGV_SIZE + 1];
	int			ac = 0;
	int			i;
	struct timeval	now;
	struct timezone	tz;

	/*
	 *	Let's clean up ourselves as the postmaster child
	 */
	
	on_exit_reset(); /* we don't want the postmaster's proc_exit() handlers */

	/* ----------------
	 *	register signal handlers.
	 *  Thanks to the postmaster, these are currently blocked.
	 * ----------------
	 */
	pqsignal(SIGINT, die);

	pqsignal(SIGHUP, die);
	pqsignal(SIGTERM, die);
	pqsignal(SIGPIPE, die);
	pqsignal(SIGUSR1, quickdie);
	pqsignal(SIGUSR2, Async_NotifyHandler);
	pqsignal(SIGFPE, FloatExceptionHandler);

	pqsignal(SIGCHLD, SIG_DFL);
	pqsignal(SIGTTIN, SIG_DFL);
	pqsignal(SIGTTOU, SIG_DFL);
	pqsignal(SIGCONT, SIG_DFL);

	/* OK, let's unblock our signals, all together now... */
	sigprocmask(SIG_SETMASK, &oldsigmask, 0);

	/* Close the postmater sockets */
	if (NetServer)
		StreamClose(ServerSock_INET);
	StreamClose(ServerSock_UNIX);

	/* Save port for ps status */
	MyProcPort = port;

	/*
	 * Don't want backend to be able to see the postmaster random number
	 * generator state.  We have to clobber the static random_seed *and*
	 * start a new random sequence in the random() library function.
	 */
	random_seed = 0;
	gettimeofday(&now, &tz);
	srandom(now.tv_usec);

	/* Now, on to standard postgres stuff */
	
	MyProcPid = getpid();

	strncpy(execbuf, Execfile, MAXPATHLEN - 1);
	av[ac++] = execbuf;

	/*
	 *	We need to set our argv[0] to an absolute path name because
	 *	some OS's use this for dynamic loading, like BSDI.  Without it,
	 *	when we change directories to the database dir, the dynamic
	 *	loader can't find the base executable and fails.
	 *	Another advantage is that this changes the 'ps' displayed
	 *	process name on some platforms.  It does on BSDI.  That's
	 *	a big win.
	 */
	
#ifndef linux
	/*
	 * This doesn't work on linux and overwrites the only valid
	 * pointer to the argv buffer.  See PS_INIT_STATUS macro.
	 */
	real_argv[0] = Execfile;
#endif

	/* Tell the backend it is being called from the postmaster */
	av[ac++] = "-p";

	/*
	 * Pass the requested debugging level along to the backend.  We
	 * decrement by one; level one debugging in the postmaster traces
	 * postmaster connection activity, and levels two and higher are
	 * passed along to the backend.  This allows us to watch only the
	 * postmaster or the postmaster and the backend.
	 */

	if (DebugLvl > 1)
	{
		sprintf(debugbuf, "-d%d", DebugLvl);
		av[ac++] = debugbuf;
	}

	/* Pass the requested debugging output file */
	if (port->tty[0])
	{
		strncpy(ttybuf, port->tty, ARGV_SIZE);
		av[ac++] = "-o";
		av[ac++] = ttybuf;
	}

	/* Tell the backend the descriptor of the fe/be socket */
	sprintf(portbuf, "-P%d", port->sock);
	av[ac++] = portbuf;

	StrNCpy(argbuf, port->options, ARGV_SIZE);
	strncat(argbuf, ExtraOptions, ARGV_SIZE);
	argbuf[(2 * ARGV_SIZE)] = '\0';
	split_opts(av, &ac, argbuf);

	/* Tell the backend what protocol the frontend is using. */

	sprintf(protobuf, "-v%u", port->proto);
	av[ac++] = protobuf;

	StrNCpy(dbbuf, port->database, ARGV_SIZE);
	av[ac++] = dbbuf;

	av[ac] = (char *) NULL;

	if (DebugLvl > 1)
	{
		fprintf(stderr, "%s child[%d]: starting with (",
				progname, MyProcPid);
		for (i = 0; i < ac; ++i)
			fprintf(stderr, "%s, ", av[i]);
		fprintf(stderr, ")\n");
	}

    return(PostgresMain(ac, av, real_argc, real_argv));
}

/*
 * ExitPostmaster -- cleanup
 */
static void
ExitPostmaster(int status)
{
	/* should cleanup shared memory and kill all backends */

	/*
	 * Not sure of the semantics here.	When the Postmaster dies, should
	 * the backends all be killed? probably not.
	 */
	if (ServerSock_INET != INVALID_SOCK)
		StreamClose(ServerSock_INET);
	if (ServerSock_UNIX != INVALID_SOCK)
		StreamClose(ServerSock_UNIX);
	proc_exit(status);
}

static void
dumpstatus(SIGNAL_ARGS)
{
	Dlelem	   *curr = DLGetHead(PortList);

	while (curr)
	{
		Port	   *port = DLE_VAL(curr);

		fprintf(stderr, "%s: dumpstatus:\n", progname);
		fprintf(stderr, "\tsock %d\n", port->sock);
		curr = DLGetSucc(curr);
	}
}

/*
 * CharRemap
 */
static char
CharRemap(long int ch)
{

	if (ch < 0)
		ch = -ch;

	ch = ch % 62;
	if (ch < 26)
		return ('A' + ch);

	ch -= 26;
	if (ch < 26)
		return ('a' + ch);

	ch -= 26;
	return ('0' + ch);
}

/*
 * RandomSalt
 */
static void
RandomSalt(char *salt)
{
	long rand = PostmasterRandom();
	
	*salt = CharRemap(rand % 62);
	*(salt + 1) = CharRemap(rand / 62);
}

/*
 * PostmasterRandom
 */
static long
PostmasterRandom(void)
{

	static bool initialized = false;

	if (!initialized)
	{
		Assert(random_seed != 0 && !IsUnderPostmaster);
		srandom(random_seed);
		initialized = true;
	}

	return random() ^ random_seed;
}
