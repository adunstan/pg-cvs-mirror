/*-------------------------------------------------------------------------
 *
 * page.h--
 *    POSTGRES buffer page abstraction definitions.
 *
 *
 * Copyright (c) 1994, Regents of the University of California
 *
 * $Id: page.h,v 1.1 1996/08/28 01:58:20 scrappy Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef	PAGE_H
#define PAGE_H

#include "c.h"

typedef Pointer	Page;

/*
 * PageIsValid --
 *	True iff page is valid.
 */
#define	PageIsValid(page) PointerIsValid(page)

#endif	/* PAGE_H */
