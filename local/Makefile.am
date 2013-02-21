# SconeServer (http://www.sconemad.com)

AM_CPPFLAGS = -I$(top_srcdir)

if BUILD_LOCAL
  pkglib_LTLIBRARIES = local.la
endif

local_la_SOURCES = \
LocalModule.cpp \
LocalSocketAddress.cpp

noinst_HEADERS = \
LocalSocketAddress.h

local_la_LDFLAGS = -module -avoid-version

modconfdir = $(sysconfdir)/modules.d
modconf_DATA = local.conf

EXTRA_DIST = $(modconf_DATA)
