/*-------------------------------------------------------------------------
 *
 * like.c
 *	  like expression handling code.
 *
 *	 NOTES
 *		A big hack of the regexp.c code!! Contributed by
 *		Keith Parks <emkxp01@mtcc.demon.co.uk> (7/95).
 *
 * Portions Copyright (c) 1996-2000, PostgreSQL, Inc
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * IDENTIFICATION
 *	$Header: /home/cvsmirror/pg/pgsql/src/backend/utils/adt/like.c,v 1.37 2000/07/07 21:12:50 tgl Exp $
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include "mb/pg_wchar.h"
#include "utils/builtins.h"

static bool like(pg_wchar * text, pg_wchar * p);

/*
 *	interface routines called by the function manager
 */

/*
   fixedlen_like:

   a generic fixed length like routine
		 s		- the string to match against  (not necessarily null-terminated)
		 p		   - the pattern (as text*)
		 charlen   - the length of the string
*/
static bool
fixedlen_like(char *s, text *p, int charlen)
{
	pg_wchar   *sterm,
			   *pterm;
	bool		result;
	int			len;

	/* be sure sterm is null-terminated */
#ifdef MULTIBYTE
	sterm = (pg_wchar *) palloc((charlen + 1) * sizeof(pg_wchar));
	(void) pg_mb2wchar_with_len((unsigned char *) s, sterm, charlen);
#else
	sterm = (char *) palloc(charlen + 1);
	memcpy(sterm, s, charlen);
	sterm[charlen] = '\0';
#endif

	/*
	 * p is a text, not a string so we have to make a string
	 * from the vl_data field of the struct.
	 */

	/* palloc the length of the text + the null character */
	len = VARSIZE(p) - VARHDRSZ;
#ifdef MULTIBYTE
	pterm = (pg_wchar *) palloc((len + 1) * sizeof(pg_wchar));
	(void) pg_mb2wchar_with_len((unsigned char *) VARDATA(p), pterm, len);
#else
	pterm = (char *) palloc(len + 1);
	memcpy(pterm, VARDATA(p), len);
	*(pterm + len) = '\0';
#endif

	/* do the regexp matching */
	result = like(sterm, pterm);

	pfree(sterm);
	pfree(pterm);

	return result;
}

Datum
namelike(PG_FUNCTION_ARGS)
{
	Name		n = PG_GETARG_NAME(0);
	text	   *p = PG_GETARG_TEXT_P(1);

	PG_RETURN_BOOL(fixedlen_like(NameStr(*n), p, strlen(NameStr(*n))));
}

Datum
namenlike(PG_FUNCTION_ARGS)
{
	Name		n = PG_GETARG_NAME(0);
	text	   *p = PG_GETARG_TEXT_P(1);

	PG_RETURN_BOOL(! fixedlen_like(NameStr(*n), p, strlen(NameStr(*n))));
}

Datum
textlike(PG_FUNCTION_ARGS)
{
	text	   *s = PG_GETARG_TEXT_P(0);
	text	   *p = PG_GETARG_TEXT_P(1);

	PG_RETURN_BOOL(fixedlen_like(VARDATA(s), p, VARSIZE(s) - VARHDRSZ));
}

Datum
textnlike(PG_FUNCTION_ARGS)
{
	text	   *s = PG_GETARG_TEXT_P(0);
	text	   *p = PG_GETARG_TEXT_P(1);

	PG_RETURN_BOOL(! fixedlen_like(VARDATA(s), p, VARSIZE(s) - VARHDRSZ));
}


/*
**	Originally written by Rich $alz, mirror!rs, Wed Nov 26 19:03:17 EST 1986.
**	Rich $alz is now <rsalz@bbn.com>.
**	Special thanks to Lars Mathiesen <thorinn@diku.dk> for the LABORT code.
**
**	This code was shamelessly stolen from the "pql" code by myself and
**	slightly modified :)
**
**	All references to the word "star" were replaced by "percent"
**	All references to the word "wild" were replaced by "like"
**
**	All the nice shell RE matching stuff was replaced by just "_" and "%"
**
**	As I don't have a copy of the SQL standard handy I wasn't sure whether
**	to leave in the '\' escape character handling.
**
**	Keith Parks. <keith@mtcc.demon.co.uk>
**
**	[SQL92 lets you specify the escape character by saying
**	 LIKE <pattern> ESCAPE <escape character>. We are a small operation
**	 so we force you to use '\'. - ay 7/95]
**
*/

#define LIKE_TRUE						1
#define LIKE_FALSE						0
#define LIKE_ABORT						(-1)

/*--------------------
 *	Match text and p, return LIKE_TRUE, LIKE_FALSE, or LIKE_ABORT.
 *
 *	LIKE_TRUE: they match
 *	LIKE_FALSE: they don't match
 *	LIKE_ABORT: not only don't they match, but the text is too short.
 *
 * If LIKE_ABORT is returned, then no suffix of the text can match the
 * pattern either, so an upper-level % scan can stop scanning now.
 *--------------------
 */
static int
DoMatch(pg_wchar * text, pg_wchar * p)
{
	for (; *p && *text; text ++, p++)
	{
		switch (*p)
		{
			case '\\':
				/* Literal match with following character. */
				p++;
				/* FALLTHROUGH */
			default:
				if (*text !=*p)
					return LIKE_FALSE;
				break;
			case '_':
				/* Match any single character. */
				break;
			case '%':
				/* %% is the same as % according to the SQL standard */
				/* Advance past all %'s */
				while (*p == '%')
					p++;
				/* Trailing percent matches everything. */
				if (*p == '\0')
					return LIKE_TRUE;

				/*
				 * Otherwise, scan for a text position at which we can
				 * match the rest of the pattern.
				 */
				for (; *text; text ++)
				{

					/*
					 * Optimization to prevent most recursion: don't
					 * recurse unless first pattern char might match this
					 * text char.
					 */
					if (*text == *p || *p == '\\' || *p == '_')
					{
						int			matched = DoMatch(text, p);

						if (matched != LIKE_FALSE)
							return matched;		/* TRUE or ABORT */
					}
				}

				/*
				 * End of text with no match, so no point in trying later
				 * places to start matching this pattern.
				 */
				return LIKE_ABORT;
		}
	}

	if (*text !='\0')
		return LIKE_FALSE;		/* end of pattern, but not of text */

	/* End of input string.  Do we have matching pattern remaining? */
	while (*p == '%')			/* allow multiple %'s at end of pattern */
		p++;
	if (*p == '\0')
		return LIKE_TRUE;

	/*
	 * End of text with no match, so no point in trying later places to
	 * start matching this pattern.
	 */
	return LIKE_ABORT;
}

/*
**	User-level routine.  Returns TRUE or FALSE.
*/
static bool
like(pg_wchar * text, pg_wchar * p)
{
	/* Fast path for match-everything pattern */
	if (p[0] == '%' && p[1] == '\0')
		return true;
	return DoMatch(text, p) == LIKE_TRUE;
}
