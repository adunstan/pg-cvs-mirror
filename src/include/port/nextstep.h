/* $PostgreSQL: pgsql/src/backend/utils/misc/guc.c,v 1.314 2006/03/07 02:54:23 momjian Exp $ */

#include "libc.h"
#include <sys/ioctl.h>
#if defined(__STRICT_ANSI__)
#define isascii(c)	((unsigned)(c)<=0177)
#endif
extern char *strdup(const char *string);

#ifndef _POSIX_SOURCE
typedef unsigned short mode_t;
typedef int sigset_t;

#define SIG_BLOCK	  00
#define SIG_UNBLOCK   01
#define SIG_SETMASK   02
#endif

#define NO_WAITPID
