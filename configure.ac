#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([libgdaex], [0.4.0], [azagli@libero.it])
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
AC_PROG_RANLIB

GTK_DOC_CHECK(1.0)
IT_PROG_INTLTOOL

dnl ******************************
dnl Translations
dnl ******************************
GETTEXT_PACKAGE=libgdaex
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE",
                   [The prefix for our gettext translation domains.])
AM_GLIB_GNU_GETTEXT

# Checks for libraries.
PKG_CHECK_MODULES(GDAEX, [gmodule-2.0 >= 2
                          libgda-4.0 >= 4
                          gio-2.0 >= 2.24
                          gtk+-2.0 >= 2.20])

AC_SUBST(GDAEX_CFLAGS)
AC_SUBST(GDAEX_LIBS)

PKG_CHECK_EXISTS(libreptool >= 0.2.2, [LIBREPTOOL_FOUND=yes], [LIBREPTOOL_FOUND=no])

AM_CONDITIONAL(LIBREPTOOL_FOUND, test $LIBREPTOOL_FOUND = yes)

if test $LIBREPTOOL_FOUND = yes; then
PKG_CHECK_MODULES(REPTOOL, libreptool >= 0.2.2)

AC_SUBST(REPTOOL_CFLAGS)
AC_SUBST(REPTOOL_LIBS)
fi

PKG_CHECK_EXISTS(libsolipa >= 0.4.0, [LIBSOLIPA_FOUND=yes], [LIBSOLIPA_FOUND=no])

AM_CONDITIONAL(LIBSOLIPA_FOUND, test $LIBSOLIPA_FOUND = yes)

if test $LIBSOLIPA_FOUND = yes; then
PKG_CHECK_MODULES(SOLIPA, libsolipa >= 0.4.0)

AC_SUBST(SOLIPA_CFLAGS)
AC_SUBST(SOLIPA_LIBS)
fi

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for library functions.
AC_CHECK_FUNCS([strchr strcasecmp strtol])

AC_CONFIG_FILES([
	libgdaex.pc
	Makefile
	src/Makefile
	data/Makefile
	data/libgdaex/Makefile
	data/libgdaex/gui/Makefile
	docs/Makefile
	docs/reference/Makefile
	docs/reference/version.xml
	tests/Makefile
	po/Makefile.in
])
AC_OUTPUT
