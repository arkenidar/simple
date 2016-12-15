#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([simple], [1.0.0], [dario.cangialosi@gmail.com])
AC_CONFIG_SRCDIR([simple.c])
AC_CONFIG_HEADERS([config.h])

# automake
AM_INIT_AUTOMAKE([foreign subdir-objects])
AC_OUTPUT(Makefile)

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdint.h stdlib.h string.h termios.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset])

AC_CONFIG_FILES([makefile])
AC_OUTPUT
