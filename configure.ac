AC_PREREQ([2.69])
AC_INIT([Houkai], [0.1])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_SRCDIR([src])
AM_INIT_AUTOMAKE([1.9 foreign subdir-objects])

AC_LANG([C])

# Checks for programs.
AM_PROG_AR
AM_PROG_AS

AC_PROG_CC
AC_PROG_CC_STDC

AM_PROG_CC_C_O

AM_PATH_GTK_2_0([2.10.0],,AC_MSG_ERROR([Gtk+ 2.10.0 or higher required.]))

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_FILES([
        Makefile
        src/Makefile
        tests/Makefile
])

AC_OUTPUT
