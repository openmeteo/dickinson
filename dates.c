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
#include "mktime_mod.h"
#include "strings.h"
extern char *strptime(const char *buf, const char *fmt, struct tm *tm);

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

void igmtime(long long gm_time, struct tm *tm)
{
    long long delta_days_1970;
    long long aprox_year;
    long long curr_year_t;
    long long actual_delta;
    int year, seconds, month, curmdays; 

    delta_days_1970 = gm_time / 86400;
    aprox_year = 1970 + delta_days_1970 * 10000 / 3652425;
    aprox_year--;
    while(1){
        curr_year_t = ydhms_diffl(aprox_year-1900,0,0,0,0,70,0,0,0,0);
        if(curr_year_t>gm_time)
            break;
        aprox_year++;
    };
    curr_year_t = ydhms_diffl(--aprox_year-1900,0,0,0,0,70,0,0,0,0);
    year = aprox_year;
    tm->tm_year = year-1900;
    actual_delta = gm_time - curr_year_t;
    tm->tm_yday = actual_delta / 86400;
    seconds = actual_delta % 86400;
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
