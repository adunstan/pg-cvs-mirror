/*-------------------------------------------------------------------------
 *
 * rint.c
 *	  rint() implementation
 *
 * IDENTIFICATION
 *	  $PostgreSQL: pgsql/src/port/rint.c,v 1.2 2003/11/29 19:52:13 pgsql Exp $
 *
 *-------------------------------------------------------------------------
 */

#include "c.h"
#include <math.h>

double
rint(double x)
{
	return (x > 0.0) ? floor(x + 0.5) : ceil(x - 0.5);
}
