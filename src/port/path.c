/*-------------------------------------------------------------------------
 *
 * path.c
 *	  portable path handling routines
 *
 * Portions Copyright (c) 1996-2004, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  $PostgreSQL: pgsql/src/port/path.c,v 1.42 2004/11/06 01:16:22 tgl Exp $
 *
 *-------------------------------------------------------------------------
 */

#include "c.h"

#include <ctype.h>

#include "pg_config_paths.h"


#ifndef WIN32
#define IS_DIR_SEP(ch)	((ch) == '/')
#else
#define IS_DIR_SEP(ch)	((ch) == '/' || (ch) == '\\')
#endif

#ifndef WIN32
#define IS_PATH_SEP(ch) ((ch) == ':')
#else
#define IS_PATH_SEP(ch) ((ch) == ';')
#endif

static const char *relative_path(const char *bin_path, const char *other_path);
static void make_relative(const char *my_exec_path, const char *p, char *ret_path);
static void trim_directory(char *path);
static void trim_trailing_separator(char *path);

/* Move to last of consecutive separators or to null byte */
#define MOVE_TO_SEP_END(p) \
{ \
	while (IS_DIR_SEP((p)[0]) && (IS_DIR_SEP((p)[1]) || !(p)[1])) \
		(p)++; \
}

/*
 * skip_drive
 *
 * On Windows, a path may begin with "C:" or "//network/".  Advance over
 * this and point to the effective start of the path.
 */
#ifdef WIN32

static char *
skip_drive(const char *path)
{
	if (IS_DIR_SEP(path[0]) && IS_DIR_SEP(path[1]))
	{
		path += 2;
		while (*path && !IS_DIR_SEP(*path))
			path++;
	}
	else if (isalpha(path[0]) && path[1] == ':')
	{
		path += 2;
	}
	return (char *) path;
}

#else

#define skip_drive(path)	(path)

#endif

/*
 *	first_dir_separator
 *
 * Find the location of the first directory separator, return
 * NULL if not found.
 */
char *
first_dir_separator(const char *filename)
{
	const char *p;

	for (p = skip_drive(filename); *p; p++)
		if (IS_DIR_SEP(*p))
			return (char *) p;
	return NULL;
}

/*
 *	first_path_separator
 *
 * Find the location of the first path separator (i.e. ':' on
 * Unix, ';' on Windows), return NULL if not found.
 */
char *
first_path_separator(const char *pathlist)
{
	const char *p;

	/* skip_drive is not needed */
	for (p = pathlist; *p; p++)
		if (IS_PATH_SEP(*p))
			return (char *) p;
	return NULL;
}

/*
 *	last_dir_separator
 *
 * Find the location of the last directory separator, return
 * NULL if not found.
 */
char *
last_dir_separator(const char *filename)
{
	const char *p,
			   *ret = NULL;

	for (p = skip_drive(filename); *p; p++)
		if (IS_DIR_SEP(*p))
			ret = p;
	return (char *) ret;
}


/*
 *	make_native_path - on WIN32, change / to \ in the path
 *
 *	This effectively undoes canonicalize_path.
 *
 *	This is required because WIN32 COPY is an internal CMD.EXE
 *	command and doesn't process forward slashes in the same way
 *	as external commands.  Quoting the first argument to COPY
 *	does not convert forward to backward slashes, but COPY does
 *	properly process quoted forward slashes in the second argument.
 *
 *	COPY works with quoted forward slashes in the first argument
 *	only if the current directory is the same as the directory
 *	of the first argument.
 */
void
make_native_path(char *filename)
{
#ifdef WIN32
	char	   *p;

	for (p = filename; *p; p++)
		if (*p == '/')
			*p = '\\';
#endif
}


/*
 * join_path_components - join two path components, inserting a slash
 *
 * ret_path is the output area (must be of size MAXPGPATH)
 *
 * ret_path can be the same as head, but not the same as tail.
 */
void
join_path_components(char *ret_path,
					 const char *head, const char *tail)
{
	if (ret_path != head)
		StrNCpy(ret_path, head, MAXPGPATH);
	/*
	 * Remove any leading "." and ".." in the tail component,
	 * adjusting head as needed.
	 */
	for (;;)
	{
		if (tail[0] == '.' && IS_DIR_SEP(tail[1]))
		{
			tail += 2;
		}
		else if (tail[0] == '.' && tail[1] == '\0')
		{
			tail += 1;
			break;
		}
		else if (tail[0] == '.' && tail[1] == '.' && IS_DIR_SEP(tail[2]))
		{
			trim_directory(ret_path);
			tail += 3;
		}
		else if (tail[0] == '.' && tail[1] == '.' && tail[2] == '\0')
		{
			trim_directory(ret_path);
			tail += 2;
			break;
		}
		else
			break;
	}
	if (*tail)
		snprintf(ret_path + strlen(ret_path), MAXPGPATH - strlen(ret_path),
				 "/%s", tail);
}


/*
 *	Clean up path by:
 *		o  make Win32 path use Unix slashes
 *		o  remove trailing quote on Win32
 *		o  remove trailing slash
 *		o  remove trailing '.'
 *		o  process trailing '..' ourselves
 */
void
canonicalize_path(char *path)
{
#ifdef WIN32

	/*
	 * The Windows command processor will accept suitably quoted paths
	 * with forward slashes, but barfs badly with mixed forward and back
	 * slashes.
	 */
	char	   *p;

	for (p = path; *p; p++)
	{
		if (*p == '\\')
			*p = '/';
	}

	/*
	 * In Win32, if you do: prog.exe "a b" "\c\d\" the system will pass
	 * \c\d" as argv[2].
	 */
	if (p > path && *(p - 1) == '"')
		*(p - 1) = '/';
#endif

	/*
	 * Removing the trailing slash on a path means we never get ugly
	 * double trailing slashes.	Also, Win32 can't stat() a directory
	 * with a trailing slash. Don't remove a leading slash, though.
	 */
	trim_trailing_separator(path);

	/*
	 * Remove any trailing uses of "." and process ".." ourselves
	 */
	for (;;)
	{
		int			len = strlen(path);

		if (len >= 2 && strcmp(path + len - 2, "/.") == 0)
		{
			trim_directory(path);
		}
		else if (len >= 3 && strcmp(path + len - 3, "/..") == 0)
		{
			trim_directory(path);
			trim_directory(path);	/* remove directory above */
		}
		else
			break;
	}
}


/*
 * Extracts the actual name of the program as called - 
 * stripped of .exe suffix if any
 */
const char *
get_progname(const char *argv0)
{
	const char *nodir_name;

	nodir_name = last_dir_separator(argv0);
	if (nodir_name)
		nodir_name++;
	else
		nodir_name = skip_drive(argv0);

#if defined(__CYGWIN__) || defined(WIN32)
	/* strip .exe suffix, regardless of case */
	if (strlen(nodir_name) > sizeof(EXE) - 1 &&
		pg_strcasecmp(nodir_name + strlen(nodir_name)-(sizeof(EXE)-1), EXE) == 0)
	{
		char *progname;

		progname = strdup(nodir_name);
		if (progname == NULL)
		{
			fprintf(stderr, "%s: out of memory\n", nodir_name);
			exit(1);	/* This could exit the postmaster */
		}
		progname[strlen(progname) - (sizeof(EXE) - 1)] = '\0';
		nodir_name = progname; 
	}
#endif

	return nodir_name;
}


/*
 *	get_share_path
 */
void
get_share_path(const char *my_exec_path, char *ret_path)
{
	const char *p;

	if ((p = relative_path(PGBINDIR, PGSHAREDIR)))
		make_relative(my_exec_path, p, ret_path);
	else
		StrNCpy(ret_path, PGSHAREDIR, MAXPGPATH);
	canonicalize_path(ret_path);
}


/*
 *	get_etc_path
 */
void
get_etc_path(const char *my_exec_path, char *ret_path)
{
	const char *p;

	if ((p = relative_path(PGBINDIR, SYSCONFDIR)))
		make_relative(my_exec_path, p, ret_path);
	else
		StrNCpy(ret_path, SYSCONFDIR, MAXPGPATH);
	canonicalize_path(ret_path);
}


/*
 *	get_include_path
 */
void
get_include_path(const char *my_exec_path, char *ret_path)
{
	const char *p;

	if ((p = relative_path(PGBINDIR, INCLUDEDIR)))
		make_relative(my_exec_path, p, ret_path);
	else
		StrNCpy(ret_path, INCLUDEDIR, MAXPGPATH);
	canonicalize_path(ret_path);
}


/*
 *	get_pkginclude_path
 */
void
get_pkginclude_path(const char *my_exec_path, char *ret_path)
{
	const char *p;

	if ((p = relative_path(PGBINDIR, PKGINCLUDEDIR)))
		make_relative(my_exec_path, p, ret_path);
	else
		StrNCpy(ret_path, PKGINCLUDEDIR, MAXPGPATH);
	canonicalize_path(ret_path);
}


/*
 *	get_includeserver_path
 */
void
get_includeserver_path(const char *my_exec_path, char *ret_path)
{
	const char *p;

	if ((p = relative_path(PGBINDIR, INCLUDEDIRSERVER)))
		make_relative(my_exec_path, p, ret_path);
	else
		StrNCpy(ret_path, INCLUDEDIRSERVER, MAXPGPATH);
	canonicalize_path(ret_path);
}


/*
 *	get_lib_path
 */
void
get_lib_path(const char *my_exec_path, char *ret_path)
{
	const char *p;

	if ((p = relative_path(PGBINDIR, LIBDIR)))
		make_relative(my_exec_path, p, ret_path);
	else
		StrNCpy(ret_path, LIBDIR, MAXPGPATH);
	canonicalize_path(ret_path);
}


/*
 *	get_pkglib_path
 */
void
get_pkglib_path(const char *my_exec_path, char *ret_path)
{
	const char *p;

	if ((p = relative_path(PGBINDIR, PKGLIBDIR)))
		make_relative(my_exec_path, p, ret_path);
	else
		StrNCpy(ret_path, PKGLIBDIR, MAXPGPATH);
	canonicalize_path(ret_path);
}


/*
 *	get_locale_path
 *
 *	Return locale path, either relative to /bin or hardcoded
 */
void
get_locale_path(const char *my_exec_path, char *ret_path)
{
	const char *p;

	if ((p = relative_path(PGBINDIR, LOCALEDIR)))
		make_relative(my_exec_path, p, ret_path);
	else
		StrNCpy(ret_path, LOCALEDIR, MAXPGPATH);
	canonicalize_path(ret_path);
}


/*
 *	get_home_path
 */
bool
get_home_path(char *ret_path)
{
	if (getenv(HOMEDIR) == NULL)
	{
		*ret_path = '\0';
		return false;
	}
	else
	{
		StrNCpy(ret_path, getenv(HOMEDIR), MAXPGPATH);
		canonicalize_path(ret_path);
		return true;
	}
}


/*
 * get_parent_directory
 *
 * Modify the given string in-place to name the parent directory of the
 * named file.
 */
void
get_parent_directory(char *path)
{
	trim_directory(path);
}


/*
 *	set_pglocale_pgservice
 *
 *	Set application-specific locale and service directory
 *
 *	This function takes an argv[0] rather than a full path.
 */
void
set_pglocale_pgservice(const char *argv0, const char *app)
{
	char		path[MAXPGPATH];
	char		my_exec_path[MAXPGPATH];
	char		env_path[MAXPGPATH + sizeof("PGSYSCONFDIR=")];	/* longer than
																 * PGLOCALEDIR */

	/* don't set LC_ALL in the backend */
	if (strcmp(app, "postgres") != 0)
		setlocale(LC_ALL, "");

	if (find_my_exec(argv0, my_exec_path) < 0)
		return;

#ifdef ENABLE_NLS
	get_locale_path(my_exec_path, path);
	bindtextdomain(app, path);
	textdomain(app);

	if (getenv("PGLOCALEDIR") == NULL)
	{
		/* set for libpq to use */
		snprintf(env_path, sizeof(env_path), "PGLOCALEDIR=%s", path);
		canonicalize_path(env_path + 12);
		putenv(strdup(env_path));
	}
#endif

	if (getenv("PGSYSCONFDIR") == NULL)
	{
		get_etc_path(my_exec_path, path);

		/* set for libpq to use */
		snprintf(env_path, sizeof(env_path), "PGSYSCONFDIR=%s", path);
		canonicalize_path(env_path + 13);
		putenv(strdup(env_path));
	}
}


/*
 *	make_relative - adjust path to be relative to bin/
 *
 * ret_path is the output area (must be of size MAXPGPATH)
 */
static void
make_relative(const char *my_exec_path, const char *p, char *ret_path)
{
	char		path[MAXPGPATH];

	StrNCpy(path, my_exec_path, MAXPGPATH);
	trim_directory(path);		/* remove my executable name */
	trim_directory(path);		/* remove last directory component (/bin) */
	join_path_components(ret_path, path, p);
}


/*
 *	relative_path
 *
 *	Do the supplied paths differ only in their last component?
 */
static const char *
relative_path(const char *bin_path, const char *other_path)
{
#ifdef WIN32
	/* Driver letters match? */
	if (isalpha(*bin_path) && bin_path[1] == ':' &&
		(!isalpha(*other_path) || !other_path[1] == ':'))
		return NULL;
	if ((!isalpha(*bin_path) || !bin_path[1] == ':') &&
		(isalpha(*other_path) && other_path[1] == ':'))
		return NULL;
	if (isalpha(*bin_path) && bin_path[1] == ':' &&
		isalpha(*other_path) && other_path[1] == ':')
	{
		if (toupper(*bin_path) != toupper(*other_path))
			return NULL;
		bin_path += 2;
		other_path += 2;
	}
#endif

	while (1)
	{
		/* Move past adjacent slashes like //, and trailing ones */
		MOVE_TO_SEP_END(bin_path);
		MOVE_TO_SEP_END(other_path);

		/* One of the paths is done? */
		if (!*bin_path || !*other_path)
			break;

		/* Win32 filesystem is case insensitive */
		if ((!IS_DIR_SEP(*bin_path) || !IS_DIR_SEP(*other_path)) &&
#ifndef WIN32
			*bin_path != *other_path
#else
			toupper((unsigned char) *bin_path) != toupper((unsigned char) *other_path)
#endif
			)
			break;

		bin_path++;
		other_path++;
	}

	/* identical? */
	if (!*bin_path && !*other_path)
		return NULL;

	/* advance past directory name */
	while (!IS_DIR_SEP(*bin_path) && *bin_path)
		bin_path++;

	MOVE_TO_SEP_END(bin_path);

	/* Is bin done? */
	if (!*bin_path)
		return other_path;
	else
		return NULL;
}


/*
 *	trim_directory
 *
 *	Trim trailing directory from path, that is, remove any trailing slashes,
 *	the last pathname component, and the slash just ahead of it --- but never
 *	remove a leading slash.
 */
static void
trim_directory(char *path)
{
	char	   *p;

	path = skip_drive(path);

	if (path[0] == '\0')
		return;

	/* back up over trailing slash(es) */
	for (p = path + strlen(path) - 1; IS_DIR_SEP(*p) && p > path; p--)
		;
	/* back up over directory name */
	for (; !IS_DIR_SEP(*p) && p > path; p--)
		;
	/* if multiple slashes before directory name, remove 'em all */
	for (; p > path && IS_DIR_SEP(*(p - 1)); p--)
		;
	/* don't erase a leading slash */
	if (p == path && IS_DIR_SEP(*p))
		p++;
	*p = '\0';
}


/*
 *	trim_trailing_separator
 *
 * trim off trailing slashes, but not a leading slash
 */
static void
trim_trailing_separator(char *path)
{
	char	   *p;

	path = skip_drive(path);
	p = path + strlen(path);
	if (p > path)
		for (p--; p > path && IS_DIR_SEP(*p); p--)
			*p = '\0';
}
