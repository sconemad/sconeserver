# SconeServer (http://www.sconemad.com)

AM_CPPFLAGS = -I$(top_srcdir)

if BUILD_SQLITE
  pkglib_LTLIBRARIES = sqlite.la
endif

sqlite_la_SOURCES = \
SQLiteModule.cpp \
SQLiteProfile.cpp \
SQLiteQuery.cpp

noinst_HEADERS = \
SQLiteModule.h \
SQLiteProfile.h \
SQLiteQuery.h

sqlite_la_LDFLAGS = -module -avoid-version
sqlite_la_LIBADD = $(SQLITE_LIBADD)

sysconf_DATA = sqlite.conf.example
EXTRA_DIST = $(sysconf_DATA)