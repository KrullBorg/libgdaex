LIBS = $(GDAEX_LIBS) \
       -export-dynamic

AM_CPPFLAGS = $(GDAEX_CFLAGS) \
              -I$(top_srcdir)/src \
              -DTESTSDIR="\"@abs_builddir@\""

noinst_PROGRAMS = fill_liststore \
                  getsql \
                  query_editor \
                  select \
                  sqlbuilder \
                  test_prefix

LDADD = $(top_builddir)/src/libgdaex.la

EXTRA_DIST = test_prefix.db
