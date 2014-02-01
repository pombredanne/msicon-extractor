bin_PROGRAMS=msicon-extractor

msicon_extractor_SOURCES = \
	extract-common.c \
	extract-icon.c \
	extract-pe.c \
	file.c \
	iconbmp2png.c \
	main.c \
	write-icons.c 

EXTRA_DIST = \
    extract-common.h \
    extract-icon.h \
    extract-pe.h \
    file.h \
    iconbmp2png.h \
    icon-dir.h \
    pe.h \
    write-icons.h

msicon_extractor_CFLAGS = $(glib_CFLAGS) $(libpng_CFLAGS) -std=c99 -Wall -pedantic -I$(srcdir)
msicon_extractor_LDFLAGS = $(glib_LIBS) $(libpng_LIBS)

