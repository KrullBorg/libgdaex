guidir = $(datadir)/$(PACKAGE)/gui

AM_CPPFLAGS = -I$(top_srcdir) \
              $(GDAEX_CFLAGS) \
              -DGUIDIR=\""$(guidir)"\" \
              -DLOCALEDIR=\"$(localedir)\" \
              -DMODULESDIR=\""$(libdir)/$(PACKAGE)/modules"\" \
              -DG_LOG_DOMAIN=\"GdaEx\"

LIBS = $(GDAEX_LIBS)

lib_LTLIBRARIES = libgdaex.la

queryeditormarshal.c: queryeditormarshal.h queryeditor_marshal.list $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $(srcdir)/queryeditor_marshal.list --body --prefix=_gdaex_query_editor_marshal > $(srcdir)/$@

queryeditormarshal.h: queryeditor_marshal.list $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $(srcdir)/queryeditor_marshal.list --header --prefix=_gdaex_query_editor_marshal > $(srcdir)/$@

libgdaex_la_SOURCES = gdaex.c \
                      queryeditormarshal.c \
                      queryeditor.c \
                      queryeditor_widget_interface.c \
                      queryeditorcheck.c \
                      queryeditorentry.c \
                      queryeditorentrydate.c \
                      sqlbuilder.c

libgdaex_la_LDFLAGS = -no-undefined

libgdaex_include_HEADERS = libgdaex.h \
                           gdaex.h \
                           queryeditor.h \
                           queryeditor_widget_interface.h \
                           queryeditorcheck.h \
                           queryeditorentry.h \
                           queryeditorentrydate.h \
                           sqlbuilder.h

libgdaex_includedir = $(includedir)/libgdaex

noinst_HEADERS = \
                 queryeditormarshal.h

install-exec-hook:
	mkdir -p "$(libdir)/$(PACKAGE)/modules"

uninstall-hook:
	rm -rf "$(libdir)/$(PACKAGE)"

CLEANFILES =

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(srcdir) --warn-all --accept-unprefixed --identifier-prefix=""
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir)

if HAVE_INTROSPECTION
introspection_sources = $(libgdaex_la_SOURCES) $(libgdaex_include_HEADERS)

Gdaex-1.0.gir: libgdaex.la
Gdaex_1_0_gir_INCLUDES = GObject-2.0 GLib-2.0 Gda-5.0
Gdaex_1_0_gir_CFLAGS = $(AM_CPPFLAGS)
Gdaex_1_0_gir_LIBS = libgdaex.la
Gdaex_1_0_gir_FILES = $(introspection_sources)
INTROSPECTION_GIRS += Gdaex-1.0.gir

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(gir_DATA) $(typelib_DATA)
endif
