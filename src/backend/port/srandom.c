/* $Id: srandom.c,v 1.10 2001/08/24 14:07:49 petere Exp $ */

#include "c.h"

#include <stdlib.h>
#include <math.h>
#include <errno.h>

void
srandom(unsigned int seed)
{
	srand48((long int) seed);
}
