# SconeServer (http://www.sconemad.com)
# automake file for stat module

AM_CPPFLAGS = -I$(top_srcdir)

pkglib_LTLIBRARIES = stat.la

stat_la_SOURCES = \
StatModule.cpp \
StatChannel.cpp \
StatStream.cpp

noinst_HEADERS = \
StatChannel.h \
StatModule.h \
StatStream.h

stat_la_LDFLAGS = -module -avoid-version

sysconf_DATA = stat.conf.example

EXTRA_DIST = $(sysconf_DATA)
