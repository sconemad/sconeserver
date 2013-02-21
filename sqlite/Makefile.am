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

modconfdir = $(sysconfdir)/modules.d
modconf_DATA = sqlite.conf

EXTRA_DIST = $(modconf_DATA)
