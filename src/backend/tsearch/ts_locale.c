/*-------------------------------------------------------------------------
 *
 * ts_locale.c
 *		locale compatibility layer for tsearch
 *
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
 *
 *
 * IDENTIFICATION
 *	  $PostgreSQL: pgsql/src/backend/tsearch/ts_locale.c,v 1.8 2008/06/17 16:09:06 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include "tsearch/ts_locale.h"
#include "tsearch/ts_public.h"

#ifdef USE_WIDE_UPPER_LOWER

int
t_isdigit(const char *ptr)
{
	int			clen = pg_mblen(ptr);
	wchar_t		character[2];

	if (clen == 1 || lc_ctype_is_c())
		return isdigit(TOUCHAR(ptr));

	char2wchar(character, 2, ptr, clen);

	return iswdigit((wint_t) character[0]);
}

int
t_isspace(const char *ptr)
{
	int			clen = pg_mblen(ptr);
	wchar_t		character[2];

	if (clen == 1 || lc_ctype_is_c())
		return isspace(TOUCHAR(ptr));

	char2wchar(character, 2, ptr, clen);

	return iswspace((wint_t) character[0]);
}

int
t_isalpha(const char *ptr)
{
	int			clen = pg_mblen(ptr);
	wchar_t		character[2];

	if (clen == 1 || lc_ctype_is_c())
		return isalpha(TOUCHAR(ptr));

	char2wchar(character, 2, ptr, clen);

	return iswalpha((wint_t) character[0]);
}

int
t_isprint(const char *ptr)
{
	int			clen = pg_mblen(ptr);
	wchar_t		character[2];

	if (clen == 1 || lc_ctype_is_c())
		return isprint(TOUCHAR(ptr));

	char2wchar(character, 2, ptr, clen);

	return iswprint((wint_t) character[0]);
}
#endif   /* USE_WIDE_UPPER_LOWER */


/*
 * Read the next line from a tsearch data file (expected to be in UTF-8), and
 * convert it to database encoding if needed. The returned string is palloc'd.
 * NULL return means EOF.
 */
char *
t_readline(FILE *fp)
{
	int			len;
	char	   *recoded;
	char		buf[4096];		/* lines must not be longer than this */

	if (fgets(buf, sizeof(buf), fp) == NULL)
		return NULL;

	len = strlen(buf);

	/* Make sure the input is valid UTF-8 */
	(void) pg_verify_mbstr(PG_UTF8, buf, len, false);

	/* And convert */
	recoded = (char *) pg_do_encoding_conversion((unsigned char *) buf,
												 len,
												 PG_UTF8,
												 GetDatabaseEncoding());

	if (recoded == NULL)		/* should not happen */
		elog(ERROR, "encoding conversion failed");

	if (recoded == buf)
	{
		/*
		 * conversion didn't pstrdup, so we must. We can use the length of the
		 * original string, because no conversion was done.
		 */
		recoded = pnstrdup(recoded, len);
	}

	return recoded;
}

/*
 * lowerstr --- fold null-terminated string to lower case
 *
 * Returned string is palloc'd
 */
char *
lowerstr(const char *str)
{
	return lowerstr_with_len(str, strlen(str));
}

/*
 * lowerstr_with_len --- fold string to lower case
 *
 * Input string need not be null-terminated.
 *
 * Returned string is palloc'd
 */
char *
lowerstr_with_len(const char *str, int len)
{
	char	   *out;

	if (len == 0)
		return pstrdup("");

#ifdef USE_WIDE_UPPER_LOWER

	/*
	 * Use wide char code only when max encoding length > 1 and ctype != C.
	 * Some operating systems fail with multi-byte encodings and a C locale.
	 * Also, for a C locale there is no need to process as multibyte. From
	 * backend/utils/adt/oracle_compat.c Teodor
	 */
	if (pg_database_encoding_max_length() > 1 && !lc_ctype_is_c())
	{
		wchar_t    *wstr,
				   *wptr;
		int			wlen;

		/*
		 * alloc number of wchar_t for worst case, len contains number of
		 * bytes >= number of characters and alloc 1 wchar_t for 0, because
		 * wchar2char wants zero-terminated string
		 */
		wptr = wstr = (wchar_t *) palloc(sizeof(wchar_t) * (len + 1));

		wlen = char2wchar(wstr, len + 1, str, len);
		Assert(wlen <= len);

		while (*wptr)
		{
			*wptr = towlower((wint_t) *wptr);
			wptr++;
		}

		/*
		 * Alloc result string for worst case + '\0'
		 */
		len = pg_database_encoding_max_length() * wlen + 1;
		out = (char *) palloc(len);

		wlen = wchar2char(out, wstr, len);

		pfree(wstr);

		if (wlen < 0)
			ereport(ERROR,
					(errcode(ERRCODE_CHARACTER_NOT_IN_REPERTOIRE),
					 errmsg("conversion from wchar_t to server encoding failed: %m")));
		Assert(wlen < len);
	}
	else
#endif   /* USE_WIDE_UPPER_LOWER */
	{
		const char *ptr = str;
		char	   *outptr;

		outptr = out = (char *) palloc(sizeof(char) * (len + 1));
		while ((ptr - str) < len && *ptr)
		{
			*outptr++ = tolower(TOUCHAR(ptr));
			ptr++;
		}
		*outptr = '\0';
	}

	return out;
}
