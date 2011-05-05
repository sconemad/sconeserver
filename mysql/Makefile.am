# SconeServer (http://www.sconemad.com)

AM_CPPFLAGS = -I$(top_srcdir)

if BUILD_MYSQL
  pkglib_LTLIBRARIES = mysql.la
endif

mysql_la_SOURCES = \
MySqlModule.cpp \
MySqlProfile.cpp \
MySqlQuery.cpp

mysqlincludedir = $(includedir)/mysql

mysqlinclude_HEADERS = \
MySqlModule.h \
MySqlProfile.h \
MySqlQuery.h

mysql_la_LDFLAGS = -module -avoid-version
mysql_la_LIBADD = $(MYSQL_LIBADD)

sysconf_DATA = mysql.conf.example
EXTRA_DIST = $(sysconf_DATA)