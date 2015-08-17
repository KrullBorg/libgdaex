guidir = $(datadir)/$(PACKAGE)/gui

AM_CPPFLAGS = -I$(top_srcdir) \
              $(GDAEX_CFLAGS) \
              -DGUIDIR=\""$(guidir)"\" \
              -DLOCALEDIR=\"$(localedir)\" \
              -DG_LOG_DOMAIN=\"GdaEx\"

LIBS = $(GDAEX_LIBS)

lib_LTLIBRARIES = libgdaex.la

libgdaex_la_SOURCES = gdaex.c \
                      queryeditor.c \
                      queryeditor_widget_interface.c \
                      queryeditorentry.c \
                      sqlbuilder.c

libgdaex_la_LDFLAGS = -no-undefined

libgdaex_include_HEADERS = libgdaex.h \
                           gdaex.h \
                           queryeditor.h \
                           queryeditor_widget_interface.h \
                           queryeditorentry.h \
                           sqlbuilder.h

libgdaex_includedir = $(includedir)/libgdaex
