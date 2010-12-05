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
#include <limits.h>
#include "platform.h"
    
#define is_leap_year(y) !((y)%400) || ((y)%100 && !((y)%4))

typedef long long long_time_t;

struct interval {
    long_time_t start_date;
    long_time_t end_date;
};

struct interval_list {
    struct interval *intervals;
    int n;
};

extern int month_days(int mon, int year);
extern void add_minutes(struct tm *tm, int mins);
extern int tmcmp(struct tm *tm1, struct tm *tm2);
extern int parsedatestring(const char *s, struct tm *tm, char **errmsg);
extern void igmtime(long_time_t gm_time, struct tm *tm);
extern long_time_t ydhms_diffl (int year1, int yday1, int hour1, int min1,
    int sec1, int year0, int yday0, int hour0, int min0, int sec0);
extern DLLEXPORT struct interval_list *il_create(void);
extern DLLEXPORT void il_free(struct interval_list *intrvls);
extern DLLEXPORT int il_append(struct interval_list *intrvls,
                                long_time_t start_date, long_time_t end_date);
extern DLLEXPORT int il_delete(struct interval_list *intrvls, int index);
extern DLLEXPORT const long_time_t LONG_TIME_T_MIN;
extern DLLEXPORT const long_time_t LONG_TIME_T_MAX;

#ifndef HAVE_STRPTIME
    char *strptime(const char *buf, const char *fmt, struct tm *tm);
#endif

#endif /* _DATES_H */
