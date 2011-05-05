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

