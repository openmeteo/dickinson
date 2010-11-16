/*
 * openmeteo.org
 * dickinson library
 * ts.h - time series operations
 *
 * Copyright (c) 2004  Antonios Christofides
 *
 * Copyright (c) 2010  National Technical University of Athens
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

#ifndef _TS_H

#define _TS_H

#include <stdio.h>
#include <stdlib.h>
#include "platform.h"

struct record {
    long long int timestamp;
    int null;
    double value;
    char *flags;
};

struct timeseries {
    struct record *data; /* Dynamic mem block containing timeseries records */
    int nrecords; /* Size of time series (number of records) */
    size_t memblocksize; /* Size of the dynamic memory block in bytes. */
};

extern DLLEXPORT int append_record(struct timeseries *ts, long long timestamp,
    int null, double value, const char *flags, int *recindex, char **errstr);
extern DLLEXPORT int insert_record(struct timeseries *ts, long long timestamp, 
    int null, double value, const char *flags, int *recindex, char **errstr);
extern DLLEXPORT int get_next(struct timeseries *ts, long long tm);
extern DLLEXPORT int get_prev(struct timeseries *ts, long long tm);
extern DLLEXPORT int index_of(struct timeseries *ts, long long tm);
extern DLLEXPORT int delete_record(struct timeseries *ts, long long tm);
extern DLLEXPORT int delete_item(struct timeseries *ts, int index);
extern DLLEXPORT void * ts_create(void);
extern DLLEXPORT void ts_free(struct timeseries *ts);
extern DLLEXPORT int ts_length(struct timeseries *ts);
extern DLLEXPORT void ts_clear(struct timeseries *ts);
extern DLLEXPORT struct record get_item(struct timeseries *ts, int index);
extern DLLEXPORT int set_item(struct timeseries *ts, int index, 
    int null, double value, const char *flags, char **errstr);
extern DLLEXPORT int ts_readline(char *line, struct timeseries *ts,
                                                                char **errstr);
extern DLLEXPORT int merge(struct timeseries *ts1, struct timeseries *ts2, 
                                                                char **errstr);
extern DLLEXPORT int ts_writeline(char **line, struct timeseries *ts, int index,
    int precision, char **errstr);

#endif /* _TS_H */
