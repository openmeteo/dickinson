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
#include "dates.h"

struct ts_record {
    long_time_t timestamp;
    int null;
    double value;
    char *flags;
};

struct timeseries {
    struct ts_record *data; /* Dyn mem block containing timeseries records */
    int nrecords; /* Size of time series (number of records) */
    size_t memblocksize; /* Size of the dynamic memory block in bytes. */
};

struct timeseries_list {
    struct timeseries *ts;
    int n;
};

extern DLLEXPORT int ts_append_record(struct timeseries *ts,
    long_time_t timestamp, int null, double value, const char *flags,
    int *recindex, char **errstr);
extern DLLEXPORT int ts_insert_record(struct timeseries *ts,
    long_time_t timestamp, int null, double value, const char *flags,
    int allow_existing, int *recindex, char **errstr);
extern DLLEXPORT int ts_get_next(const struct timeseries *ts, long_time_t tm);
extern DLLEXPORT int ts_get_prev(const struct timeseries *ts, long_time_t tm);
extern DLLEXPORT int ts_index_of(const struct timeseries *ts, long_time_t tm);
extern DLLEXPORT int ts_delete_record(struct timeseries *ts, long_time_t tm);
extern DLLEXPORT int ts_delete_item(struct timeseries *ts, int index);
extern DLLEXPORT void *ts_create(void);
extern DLLEXPORT void ts_free(struct timeseries *ts);
extern DLLEXPORT int ts_length(const struct timeseries *ts);
extern DLLEXPORT void ts_clear(struct timeseries *ts);
extern DLLEXPORT struct ts_record ts_get_item(struct timeseries *ts, int index);
extern DLLEXPORT int ts_set_item(struct timeseries *ts, int index, 
    int null, double value, const char *flags, char **errstr);
extern DLLEXPORT int ts_readline(char *line, struct timeseries *ts,
                                                                char **errstr);
extern DLLEXPORT int ts_merge(struct timeseries *ts1, struct timeseries *ts2, 
                                                                char **errstr);
extern DLLEXPORT int ts_merge_anyway(struct timeseries *ts1,
                                  const struct timeseries *ts2, char **errstr);
extern DLLEXPORT int ts_writeline(char **line, struct timeseries *ts, int index,
    int precision, char **errstr);
extern DLLEXPORT int ts_identify_events(const struct timeseries_list *ts,
    struct interval range, int reverse,
    double start_threshold, double end_threshold,
    int ntimeseries_start_threshold, int ntimeseries_end_threshold,
    long_time_t time_separator, struct interval_list *events, char **errstr);

#endif /* _TS_H */
