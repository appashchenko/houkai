AC_PREREQ([2.69])
AC_INIT([Houkai], [0.1])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_SRCDIR([src/main.c])
AM_INIT_AUTOMAKE([1.9 foreign subdir-objects])
AM_SILENT_RULES([yes])

AC_LANG([C])

PKG_PROG_PKG_CONFIG

AM_PROG_AR
AM_PROG_AS
AC_PROG_CC
AC_PROG_CC_STDC
AM_PROG_CC_C_O

PKG_CHECK_MODULES(GLIB, [glib-2.0])
PKG_CHECK_MODULES(GTK, [gtk+-3.0])

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_FILES([
        Makefile
        src/Makefile
        tests/Makefile
])

AC_OUTPUT
