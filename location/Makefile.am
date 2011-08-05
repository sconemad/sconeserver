# SconeServer (http://www.sconemad.com)

AM_CPPFLAGS = -I$(top_srcdir)

if BUILD_LOCATION
  pkglib_LTLIBRARIES = location.la
endif

location_la_SOURCES = \
LocationModule.cpp

location_la_LDFLAGS = -module -avoid-version
location_la_LIBADD = -lgps
