/* These are some modified functions from mktime.c, part of
 * GNU C Library.
 * Using long long data type for timestamps storage instead of
 * time_t structure which is 32-bit with 2038 year problem aware.
 * When standard functions will be compliant with 64 bit storage
 * of time_t, then we gonna use the standard functions.
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

#include "mktime_mod.h"

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

long long
ydhms_diffl (long int year1, long int yday1, int hour1, int min1, int sec1,
          int year0, int yday0, int hour0, int min0, int sec0)
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

  /* Compute the desired time in time_t precision.  Overflow might
     occur here.  */
  long long tyear1 = year1;
  long long years = tyear1 - year0;
  long long days = 365 * years + yday1 - yday0 + intervening_leap_days;
  long long hours = 24 * days + hour1 - hour0;
  long long minutes = 60 * hours + min1 - min0;
  long long seconds = 60 * minutes + sec1 - sec0;
  return seconds;
}

