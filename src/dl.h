/*
 * openmeteo.org
 * dickinson library
 * dl.h - date time lists
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

#ifndef _DL_H

#define _DL_H

#include <stdio.h>
#include <stdlib.h>
#include "platform.h"
#include "dates.h"

struct datetimelist {
    long_time_t *data;
    int nrecords;
    size_t memblocksize;
};

extern DLLEXPORT int dl_append_record(struct datetimelist *dl, long_time_t timestamp,
    int *recindex, char **errstr);
extern DLLEXPORT int dl_insert_record(struct datetimelist *dl, long_time_t timestamp,
    int *recindex, char **errstr);
extern DLLEXPORT long_time_t *dl_get_next(const struct datetimelist *dl,
                                                        long_time_t timestamp);
extern DLLEXPORT int dl_get_next_i(const struct datetimelist *dl, long_time_t timestamp);
extern DLLEXPORT long_time_t *dl_get_prev(const struct datetimelist *dl,
                                                        long_time_t timestamp);
extern DLLEXPORT int dl_get_prev_i(const struct datetimelist *dl, long_time_t timestamp);
extern DLLEXPORT long_time_t *dl_get(const struct datetimelist *dl,
                                                        long_time_t timestamp);
extern DLLEXPORT int dl_get_i(const struct datetimelist *dl, long_time_t timestamp);
extern DLLEXPORT int dl_delete_item(struct datetimelist *dl, int index);
extern DLLEXPORT long_time_t *dl_delete_records(struct datetimelist *dl,
                                    long_time_t *r1, long_time_t *r2);
extern DLLEXPORT int dl_delete_record(struct datetimelist *dl, long_time_t tm);
extern DLLEXPORT struct datetimelist *dl_create(void);
extern DLLEXPORT void dl_free(struct datetimelist *dl);
extern DLLEXPORT int dl_length(const struct datetimelist *dl);
extern DLLEXPORT void dl_clear(struct datetimelist *dl);
extern DLLEXPORT long_time_t dl_get_item(struct datetimelist *dl, int index);

#endif /* _DL_H */
