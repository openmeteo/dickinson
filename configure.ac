AC_INIT([dickinson], [0.1], [anthony@itia.ntua.gr])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
AC_PROG_CC_C99
AC_PROG_LIBTOOL
AC_CONFIG_FILES([Makefile])
AC_CHECK_FUNCS([strdup strptime snprintf])
AC_CHECK_DECLS([isnan, fmin, fmax, NAN], [], [], [[#include <math.h>]])
AC_DEFINE([_XOPEN_SOURCE], [500], ["Required for strdup and strptime"])
AC_OUTPUT
