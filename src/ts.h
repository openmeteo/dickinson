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
    struct timeseries **ts;
    int n;
};

extern DLLEXPORT int ts_append_record(struct timeseries *ts,
    long_time_t timestamp, int null, double value, const char *flags,
    int *recindex, char **errstr);
extern DLLEXPORT int ts_insert_record(struct timeseries *ts,
    long_time_t timestamp, int null, double value, const char *flags,
    int allow_existing, int *recindex, char **errstr);
extern DLLEXPORT struct ts_record *ts_get_next(const struct timeseries *ts,
                                                        long_time_t timestamp);
extern DLLEXPORT int ts_get_next_i(const struct timeseries *ts,
                                                        long_time_t timestamp);
extern DLLEXPORT struct ts_record *ts_get_prev(const struct timeseries *ts,
                                                        long_time_t timestamp);
extern DLLEXPORT int ts_get_prev_i(const struct timeseries *ts,
                                                        long_time_t timestamp);
extern DLLEXPORT struct ts_record *ts_get(const struct timeseries *ts,
                                                        long_time_t timestamp);
extern DLLEXPORT int ts_get_i(const struct timeseries *ts,
                                                        long_time_t timestamp);
extern DLLEXPORT int ts_delete_record(struct timeseries *ts, long_time_t tm);
extern DLLEXPORT int ts_delete_item(struct timeseries *ts, int index);
extern DLLEXPORT struct ts_record *ts_delete_records(struct timeseries *ts,
                                   struct ts_record *r1, struct ts_record *r2);
extern DLLEXPORT struct timeseries *ts_create(void);
extern DLLEXPORT void ts_free(struct timeseries *ts);
extern DLLEXPORT int ts_length(const struct timeseries *ts);
extern DLLEXPORT void ts_clear(struct timeseries *ts);
extern DLLEXPORT struct ts_record ts_get_item(struct timeseries *ts, int index);
extern DLLEXPORT int ts_set_item(struct timeseries *ts, int index, 
    int null, double value, const char *flags, char **errstr);
extern DLLEXPORT int ts_readline(char *line, struct timeseries *ts,
                                                                char **errstr);
extern DLLEXPORT int ts_readfile(FILE *fp, struct timeseries *ts, int *errline,
                                                                char **errstr);
extern DLLEXPORT int ts_readfromstring(char *string, struct timeseries *ts, 
                                                  int *errline, char **errstr);
extern DLLEXPORT int ts_merge(struct timeseries *ts1, struct timeseries *ts2, 
                                                                char **errstr);
extern DLLEXPORT int ts_merge_anyway(struct timeseries *ts1,
                                  const struct timeseries *ts2, char **errstr);
extern DLLEXPORT int ts_writeline(struct ts_record *r, int precision, char *str,
                                                        size_t max_length);
extern DLLEXPORT char *ts_write(struct timeseries *ts, int precision,
                long_time_t start_date, long_time_t end_date, char **errstr);
extern DLLEXPORT struct timeseries_list *tsl_create(void);
extern DLLEXPORT void tsl_free(struct timeseries_list *tsl);
extern DLLEXPORT int tsl_append(struct timeseries_list *tsl, struct timeseries
                                                                    *t);
extern DLLEXPORT int tsl_delete(struct timeseries_list *tsl, int index);
extern DLLEXPORT double ts_min(struct timeseries *ts, long_time_t start_date,
                                                        long_time_t end_date);
extern DLLEXPORT double ts_max(struct timeseries *ts, long_time_t start_date,
                                                        long_time_t end_date);
extern DLLEXPORT double ts_average(struct timeseries *ts,
                                long_time_t start_date, long_time_t end_date);
extern DLLEXPORT double ts_sum(struct timeseries *ts, long_time_t start_date,
                                                        long_time_t end_date);
extern DLLEXPORT int ts_identify_events(struct timeseries_list *ts,
    struct interval range, int reverse,
    double start_threshold, double end_threshold,
    int ntimeseries_start_threshold, int ntimeseries_end_threshold,
    long_time_t time_separator, struct interval_list *events, char **errstr);

#endif /* _TS_H */
