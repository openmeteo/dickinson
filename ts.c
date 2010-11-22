/*
 * openmeteo.org
 * dickinson library
 * ts.c - time series operations
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

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "strings.h"
#include "csv.h"
#include "dates.h"
#include "ts.h"
#include "platform.h"

/* Makes sure that the data block allocated for the timeseries data is large
 * enough to hold the specified number of records. If not, it reallocs it.
 * Returns nonzero on insufficient memory.
 */
#define CHUNKSIZE 262144
static int check_block_size(struct timeseries *ts, int nrecords)
{
    void *p;
    size_t s;

    while(ts->memblocksize < nrecords*sizeof(struct record)) {
        s = ts->memblocksize + CHUNKSIZE;
        p = realloc(ts->data, s);
        if(!p) return errno;
        ts->data = p;
        ts->memblocksize = s;
    }
    return 0;
}

DLLEXPORT int set_item(struct timeseries *ts, int index, 
    int null, double value, const char *flags, char **errstr)
{
    struct record *r;
    char *s;

    if(index<0 || index>=ts_length(ts)) {
        *errstr = "Invalid record";
        return EINVAL;
    }
    r = &ts->data[index];
    free(r->flags);
    r->flags = NULL;
    s = strdup(flags); if(!s) goto GENFAIL;
    r->null = null;
    r->value = value;
    r->flags = s;
    return 0;

GENFAIL:
    *errstr = strerror(errno);
    return errno;
}

DLLEXPORT int append_record(struct timeseries *ts, long_time_t timestamp,
    int null, double value, const char *flags, int *recindex, char **errstr)
{
    struct record *r;
    char *s;
    int i;

    i = check_block_size(ts, ts->nrecords+1); if(i) goto GENFAIL;
    r = ts->nrecords>0 ? ts->data + ts->nrecords - 1 : NULL;
    if(r!=NULL && timestamp <= r->timestamp) {
        *errstr = "Record out of order";
        return EINVAL;
    }
    s = strdup(flags); if(!s) goto GENFAIL;
    r = ts->data + ts->nrecords++;
    *recindex = ts->nrecords-1;
    r->timestamp = timestamp;
    r->null = null;
    r->value = value;
    r->flags = s;
    return 0;

GENFAIL:
    *errstr = strerror(errno);
    return errno;
}

DLLEXPORT int insert_record(struct timeseries *ts, long_time_t timestamp, int null,
    double value, const char *flags, int *recindex, char **errstr)
{
    struct record *r;
    char *s;
    int i;
    int next_item;

    next_item = get_next(ts, timestamp);
    if(next_item==-1)
        return append_record(ts, timestamp, null, value, flags, 
                 recindex, errstr);

    if(ts->data[next_item].timestamp==timestamp){
        *errstr = "Record already exist";
        return EINVAL;
    }

    i = check_block_size(ts, ts->nrecords+1); if(i) goto GENFAIL;
    s = strdup(flags); if(!s) goto GENFAIL;

    memmove(ts->data+next_item+1, ts->data+next_item,
            (ts->nrecords - next_item)*sizeof(struct record));

    ts->nrecords++;
    r = ts->data + next_item;
    *recindex = next_item;
    r->timestamp = timestamp;
    r->null = null;
    r->value = value;
    r->flags = s;
    return 0;

GENFAIL:
    *errstr = strerror(errno);
    return errno;
}

DLLEXPORT int get_next(struct timeseries *ts, long_time_t tm)
{
    int len, low, high, mid;
    long_time_t diff;

    len = ts_length(ts);
    if(len==0)
        return -1;
    low = 0;
    high = len-1;
    while(low<=high)
    {
        mid = (low+high)/2;
        diff = tm - ts->data[mid].timestamp;
        if(diff<0)
            high = mid-1;
        else if(diff>0)
            low = mid+1;
        else
        {
            low = mid;
            break;
        }
    };
    if(low>=len)
        return -1;
    else
        return low;
}

DLLEXPORT int get_prev(struct timeseries *ts, long_time_t tm)
{
    int i, len;
    len = ts_length(ts);
    if(len==0 || tm < ts->data[0].timestamp)
        return -1;
    i = get_next(ts, tm);
    if(i==-1)
        i = len-1;
    else if(tm!=ts->data[i].timestamp)
        i--;
    return i;
}

DLLEXPORT int index_of(struct timeseries *ts, long_time_t tm)
{
    int i;

    i = get_next(ts, tm);
    if(i>=0 && ts->data[i].timestamp==tm)
        return i;
    else
        return -1;
}

DLLEXPORT int delete_item(struct timeseries *ts, int index){
    if(!ts->nrecords) return -1;
    if(index>=ts->nrecords) return -1;
    free(ts->data[index].flags);
    ts->data[index].flags=NULL;
    if(index<ts->nrecords-1)
        memmove(ts->data+index, ts->data+index+1,
            (ts->nrecords - index -1)*sizeof(struct record));
    ts->nrecords -= 1;
    return index;
}

DLLEXPORT int delete_record(struct timeseries *ts, long_time_t tm){
    int i;

    if(!ts->nrecords) return -1;
    if((i = index_of(ts, tm))<0) return -1;
    return delete_item(ts, i);
}

DLLEXPORT void * ts_create(void)
{
    struct timeseries *ts;

    if(!(ts = (struct timeseries *) malloc(sizeof(struct timeseries))))
        return 0;
    ts->nrecords = 0;
    ts->data = NULL;
    ts->memblocksize = 0;
    return ts;
}

DLLEXPORT void ts_free(struct timeseries *ts)
{
    ts_clear(ts);
    free(ts->data);
    ts->data=NULL;
    free(ts);
}

DLLEXPORT int ts_length(struct timeseries *ts)
{
    return ts->nrecords;
}

DLLEXPORT void ts_clear(struct timeseries *ts)
{
    int i;

    for(i=0;i<ts->nrecords;++i){
        free(ts->data[i].flags);
        ts->data[i].flags = NULL;
    }
    ts->nrecords = 0;
}

DLLEXPORT struct record get_item(struct timeseries *ts, int index)
{
    return ts->data[index];
}

DLLEXPORT int ts_readline(char *line, struct timeseries *ts, char **errstr)
{
    char *b;
    int null;
    char *p, *flags, *q;
    struct tm tm;
    double value = 0.0;
    int retval;
    long_time_t timestamp;
    int index;
    
    /* Parse and read line fields. */
    b = line;
    p = csvtok(&b);                                  if(!p) goto INVSYNTAX;
    retval = parsedatestring(strip(p), &tm, errstr); if(retval) goto END;
    p = csvtok(&b);                                  if(!p) goto INVSYNTAX;
    timestamp = ydhms_diffl(tm.tm_year, tm.tm_yday, tm.tm_hour, 
        tm.tm_min, tm.tm_sec, 70, 0, 0, 0, 0);
    null = (*(strip(p))=='\0');
    if(!null) {
        value = strtod(p, &q);
        if(*q) {
            retval = EINVAL;
            *errstr = "Invalid floating point value";
            goto END;
        }
    }
    flags = (p=csvtok(&b)) ? strip(p) : "";
    /* Nothing must remain */
    if(p && csvtok(&b)) goto INVSYNTAX;
    
    /* Add record to array. */
    index = index_of(ts, timestamp);
    if(index<0)
        retval = insert_record(ts, timestamp, null, value, flags, &index, errstr);
    else
        retval = set_item(ts, index, null, value, flags, errstr);

    if(retval) goto END;

    retval = 0;

END:
    return retval;

INVSYNTAX:
    retval = EINVAL;
    *errstr = "Invalid syntax";
    goto END;
}

DLLEXPORT int merge(struct timeseries *ts1, struct timeseries *ts2, char **errstr)
{
    int i, i1, i2;
    struct record *r1;
    struct record r2;
    char *s;

    /* Nothing to do if ts2 empty. */
    if(!ts2->nrecords)
        return 0;

    /* Special case: ts1 empty. */
    if(!ts1->nrecords) {
        /* Make ts1 be the same as ts2. */
        if(check_block_size(ts1, ts2->nrecords)) goto GENFAIL;
        ts1->nrecords = ts2->nrecords;
        for(i=0;i<ts1->nrecords;++i)
        {
            r1 = &ts1->data[i];
            r2 = ts2->data[i];
            r1->timestamp = r2.timestamp;
            r1->null = r2.null;
            r1->value = r2.value;
            s = strdup(r2.flags); if(!s) goto GENFAIL;
            r1->flags = s;
        }
        return 0;
    }

    /* Find record i1 before which first ts2 record will be inserted. */
    if((i1 = get_next(ts1, ts2->data[0].timestamp))<0)
        i1 = ts1->nrecords;

    /* Find record i2 before which last ts2 record will be inserted. */
    if((i2 = get_next(ts1, ts2->data[ts2->nrecords-1].timestamp))<0)
        i2 = ts1->nrecords;

    /* All ts2 should go in the same place. */
    if(i1 != i2) {
        *errstr = "No record intermixing allowed when merging timeseries";
        return EINVAL;
    }

    /* No overwriting allowed either. */
    if(i1<ts1->nrecords &&
    ((ts1->data[i1].timestamp==ts2->data[0].timestamp) ||
    (ts1->data[i1].timestamp==ts2->data[ts2->nrecords-1].timestamp)))
    {
        *errstr = "No record overwriting allowed when merging timeseries";
        return EINVAL;
    }

    /* OK, proceed with the merging. */
    if(check_block_size(ts1, ts1->nrecords + ts2->nrecords)) goto GENFAIL;
    memmove(ts1->data + i1 + ts2->nrecords, ts1->data + i1,
            (ts1->nrecords - i1) * sizeof(struct record));
    for(i=i1;i<i1+ts2->nrecords;++i)
    {
        r1 = &ts1->data[i];
        r2 = ts2->data[i-i1];
        r1->timestamp = r2.timestamp;
        r1->null = r2.null;
        r1->value = r2.value;
        s = strdup(r2.flags); if(!s) goto GENFAIL;
        r1->flags = s;
    }
    ts1->nrecords += ts2->nrecords;

    return 0;

GENFAIL:
    *errstr = strerror(errno);
    return errno;
}

DLLEXPORT int ts_writeline(char **line, struct timeseries *ts, int index, int precision,
                                                                char **errstr)
{
    int retval;
    struct tm timestamp;
    char datestring[40], valuestring[40], fmtstring[10];
    static char outstr[256];
    struct record *r = ts->data+index;

    strcpy(fmtstring, "%G");
    if(precision != -9999) {
        if(precision<0)
            precision = 0;
        if(precision>17) {
            retval = ERANGE;
            *errstr = "ts_writefile: precision may not be greater than 17";
            goto END;
        }
        sprintf(fmtstring, "%%.%df", precision);
    }
    igmtime(r->timestamp, &timestamp);
    if(!strftime(datestring,40,"%Y-%m-%d %H:%M",&timestamp)) {
        retval = ERANGE;
        *errstr = "Internal error in ts_writefile (1)";
        goto END;
    }
    if(r->null)
        *valuestring = '\0';
    else if(sprintf(valuestring, fmtstring, r->value) >= 40) {
        retval = ERANGE;
        *errstr = "Internal error in ts_writefile (2)";
        goto END;
    }
    if(strlen(datestring)+strlen(valuestring)+strlen(r->flags)+4 > 255){
        retval = EINVAL;
        *errstr = "ts_writefile: line too long (more than 255 characters)";
        goto END;
    }
    if(sprintf(outstr,"%s,%s,%s\r\n",datestring, valuestring, r->flags) < 0) {
        retval = EINVAL;
        *errstr = "ts_writefile: sprintf error (make sure flags are ASCII)";
        goto END;
    }
    *line = outstr;
    retval = 0;

END:
    return retval;
}

DLLEXPORT int ts_identify_events(const struct timeseries **ts, int ntimeseries,
    long_time_t start_date, long_time_t end_date, int reverse,
    double start_threshold, double end_threshold,
    int ntimeseries_start_threshold, int ntimeseries_end_threshold,
    long_time_t time_separator,
    long_time_t *start_dates, long_time_t *end_dates,
    int *nevents, char **errstr)
{
}
