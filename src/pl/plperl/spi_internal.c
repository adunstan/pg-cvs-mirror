/*
 * This kludge is necessary because of the conflicting
 * definitions of 'DEBUG' between postgres and perl.
 * we'll live.
 */

#include "postgres.h"
/* Defined by Perl */
#undef _


/* stop perl from hijacking stdio and other stuff */
#ifdef WIN32
#define WIN32IO_IS_STDIO
#endif 

#include "spi_internal.h"


int
spi_DEBUG(void)
{
	return DEBUG2;
}

int
spi_LOG(void)
{
	return LOG;
}

int
spi_INFO(void)
{
	return INFO;
}

int
spi_NOTICE(void)
{
	return NOTICE;
}

int
spi_WARNING(void)
{
	return WARNING;
}

int
spi_ERROR(void)
{
	return ERROR;
}
