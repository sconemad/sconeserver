# SconeServer (http://www.sconemad.com)
# automake file for DB SQL module

AM_CPPFLAGS = -I$(top_srcdir)

if BUILD_DBSQL
  pkglib_LTLIBRARIES = dbsql.la
endif

dbsql_la_SOURCES = \
DbSqlModule.cpp \
DbSqlProfile.cpp \
DbSqlQuery.cpp

dbsqlincludedir = $(includedir)/dbsql

dbsqlinclude_HEADERS = \
DbSqlModule.h \
DbSqlProfile.h \
DbSqlQuery.h

dbsql_la_LDFLAGS = -module -avoid-version
dbsql_la_LIBADD = $(DBSQL_LIBADD)

sysconf_DATA = dbsql.conf
EXTRA_DIST = $(sysconf_DATA)