/*
 * dates.h - operations with dates
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

#ifndef _DATES_H

#define _DATES_H

#include <time.h>
    
#define is_leap_year(y) !((y)%400) || ((y)%100 && !((y)%4))

/* Returns number of days in specified month (0 to 11) of specified year. */
extern int month_days(int mon, int year);

/* Increases or decreases tm by the specified number of minutes. */
extern void add_minutes(struct tm *tm, int mins);

/* Returns -1, 0, or 1 if tm1 is less than, equal to, or greater than tm2. Uses
 * minute precision.
 */ 
extern int tmcmp(struct tm *tm1, struct tm *tm2);

/* Parses supplied string, which must be in one of the following formats:
 * "%Y-%m-%d %H:%M", "%Y-%m-%d %H:%M:00", "%Y-%m-%d %H:%M:00:00",
 * "%Y-%m-%d %H", "%Y-%m-%d", "%Y-%m", "%Y". Sets tm to the parsed date.
 * Returns nonzero on error, setting errmsg to a static error message. The
 * return value is EINVAL if supplied string is not a valid date, or ENOMEM on
 * insufficient memory.
 */
extern int parsedatestring(const char *s, struct tm *tm, char **errmsg);

/* Simulates the time.h function gmtim(), except that it reads from
   a long lonng (64bit int) time value (gm_time) instead of the
   standard (32 bit currently) type time_t.
   The result is written in the structure tm
*/
extern void igmtime(long long gm_time, struct tm *tm);

#endif /* _DATES_H */
