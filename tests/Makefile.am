LIBS = $(GDAEX_LIBS) \
       -export-dynamic

AM_CPPFLAGS = $(GDAEX_CFLAGS) \
              -I$(top_srcdir)/src \
              -DTESTSDIR="\"@abs_builddir@\""

noinst_PROGRAMS = grid \
                  query_editor \
                  select \
                  test_prefix

LDADD = $(top_builddir)/src/libgdaex.la

EXTRA_DIST = test_prefix.db \
             grid.db
