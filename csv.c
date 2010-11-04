/*
 * csv.c - operations with comma-delimited files
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
#include <stdlib.h>
#include <assert.h>
#include "csv.h"

/* Starting with the first character after the initial quote, find_end_quote
 * attempts to find the end quote, and returns its position, or NULL if there's
 * no end quote.
 */
static char *find_end_quote(char *s) {
    for(++s; *s; ++s)
        if(*s=='"') {
            char *p = s+1;
            if (*p=='"')
                ++s;
            else if(*p==',' || *p=='\n' || *p=='\0')
                return s;
        }
    return NULL;
}

char *csvtok(char **stringp) {
    char *end_quote;
    char *p;
    char *retval;

    if(!*stringp) return NULL;

    retval = *stringp;

    /* Is the field quoted? */
    if(**stringp == '"'  && (end_quote = find_end_quote(*stringp))) {
        ++retval;
        *stringp = end_quote+1;
        if(**stringp == ',') ++*stringp;
        else *stringp = NULL;

        /* Convert double quotes to single quotes and ignore single quotes. */
        for(p=retval; p<end_quote; ++p)
            if(*p == '"'  ) {
                memmove(p, p+1, end_quote-p);
                --end_quote;
            }

        *end_quote = '\0';
    } else {
        for(p = *stringp; *p && (*p!=','); ++p)
            ;
        *stringp = p;
        if(**stringp == ',') ++*stringp;
        else *stringp = NULL;
        *p = '\0';
    }
    return retval;
}

char *csvquote(const char *s) {
    size_t newlen;
    const char *p;
    char *d;
    char *newstring;

    if(!strpbrk(s, ",\""))
        return strdup(s);
    newlen = strlen(s)+2; /* Initial size plus leading and trailing quote. */
    for(p = s; *p; ++p)
        if(*p == '"')
            ++newlen;
    newstring = malloc(newlen+1);  if(!newstring) return NULL;
    *newstring = '"';
    for(d = newstring+1; *s; ++d, ++s) {
        *d = *s;
        if(*s == '"') *(++d) = *s;
    }
    *d = '"';
    *(++d) = '\0';
    assert(d-newstring == newlen);
    return newstring;
}
