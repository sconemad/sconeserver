# SconeServer (http://www.sconemad.com)

AM_CPPFLAGS = -I$(top_srcdir)

if BUILD_IP
  pkglib_LTLIBRARIES = ip.la
endif

ip_la_SOURCES = \
IPModule.cpp \
IPSocketAddress.cpp

noinst_HEADERS = \
IPSocketAddress.h

ip_la_LDFLAGS = -module -avoid-version

modconfdir = $(sysconfdir)/modules.d
modconf_DATA = ip.conf

EXTRA_DIST = $(modconf_DATA)
