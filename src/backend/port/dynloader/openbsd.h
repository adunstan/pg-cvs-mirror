/*-------------------------------------------------------------------------
 *
 * Dynamic loader for OpenBSD
 *
 * Portions Copyright (c) 1996-2001, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $Id: openbsd.h,v 1.6 2001/06/20 18:33:31 petere Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef PORT_PROTOS_H
#define PORT_PROTOS_H

#include <sys/types.h>
#include <nlist.h>
#include <link.h>
#include <dlfcn.h>

#include "utils/dynamic_loader.h"

/* dynloader.c */
/*
 * Dynamic Loader on NetBSD 1.0.
 *
 * this dynamic loader uses the system dynamic loading interface for shared
 * libraries (ie. dlopen/dlsym/dlclose). The user must specify a shared
 * library as the file to be dynamically loaded.
 *
 * agc - I know this is all a bit crufty, but it does work, is fairly
 * portable, and works (the stipulation that the d.l. function must
 * begin with an underscore is fairly tricky, and some versions of
 * NetBSD (like 1.0, and 1.0A pre June 1995) have no dlerror.)
 */
#define		   pg_dlopen(f)    BSD44_derived_dlopen((f), RTLD_LAZY)
#define		   pg_dlsym		   BSD44_derived_dlsym
#define		   pg_dlclose	   BSD44_derived_dlclose
#define		   pg_dlerror	   BSD44_derived_dlerror

char	   *BSD44_derived_dlerror(void);
void	   *BSD44_derived_dlopen(const char *filename, int num);
void	   *BSD44_derived_dlsym(void *handle, const char *name);
void		BSD44_derived_dlclose(void *handle);

#endif	 /* PORT_PROTOS_H */
