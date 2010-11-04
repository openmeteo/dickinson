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

struct record {
    long long int timestamp;
    int null;
    double value;
    char *flags;
};

struct timeseries {
    struct record *data;
    int nrecords;
    size_t memblocksize;
};

extern int append_record(struct timeseries *ts, long long timestamp, int null,
    double value, const char *flags, int *recindex, char **errstr);

/* Insert a record to the specified time series. Returns nozero on
 * error, setting errstr to a static error message; the return value
 * is an appropriate errno. Returns the actual index after adding the
 * record with recindex. Insert returns error if a record with
 * timestamp already exists or else it places the record in the
 * appropriate position to keep the time series list sorted.
*/
extern int insert_record(struct timeseries *ts, long long timestamp, int null,
    double value, const char *flags, int *recindex, char **errstr);

/* Returns index of first record with date >= tm, or -1 if such a record
 * does not exist. Searching by binary search.
 */
extern int get_next(struct timeseries *ts, long long tm);

/* Returns index of last record with date <= tm, or -1 if such a record
 * does not exist.
 */
extern int get_prev(struct timeseries *ts, long long tm);

/* Returns the index of a record with a time stamp of tm, or
   -1 is no such record exists.
*/
extern int index_of(struct timeseries *ts, long long tm);

/* delete the record a time stamp tm, by freeing the record and the
   associated flag string. Finally it shifts former records.
   Returns -1 if no such record exist or the index of the
   record deleted */
extern int delete_record(struct timeseries *ts, long long tm);

/* Delete the record at index, if such index exists */
extern int delete_item(struct timeseries *ts, int index);

/* Returns a pointer (handle) to a new created time series with
   malloc */
extern void * ts_create();

/* Frees the time series ts by freeing associated memory and
   flag strings */
extern void ts_free(struct timeseries *ts);

/* Returns the length (number of records) of the time series */
extern int ts_length(struct timeseries *ts);

/* Delete all time series records */
extern void ts_clear(struct timeseries *ts);

/* Seturns a single record of type (struct record) of the time series
   at index. Index should exists or else an unidentified object is
   return */
extern struct record get_item(struct timeseries *ts, int index);

/* Set the time series record with value, flags, null at index.
   Returns 0 on success.
*/
extern int set_item(struct timeseries *ts, int index, 
    int null, double value, const char *flags, char **errstr);

/* Reads a comma delimited line of input and insert that record in
   the timeseries ts. Accepted format is:
   Datestr,value,flags
   value and flags strings could be empty (null value, no flags).
   Flags are separated by space.
   Datestr can be from a set of valid date formats such as:
   yyyy-mm-dd hh:mm, yyyy-mm-dd, etc.
   where - and / could be used equally, blank space ' ' or
   T character (date time separator) and : or . for hh, mm
   separation.
   If a line could not be parsed then an error code is
   returned.
*/
extern int ts_readline(char *line, struct timeseries *ts, char **errstr);

/* Merge ts2 into ts1 with some constraints.
   No intermixing is allowed and ts1, ts2 records should be unique
   (no identical records). */
extern int merge(struct timeseries *ts1, struct timeseries *ts2, 
char **errstr);

/* Outputs a text line for file writing, at index. 
 * Uses vfromatstr for formating the float value, usually "%G"
*/
extern int ts_writeline(char **line, struct timeseries *ts, int index,
    const char *vformatstr, char **errstr);

#endif /* _TS_H */
