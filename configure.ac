AC_PREREQ([2.69])
AC_INIT([Houkai], [0.1])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_SRCDIR([src/main.c])
AM_INIT_AUTOMAKE([1.9 foreign subdir-objects])
AM_SILENT_RULES([yes])

AC_LANG([C])

AM_PROG_AR
AM_PROG_AS
AC_PROG_CC
AM_PROG_CC_C_O

AC_CHECK_FUNCS([mkdir])
AC_CHECK_HEADERS([inttypes.h])
AC_CHECK_HEADERS([memory.h])
AC_CHECK_HEADERS([stddef.h])
AC_CHECK_HEADERS([stdint.h])
AC_CHECK_HEADERS([stdlib.h])
AC_CHECK_HEADERS([string.h])
AC_CHECK_HEADERS([unistd.h])
AC_FUNC_MALLOC
AC_TYPE_INT32_T
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_FILES([
        Makefile
        src/Makefile
        tests/Makefile
])

AC_OUTPUT
