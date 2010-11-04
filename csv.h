/*
 * csv.h - operations with csv files
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

#ifndef _CSV_H

#define _CSV_H

/* The word 'quote' thereafter means the double-quote character, '"'.
 *
 * csvtok assumes that the string *stringp is a line from a CSV file. It finds
 * the first item in the string, modifies it, if it is quoted necessary, by
 * converting double quotes to single quotes, terminates it with '\0' (by
 * overwriting the delimiting comma or the end quote, or some character before
 * those if the item has shrinked because of double quote interpretation) and
 * *stringp is updated to point past the item. If there is no comma in
 * *stringp, or if the entire *stringp is quoted, csvtok sets *stringp to NULL.
 * If *stringp is NULL, csvtok does nothing.
 *
 * csvtok returns the beginning of the field, which is the original value of
 * *stringp, unless the field is quoted, in which case it is the original value
 * incremented. If *stringp is NULL, csvtok returns NULL.
 *
 * Unfortunately there is no universally accepted CSV standard, and not all
 * applications behave the same. The definition accepted by cvstok is this: a
 * field is a sequence of zero or more characters; fields are delimited by
 * commas; leading and trailing white space characters are preserved; fields
 * cannot contain newline characters; fields can begin and end with quotes, in
 * which case they may contain commas; inside a quoted field, quotes are
 * designated by double quotes; a field is considered to be quoted if it begins
 * with a quote and ends with the character sequence ", (quote followed by
 * comma) or "\n (quote followed by newline), or "\0 (quote ends the string),
 * provided the end quote is not the second character of a double quote; if no
 * such field ending sequence can be found on the line, the field is considered
 * unquoted; single quotes inside a quoted string are ignored.
 */
char *csvtok(char **stringp);

/* The word 'quote' thereafter means the double-quote character, '"'.
 *
 * csvquote is like strdup, except that if the original string contains commas
 * or quotes, the returned string is quoted as needed in order to be a CSV
 * field; that is, a leading and trailing quote is added, and any other quotes
 * are converted into double quotes. Like strdup, it returns a dynamically
 * allocated string, or NULL on insufficient memory.
 */
char *csvquote(const char *s);

#endif
