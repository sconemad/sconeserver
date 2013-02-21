# SconeServer (http://www.sconemad.com)
# automake file for Rot13 example module

AM_CPPFLAGS = -I$(top_srcdir)

if BUILD_EXAMPLES
  pkglib_LTLIBRARIES = rot13.la
endif

rot13_la_SOURCES = \
Rot13Module.cpp \
Rot13Stream.cpp

rot13_la_LDFLAGS = -module -avoid-version

noinst_HEADERS = \
Rot13Module.h \
Rot13Stream.h

modconfdir = $(sysconfdir)/modules.d
modconf_DATA = rot13.conf

EXTRA_DIST = $(modconf_DATA) \
Makefile.example
