Dickinson --- C library for time series operations
==================================================

Dickinson is a C library containing various utilities, mainly for time
series operations. The Python pthelma library is partly a front end to
Dickinson.

ts - Time series operations
---------------------------

The :mod:`ts` module contains utilities for time series management. It
has two data types: :ctype:`record` for time series records,
and :ctype:`timeseries` for time series. Memory management and
other operations on these two data types are performed by the
accompanying functions.

Basic operations
^^^^^^^^^^^^^^^^

.. ctype:: struct ts_record

   Represents a time series record.

   .. cmember:: long_time_t int timestamp

      Number of seconds since 1 January 1970.

   .. cmember:: int null

      Boolean, indicating whether the value is missing.

   .. cmember:: double value

      The value of the time series record; irrelevant if
      :cmember:`null` is true.

   .. cmember:: char *flags

      A pointer to a string holding flags as space separated ASCII
      words.

.. ctype:: struct timeseries

   Represents a time series. Contains some internal attributes which
   you should not attempt to access directly; instead, you should
   create and destroy :ctype:`timeseries` objects using
   :cfunc:`ts_create()` and :cfunc:`ts_free()`, and use the functions
   described below to insert, delete, and retrieve records, and
   otherwise manipulate :ctype:`timeseries` objects.

.. cfunction:: int ts_append_record(struct timeseries *ts, long_time_t timestamp, int null, double value, const char *flags, int *recindex, char **errstr)

   Append a record to the specified time series.  Returns nonzero on
   error, setting *errstr* to a static error message; the return value
   is an appropriate errno. Returns in *recindex* the actual index
   after adding the record.

.. cfunction:: void ts_clear(struct timeseries *ts)

   Delete all time series records.

.. cfunction:: struct timeseries *ts_create()

   Create and return a time series object. What it actually does is
   malloc a memory block enough to hold the :ctype:`timeseries`, which
   it initializes with 0 records and a nonexistent memory block for
   records. The memory block for records will automatically be
   initialized when records are added to the time series. Returns NULL if
   :cfunc:`malloc` returns NULL.

.. cfunction:: int ts_delete_item(struct timeseries *ts, int index)

   Delete the record at *index*. Return *index* or -1 if such index
   does not exist.

.. cfunction:: struct ts_record *ts_delete_records(struct timeseries *ts, struct ts_record *r1, struct ts_record *r2)

   Delete all records from *r1* to *r2* (inclusive), which must be
   valid pointers to existing records within *ts*. Return *r1* or
   :const:`NULL` if there is any error in the supplied pointers,
   including *r2*<*r1*.

.. cfunction:: int ts_delete_record(struct timeseries *ts, long_time_t timestamp)

   Delete the record that has time stamp *tm*. Frees the memory
   occupied by the record and the associated flag string, and shifts
   following records as needed.  Returns -1 if no such record exist or
   the index of the record deleted.

.. cfunction:: void ts_free(struct timeseries *ts)

   Destroy a time series object. It frees all memory occupied by the
   flag strings, the records, and the structure itself. Only call this
   function if the object has been created by :cfunc:`ts_create()`; do
   not call it if the object is an automatic or static variable, since
   in that case it will attempt to free memory that has not been
   dynamically allocated.

.. cfunction:: struct ts_record ts_get_item(struct timeseries *ts, int index)

   Return the :ctype:`record` at *index*. If such a record does not
   exist, a segmentation violation is likely.

.. cfunction:: struct ts_record *ts_get_next(struct timeseries *ts, long_time_t timestamp)

   Return first record with date >= *timestamp*, or :const:`NULL` if such a
   record does not exist.

.. cfunction:: struct ts_record *ts_get_prev(struct timeseries *ts, long_time_t timestamp)

   Return last record with date <= *timestamp*, or :const:`NULL` if such a
   record does not exist.

.. cfunction:: int ts_get(struct timeseries *ts, long_time_t timestamp)

   Return the record with given *timestamp*, or :const:`NULL` if no such record
   exists.

.. cfunction:: int ts_get_next_i(struct timeseries *ts, long_time_t timestamp)
               int ts_get_prev_i(struct timeseries *ts, long_time_t timestamp)
               int ts_get_i(struct timeseries *ts, long_time_t timestamp)

   These functions are the same as the ones without the *_i* suffix, except
   that they return an index instead of a pointer to a
   :ctype:`ts_record`, and -1 if the record is not found.

.. cfunction:: int ts_insert_record(struct timeseries *ts, long_time_t timestamp, int null, double value, const char *flags, int allow_existing, int *recindex, char **errstr)

   Insert a record to the specified time series. Returns nonzero on
   error, setting *errstr* to a static error message; the return value
   is an appropriate errno. Returns in *recindex* the actual index
   after adding the record. If a record with the specified timestamp
   already exists, it returns an error, except if *allow_existing* is
   nonzero, in which case the existing record is overwritten.

.. cfunction:: int ts_length(struct timeseries *ts)

   Return the number of records of the time series.

.. cfunction:: int ts_merge(struct timeseries *ts1, struct timeseries *ts2, char **errstr)

   Merge *ts2* into *ts1*.  The two time series must not have any
   common timestamps, and after merging *ts2* records must be
   consecutive in *ts1* (i.e. there must be no intermixing of
   records).  Returns 0 on success, or an appropriate errno on error,
   in which case it also sets *errstr* to an appropriate error
   message.

.. cfunction:: double ts_min(struct timeseries *ts, long_time_t start_date, long_time_t end_date)
               double ts_max(struct timeseries *ts, long_time_t start_date, long_time_t end_date)
               double ts_average(struct timeseries *ts, long_time_t start_date, long_time_t end_date)
               double ts_sum(struct timeseries *ts, long_time_t start_date, long_time_t end_date)

   Return minimum, maximum, average, or sum of the time series in the
   specified interval. Use :const:`LLONG_MIN` and :const:`LLONG_MAX`
   as the *start_date* and *end_date* to return the value for the
   entire time series.

   If the value cannot be computed (e.g. because the time series
   does not have any not-null values in the specified interval),
   these functions return :const:`NAN`.

.. cfunction:: int ts_merge_anyway(struct timeseries *ts1, struct timeseries *ts2, char **errstr)

   Merge *ts2* into *ts1*. *ts1* records with timestamps that exist in
   *ts2* are overwritten. *ts2* records can be interspersed with *ts1*
   records. Returns 0 on success, or an appropriate errno on error,
   in which case it also sets *errstr* to an appropriate error
   message.

.. cfunction:: int ts_readline(char *line, struct timeseries *ts, char **errstr)

   Read a comma delimited line of input and insert that record in
   the time series.
   
   The line must have the format :samp:`{datestr},{value},{flags}`,
   where *value* is a floating point number (using a dot as the
   decimal separator, regardless of system settings), and *flags* is
   string of space separated ASCII words; *value* and *flags* can be
   empty. *datestr* is the date in one of the date formats accepted by
   :cfunc:`parsedatestring()`.  If a record with that date already
   exists in the time series, it is replaced; otherwise, a new record
   is inserted in the appropriate position.  Returns 0 on success, or
   an appropriate errno on error, in which case it also sets *errstr*
   to an appropriate error message.

.. cfunction:: int ts_readfile(FILE* fp, struct timeseries *ts, int *errline, char **errstr)

   Read data from FILE* fp stream, by using the ts_readline function.


.. cfunction:: int ts_readfromstring(char *string, struct timeseries *ts, int *errline, char **errstr)

   Read data from a string containing time series records separated by
   line feeds, or carriage returns, or both. ts_readline is used for
   string parsing of each line (time series record).

.. cfunction:: int ts_set_item(struct timeseries *ts, int index, int null, double value, const char *flags, char **errstr)

   Set the time series record at *index*. A record with that index
   must exist, or an error is returned. Returns 0 on success, or an
   appropriate errno on error, in which case *errstr* is also set to
   an appropriate error message.

.. cfunction:: int ts_writeline(struct ts_record *r, int precision, char *str, size_t max_length)

   Converts the record pointed to by *r*  to an ASCII representation
   for including in a file format, and writes that representation,
   including a terminating null byte, to string *str* of size
   *max_length*.  *precision* is an integer indicating the required
   value precision, in number of decimal digits; *precision* can be
   -9999, meaning to use "%G" as the printf formatting string.
   
   Returns the number of characters written to *str*, not including
   the null byte. This number is at most *max_length* minus 1. If
   writing the result would exceed that number, then it returns zero,
   in which case the contents of *str* are undefined.

.. ctype:: struct timeseries_list

   Contains two members, the number of timeseries *n* (an
   :ctype:`int`), and a pointer to a :ctype:`timeseries` array,
   *ts*, which is normally dynamically allocated. Use the
   following functions to play with :ctype:`timeseries_list`:

   .. cfunction:: struct timeseries_list *tsl_create(void)
                  void tsl_free(struct timeseries_list *tsl)
                  int tsl_append(struct timeseries_list *tsl, struct timeseries *t)
                  int tsl_delete(struct timeseries_list *tsl, int index)

      These functions perform dynamic memory allocation of
      :ctype:`timeseries_list` objects. :cfunc:`tsl_create()`
      creates and returns a :ctype`timeseries_list` object
      containing zero elements, or :const:`NULL` if insufficient
      memory. :cfunc:`tsl_free()` frees such an object.
      :cfunc:`tsl_append()` and :cfunc:`tsl_delete()` append or delete
      an element, returning zero or an appropriate *errno* on
      insufficient memory or invalid argument.

      .. admonition:: Important

         These functions handle memory allocation of the
         :ctype:`timeseries_list` object and its contained array of
         pointers to :ctype:`timeseries` objects, but does not touch
         the :ctype:`timeseries` objects themselves. It is the
         caller's responsibility to allocate and free the
         :ctype:`timeseries` objects.

Extended operations
^^^^^^^^^^^^^^^^^^^

.. cfunction:: int ts_identify_events(const struct timeseries_list *ts, struct interval range, int reverse, double start_threshold, double end_threshold, int ntimeseries_start_threshold, int ntimeseries_end_threshold, long_time_t time_separator, struct interval_list *events, char **errstr)

    This function is intended to find precipitation events in *ts*,
    which is supposed to be a set of spatially proximate time series,
    but it can also be used to find any kind of event where the value
    of a time series goes beyond a threshold, such as events of heat
    or cold.  An event is defined as a time interval at the start of
    which there is a value at least *start_threshold* in at least
    *ntimeseries_start_threshold* time series, at the end of which
    there is a value less than *end_threshold* in at least all but
    *ntimeseries_end_threshold* time series, and separated by at least
    *time_separator* from the nearest similar event. Only the interval
    specified by *range* is examined, and all time series should have
    the same time stamps within that interval. If *reverse* is
    nonzero, then the function finds events that are smaller than the
    thresholds instead of greater (e.g. cold events).  The events are
    returned in *events*, which must have been allocated with
    :cfunc:`il_create()` by the caller and must also be freed by the
    caller.  Returns 0 on success, or an approriate :cdata:`errno` on
    error, in which case it also sets *errstr* to an appropriate error
    message.

dates - Date utilities
----------------------

.. ctype:: long_time_t

   This type is like :ctype:`time_t`, but is guaranteed to be at least
   64 bits, therefore ensuring that it spans many years.

.. ctype:: struct interval

   Contains two :ctype:`long_time_t` members, *start_date* and *end_date*.

.. ctype:: struct interval_list

   Contains two members, the number of intervals *n* (an
   :ctype:`int`), and a pointer to a :ctype:`interval` array,
   *intervals*, which is normally dynamically allocated. Use the
   following functions to play with :ctype:`interval_list`:

   .. cfunction:: struct interval_list *il_create(void)
                  void il_free(struct interval_list *intrvls)
                  int il_append(struct interval_list *intrvls, long_time_t start_date, long_time_t end_date)
                  int il_delete(struct interval_list *intrvls, int index)

      These functions perform dynamic memory allocation of
      :ctype:`interval_list` objects. :cfunc:`il_create()`
      creates and returns a :ctype`interval_list` object
      containing zero elements, or :const:`NULL` if insufficient
      memory. :cfunc:`il_free()` frees such an object.
      :cfunc:`il_append()` and :cfunc:`il_delete()` append or delete
      an element, returning zero or an appropriate *errno* on
      insufficient memory or invalid argument.

.. cfunction:: void add_minutes(struct tm *tm, int mins)

   Increases or decreases *tm* by the specified number of minutes.

.. cfunction:: void igmtime(long_time_t gm_time, struct tm *tm)

   Do the same thing as the :mod:`time.h` :cfunc:`gmtime()` function,
   except using a :ctype:`long_time_t` value (gm_time) instead of the
   standard :ctype:`time_t`.  The result is written in the *tm*.

.. cfunction:: int is_leap_year(int y)

   Return nonzero of *y* is a leap year. Not that this is a macro and
   may evaluate *y* multiple times.

.. cfunction:: int month_days(int mon, int year)

   Return number of days in specified month (0 to 11) of specified
   year.

.. cfunction:: int parsedatestring(const char *s, struct tm *tm, char **errmsg)

   Parse supplied string and set *tm* to the parsed date. *s* must be
   in one of the following formats: ``%Y-%m-%d %H:%M``, ``%Y-%m-%d
   %H:%M:00``, ``%Y-%m-%d %H:%M:00:00``, ``%Y-%m-%d %H``, ``%Y-%m-%d``,
   ``%Y-%m``, ``%Y``.  A slash may also be used instead of a hyphen as
   the date separator, a "T" instead of a space as the date/time
   separator, and a full stop instead of a colon as the time
   separator.  Returns nonzero on error, setting *errmsg* to a static
   error message. The return value is :const:`EINVAL` if supplied
   string is not a valid date, or :const:`ENOMEM` on insufficient
   memory.

.. cfunction:: int tmcmp(struct tm *tm1, struct tm *tm2)

   Return -1, 0, or 1 if *tm1* is less than, equal to, or greater than
   *tm2*. Uses minute precision.

strings - string utilities
--------------------------

.. cfunction:: char *strip(char *s)

   Strip leading and trailing whitespace from *s* in place, and
   return *s*.

csv - operations with CSV files
-------------------------------

The word "quote" thereafter means the double-quote character, ``"``.

Unfortunately there is no universally accepted CSV standard, and not
all applications behave the same. The definition we accept here is
this: a field is a sequence of zero or more characters; fields are
delimited by commas; leading and trailing white space characters are
preserved; fields cannot contain newline characters; fields can begin
and end with quotes, in which case they may contain commas; inside a
quoted field, quotes are designated by double quotes; a field is
considered to be quoted if it begins with a quote and ends with the
character sequence ``",`` (quote followed by comma) or ``"\n`` (quote
followed by newline), or ``"\0`` (quote ends the string), provided the
end quote is not the second character of a double quote; if no such
field ending sequence can be found on the line, the field is
considered unquoted; single quotes inside a quoted string are ignored.

.. cfunction:: char *csvtok(char **stringp)

   :cfunc:`csvtok()` assumes that *stringp* points to a line from a
   CSV file. It finds the first item in the string, modifies it, if it
   is quoted, by converting double quotes to single quotes, terminates
   it with '\0' (by overwriting the delimiting comma or the end quote,
   or some character before those if the item has shrinked because of
   double quote interpretation) and updates *stringp* to point past
   the item. If there is no comma in *stringp*, or if the entire
   *stringp* is quoted, :cfunc:`csvtok()` sets *stringp* to
   :const:`NULL`.  If *stringp* is :const:`NULL`, :cfunc:`csvtok()`
   does nothing.

   :cfunc:`csvtok()` returns the beginning of the field, which is the
   original value of *stringp*, unless the field is quoted, in which
   case it is the original value incremented. If *stringp* is
   :const:`NULL`, :cfunc:`csvtok()` returns :const:`NULL`.

.. cfunction:: char *csvquote(const char *s)

   :cfunc:`csvquote()` is like :cfunc:`strdup()`, except that if the
   original string contains commas or quotes, the returned string is
   quoted as needed in order to be a CSV field; that is, a leading and
   trailing quote is added, and any other quotes are converted into
   double quotes. Like :cfunc:`strdup()`, it returns a dynamically
   allocated string, or :const:`NULL` on insufficient memory.


Copyright and credits
=====================

Dickinson is

| Copyright (C) 2005-2011 National Technical University of Athens

Dickinson is free software: you can redistribute and/or modify it
under the terms of the GNU General Public License, as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version.

The software is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
licenses for more details.

You should have received a copy of the licenses along with this
program.  If not, see http://www.gnu.org/licenses/.

Dickinson was originally written by Stefanos Kozanis and Antonis
Christofides of the National Technical University of Athens.
