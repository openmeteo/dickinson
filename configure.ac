AC_PREREQ([2.69])
AC_INIT([dickinson], [0.1], [anthony@itia.ntua.gr])
AM_INIT_AUTOMAKE(dickinson, 2.6)
AC_CONFIG_SRCDIR([src/strings.c])
#AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_CC_C99
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([limits.h locale.h stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_UINT64_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STRTOD
AC_CHECK_FUNCS([memmove strchr strdup strerror strpbrk])
AC_CHECK_FUNCS([strdup strptime snprintf])
AC_CHECK_DECLS([isnan, fmin, fmax, NAN], [], [], [[#include <math.h>]])
AC_DEFINE([_XOPEN_SOURCE], [500], ["Required for strdup and strptime"])

AC_OUTPUT(Makefile src/Makefile)
