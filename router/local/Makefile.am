# SconeServer (http://www.sconemad.com)
# automake file for router local module

AM_CPPFLAGS = -I$(top_srcdir)

if BUILD_ROUTER_LOCAL
  pkglib_LTLIBRARIES = local.la
endif

local_la_SOURCES = \
LocalModule.cpp \
LocalSocketAddress.cpp

noinst_HEADERS = \
LocalSocketAddress.h

local_la_LDFLAGS = -module -avoid-version

