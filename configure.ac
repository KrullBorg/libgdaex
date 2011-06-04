#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([libgdaex], [0.2.2], [azagli@libero.it])
AC_CONFIG_SRCDIR([src/gdaex.c])
AM_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE(-Wall)
AM_MAINTAINER_MODE

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AC_LIBTOOL_WIN32_DLL

AC_CANONICAL_SYSTEM

AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL

GTK_DOC_CHECK(1.0)

# Checks for libraries.
PKG_CHECK_MODULES(GDAEX, [libgda-4.0 >= 4
                          gio-2.0 >= 2.24
                          gtk+-2.0 >= 2.20])

AC_SUBST(GDAEX_CFLAGS)
AC_SUBST(GDAEX_LIBS)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for library functions.
AC_CHECK_FUNCS([strchr strcasecmp])

AC_CONFIG_FILES([
  libgdaex.pc
  Makefile
  src/Makefile
  docs/Makefile
  docs/reference/Makefile
  docs/reference/version.xml
  tests/Makefile
])
AC_OUTPUT
