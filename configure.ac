#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([infinisql], [0.1.0alpha], [mtravis15432+src@gmail.com])
AM_INIT_AUTOMAKE
LT_INIT
AC_CONFIG_SRCDIR([infinisqld/infinisql_defs.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile infinisqld/Makefile infinisqld/version.h])

# Checks for programs.
: ${CXXFLAGS=""}
AC_PROG_CXX
AC_PROG_CC
AC_PROG_YACC
AC_PROG_LEX

# Checks for libraries.
AC_CHECK_LIB([dl], [main])
AC_CHECK_LIB([rt], [main])
AC_CHECK_LIB([pthread], [main])
AC_CHECK_LIB([cryptopp], [main])
AC_CHECK_LIB([pcrecpp], [main])
AC_CHECK_LIB([pcre], [main])
AC_CHECK_LIB([zmq], [main])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h memory.h netdb.h stddef.h stdint.h stdlib.h string.h sys/time.h sys/socket.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT64_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([gettimeofday memset select socket])

AC_OUTPUT
AC_PROG_MKDIR_P

YACC=bison
LEX=flex
