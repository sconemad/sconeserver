# SconeServer (http://www.sconemad.com)
# automake file for TuesdayOnly example module

AM_CPPFLAGS = -I$(top_srcdir)

if BUILD_EXAMPLES
  pkglib_LTLIBRARIES = tuesdayonly.la
endif

tuesdayonly_la_SOURCES = \
TuesdayOnly.cpp 

tuesdayonly_la_LDFLAGS = -module -avoid-version
