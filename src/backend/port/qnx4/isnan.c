/*-------------------------------------------------------------------------
 *
 * isnan.c
 *	  isnan() implementation
 *
 * Copyright (c) 1999, repas AEG Automation GmbH
 *
 *
 * IDENTIFICATION
 *	  $PostgreSQL: pgsql-server/src/backend/port/qnx4/isnan.c,v 1.4 2003/11/29 19:51:54 pgsql Exp $
 *
 *-------------------------------------------------------------------------
 */

#include "c.h"

#include <math.h>

#ifndef __nan_bytes
#define __nan_bytes			{ 0, 0, 0, 0, 0, 0, 0xf8, 0x7f }
#endif   /* __nan_bytes */

static unsigned char __nan[8] = __nan_bytes;

int
isnan(double dsrc)
{
	return memcmp(&dsrc, __nan, sizeof(double)) == 0;
}
