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
#include <math.h>
#include "strings.h"
#include "csv.h"
#include "dates.h"
#include "ts.h"
#include "platform.h"

/* Added the following lines in order to compile in VC++ 2010
   may need cleanup, Stefanos Kozanis 2012-02-16 */
#ifdef WIN32
#include <float.h>
#define isnan(x) (x!=x)
#define NAN 0x7ff8000000000000
#define fmax(a,b) (a>b?a:b)
#define fmin(a,b) (a<b?a:b)
#define snprintf _snprintf
#endif


/* Makes sure that the data block allocated for the timeseries data is large
 * enough to hold the specified number of records. If not, it reallocs it.
 * Returns nonzero on insufficient memory.
 */
#define CHUNKSIZE 262144
static int check_block_size(struct timeseries *ts, int nrecords)
{
    void *p;
    size_t s;

    while(ts->memblocksize < nrecords*sizeof(struct ts_record)) {
        s = ts->memblocksize + CHUNKSIZE;
        p = realloc(ts->data, s);
        if(!p) return errno;
        ts->data = p;
        ts->memblocksize = s;
    }
    return 0;
}

DLLEXPORT int ts_set_item(struct timeseries *ts, int index, 
    int null, double value, const char *flags, char **errstr)
{
    struct ts_record *r;
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

DLLEXPORT int ts_append_record(struct timeseries *ts, long_time_t timestamp,
    int null, double value, const char *flags, int *recindex, char **errstr)
{
    struct ts_record *r;
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

DLLEXPORT int ts_insert_record(struct timeseries *ts, long_time_t timestamp,
    int null, double value, const char *flags, int allow_existing,
    int *recindex, char **errstr)
{
    struct ts_record *r;
    char *s;
    int i;
    int next_item;

    next_item = ts_get_next_i(ts, timestamp);
    if(next_item==-1)
        return ts_append_record(ts, timestamp, null, value, flags, 
                 recindex, errstr);

    if(ts->data[next_item].timestamp==timestamp){
        if(!allow_existing) {
            *errstr = "Record already exists";
            return EINVAL;
        } else {
            return ts_set_item(ts, next_item, null, value, flags, errstr);
        }
    }

    i = check_block_size(ts, ts->nrecords+1); if(i) goto GENFAIL;
    s = strdup(flags); if(!s) goto GENFAIL;

    memmove(ts->data+next_item+1, ts->data+next_item,
            (ts->nrecords - next_item)*sizeof(struct ts_record));

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

DLLEXPORT struct ts_record *ts_get_next(const struct timeseries *ts,
                                                        long_time_t timestamp)
{
    struct ts_record *low, *high, *mid;

    if(!ts->nrecords)
        return NULL;
    low = ts->data;
    high = ts->data + (ts->nrecords - 1);
    while(low<=high) {
        mid = low + (high-low)/2;
        if(timestamp < mid->timestamp)
            high = mid-1;
        else if(timestamp > mid->timestamp)
            low = mid+1;
        else {
            low = mid;
            break;
        }
    };
    if(low>=ts->data + ts->nrecords)
        return NULL;
    else
        return low;
}

DLLEXPORT int ts_get_next_i(const struct timeseries *ts, long_time_t timestamp)
{
    struct ts_record *r = ts_get_next(ts, timestamp);
    if(r==NULL)
        return -1;
    return r - ts->data;
}

DLLEXPORT struct ts_record *ts_get_prev(const struct timeseries *ts,
                                                        long_time_t timestamp)
{
    struct ts_record *r;
    if(ts->nrecords==0 || timestamp < ts->data->timestamp)
        return NULL;
    if((r = ts_get_next(ts, timestamp))==NULL)
        r = ts->data + ts->nrecords - 1;
    else if(timestamp!=r->timestamp)
        r--;
    return r;
}

DLLEXPORT int ts_get_prev_i(const struct timeseries *ts, long_time_t timestamp)
{
    struct ts_record *r = ts_get_prev(ts, timestamp);
    if(r==NULL)
        return -1;
    return r - ts->data;
}

DLLEXPORT struct ts_record *ts_get(const struct timeseries *ts,
                                                        long_time_t timestamp)
{
    struct ts_record *r = ts_get_next(ts, timestamp);
    return (r && r->timestamp==timestamp) ? r : NULL;
}

DLLEXPORT int ts_get_i(const struct timeseries *ts, long_time_t timestamp)
{
    struct ts_record *r = ts_get(ts, timestamp);
    return r ? r - ts->data : -1;
}

DLLEXPORT int ts_delete_item(struct timeseries *ts, int index)
{
    struct ts_record *r = ts->data + index;
    return ts_delete_records(ts, r, r) ? index : -1;
}

DLLEXPORT struct ts_record *ts_delete_records(struct timeseries *ts,
                                    struct ts_record *r1, struct ts_record *r2)
{
    struct ts_record *start = ts->data;
    struct ts_record *end   = ts->data + ts->nrecords - 1;
    struct ts_record *r;
    if(!ts->nrecords || r1<start || r2<start || r1>end || r2>end || r2<r1)
        return NULL;
    for(r = r1; r <= r2; ++r) {
        free(r->flags);
        r->flags = NULL;
    }
    memmove(r1, r2+1, (end-r2)*sizeof(struct ts_record));
    ts->nrecords -= r2-r1+1;
    return r1;
}

DLLEXPORT int ts_delete_record(struct timeseries *ts, long_time_t tm)
{
    int i;

    if(!ts->nrecords) return -1;
    if((i = ts_get_i(ts, tm))<0) return -1;
    return ts_delete_item(ts, i);
}

DLLEXPORT struct timeseries *ts_create(void)
{
    struct timeseries *ts;

    if(!(ts = (struct timeseries *) malloc(sizeof(struct timeseries))))
        return NULL;
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

DLLEXPORT int ts_length(const struct timeseries *ts)
{
    return ts->nrecords;
}

DLLEXPORT void ts_clear(struct timeseries *ts)
{
    struct ts_record *r;

    for(r = ts->data; r < ts->data + ts->nrecords; ++r){
        free(r->flags);
        r->flags = NULL;
    }
    ts->nrecords = 0;
}

DLLEXPORT struct ts_record ts_get_item(struct timeseries *ts, int index)
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
    retval = ts_insert_record(ts, timestamp, null, value, flags, 1, &index,
                                                                    errstr);

END:
    return retval;

INVSYNTAX:
    retval = EINVAL;
    *errstr = "Invalid syntax";
    goto END;
}

DLLEXPORT int ts_readfile(FILE* fp, struct timeseries *ts, int *errline, char **errstr)
{
    char buf[256];
    int lerrline;
    int retval;

    lerrline = 0;
    while(fgets(buf, 256, fp)!=NULL) {
        ++lerrline;
        
        /* Check for file or line-too-long errors. */
        if(ferror(fp)) goto GENFAIL;
        buf[255] = '\0';
        if (strchr(buf, '\n')==NULL) {
            *errstr = "Line too long or unterminated";
            retval = EOVERFLOW;
            goto END;
        }

        retval = ts_readline(buf, ts, errstr); 
        if(retval) goto END;
    }

    retval = 0;

END:
    if(retval) *errline = lerrline;
    return retval;

GENFAIL:
    retval = errno;
    *errstr = strerror(errno);
    goto END;

}

DLLEXPORT int ts_readfromstring(char *string, struct timeseries *ts, int *errline, char **errstr)
{
    char *buf;
    char *buf_s, *buf_e;
    int lerrline;
    int retval;

    buf = (char*) malloc(256*sizeof(char));
    lerrline = 0;
    buf_s=buf;
    buf_e = buf_s + 256;
    while(1) {
        
        /* Check for line-too-long errors. */
        if(buf==buf_e) {
            *errstr = "Line too long or unterminated";
            retval = EOVERFLOW;
            goto END;
        }

        if(*string=='\n'||*string=='\r'||*string=='\0') {
            if(*string=='\0')
                if(buf==buf_s)
                    break;
            *buf='\0';
            buf=buf_s;
            ++lerrline;
            retval = ts_readline(buf, ts, errstr); 
            if(retval) goto END;
            
            if(*string=='\0') break;

            if(*string=='\n') {
                if(*(++string)=='\r')
                    string++;
            }
            else if(*string=='\r') {
                if(*(++string)=='\n')
                    string++;
            }
            continue;
        }
        *(buf++) = *(string++);
    }

    retval = 0;

END:
    buf=buf_s;
    free(buf);
    if(retval) *errline = lerrline;
    return retval;

}

DLLEXPORT int ts_merge(struct timeseries *ts1, struct timeseries *ts2,
                            char **errstr)
{
    int i, i1, i2;
    struct ts_record *r1;
    struct ts_record r2;
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
    if((i1 = ts_get_next_i(ts1, ts2->data[0].timestamp))<0)
        i1 = ts1->nrecords;

    /* Find record i2 before which last ts2 record will be inserted. */
    if((i2 = ts_get_next_i(ts1, ts2->data[ts2->nrecords-1].timestamp))<0)
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
            (ts1->nrecords - i1) * sizeof(struct ts_record));
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

DLLEXPORT int ts_merge_anyway(struct timeseries *ts1,
                                const struct timeseries *ts2, char **errstr)
{
    struct ts_record *r;
    int result, dummy;

    for(r=ts2->data; r < ts2->data + ts2->nrecords; ++r)
        if((result = ts_insert_record(ts1, r->timestamp, r->null, r->value,
                                                r->flags, 1, &dummy, errstr)))
            return result;
    return 0;
}

DLLEXPORT int ts_writeline(struct ts_record *r, int precision, char *str,
                                                            size_t max_length)
{
    int remaining_length = max_length;
    int i;
    struct tm tm_timestamp;
    char fmtstring[10];

    /* Date */
    igmtime(r->timestamp, &tm_timestamp);
    i = strftime(str, remaining_length, "%Y-%m-%d %H:%M,", &tm_timestamp);
    if(i==0 || i==remaining_length)
        return 0;
    str += i;
    remaining_length -= i;

    /* Value */
    strcpy(fmtstring, "%G");
    if(precision != -9999) {
        if(precision<0) precision = 0;
        if(precision>17) precision = 17;
        snprintf(fmtstring, 10, "%%.%df", precision);
    }
    i = r->null ? 0 : snprintf(str, remaining_length, fmtstring, r->value);
    if(i>=remaining_length)
        return 0;
    str += i;
    remaining_length -= i;
    /* Comma */
    if(remaining_length<2)
        return 0;
    *(str++) = ',';
    --remaining_length;

    /* Flags */
    if((i = strlen(r->flags)) >= remaining_length)
        return 0;
    strcpy(str, r->flags);
    str += i;
    remaining_length -= i;

    /* Line terminator */
    if(remaining_length<3)
        return 0;
    strcpy(str, "\r\n");
    str += 2;
    remaining_length -= 2;

    return max_length - remaining_length;
}

DLLEXPORT char *ts_write(struct timeseries *ts, int precision,
                long_time_t start_date, long_time_t end_date, char **errstr)
{
    struct ts_record *r = ts_get_next(ts, start_date);
    struct ts_record *end = ts_get_prev(ts, end_date);
    size_t blocksize = 0;
    char *result = NULL;
    char *p;
    *errstr = NULL;
    if(!r || !end || r>end)
        return result;
    if((result = malloc(blocksize += CHUNKSIZE))==NULL)
        goto ERROR;
    p = result;
    while(r<=end) {
        int i = ts_writeline(r, precision, p, blocksize-(p-result));
        if(!i) {
            char *nresult = realloc(result, blocksize += CHUNKSIZE);
            if(!nresult)
                goto ERROR;
            p = nresult + (p - result);
            result = nresult;
            continue;
        }
        p += i;
        ++r;
        continue;
    }
    return result;

ERROR:
    free(result);
    *errstr = strerror(errno);
    return NULL;
}

DLLEXPORT struct timeseries_list *tsl_create(void)
{
    struct timeseries_list *tsl;

    if(!(tsl = malloc(sizeof(struct timeseries_list))))
        return NULL;
    tsl->n = 0;
    tsl->ts = NULL;
    return tsl;
}

DLLEXPORT void tsl_free(struct timeseries_list *tsl)
{
    if(!tsl) return;
    free(tsl->ts);
    tsl->ts=NULL;
    free(tsl);
}

DLLEXPORT int tsl_append(struct timeseries_list *tsl, struct timeseries *t)
{
    struct timeseries **p = realloc(tsl->ts,
									(tsl->n + 1)*sizeof(struct timeseries *));
    if(p==NULL) return errno;
    tsl->ts = p;
    tsl->ts[(tsl->n)++] = t;
    return 0;
}

DLLEXPORT int tsl_delete(struct timeseries_list *tsl, int index)
{
    if(index >= tsl->n || index<0)
        return EINVAL;
    memmove(tsl->ts + index, tsl->ts + (index + 1), 
                            (tsl->n - index - 1)*sizeof(struct timeseries));
    tsl->ts = realloc(tsl->ts, (--(tsl->n))*sizeof(struct timeseries));
    return 0;
}

DLLEXPORT double ts_min(struct timeseries *ts, long_time_t start_date,
                                                        long_time_t end_date)
{
    double result = NAN;
    struct ts_record *r = ts_get_next(ts, start_date);
    struct ts_record *end = ts_get_prev(ts, end_date);
    if(!r || !end)
        return result;
    while(r<=end) {
        if(!(r->null))
            result = isnan(result) ? r->value : fmin(result, r->value);
        ++r;
    }
    return result;
}

DLLEXPORT double ts_max(struct timeseries *ts, long_time_t start_date,
                                                        long_time_t end_date)
{
    double result = NAN;
    struct ts_record *r = ts_get_next(ts, start_date);
    struct ts_record *end = ts_get_prev(ts, end_date);
    if(!r || !end)
        return result;
    while(r<=end) {
        if(!(r->null))
            result = isnan(result) ? r->value : fmax(result, r->value);
        ++r;
    }
    return result;
}

DLLEXPORT double ts_average(struct timeseries *ts, long_time_t start_date,
                                                        long_time_t end_date)
{
    double sum = 0.0;
    struct ts_record *r = ts_get_next(ts, start_date);
    struct ts_record *end = ts_get_prev(ts, end_date);
    int divider = 0;
    if(!r || !end)
        return NAN;
    while(r<=end) {
        if(!(r->null)) {
            sum += r->value;
            ++divider;
        }
        ++r;
    }
    return divider ? sum/divider : NAN;
}

DLLEXPORT double ts_sum(struct timeseries *ts, long_time_t start_date,
                                                        long_time_t end_date)
{
    double result = NAN;
    struct ts_record *r = ts_get_next(ts, start_date);
    struct ts_record *end = ts_get_prev(ts, end_date);
    if(!r || !end)
        return NAN;
    while(r<=end) {
        if(!(r->null))
            result = isnan(result) ? r->value : result + r->value;
        ++r;
    }
    return result;
}

/* ts_identify_events */

/* The function uses state-transition. The state data are in struct state_data.
 * Each state (e.g. start, end, in_event) is managed by one function, (e.g.
 * tsie_start, tsie_end, tsie_in_event). The state function (a pointer to it)
 * is used as the state symbol. The state functions return nothing, but they
 * change the state data structure as needed, in which they also set the new
 * state. struct state_data contains mostly the arguments supplied to
 * ts_identify_events, and a few more members that are commented below.
 */

struct state_data {
    struct timeseries_list *ts;
    struct timeseries *all_timestamps; /* All timestamps of all ts merged. */
    struct ts_record *current_record;  /* Pointer in all_timestamps. */
    void (*state)(struct state_data *);/* The current state. */
    int result;                        /* Stays at 0 until finding error. */
    struct interval range;             
    int reverse;
    double start_threshold, end_threshold;
    int ntimeseries_start_threshold, ntimeseries_end_threshold;
    long_time_t time_separator;
    struct interval_list *events;
    char **errstr;
};

static int num_of_timeseries_crossing_threshold(struct state_data *sd,
                                                double threshold)
{
    struct timeseries **tp = sd->ts->ts;
    int result = 0;
    int sign = sd->reverse ? -1 : 1;
    for(tp = sd->ts->ts; tp < sd->ts->ts + sd->ts->n; ++tp) {
        struct ts_record *r = ts_get(*tp, sd->current_record->timestamp);
        if(r && !r->null && sign*r->value > sign*threshold)
            ++result;
    }
    return result;
}

static void tsie_end(struct state_data *sd);
static void tsie_not_in_event(struct state_data *sd);
static void tsie_start_event(struct state_data *sd);
static void tsie_in_event(struct state_data *sd);
static void tsie_maybe_end_of_event(struct state_data *sd);

static void tsie_start(struct state_data *sd)
{
    struct timeseries *tmstmps;
    struct timeseries **tp;
    struct ts_record *r1;
    struct ts_record *r2;
    struct ts_record *end_record;

    sd->result = 0; /* This will always stay at zero until we find an error. */

    /* all_timestamps is a dummy time series that is used to hold all the time
     * stamps of all the time series.
     */
    if((sd->all_timestamps = ts_create())==NULL) {
        *(sd->errstr) = strerror(errno);
        sd->result = errno;
        sd->state = tsie_end;
        return;
    }
    tmstmps = sd->all_timestamps;
    for(tp = sd->ts->ts; tp < sd->ts->ts + sd->ts->n; ++tp)
        if((sd->result = ts_merge_anyway(tmstmps, *tp, sd->errstr))) {
            sd->state = tsie_end;
            return;
        }
    r1 = ts_get_next(tmstmps, sd->range.start_date);
    r2 = ts_get_prev(tmstmps, sd->range.end_date);
    end_record = tmstmps->data + (tmstmps->nrecords - 1);
    if(r2<end_record)
		if(ts_delete_records(tmstmps, r2+1, end_record)==NULL)
			goto INTERN_ERROR;
    if(r1>tmstmps->data)
    	if(ts_delete_records(tmstmps, tmstmps->data, r1-1)==NULL)
    		goto INTERN_ERROR;
    if(!tmstmps->nrecords) {
        sd->state = tsie_end;
        return;
    }
    sd->current_record = tmstmps->data;
    sd->state = tsie_not_in_event;
    return;

INTERN_ERROR:
    *(sd->errstr) = "Internal error in tsie_start";
    sd->result = 255;
    sd->state = tsie_end;
}

static void tsie_not_in_event(struct state_data *sd)
{
    struct timeseries *tmstmps = sd->all_timestamps;
    while(sd->current_record < tmstmps->data + tmstmps->nrecords) {
        int i = num_of_timeseries_crossing_threshold(sd, sd->start_threshold);
        if(i >= sd->ntimeseries_start_threshold) {
            sd->state = tsie_start_event;
            return;
        }
        ++(sd->current_record);
    }
    sd->state = tsie_end;
}

static void tsie_start_event(struct state_data *sd)
{
    long_time_t t = sd->current_record->timestamp;
    int err = il_append(sd->events, t, t);
    if(err) {
        sd->result = errno;
        *(sd->errstr) = strerror(errno);
        sd->state = tsie_end;
        return;
    }
    sd->state = tsie_in_event;
}

static void tsie_in_event(struct state_data *sd)
{
    struct interval *current_event = sd->events->intervals + sd->events->n - 1;
    struct timeseries *tmstmps = sd->all_timestamps;
    while(sd->current_record < tmstmps->data + tmstmps->nrecords) {
        int i = num_of_timeseries_crossing_threshold(sd, sd->end_threshold);
        if(i < sd->ntimeseries_end_threshold) {
            sd->state = tsie_maybe_end_of_event;
            return;
        }
        current_event->end_date = sd->current_record->timestamp;
        ++(sd->current_record);
    }
    sd->state = tsie_end;
}

static void tsie_maybe_end_of_event(struct state_data *sd)
{
    struct interval *current_event = sd->events->intervals + sd->events->n - 1;
    struct timeseries *tmstmps = sd->all_timestamps;
    while(sd->current_record < tmstmps->data + tmstmps->nrecords) {
        int i = num_of_timeseries_crossing_threshold(sd, sd->end_threshold);
        if(i >= sd->ntimeseries_end_threshold) {
            sd->state = tsie_in_event;
            return;
        }
        if((++(sd->current_record))->timestamp - current_event->end_date >=
                                                        sd->time_separator) {
            sd->state = tsie_not_in_event;
            return;
        }
    }
    sd->state = tsie_end;
}

static void tsie_end(struct state_data *sd)
{
    ts_free(sd->all_timestamps);
    sd->state = NULL;
}

DLLEXPORT int ts_identify_events(struct timeseries_list *ts, 
    struct interval range, int reverse, double start_threshold,
    double end_threshold, int ntimeseries_start_threshold,
    int ntimeseries_end_threshold, long_time_t time_separator,
    struct interval_list *events, char **errstr)
{
    struct state_data state_data;

    state_data.all_timestamps = ts_create();
    state_data.ts = ts;
    state_data.range = range;
    state_data.reverse = reverse;
    state_data.start_threshold = start_threshold;
    state_data.end_threshold = end_threshold;
    state_data.ntimeseries_start_threshold = ntimeseries_start_threshold;
    state_data.ntimeseries_end_threshold = ntimeseries_end_threshold;
    state_data.time_separator = time_separator;
    state_data.events = events;
    state_data.errstr = errstr;

    state_data.state = tsie_start;
    while(state_data.state)
        (*(state_data.state))(&state_data);
    return state_data.result;
}
    
/* end ts_identify_events */
