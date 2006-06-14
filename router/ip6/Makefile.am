# SconeServer (http://www.sconemad.com)
# automake file for router ip module

AM_CPPFLAGS = -I$(top_srcdir)

if BUILD_ROUTER_IP6
  pkglib_LTLIBRARIES = ip6.la
endif

ip6_la_SOURCES = \
IP6Module.cpp \
IP6SocketAddress.cpp

noinst_HEADERS = \
IP6SocketAddress.h

ip6_la_LDFLAGS = -module -avoid-version

