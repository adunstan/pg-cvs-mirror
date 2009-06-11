/*
 * $PostgreSQL: pgsql/src/port/strtol.c,v 1.16 2009/01/01 17:24:04 momjian Exp $
 *
 *
 * Portions Copyright (c) 1996-2009, PostgreSQL Global Development Group
 * Portions Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *	  notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *	  notice, this list of conditions and the following disclaimer in the
 *	  documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *	  may be used to endorse or promote products derived from this software
 *	  without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.	IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)strtol.c	5.4 (Berkeley) 2/23/91";
#endif   /* LIBC_SCCS and not lint */

#include "c.h"

#include <limits.h>
#include <ctype.h>


#define const

/*
 *	Usage Tip:
 *
 *	strtol() doesn't give a unique return value to indicate that errno
 *	should be consulted, so in most cases it is best to set errno = 0
 *	before calling this function, and then errno != 0 can be tested
 *	after the function completes.
 */

/*
 * Convert a string to a long integer.
 *
 * Ignores `locale' stuff.  Assumes that the upper and lower case
 * alphabets and digits are each contiguous.
 */
long
strtol(nptr, endptr, base)
const char *nptr;
char	  **endptr;
int			base;
{
	const char *s = nptr;
	unsigned long acc;
	unsigned char c;
	unsigned long cutoff;
	int			neg = 0,
				any,
				cutlim;

	/*
	 * Skip white space and pick up leading +/- sign if any. If base is 0,
	 * allow 0x for hex and 0 for octal, else assume decimal; if base is
	 * already 16, allow 0x.
	 */
	do
	{
		c = *s++;
	} while (isspace(c));
	if (c == '-')
	{
		neg = 1;
		c = *s++;
	}
	else if (c == '+')
		c = *s++;
	if ((base == 0 || base == 16) &&
		c == '0' && (*s == 'x' || *s == 'X'))
	{
		c = s[1];
		s += 2;
		base = 16;
	}
	if (base == 0)
		base = c == '0' ? 8 : 10;

	/*
	 * Compute the cutoff value between legal numbers and illegal numbers.
	 * That is the largest legal value, divided by the base.  An input number
	 * that is greater than this value, if followed by a legal input
	 * character, is too big.  One that is equal to this value may be valid or
	 * not; the limit between valid and invalid numbers is then based on the
	 * last digit.	For instance, if the range for longs is
	 * [-2147483648..2147483647] and the input base is 10, cutoff will be set
	 * to 214748364 and cutlim to either 7 (neg==0) or 8 (neg==1), meaning
	 * that if we have accumulated a value > 214748364, or equal but the next
	 * digit is > 7 (or 8), the number is too big, and we will return a range
	 * error.
	 *
	 * Set any if any `digits' consumed; make it negative to indicate
	 * overflow.
	 */
	cutoff = neg ? -(unsigned long) LONG_MIN : LONG_MAX;
	cutlim = cutoff % (unsigned long) base;
	cutoff /= (unsigned long) base;
	for (acc = 0, any = 0;; c = *s++)
	{
		if (isdigit(c))
			c -= '0';
		else if (isalpha(c))
			c -= isupper(c) ? 'A' - 10 : 'a' - 10;
		else
			break;
		if ((int) c >= base)
			break;
		if (any < 0 || acc > cutoff || acc == cutoff && (int) c > cutlim)
			any = -1;
		else
		{
			any = 1;
			acc *= base;
			acc += c;
		}
	}
	if (any < 0)
	{
		acc = neg ? LONG_MIN : LONG_MAX;
		errno = ERANGE;
	}
	else if (neg)
		acc = -acc;
	if (endptr != 0)
		*endptr = any ? s - 1 : (char *) nptr;
	return acc;
}
