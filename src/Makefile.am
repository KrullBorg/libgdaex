guidir = $(datadir)/$(PACKAGE)/gui

AM_CPPFLAGS = -I$(top_srcdir) \
              $(GDAEX_CFLAGS) \
              -DGUIDIR=\""$(guidir)"\" \
              -DG_LOG_DOMAIN=\"GdaEx\"

LIBS = $(GDAEX_LIBS)

lib_LTLIBRARIES = libgdaex.la

libgdaex_la_SOURCES = gdaex.c \
                      queryeditor.c \
                      grid.c \
                      gridcolumn.c

libgdaex_la_LDFLAGS = -no-undefined

libgdaex_include_HEADERS = libgdaex.h \
                           gdaex.h \
                           grid.h \
                           gridcolumn.h \
                           queryeditor.h

libgdaex_includedir = $(includedir)/libgdaex
