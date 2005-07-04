/*-------------------------------------------------------------------------
 *
 * pgtz.h
 *	  Timezone Library Integration Functions
 *
 * Note: this file contains only definitions that are private to the
 * timezone library.  Public definitions are in pgtime.h.
 *
 * Portions Copyright (c) 1996-2005, PostgreSQL Global Development Group
 *
 * IDENTIFICATION
 *	  $PostgreSQL: pgsql/src/timezone/pgtz.h,v 1.12 2005/06/15 00:34:11 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef _PGTZ_H
#define _PGTZ_H

#include "tzfile.h"

/*
 *	Prevent the use of /port functions because
 *	the are not included in this binary.
 */
#undef vsnprintf
#undef snprintf
#undef sprintf
#undef fprintf
#undef printf

extern char *pg_TZDIR(void);

#define BIGGEST(a, b)	(((a) > (b)) ? (a) : (b))

struct ttinfo
{								/* time type information */
	long		tt_gmtoff;		/* UTC offset in seconds */
	int			tt_isdst;		/* used to set tm_isdst */
	int			tt_abbrind;		/* abbreviation list index */
	int			tt_ttisstd;		/* TRUE if transition is std time */
	int			tt_ttisgmt;		/* TRUE if transition is UTC */
};

struct lsinfo
{								/* leap second information */
	pg_time_t	ls_trans;		/* transition time */
	long		ls_corr;		/* correction to apply */
};

struct state
{
	int			leapcnt;
	int			timecnt;
	int			typecnt;
	int			charcnt;
	pg_time_t	ats[TZ_MAX_TIMES];
	unsigned char types[TZ_MAX_TIMES];
	struct ttinfo ttis[TZ_MAX_TYPES];
	char		chars[BIGGEST(BIGGEST(TZ_MAX_CHARS + 1, 3 /* sizeof gmt */),
										  (2 * (TZ_STRLEN_MAX + 1)))];
	struct lsinfo lsis[TZ_MAX_LEAPS];
};


struct pg_tz {
	char TZname[TZ_STRLEN_MAX + 1];
	struct state state;
};

int	tzload(const char *name, struct state * sp);
int	tzparse(const char *name, struct state * sp, int lastditch);

#endif   /* _PGTZ_H */
