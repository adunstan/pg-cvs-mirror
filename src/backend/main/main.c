/*-------------------------------------------------------------------------
 *
 * main.c
 *	  Stub main() routine for the postgres executable.
 *
 * This does some essential startup tasks for any incarnation of postgres
 * (postmaster, standalone backend, or standalone bootstrap mode) and then
 * dispatches to the proper FooMain() routine for the incarnation.
 *
 *
 * Portions Copyright (c) 1996-2003, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  $PostgreSQL: pgsql-server/src/backend/main/main.c,v 1.82 2004/05/25 01:00:20 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include <errno.h>
#include <pwd.h>
#include <unistd.h>

#if defined(__alpha) && defined(__osf__)		/* no __alpha__ ? */
#include <sys/sysinfo.h>
#include "machine/hal_sysinfo.h"
#define ASSEMBLER
#include <sys/proc.h>
#undef ASSEMBLER
#endif
#if defined(__NetBSD__)
#include <sys/param.h>
#endif

#include "miscadmin.h"
#include "bootstrap/bootstrap.h"
#include "tcop/tcopprot.h"
#include "utils/help_config.h"
#include "utils/ps_status.h"
#include "pgstat.h"
#ifdef WIN32
#include "libpq/pqsignal.h"
#endif



int
main(int argc, char *argv[])
{
	int			len;

#ifndef WIN32
	struct passwd *pw;
#endif
	char	   *pw_name_persist;

	/*
	 * Place platform-specific startup hacks here.	This is the right
	 * place to put code that must be executed early in launch of either a
	 * postmaster, a standalone backend, or a standalone bootstrap run.
	 * Note that this code will NOT be executed when a backend or
	 * sub-bootstrap run is forked by the postmaster.
	 *
	 * XXX The need for code here is proof that the platform in question is
	 * too brain-dead to provide a standard C execution environment
	 * without help.  Avoid adding more here, if you can.
	 */

#if defined(__alpha)			/* no __alpha__ ? */
#ifdef NOFIXADE
	int			buffer[] = {SSIN_UACPROC, UAC_SIGBUS};
#endif   /* NOFIXADE */
#ifdef NOPRINTADE
	int			buffer[] = {SSIN_UACPROC, UAC_NOPRINT};
#endif   /* NOPRINTADE */
#endif   /* __alpha */

#ifdef WIN32
	char	   *env_locale;
#endif

#if defined(NOFIXADE) || defined(NOPRINTADE)

#if defined(ultrix4)
	syscall(SYS_sysmips, MIPS_FIXADE, 0, NULL, NULL, NULL);
#endif

#if defined(__alpha)			/* no __alpha__ ? */
	if (setsysinfo(SSI_NVPAIRS, buffer, 1, (caddr_t) NULL,
				   (unsigned long) NULL) < 0)
		fprintf(stderr, gettext("%s: setsysinfo failed: %s\n"),
				argv[0], strerror(errno));
#endif
#endif   /* NOFIXADE || NOPRINTADE */

#if defined(WIN32)
	{
		WSADATA		wsaData;
		int			err;

		/* Make output streams unbuffered by default */
		setvbuf(stdout, NULL, _IONBF, 0);
		setvbuf(stderr, NULL, _IONBF, 0);

		/* Prepare Winsock */
		err = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (err != 0)
		{
			fprintf(stderr, "%s: WSAStartup failed: %d\n",
					argv[0], err);
			exit(1);
		}

		/* Start our win32 signal implementation */
		pgwin32_signal_initialize();
	}
#endif

#ifdef __BEOS__
	/* BeOS-specific actions on startup */
	beos_startup(argc, argv);
#endif

	/*
	 * Not-quite-so-platform-specific startup environment checks. Still
	 * best to minimize these.
	 */

	/*
	 * Remember the physical location of the initially given argv[] array
	 * for possible use by ps display.	On some platforms, the argv[]
	 * storage must be overwritten in order to set the process title for
	 * ps. In such cases save_ps_display_args makes and returns a new copy
	 * of the argv[] array.
	 *
	 * save_ps_display_args may also move the environment strings to make
	 * extra room. Therefore this should be done as early as possible
	 * during startup, to avoid entanglements with code that might save a
	 * getenv() result pointer.
	 */
	argv = save_ps_display_args(argc, argv);

	/*
	 * Set up locale information from environment.	Note that LC_CTYPE and
	 * LC_COLLATE will be overridden later from pg_control if we are in an
	 * already-initialized database.  We set them here so that they will
	 * be available to fill pg_control during initdb.  LC_MESSAGES will
	 * get set later during GUC option processing, but we set it here to
	 * allow startup error messages to be localized.
	 */

	set_pglocale(argv[0], "postgres");

#ifdef WIN32

	/*
	 * Windows uses codepages rather than the environment, so we work
	 * around that by querying the environment explicitly first for
	 * LC_COLLATE and LC_CTYPE. We have to do this because initdb passes
	 * those values in the environment. If there is nothing there we fall
	 * back on the codepage.
	 */

	if ((env_locale = getenv("LC_COLLATE")) != NULL)
		setlocale(LC_COLLATE, env_locale);
	else
		setlocale(LC_COLLATE, "");

	if ((env_locale = getenv("LC_CTYPE")) != NULL)
		setlocale(LC_CTYPE, env_locale);
	else
		setlocale(LC_CTYPE, "");
#else
	setlocale(LC_COLLATE, "");
	setlocale(LC_CTYPE, "");
#endif

#ifdef LC_MESSAGES
	setlocale(LC_MESSAGES, "");
#endif

	/*
	 * We keep these set to "C" always, except transiently in pg_locale.c;
	 * see that file for explanations.
	 */
	setlocale(LC_MONETARY, "C");
	setlocale(LC_NUMERIC, "C");
	setlocale(LC_TIME, "C");

	/*
	 * Skip permission checks if we're just trying to do --help or
	 * --version; otherwise root will get unhelpful failure messages from
	 * initdb.
	 */
	if (!(argc > 1
		  && (strcmp(argv[1], "--help") == 0 ||
			  strcmp(argv[1], "-?") == 0 ||
			  strcmp(argv[1], "--version") == 0 ||
			  strcmp(argv[1], "-V") == 0)))
	{
#ifndef WIN32
#ifndef __BEOS__

		/*
		 * Make sure we are not running as root.
		 *
		 * BeOS currently runs everything as root :-(, so this check must be
		 * temporarily disabled there...
		 */
		if (geteuid() == 0)
		{
			fprintf(stderr,
					gettext("\"root\" execution of the PostgreSQL server is not permitted.\n"
							"The server must be started under an unprivileged user ID to prevent\n"
							"possible system security compromise.  See the documentation for\n"
				"more information on how to properly start the server.\n"
							));
			exit(1);
		}
#endif   /* !__BEOS__ */

		/*
		 * Also make sure that real and effective uids are the same.
		 * Executing Postgres as a setuid program from a root shell is a
		 * security hole, since on many platforms a nefarious subroutine
		 * could setuid back to root if real uid is root.  (Since nobody
		 * actually uses Postgres as a setuid program, trying to actively
		 * fix this situation seems more trouble than it's worth; we'll
		 * just expend the effort to check for it.)
		 */
		if (getuid() != geteuid())
		{
			fprintf(stderr,
				 gettext("%s: real and effective user IDs must match\n"),
					argv[0]);
			exit(1);
		}
#endif   /* !WIN32 */
	}

	/*
	 * Now dispatch to one of PostmasterMain, PostgresMain, GucInfoMain,
	 * SubPostmasterMain, pgstat_main, pgstat_mainChild or BootstrapMain
	 * depending on the program name (and possibly first argument) we were
	 * called with. The lack of consistency here is historical.
	 */
	len = strlen(argv[0]);

	if ((len >= 10 && strcmp(argv[0] + len - 10, "postmaster") == 0)
#ifdef WIN32
	  || (len >= 14 && strcmp(argv[0] + len - 14, "postmaster.exe") == 0)
#endif
		)
	{
		/* Called as "postmaster" */
		exit(PostmasterMain(argc, argv));
	}

	/*
	 * If the first argument is "-boot", then invoke bootstrap mode. Note
	 * we remove "-boot" from the arguments passed on to BootstrapMain.
	 */
	if (argc > 1 && strcmp(argv[1], "-boot") == 0)
		exit(BootstrapMain(argc - 1, argv + 1));

#ifdef EXEC_BACKEND

	/*
	 * If the first argument is "-forkexec", then invoke
	 * SubPostmasterMain. Note we remove "-forkexec" from the arguments
	 * passed on to SubPostmasterMain.
	 */
	if (argc > 1 && strcmp(argv[1], "-forkexec") == 0)
	{
		SubPostmasterMain(argc - 2, argv + 2);
		exit(0);
	}

	/*
	 * If the first argument is "-statBuf", then invoke pgstat_main.
	 */
	if (argc > 1 && strcmp(argv[1], "-statBuf") == 0)
	{
		pgstat_main(argc, argv);
		exit(0);
	}

	/*
	 * If the first argument is "-statCol", then invoke pgstat_mainChild.
	 */
	if (argc > 1 && strcmp(argv[1], "-statCol") == 0)
	{
		pgstat_mainChild(argc, argv);
		exit(0);
	}
#endif

	/*
	 * If the first argument is "--describe-config", then invoke runtime
	 * configuration option display mode.
	 */
	if (argc > 1 && strcmp(argv[1], "--describe-config") == 0)
		exit(GucInfoMain());

	/*
	 * Otherwise we're a standalone backend.  Invoke PostgresMain,
	 * specifying current userid as the "authenticated" Postgres user
	 * name.
	 */
#ifndef WIN32
	pw = getpwuid(geteuid());
	if (pw == NULL)
	{
		fprintf(stderr, gettext("%s: invalid effective UID: %d\n"),
				argv[0], (int) geteuid());
		exit(1);
	}
	/* Allocate new memory because later getpwuid() calls can overwrite it */
	pw_name_persist = strdup(pw->pw_name);
#else
	{
		long		namesize = 256 /* UNLEN */ + 1;

		pw_name_persist = malloc(namesize);
		if (!GetUserName(pw_name_persist, &namesize))
		{
			fprintf(stderr, gettext("%s: could not determine user name (GetUserName failed)\n"),
					argv[0]);
			exit(1);
		}
	}
#endif

	exit(PostgresMain(argc, argv, pw_name_persist));
}
