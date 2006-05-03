# SconeServer (http://www.sconemad.com)
# automake file for router ip module

AM_CPPFLAGS = -I$(top_srcdir)

if BUILD_ROUTER_IP
  pkglib_LTLIBRARIES = ip.la
endif

ip_la_SOURCES = \
IPModule.cpp \
IPSocketAddress.cpp

noinst_HEADERS = \
IPSocketAddress.h

ip_la_LDFLAGS = -module -avoid-version

EXTRA_DIST = ip.dsp
