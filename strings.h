/*
 * strings.h - string operations
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

#ifndef _STRINGS_H

#define _STRINGS_H

/* Strips leading and trailing whitespace from s inplace, and returns s. */
extern char *strip(char *s);

/* man strdup. The function does not exist in Windows and therefore it is
 * re-implemented here.
 */
extern char *strdup(const char *s);

#endif /* _STRINGS_H */
