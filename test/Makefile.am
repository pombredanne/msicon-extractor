include $(top_srcdir)/build/glib-tap.mk

test_programs = \
    extractor-test

dist_test_data = \
    data

extractor_test_SOURCES = \
    test.c \
    util.c

EXTRA_DIST += \
    util.h \
    data

extractor_test_CFLAGS = \
    $(glib_CFLAGS) $(libpng_CFLAGS) \
    -std=c99 -Wall -pedantic \
    -DEXTRACTOR_BINARY='"$(top_builddir)/src/msicon-extractor"'
extractor_test_LDFLAGS = $(glib_LIBS) $(libpng_LIBS)
