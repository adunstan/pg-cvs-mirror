/*
 *	opt.c
 *
 *	options functions
 */

#include "pg_upgrade.h"

#include "getopt_long.h"

#ifdef WIN32
#include <io.h>
#endif


static void usage(migratorContext *ctx);
static void validateDirectoryOption(migratorContext *ctx, char **dirpath,
				   char *envVarName, char *cmdLineOption, char *description);
static void get_pkglibdirs(migratorContext *ctx);
static char *get_pkglibdir(migratorContext *ctx, const char *bindir);


/*
 * parseCommandLine()
 *
 *	Parses the command line (argc, argv[]) into the given migratorContext object
 *	and initializes the rest of the object.
 */
void
parseCommandLine(migratorContext *ctx, int argc, char *argv[])
{
	static struct option long_options[] = {
		{"old-datadir", required_argument, NULL, 'd'},
		{"new-datadir", required_argument, NULL, 'D'},
		{"old-bindir", required_argument, NULL, 'b'},
		{"new-bindir", required_argument, NULL, 'B'},
		{"old-port", required_argument, NULL, 'p'},
		{"new-port", required_argument, NULL, 'P'},

		{"user", required_argument, NULL, 'u'},
		{"check", no_argument, NULL, 'c'},
		{"debug", no_argument, NULL, 'g'},
		{"debugfile", required_argument, NULL, 'G'},
		{"link", no_argument, NULL, 'k'},
		{"logfile", required_argument, NULL, 'l'},
		{"verbose", no_argument, NULL, 'v'},
		{NULL, 0, NULL, 0}
	};
	char		option;			/* Command line option */
	int			optindex = 0;	/* used by getopt_long */

	if (getenv("PGUSER"))
	{
		pg_free(ctx->user);
		ctx->user = pg_strdup(ctx, getenv("PGUSER"));
	}

	ctx->progname = get_progname(argv[0]);
	ctx->old.port = getenv("PGPORT") ? atoi(getenv("PGPORT")) : DEF_PGPORT;
	ctx->new.port = getenv("PGPORT") ? atoi(getenv("PGPORT")) : DEF_PGPORT;
	/* must save value, getenv()'s pointer is not stable */

	ctx->transfer_mode = TRANSFER_MODE_COPY;

	if (argc > 1)
	{
		if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0 ||
			strcmp(argv[1], "-?") == 0)
		{
			usage(ctx);
			exit_nicely(ctx, false);
		}
		if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-V") == 0)
		{
			pg_log(ctx, PG_REPORT, "pg_upgrade " PG_VERSION "\n");
			exit_nicely(ctx, false);
		}
	}

	if ((get_user_info(ctx, &ctx->user)) == 0)
		pg_log(ctx, PG_FATAL, "%s: cannot be run as root\n", ctx->progname);

#ifndef WIN32
	get_home_path(ctx->home_dir);
#else
	{
		char	   *tmppath;

		/* TMP is the best place on Windows, rather than APPDATA */
		if ((tmppath = getenv("TMP")) == NULL)
			pg_log(ctx, PG_FATAL, "TMP environment variable is not set.\n");
		snprintf(ctx->home_dir, MAXPGPATH, "%s", tmppath);
	}
#endif

	snprintf(ctx->output_dir, MAXPGPATH, "%s/" OUTPUT_SUBDIR, ctx->home_dir);

	while ((option = getopt_long(argc, argv, "d:D:b:B:cgG:kl:p:P:u:v",
								 long_options, &optindex)) != -1)
	{
		switch (option)
		{
			case 'd':
				ctx->old.pgdata = pg_strdup(ctx, optarg);
				break;

			case 'D':
				ctx->new.pgdata = pg_strdup(ctx, optarg);
				break;

			case 'b':
				ctx->old.bindir = pg_strdup(ctx, optarg);
				break;

			case 'B':
				ctx->new.bindir = pg_strdup(ctx, optarg);
				break;

			case 'c':
				ctx->check = true;
				break;

			case 'g':
				pg_log(ctx, PG_REPORT, "Running in debug mode\n");
				ctx->debug = true;
				break;

			case 'G':
				if ((ctx->debug_fd = fopen(optarg, "w")) == NULL)
				{
					pg_log(ctx, PG_FATAL, "cannot open debug file\n");
					exit_nicely(ctx, false);
				}
				break;

			case 'k':
				ctx->transfer_mode = TRANSFER_MODE_LINK;
				break;

			case 'l':
				ctx->logfile = pg_strdup(ctx, optarg);
				break;

			case 'p':
				if ((ctx->old.port = atoi(optarg)) <= 0)
				{
					pg_log(ctx, PG_FATAL, "invalid old port number\n");
					exit_nicely(ctx, false);
				}
				break;

			case 'P':
				if ((ctx->new.port = atoi(optarg)) <= 0)
				{
					pg_log(ctx, PG_FATAL, "invalid new port number\n");
					exit_nicely(ctx, false);
				}
				break;

			case 'u':
				pg_free(ctx->user);
				ctx->user = pg_strdup(ctx, optarg);
				break;

			case 'v':
				pg_log(ctx, PG_REPORT, "Running in verbose mode\n");
				ctx->verbose = true;
				break;

			default:
				pg_log(ctx, PG_FATAL,
					   "Try \"%s --help\" for more information.\n",
					   ctx->progname);
				break;
		}
	}

	if (ctx->logfile != NULL)
	{
		/*
		 * We must use append mode so output generated by child processes via
		 * ">>" will not be overwritten, and we want the file truncated on
		 * start.
		 */
		/* truncate */
		ctx->log_fd = fopen(ctx->logfile, "w");
		if (!ctx->log_fd)
			pg_log(ctx, PG_FATAL, "Cannot write to log file %s\n", ctx->logfile);
		fclose(ctx->log_fd);
		ctx->log_fd = fopen(ctx->logfile, "a");
		if (!ctx->log_fd)
			pg_log(ctx, PG_FATAL, "Cannot write to log file %s\n", ctx->logfile);
	}
	else
		ctx->logfile = strdup(DEVNULL);

	/* if no debug file name, output to the terminal */
	if (ctx->debug && !ctx->debug_fd)
	{
		ctx->debug_fd = fopen(DEVTTY, "w");
		if (!ctx->debug_fd)
			pg_log(ctx, PG_FATAL, "Cannot write to terminal\n");
	}

	/* Get values from env if not already set */
	validateDirectoryOption(ctx, &ctx->old.pgdata, "OLDDATADIR", "-d",
							"old cluster data resides");
	validateDirectoryOption(ctx, &ctx->new.pgdata, "NEWDATADIR", "-D",
							"new cluster data resides");
	validateDirectoryOption(ctx, &ctx->old.bindir, "OLDBINDIR", "-b",
							"old cluster binaries reside");
	validateDirectoryOption(ctx, &ctx->new.bindir, "NEWBINDIR", "-B",
							"new cluster binaries reside");

	get_pkglibdirs(ctx);
}


static void
usage(migratorContext *ctx)
{
	printf(_("\nUsage: pg_upgrade [OPTIONS]...\n\
\n\
Options:\n\
 -d, --old-datadir=OLDDATADIR    old cluster data directory\n\
 -D, --new-datadir=NEWDATADIR    new cluster data directory\n\
 -b, --old-bindir=OLDBINDIR      old cluster executable directory\n\
 -B, --new-bindir=NEWBINDIR      new cluster executable directory\n\
 -p, --old-port=portnum          old cluster port number (default %d)\n\
 -P, --new-port=portnum          new cluster port number (default %d)\n\
 \n\
 -u, --user=username             clusters superuser (default \"%s\")\n\
 -c, --check                     check clusters only, don't change any data\n\
 -g, --debug                     enable debugging\n\
 -G, --debugfile=DEBUGFILENAME   output debugging activity to file\n\
 -k, --link                      link instead of copying files to new cluster\n\
 -l, --logfile=LOGFILENAME       log session activity to file\n\
 -v, --verbose                   enable verbose output\n\
 -V, --version                   display version information, then exit\n\
 -h, --help                      show this help, then exit\n\
\n\
Before running pg_upgrade you must:\n\
  create a new database cluster (using the new version of initdb)\n\
  shutdown the postmaster servicing the old cluster\n\
  shutdown the postmaster servicing the new cluster\n\
\n\
When you run pg_upgrade, you must provide the following information:\n\
  the data directory for the old cluster  (-d OLDDATADIR)\n\
  the data directory for the new cluster  (-D NEWDATADIR)\n\
  the 'bin' directory for the old version (-b OLDBINDIR)\n\
  the 'bin' directory for the new version (-B NEWBINDIR)\n\
\n\
For example:\n\
  pg_upgrade -d oldCluster/data -D newCluster/data -b oldCluster/bin -B newCluster/bin\n\
or\n"), ctx->old.port, ctx->new.port, ctx->user);
#ifndef WIN32
	printf(_("\
  $ export OLDDATADIR=oldCluster/data\n\
  $ export NEWDATADIR=newCluster/data\n\
  $ export OLDBINDIR=oldCluster/bin\n\
  $ export NEWBINDIR=newCluster/bin\n\
  $ pg_upgrade\n"));
#else
	printf(_("\
  C:\\> set OLDDATADIR=oldCluster/data\n\
  C:\\> set NEWDATADIR=newCluster/data\n\
  C:\\> set OLDBINDIR=oldCluster/bin\n\
  C:\\> set NEWBINDIR=newCluster/bin\n\
  C:\\> pg_upgrade\n"));
#endif
	printf(_("\n\
You may find it useful to save the preceding 5 commands in a shell script\n\
\n\
Report bugs to <pg-migrator-general@lists.pgfoundry.org>\n"));
}


/*
 * validateDirectoryOption()
 *
 * Validates a directory option.
 *	dirpath		  - the directory name supplied on the command line
 *	envVarName	  - the name of an environment variable to get if dirpath is NULL
 *	cmdLineOption - the command line option corresponds to this directory (-o, -O, -n, -N)
 *	description   - a description of this directory option
 *
 * We use the last two arguments to construct a meaningful error message if the
 * user hasn't provided the required directory name.
 */
static void
validateDirectoryOption(migratorContext *ctx, char **dirpath,
					char *envVarName, char *cmdLineOption, char *description)
{
	if (*dirpath == NULL || (strlen(*dirpath) == 0))
	{
		const char *envVar;

		if ((envVar = getenv(envVarName)) && strlen(envVar))
			*dirpath = pg_strdup(ctx, envVar);
		else
		{
			pg_log(ctx, PG_FATAL, "You must identify the directory where the %s\n"
				   "Please use the %s command-line option or the %s environment variable\n",
				   description, cmdLineOption, envVarName);
		}
	}

	/*
	 * Trim off any trailing path separators
	 */
#ifndef WIN32
	if ((*dirpath)[strlen(*dirpath) - 1] == '/')
#else
	if ((*dirpath)[strlen(*dirpath) - 1] == '/' ||
	    (*dirpath)[strlen(*dirpath) - 1] == '\\')
#endif
		(*dirpath)[strlen(*dirpath) - 1] = 0;
}


static void
get_pkglibdirs(migratorContext *ctx)
{
	ctx->old.libpath = get_pkglibdir(ctx, ctx->old.bindir);
	ctx->new.libpath = get_pkglibdir(ctx, ctx->new.bindir);
}


static char *
get_pkglibdir(migratorContext *ctx, const char *bindir)
{
	char		cmd[MAXPGPATH];
	char		bufin[MAX_STRING];
	FILE	   *output;
	int			i;

	snprintf(cmd, sizeof(cmd), "\"%s/pg_config\" --pkglibdir", bindir);

	if ((output = popen(cmd, "r")) == NULL)
		pg_log(ctx, PG_FATAL, "Could not get pkglibdir data: %s\n",
			   getErrorText(errno));

	fgets(bufin, sizeof(bufin), output);

	if (output)
		pclose(output);

	/* Remove trailing newline */
	i = strlen(bufin) - 1;

	if (bufin[i] == '\n')
		bufin[i] = '\0';

	return pg_strdup(ctx, bufin);
}
