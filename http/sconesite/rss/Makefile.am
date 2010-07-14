# SconeServer (http://www.sconemad.com)
# automake file for sconesite rss module

AM_CPPFLAGS = -I$(top_srcdir)

if BUILD_SCONESITE_RSS
  pkglib_LTLIBRARIES = rss.la
endif

rss_la_SOURCES = \
Feed.cpp \
RSSModule.cpp 

rss_la_LDFLAGS = -module -avoid-version
rss_la_LIBADD = -lxml2


