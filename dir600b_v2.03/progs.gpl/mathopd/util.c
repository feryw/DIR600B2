/*
 *   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002 Michiel Boland.
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or
 *   without modification, are permitted provided that the following
 *   conditions are met:
 *
 *   1. Redistributions of source code must retain the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials
 *      provided with the distribution.
 *
 *   3. The name of the author may not be used to endorse or promote
 *      products derived from this software without specific prior
 *      written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY
 *   EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 *   PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR
 *   BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 *   TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 *   IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *   THE POSSIBILITY OF SUCH DAMAGE.
 */

/* San Jacinto */

static const char rcsid[] = "$Id: util.c,v 1.1.1.1 2005/05/19 10:53:06 r01122 Exp $";

#include <ctype.h>
#include "mathopd.h"

#define HEXDIGIT(x) (((x) <= '9') ? (x) - '0' : ((x) & 7) + 9)

int unescape_url(const char *from, char *to)
{
	char c;
	int x1, x2;

	while ((c = *from++) != 0)
		if (c == '%') {
			x1 = *from++;
			if (!isxdigit(x1))
				return -1;
			x2 = *from++;
			if (!isxdigit(x2))
				return -1;
			*to++ = (HEXDIGIT(x1) << 4) + HEXDIGIT(x2);
		} else
			*to++ = c;
	*to = 0;
	return 0;
}

int unescape_url_n(const char *from, char *to, size_t n)
{
	char c;
	int x1, x2;

	while (n-- && (c = *from++) != 0)
		if (c == '%') {
			x1 = *from++;
			if (!isxdigit(x1))
				return -1;
			x2 = *from++;
			if (!isxdigit(x2))
				return -1;
			*to++ = (HEXDIGIT(x1) << 4) + HEXDIGIT(x2);
		} else
			*to++ = c;
	*to = 0;
	return 0;
}

void sanitize_host(char *s)
{
	int c, l;

	l = 0;
	while ((c = *s) != 0) {
		if (c == ':') {
			*s = 0;
			break;
		}
		*s++ = tolower(c);
		l = c;
	}
	if (l == '.')
		s[-1] = 0;
}

#if defined(ELBOX_USE_IPV6) && ( \
	defined(ELBOX_MODEL_DAP2590) || \
	defined(ELBOX_MODEL_DAP2690))
/* a delay function with millisecond precision
 * input: n milliseconds
 * output: none
 */
void mdelay(const int n)
{
#include <sys/select.h>
	struct timeval delay;
	
	if( n <= 0 )
	{
		delay.tv_sec = 0;
		delay.tv_usec = 0;
	}
	else
	{
		delay.tv_sec = n / 1000;
		delay.tv_usec = ( n % 1000) * 1000;
	}
	
	select(0, 0, 0, 0, &delay);

	return (void)0;
}
#endif
