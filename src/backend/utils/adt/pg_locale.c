
/* -----------------------------------------------------------------------
 * pg_locale.c
 *
 * $Header: /home/cvsmirror/pg/pgsql/src/backend/utils/adt/pg_locale.c,v 1.5 2000/06/29 01:19:36 momjian Exp $
 *
 *
 *	 Portions Copyright (c) 1999-2000, PostgreSQL, Inc
 *
 *	 The PostgreSQL locale utils.
 *
 *	Karel Zak - Zakkr
 *
 * -----------------------------------------------------------------------
 */

#include <stdio.h>

#include "postgres.h"

#ifdef USE_LOCALE

#include <locale.h>
#include "utils/pg_locale.h"

/* #define DEBUG_LOCALE_UTILS  */


/*------
 * Return in PG_LocaleCategories current locale setting
 *------
 */
PG_LocaleCategories *
PGLC_current(PG_LocaleCategories * lc)
{
	lc->lang = getenv("LANG");

	lc->lc_ctype = setlocale(LC_CTYPE, NULL);
	lc->lc_numeric = setlocale(LC_NUMERIC, NULL);
	lc->lc_time = setlocale(LC_TIME, NULL);
	lc->lc_collate = setlocale(LC_COLLATE, NULL);
	lc->lc_monetary = setlocale(LC_MONETARY, NULL);
#ifdef LC_MESSAGES
	lc->lc_messages = setlocale(LC_MESSAGES, NULL);
#endif
	return lc;
}


#ifdef DEBUG_LOCALE_UTILS

/*------
 * Print a PG_LocaleCategories struct as DEBUG
 *------
 */
PG_LocaleCategories *
PGLC_debug_lc(PG_LocaleCategories * lc)
{
#ifdef LC_MESSAGES
	elog(DEBUG, "CURRENT LOCALE ENVIRONMENT:\n\nLANG:   \t%s\nLC_CTYPE:\t%s\nLC_NUMERIC:\t%s\nLC_TIME:\t%s\nLC_COLLATE:\t%s\nLC_MONETARY:\t%s\nLC_MESSAGES:\t%s\n",
#else
	elog(DEBUG, "CURRENT LOCALE ENVIRONMENT:\n\nLANG:   \t%s\nLC_CTYPE:\t%s\nLC_NUMERIC:\t%s\nLC_TIME:\t%s\nLC_COLLATE:\t%s\nLC_MONETARY:\t%s\n",
#endif
		 lc->lang,
		 lc->lc_ctype,
		 lc->lc_numeric,
		 lc->lc_time,
		 lc->lc_collate,
		 lc->lc_monetary
#ifdef LC_MESSAGES
		 , lc->lc_messages
#endif
	);

	return lc;
}

#endif

/*------
 * Set locales via a PG_LocaleCategories struct
 *------
 */
PG_LocaleCategories *
PGLC_setlocale(PG_LocaleCategories * lc)
{
	if (!setlocale(LC_CTYPE, lc->lc_ctype))
		elog(NOTICE, "pg_setlocale(): 'LC_CTYPE=%s' cannot be honored.", lc->lc_ctype);

	if (!setlocale(LC_NUMERIC, lc->lc_numeric))
		elog(NOTICE, "pg_setlocale(): 'LC_NUMERIC=%s' cannot be honored.", lc->lc_numeric);

	if (!setlocale(LC_TIME, lc->lc_time))
		elog(NOTICE, "pg_setlocale(): 'LC_TIME=%s' cannot be honored.", lc->lc_time);

	if (!setlocale(LC_COLLATE, lc->lc_collate))
		elog(NOTICE, "pg_setlocale(): 'LC_COLLATE=%s' cannot be honored.", lc->lc_collate);

	if (!setlocale(LC_MONETARY, lc->lc_monetary))
		elog(NOTICE, "pg_setlocale(): 'LC_MONETARY=%s' cannot be honored.", lc->lc_monetary);
#ifdef LC_MESSAGES
	if (!setlocale(LC_MESSAGES, lc->lc_messages))
		elog(NOTICE, "pg_setlocale(): 'LC_MESSAGE=%s' cannot be honored.", lc->lc_messages);
#endif
	return lc;
}

/*------
 * Return the POSIX lconv struct (contains number/money formatting information)
 * with locale information for *all* categories.
 * => Returned lconv is *independent* on current locale catogories setting - in
 * contrast to standard localeconv().
 *
 * ! libc prepare memory space for lconv itself and all returned strings in
 *	 lconv are *static strings*.
 *------
 */
struct lconv *
PGLC_localeconv(void)
{
	PG_LocaleCategories lc;
	struct lconv *lconv;

	/* Save current locale setting to lc */
	PGLC_current(&lc);

	/* Set all locale category for current lang */
	setlocale(LC_ALL, "");

	/* Get numeric formatting information */
	lconv = localeconv();

	/* Set previous original locale */
	PGLC_setlocale(&lc);

	return lconv;
}


#endif	 /* USE_LOCALE */
