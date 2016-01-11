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
                      queryeditorentry.c \
                      queryeditorentrydate.c \
                      sqlbuilder.c

libgdaex_la_LDFLAGS = -no-undefined

libgdaex_include_HEADERS = libgdaex.h \
                           gdaex.h \
                           queryeditor.h \
                           queryeditor_widget_interface.h \
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
