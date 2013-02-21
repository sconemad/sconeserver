# SconeServer (http://www.sconemad.com)

AM_CPPFLAGS = -I$(top_srcdir)

if BUILD_MYSQL
  pkglib_LTLIBRARIES = mysql.la
endif

mysql_la_SOURCES = \
MySqlModule.cpp \
MySqlProfile.cpp \
MySqlQuery.cpp

noinst_HEADERS = \
MySqlModule.h \
MySqlProfile.h \
MySqlQuery.h

mysql_la_LDFLAGS = -module -avoid-version
mysql_la_LIBADD = $(MYSQL_LIBADD)

modconfdir = $(sysconfdir)/modules.d
modconf_DATA = mysql.conf

EXTRA_DIST = $(modconf_DATA)
