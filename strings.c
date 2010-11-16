/*
 * strings.c - string operations
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

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "strings.h"

char *strip(char *s)
{
    char *p;

    /* Strip trailing. */
    for (p=s+strlen(s); p>s && isspace(*(--p));)
        *p = '\0';

    /* Strip leading. */
    for (p=s; isspace(*p); ++p)
            ;
    memmove(s, p, strlen(s)+1);

    return s;
}

#if ! defined(HAVE_STRDUP) && ! defined(WIN32)

char *strdup(const char *s)
{
    char *result;

    if ((result = malloc(strlen(s)+1))==NULL)
        return NULL;
    return strcpy(result, s);
}

#endif
