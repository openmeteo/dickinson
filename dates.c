/*
 * dates.c - operations with dates
 *
 * Copyright (c) 2004  Antonios Christofides
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "dates.h"
#include "strings.h"

#ifndef HAVE_STRPTIME

/* The strptime function is usually available on Unixes (man strptime), but it
 * has been included here for native compilation on Windows. The function has
 * been modified, with some functionality, such as anything
 * locale-dependent, removed, in order to make it have fewer dependencies. The
 * remaining functionality is enough for Dickinson's purposes. These
 * modifications have been made by Antonis Christofides of NTUA, November
 * 2010.  The original comment of the file follows.
 */
 
/*	$NetBSD: strptime.c,v 1.35 2009/12/14 20:45:02 matt Exp $	*/

/*-
 * Copyright (c) 1997, 1998, 2005, 2008 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code was contributed to The NetBSD Foundation by Klaus Klein.
 * Heavily optimised by David Laight
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <ctype.h>
#include <locale.h>
#include <string.h>
#include <time.h>
#define u_char unsigned char
#define uint unsigned int
#define uint64_t long long
#define TM_YEAR_BASE 1900
#define __UNCONST(x) ((char *)x)
#include <limits.h>
#define INT64_MAX LLONG_MAX

#ifdef __weak_alias
__weak_alias(strptime,_strptime)
#endif

/*
 * We do not implement alternate representations. However, we always
 * check whether a given modifier is allowed for a certain conversion.
 */
#define ALT_E			0x01
#define ALT_O			0x02
#define	LEGAL_ALT(x)		{ if (alt_format & ~(x)) return NULL; }

/* RFC-822/RFC-2822 */
static const char * const nast[5] = {
       "EST",    "CST",    "MST",    "PST",    "\0\0\0"
};
static const char * const nadt[5] = {
       "EDT",    "CDT",    "MDT",    "PDT",    "\0\0\0"
};

static const u_char *conv_num(const unsigned char *, int *, uint, uint);

char *
strptime(const char *buf, const char *fmt, struct tm *tm)
{
	unsigned char c;
	const unsigned char *bp;
	int alt_format, i, split_year = 0;
	const char *new_fmt;

	bp = (const u_char *)buf;

	while (bp != NULL && (c = *fmt++) != '\0') {
		/* Clear `alternate' modifier prior to new conversion. */
		alt_format = 0;
		i = 0;

		/* Eat up white-space. */
		if (isspace(c)) {
			while (isspace(*bp))
				bp++;
			continue;
		}

		if (c != '%')
			goto literal;


again:		switch (c = *fmt++) {
		case '%':	/* "%%" is converted to "%". */
literal:
			if (c != *bp++)
				return NULL;
			LEGAL_ALT(0);
			continue;

		/*
		 * "Alternative" modifiers. Just set the appropriate flag
		 * and start over again.
		 */
		case 'E':	/* "%E?" alternative conversion modifier. */
			LEGAL_ALT(0);
			alt_format |= ALT_E;
			goto again;

		case 'O':	/* "%O?" alternative conversion modifier. */
			LEGAL_ALT(0);
			alt_format |= ALT_O;
			goto again;

                /* Operations with locale and time zones not supported. */
		case 'c':	/* Date and time, using the locale's format. */
		case 'r':	/* The time in 12-hour clock representation. */
		case 'X':	/* The time, using the locale's format. */
		case 'x':	/* The date, using the locale's format. */
		case 'A':	/* The day of week, using the locale's form. */
		case 'a':
		case 'B':	/* The month, using the locale's form. */
		case 'b':
		case 'h':
		case 'p':	/* The locale's equivalent of AM/PM. */
		case 'Z':       /* Time-zone-related. */
		case 'z':

                        return NULL;

		/*
		 * "Complex" conversion rules, implemented through recursion.
		 */
		case 'D':	/* The date as "%m/%d/%y". */
			new_fmt = "%m/%d/%y";
			LEGAL_ALT(0);
			goto recurse;

		case 'F':	/* The date as "%Y-%m-%d". */
			new_fmt = "%Y-%m-%d";
			LEGAL_ALT(0);
			goto recurse;

		case 'R':	/* The time as "%H:%M". */
			new_fmt = "%H:%M";
			LEGAL_ALT(0);
			goto recurse;

		case 'T':	/* The time as "%H:%M:%S". */
			new_fmt = "%H:%M:%S";
			LEGAL_ALT(0);
			goto recurse;

		recurse:
			bp = (const u_char *)strptime((const char *)bp,
							    new_fmt, tm);
			LEGAL_ALT(ALT_E);
			continue;

		/*
		 * "Elementary" conversion rules.
		 */

		case 'C':	/* The century number. */
			i = 20;
			bp = conv_num(bp, &i, 0, 99);

			i = i * 100 - TM_YEAR_BASE;
			if (split_year)
				i += tm->tm_year % 100;
			split_year = 1;
			tm->tm_year = i;
			LEGAL_ALT(ALT_E);
			continue;

		case 'd':	/* The day of month. */
		case 'e':
			bp = conv_num(bp, &tm->tm_mday, 1, 31);
			LEGAL_ALT(ALT_O);
			continue;

		case 'k':	/* The hour (24-hour clock representation). */
			LEGAL_ALT(0);
			/* FALLTHROUGH */
		case 'H':
			bp = conv_num(bp, &tm->tm_hour, 0, 23);
			LEGAL_ALT(ALT_O);
			continue;

		case 'l':	/* The hour (12-hour clock representation). */
			LEGAL_ALT(0);
			/* FALLTHROUGH */
		case 'I':
			bp = conv_num(bp, &tm->tm_hour, 1, 12);
			if (tm->tm_hour == 12)
				tm->tm_hour = 0;
			LEGAL_ALT(ALT_O);
			continue;

		case 'j':	/* The day of year. */
			i = 1;
			bp = conv_num(bp, &i, 1, 366);
			tm->tm_yday = i - 1;
			LEGAL_ALT(0);
			continue;

		case 'M':	/* The minute. */
			bp = conv_num(bp, &tm->tm_min, 0, 59);
			LEGAL_ALT(ALT_O);
			continue;

		case 'm':	/* The month. */
			i = 1;
			bp = conv_num(bp, &i, 1, 12);
			tm->tm_mon = i - 1;
			LEGAL_ALT(ALT_O);
			continue;

		case 'S':	/* The seconds. */
			bp = conv_num(bp, &tm->tm_sec, 0, 61);
			LEGAL_ALT(ALT_O);
			continue;

#ifndef TIME_MAX
#define TIME_MAX	INT64_MAX
#endif
		case 's':	/* seconds since the epoch */
			{
				time_t sse = 0;
				uint64_t rulim = TIME_MAX;

				if (*bp < '0' || *bp > '9') {
					bp = NULL;
					continue;
				}

				do {
					sse *= 10;
					sse += *bp++ - '0';
					rulim /= 10;
				} while ((sse * 10 <= TIME_MAX) &&
					 rulim && *bp >= '0' && *bp <= '9');

				if (sse < 0 || (uint64_t)sse > TIME_MAX) {
					bp = NULL;
					continue;
				}
                                tm = localtime(&sse);
				if (tm == NULL)
					bp = NULL;
			}
			continue;

		case 'U':	/* The week of year, beginning on sunday. */
		case 'W':	/* The week of year, beginning on monday. */
			/*
			 * XXX This is bogus, as we can not assume any valid
			 * information present in the tm structure at this
			 * point to calculate a real value, so just check the
			 * range for now.
			 */
			 bp = conv_num(bp, &i, 0, 53);
			 LEGAL_ALT(ALT_O);
			 continue;

		case 'w':	/* The day of week, beginning on sunday. */
			bp = conv_num(bp, &tm->tm_wday, 0, 6);
			LEGAL_ALT(ALT_O);
			continue;

		case 'u':	/* The day of week, monday = 1. */
			bp = conv_num(bp, &i, 1, 7);
			tm->tm_wday = i % 7;
			LEGAL_ALT(ALT_O);
			continue;

		case 'g':	/* The year corresponding to the ISO week
				 * number but without the century.
				 */
			bp = conv_num(bp, &i, 0, 99);
			continue;

		case 'G':	/* The year corresponding to the ISO week
				 * number with century.
				 */
			do
				bp++;
			while (isdigit(*bp));
			continue;

		case 'V':	/* The ISO 8601:1988 week number as decimal */
			bp = conv_num(bp, &i, 0, 53);
			continue;

		case 'Y':	/* The year. */
			i = TM_YEAR_BASE;	/* just for data sanity... */
			bp = conv_num(bp, &i, 0, 9999);
			tm->tm_year = i - TM_YEAR_BASE;
			LEGAL_ALT(ALT_E);
			continue;

		case 'y':	/* The year within 100 years of the epoch. */
			/* LEGAL_ALT(ALT_E | ALT_O); */
			bp = conv_num(bp, &i, 0, 99);

			if (split_year)
				/* preserve century */
				i += (tm->tm_year / 100) * 100;
			else {
				split_year = 1;
				if (i <= 68)
					i = i + 2000 - TM_YEAR_BASE;
				else
					i = i + 1900 - TM_YEAR_BASE;
			}
			tm->tm_year = i;
			continue;

		/*
		 * Miscellaneous conversions.
		 */
		case 'n':	/* Any kind of white-space. */
		case 't':
			while (isspace(*bp))
				bp++;
			LEGAL_ALT(0);
			continue;


		default:	/* Unknown/unsupported conversion. */
			return NULL;
		}
	}

	return __UNCONST(bp);
}


static const u_char *
conv_num(const unsigned char *buf, int *dest, uint llim, uint ulim)
{
	uint result = 0;
	unsigned char ch;

	/* The limit also determines the number of valid digits. */
	uint rulim = ulim;

	ch = *buf;
	if (ch < '0' || ch > '9')
		return NULL;

	do {
		result *= 10;
		result += ch - '0';
		rulim /= 10;
		ch = *++buf;
	} while ((result * 10 <= ulim) && rulim && ch >= '0' && ch <= '9');

	if (result < llim || result > ulim)
		return NULL;

	*dest = result;
	return buf;
}

#endif

/* YDMS_DIFFL FUNCTION */

/* This is similar to the ydms_diff function from mktime.c of glibc,
 * modified so that it uses long_time_t instead of time_t. The
 * original comments follow.
*/

/* Convert a `struct tm' to a time_t value.
   Copyright (C) 1993-1999, 2002-2007, 2008 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Paul Eggert <eggert@twinsun.com>.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* Shift A right by B bits portably, by dividing A by 2**B and
   truncating towards minus infinity.  A and B should be free of side
   effects, and B should be in the range 0 <= B <= INT_BITS - 2, where
   INT_BITS is the number of useful bits in an int.  GNU code can
   assume that INT_BITS is at least 32.

   ISO C99 says that A >> B is implementation-defined if A < 0.  Some
   implementations (e.g., UNICOS 9.0 on a Cray Y-MP EL) don't shift
   right in the usual way when A < 0, so SHR falls back on division if
   ordinary A >> B doesn't seem to be the usual signed shift.  */
#define SHR(a, b) \
  (-1 >> 1 == -1  \
   ? (a) >> (b)         \
   : (a) / (1 << (b)) - ((a) % (1 << (b)) < 0))

/* Verify a requirement at compile-time (unlike assert, which is runtime).  */
#define verify(name, assertion) struct name { char a[(assertion) ? 1 : -1]; }

/* The code also assumes that signed integer overflow silently wraps
   around, but this assumption can't be stated without causing a
   diagnostic on some hosts.  */

#define EPOCH_YEAR 1970
#define TM_YEAR_BASE 1900
verify (base_year_is_a_multiple_of_100, TM_YEAR_BASE % 100 == 0);

/* Return an integer value measuring (YEAR1-YDAY1 HOUR1:MIN1:SEC1) -
   (YEAR0-YDAY0 HOUR0:MIN0:SEC0) in seconds, assuming that the clocks
   were not adjusted between the time stamps.

   The YEAR values uses the same numbering as TP->tm_year.  Values
   need not be in the usual range.  However, YEAR1 must not be less
   than 2 * INT_MIN or greater than 2 * INT_MAX.

   The result may overflow.  It is the caller's responsibility to
   detect overflow.  */

static long_time_t ydhms_diffl (long int year1, long int yday1, int hour1,
    int min1, int sec1, int year0, int yday0, int hour0, int min0, int sec0)
{
  verify (C99_integer_division, -1 / 2 == 0);

  /* Compute intervening leap days correctly even if year is negative.
     Take care to avoid integer overflow here.  */
  int a4 = SHR (year1, 2) + SHR (TM_YEAR_BASE, 2) - ! (year1 & 3);
  int b4 = SHR (year0, 2) + SHR (TM_YEAR_BASE, 2) - ! (year0 & 3);
  int a100 = a4 / 25 - (a4 % 25 < 0);
  int b100 = b4 / 25 - (b4 % 25 < 0);
  int a400 = SHR (a100, 2);
  int b400 = SHR (b100, 2);
  int intervening_leap_days = (a4 - b4) - (a100 - b100) + (a400 - b400);

  /* Compute the desired time in long_time_t precision. */
  int tyear1 = year1;
  int years = tyear1 - year0;
  long days = 365 * years + yday1 - yday0 + intervening_leap_days;
  long long hours = 24 * days + hour1 - hour0;
  long long minutes = 60 * hours + min1 - min0;
  long_time_t seconds = 60 * minutes + sec1 - sec0;
  return seconds;
}

/* END YDMS_DIFFL FUNCTION */

static int ydays[][12] = { {0,31,59,90,120,151,181,212,243,273,304,334},
                           {0,31,60,91,121,152,182,213,244,274,305,335}};

int year_days(int mon, int year)
{
    return ydays[is_leap_year(year)][mon];
}

int month_days(int mon, int year)
{
    static int d[][12] = { { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
                           { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 } };
    return d[(mon==1)&&(is_leap_year(year))][mon];
}

void add_minutes(struct tm *tm, int mins)
{
    tm->tm_min += mins;
    while(tm->tm_min >= 60) {
        tm->tm_min -= 60;
        if(++(tm->tm_hour)>23) {
            tm->tm_hour = 0;
            if(++(tm->tm_mday)>month_days(tm->tm_mon, tm->tm_year)) {
                tm->tm_mday = 1;
                if(++(tm->tm_mon)>11) {
                    tm->tm_mon = 0;
                    ++(tm->tm_year);
                }
            }
        }
    }
    while(tm->tm_min < 0) {
        tm->tm_min += 60;
        if (--(tm->tm_hour)<0) {
            tm->tm_hour = 23;
            if(--(tm->tm_mday)<1) {
                if(--(tm->tm_mon)<0) {
                    tm->tm_mon = 11;
                    --(tm->tm_year);
                }
                tm->tm_mday = month_days(tm->tm_mon, tm->tm_year);
            }
        }
    }
}

int tmcmp(struct tm *tm1, struct tm *tm2)
{
    if(tm1->tm_year > tm2->tm_year)
        return 1;
    else if(tm1->tm_year < tm2->tm_year)
        return -1;
    else if(tm1->tm_mon > tm2->tm_mon)
        return 1;
    else if(tm1->tm_mon < tm2->tm_mon)
        return -1;
    else if(tm1->tm_mday > tm2->tm_mday)
        return 1;
    else if(tm1->tm_mday < tm2->tm_mday)
        return -1;
    else if(tm1->tm_hour > tm2->tm_hour)
        return 1;
    else if(tm1->tm_hour < tm2->tm_hour)
        return -1;
    else if(tm1->tm_min > tm2->tm_min)
        return 1;
    else if(tm1->tm_min < tm2->tm_min)
        return -1;
    else
        return 0;
}

/*
 * The following function recomputes tm_yday from the other members. It is used
 * because strptime does not always set those (depends on strptime
 * implementation).
 */
static void fix_tm(struct tm *tm) {
    tm->tm_yday = year_days(tm->tm_mon, tm->tm_year) + tm->tm_mday - 1;
}

int parsedatestring(const char *s, struct tm *tm, char **errmsg)
{
    char *time_designator;
    char *ls = NULL;
    char *acceptedformats[] = { "%Y-%m-%d %H:%M", "%Y-%m-%d %H:%M:00",
        "%Y-%m-%d %H:%M:00:00", "%Y-%m-%d %H", "%Y-%m-%d", "%Y-%m", "%Y", "" };
    char **f;
    char *r;
    int retval = 0;

    ls = strdup(s); if(!ls) goto FAIL;

    /* Replace time designator with space if it's T or t. */
    time_designator = strchr(ls, 'T');
    if (time_designator == NULL) time_designator = strchr(ls, 't');
    if (time_designator != NULL) *time_designator = ' ';

    /* Replace dots with colons (in case dots are used as time sep. */
    /* Then slashes with hyahpens */
    for(r = ls; *r; ++r){
        if(*r == '.')
            *r = ':';
        if(*r == '/')
            *r = '-';
    }

    /* Reset tm. */
    tm->tm_sec = tm->tm_min = tm->tm_hour = tm->tm_mon = 0;
    tm->tm_mday = 1;

    /* Convert string to broken time. */
    for (f = acceptedformats; **f!='\0'; ++f) {
        r = strptime(ls, *f, tm);
        fix_tm(tm);
        if (r!=NULL && *r == '\0') break;
    }
    if (r==NULL || *r != '\0') {
        errno = EINVAL;
        goto FAIL;
    }

END:
    free(ls);
    return retval;

FAIL:
    retval = errno;
    *errmsg = errno==EINVAL ? "Invalid date" : strerror(errno);
    goto END;
}

void igmtime(long_time_t gm_time, struct tm *tm)
{
    int delta_days_1970;
    int aprox_year;
    long_time_t curr_year_t;
    long_time_t actual_delta;
    int year, seconds, month, curmdays; 

    delta_days_1970 = (int)(gm_time / 86400);
    aprox_year = 1970 + (int)(delta_days_1970 * 10000 / 3652425);
    aprox_year--;
    for(;;) {
        curr_year_t = ydhms_diffl(aprox_year-1900,0,0,0,0,70,0,0,0,0);
        if(curr_year_t>gm_time)
            break;
        aprox_year++;
    };
    curr_year_t = ydhms_diffl(--aprox_year-1900,0,0,0,0,70,0,0,0,0);
    year = aprox_year;
    tm->tm_year = year-1900;
    actual_delta = gm_time - curr_year_t;
    tm->tm_yday = (int)(actual_delta / 86400);
    seconds = (int)(actual_delta % 86400);
    tm->tm_isdst = 0;
    tm->tm_hour = seconds / 3600;
    seconds = seconds - tm->tm_hour * 3600;
    tm->tm_min = seconds / 60;
    tm->tm_sec = seconds - tm->tm_min *60;
    month = 11;
    while(tm->tm_yday < (curmdays = year_days(month, year)))
        month--;
    tm->tm_mon = month;
    tm->tm_mday = 1 + tm->tm_yday - curmdays;
    tm->tm_wday = (delta_days_1970 + 4) % 7;
}
