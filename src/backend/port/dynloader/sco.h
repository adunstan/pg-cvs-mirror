/*-------------------------------------------------------------------------
 *
 * port_protos.h
 *	  port-specific prototypes for SCO 3.2v5.2
 *
 *
 * Copyright (c) 1994, Regents of the University of California
 *
 * $Id: sco.h,v 1.4 1999/07/16 03:13:13 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef PORT_PROTOS_H
#define PORT_PROTOS_H

#include <dlfcn.h>
#include "fmgr.h"
#include "utils/dynamic_loader.h"

/* dynloader.c */
/*
 * Dynamic Loader on SCO 3.2v5.0.2
 *
 * this dynamic loader uses the system dynamic loading interface for shared
 * libraries (ie. dlopen/dlsym/dlclose). The user must specify a shared
 * library as the file to be dynamically loaded.
 *
 */
#define pg_dlopen(f)	dlopen(f,1)
#define pg_dlsym		dlsym
#define pg_dlclose		dlclose
#define pg_dlerror		dlerror

/* port.c */

#endif	 /* PORT_PROTOS_H */
