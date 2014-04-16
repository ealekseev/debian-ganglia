AM_CFLAGS  = -D_LARGEFILE64_SOURCE -I$(top_builddir)/include -I$(top_builddir)/libmetrics -I$(top_builddir)/lib 

if STATIC_BUILD
noinst_LTLIBRARIES    = libmodsmart.la
libmodsmart_la_SOURCES = mod_smart.c
libmodsmart_la_LDFLAGS = -export-all-symbols
else
pkglib_LTLIBRARIES    = modsmart.la

modsmart_la_SOURCES = mod_smart.c 
modsmart_la_LDFLAGS = -module -avoid-version
EXTRA_DIST = ../conf.d/smart.conf
modsmart_la_LIBADD = $(top_builddir)/libmetrics/libmetrics.la
endif

INCLUDES = @APR_INCLUDES@
