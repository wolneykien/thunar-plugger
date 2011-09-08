#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_COPYRIGHT([Copyright (c) 2011 Paul Wolneykien <manowar@altlinux.org>])
AC_INIT(thunar-plugger, 1.0, manowar@altlinux.org)
AC_CONFIG_SRCDIR([src/thunar-plugger.c])
AC_PREREQ(2.59)
AC_CANONICAL_TARGET()

AM_INIT_AUTOMAKE(thunar-plugger, 1.0)
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AM_PROG_CC_C_O
AC_PROG_LD
AC_PROG_INSTALL
AC_PROG_LIBTOOL

XDT_CHECK_PACKAGE([GTK], [gtk+-2.0], [2.12.0], [
  if test -z "$GTK_CFLAGS"; then
    echo "Some prerequisites for gtk+-2.0 were not found"
    exit 1
  fi
])
XDT_CHECK_PACKAGE([GLIB], [glib-2.0], [2.18.0], [
  if test -z "$GTK_CFLAGS"; then
    echo "Some prerequisites for glib-2.0 were not found"
    exit 1
  fi
])
XDT_CHECK_PACKAGE([THUNARX], [thunarx-2], [1.0.1], [
  if test -z "$GTK_CFLAGS"; then
    echo "Some prerequisites for thunarx-2 were not found"
    exit 1
  fi
])

AC_CONFIG_FILES([Makefile
                 src/Makefile])

AC_OUTPUT
