# SconeServer (http://www.sconemad.com)

AM_CPPFLAGS = -I$(top_srcdir)

if BUILD_RSS
  pkglib_LTLIBRARIES = rss.la
endif

rss_la_SOURCES = \
Feed.cpp \
RSSModule.cpp 

rss_la_LDFLAGS = -module -avoid-version
rss_la_LIBADD = -lxml2

noinst_HEADERS = \
Feed.h \
RSSModule.h

modconfdir = $(sysconfdir)/modules.d
modconf_DATA = rss.conf

EXTRA_DIST = $(modconf_DATA)
