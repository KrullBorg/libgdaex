guidir = $(datadir)/$(PACKAGE)/gui

AM_CPPFLAGS = -I$(top_srcdir) \
              $(GDAEX_CFLAGS) \
              -DGUIDIR=\""$(guidir)"\" \
              -DLOCALEDIR=\"$(localedir)\" \
              -DG_LOG_DOMAIN=\"GdaEx\"

LIBS = $(GDAEX_LIBS)

if LIBREPTOOL_FOUND
AM_CPPFLAGS += \
               $(REPTOOL_CFLAGS) \
               -DREPTOOL_FOUND=1

LIBS += \
        $(REPTOOL_LIBS)
endif

if LIBSOLIPA_FOUND
AM_CPPFLAGS += \
               $(SOLIPA_CFLAGS) \
               -DSOLIPA_FOUND=1

LIBS += \
        $(SOLIPA_LIBS)
endif

lib_LTLIBRARIES = libgdaex.la

libgdaex_la_SOURCES = gdaex.c \
                      grid.c \
                      gridcolumn.c \
                      queryeditor.c \
                      queryeditor_widget_interface.c \
                      queryeditorentry.c

libgdaex_la_LDFLAGS = -no-undefined

libgdaex_include_HEADERS = libgdaex.h \
                           gdaex.h \
                           grid.h \
                           gridcolumn.h \
                           queryeditor.h \
                           queryeditor_widget_interface.h \
                           queryeditorentry.h

libgdaex_includedir = $(includedir)/libgdaex
