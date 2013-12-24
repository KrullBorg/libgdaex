LIBS = $(GDAEX_LIBS) \
       -export-dynamic

AM_CPPFLAGS = $(GDAEX_CFLAGS) \
              -I$(top_srcdir)/src \
              -DTESTSDIR="\"@abs_builddir@\""

if LIBSOLIPA_FOUND
AM_CPPFLAGS += \
               $(SOLIPA_CFLAGS) \
               -DSOLIPA_FOUND=1

LIBS += \
        $(SOLIPA_LIBS)
endif

noinst_PROGRAMS = fill_liststore \
                  getsql \
                  grid \
                  query_editor \
                  select \
                  test_prefix

LDADD = $(top_builddir)/src/libgdaex.la

EXTRA_DIST = test_prefix.db \
             grid.db
