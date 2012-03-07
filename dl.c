/*
 * openmeteo.org
 * dickinson library
 * dl.c - date time lists
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
#include "dates.h"
#include "dl.h"
#include "platform.h"

/* Makes sure that the data block allocated for the timeseries data is large
 * enough to hold the specified number of records. If not, it reallocs it.
 * Returns nonzero on insufficient memory.
 */
#define CHUNKSIZE 262144
static int check_block_size(struct datetimelist *dl, int nrecords)
{
    void *p;
    size_t s;

    while(dl->memblocksize < nrecords*sizeof(long_time_t )) {
        s = dl->memblocksize + CHUNKSIZE;
        p = realloc(dl->data, s);
        if(!p) return errno;
        dl->data = p;
        dl->memblocksize = s;
    }
    return 0;
}

DLLEXPORT int dl_append_record(struct datetimelist *dl, long_time_t timestamp,
    int *recindex, char **errstr)
{
    long_time_t *r;
    int i;

    i = check_block_size(dl, dl->nrecords+1); if(i) goto GENFAIL;
    r = dl->nrecords>0 ? dl->data + dl->nrecords - 1 : NULL;
    if(r!=NULL && timestamp <= *r) {
        *errstr = "Record out of order";
        return EINVAL;
    }
    r = dl->data + dl->nrecords++;
    *recindex = dl->nrecords-1;
    *r = timestamp;
    return 0;

GENFAIL:
    *errstr = strerror(errno);
    return errno;
}

DLLEXPORT int dl_insert_record(struct datetimelist *dl, long_time_t timestamp,
    int *recindex, char **errstr)
{
    long_time_t *r;
    int i;
    int next_item;

    next_item = dl_get_next_i(dl, timestamp);
    if(next_item==-1)
        return dl_append_record(dl, timestamp, recindex, errstr);

    if(dl->data[next_item]==timestamp){
        *errstr = "Record already exists";
        return EINVAL;
    }

    i = check_block_size(dl, dl->nrecords+1); if(i) goto GENFAIL;

    memmove(dl->data+next_item+1, dl->data+next_item,
            (dl->nrecords - next_item)*sizeof(long_time_t));

    dl->nrecords++;
    r = dl->data + next_item;
    *recindex = next_item;
    *r = timestamp;
    return 0;

GENFAIL:
    *errstr = strerror(errno);
    return errno;
}

DLLEXPORT long_time_t *dl_get_next(const struct datetimelist *dl,
                                                        long_time_t timestamp)
{
    long_time_t *low, *high, *mid;

    if(!dl->nrecords)
        return NULL;
    low = dl->data;
    high = dl->data + (dl->nrecords - 1);
    while(low<=high) {
        mid = low + (high-low)/2;
        if(timestamp < *mid)
            high = mid-1;
        else if(timestamp > *mid)
            low = mid+1;
        else {
            low = mid;
            break;
        }
    };
    if(low>=dl->data + dl->nrecords)
        return NULL;
    else
        return low;
}

DLLEXPORT int dl_get_next_i(const struct datetimelist *dl, long_time_t timestamp)
{
    long_time_t *r = dl_get_next(dl, timestamp);
    if(r==NULL)
        return -1;
    return r - dl->data;
}

DLLEXPORT long_time_t *dl_get_prev(const struct datetimelist *dl,
                                                        long_time_t timestamp)
{
    long_time_t *r;
    if(dl->nrecords==0 || timestamp < *(dl->data))
        return NULL;
    if((r = dl_get_next(dl, timestamp))==NULL)
        r = dl->data + dl->nrecords - 1;
    else if(timestamp!=*r)
        r--;
    return r;
}

DLLEXPORT int dl_get_prev_i(const struct datetimelist *dl, long_time_t timestamp)
{
    long_time_t *r = dl_get_prev(dl, timestamp);
    if(r==NULL)
        return -1;
    return r - dl->data;
}

DLLEXPORT long_time_t *dl_get(const struct datetimelist *dl,
                                                        long_time_t timestamp)
{
    long_time_t *r = dl_get_next(dl, timestamp);
    return (r && *r==timestamp) ? r : NULL;
}

DLLEXPORT int dl_get_i(const struct datetimelist *dl, long_time_t timestamp)
{
    long_time_t *r = dl_get(dl, timestamp);
    return r ? r - dl->data : -1;
}

DLLEXPORT int dl_delete_item(struct datetimelist *dl, int index)
{
    long_time_t *r = dl->data + index;
    return dl_delete_records(dl, r, r) ? index : -1;
}

DLLEXPORT long_time_t *dl_delete_records(struct datetimelist *dl,
                                    long_time_t *r1, long_time_t *r2)
{
    long_time_t *start = dl->data;
    long_time_t *end   = dl->data + dl->nrecords - 1;
    long_time_t *r;
    if(!dl->nrecords || r1<start || r2<start || r1>end || r2>end || r2<r1)
        return NULL;
    memmove(r1, r2+1, (end-r2)*sizeof(long_time_t));
    dl->nrecords-= r2-r1+1;
    return r1;
}

DLLEXPORT int dl_delete_record(struct datetimelist *dl, long_time_t tm)
{
    int i;

    if(!dl->nrecords) return -1;
    if((i = dl_get_i(dl, tm))<0) return -1;
    return dl_delete_item(dl, i);
}

DLLEXPORT struct datetimelist *dl_create(void)
{
    struct datetimelist *dl;

    if(!(dl = (struct datetimelist *) malloc(sizeof(struct datetimelist))))
        return NULL;
    dl->nrecords = 0;
    dl->data = NULL;
    dl->memblocksize = 0;
    return dl;
}

DLLEXPORT void dl_free(struct datetimelist *dl)
{
    dl_clear(dl);
    free(dl->data);
    dl->data=NULL;
    free(dl);
}

DLLEXPORT int dl_length(const struct datetimelist *dl)
{
    return dl->nrecords;
}

DLLEXPORT void dl_clear(struct datetimelist *dl)
{
    dl->nrecords = 0;
}

DLLEXPORT long_time_t dl_get_item(struct datetimelist *dl, int index)
{
    return dl->data[index];
}

