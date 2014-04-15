AM_CFLAGS  = -I$(top_builddir)/include -I$(top_builddir)/lib 

if STATIC_BUILD
noinst_LTLIBRARIES    = libmodsmart.la
libmodsmart_la_SOURCES = mod_smart.c 
else
pkglib_LTLIBRARIES    = modsmart.la
modsmart_la_SOURCES = mod_smart.c 
modsmart_la_LDFLAGS = -module -avoid-version

EXTRA_DIST = ../conf.d/smart.conf
endif

INCLUDES = @APR_INCLUDES@

