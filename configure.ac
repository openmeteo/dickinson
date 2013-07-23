AC_PREREQ([2.69])
AC_INIT([dickinson], [0.1], [anthony@itia.ntua.gr])
AM_INIT_AUTOMAKE(dickinson, 2.6)
AC_CONFIG_SRCDIR([src/strings.c])

# Checks for programs.
AC_PROG_CC
AC_PROG_CC_C99
AC_PROG_MAKE_SET
LT_INIT([win32-dll])

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

# Some Windows compilers (namely mingw) are lacking EOVERFLOW
AC_MSG_CHECKING([if we have EOVERFLOW defined])
AC_COMPILE_IFELSE([AC_LANG_SOURCE([
        #include <errno.h>
        int i = (EOVERFLOW) + 0;
    ])], [eoverflow_present=yes], [eoverflow_present=no])
AC_MSG_RESULT([$eoverflow_present])
AS_IF([test "x$eoverflow_present" = "xyes"], [],
      [AC_DEFINE([EOVERFLOW], [EFBIG],
                 [Use EFBIG as substitude for EOVERFLOW])])

AC_OUTPUT(Makefile src/Makefile)
