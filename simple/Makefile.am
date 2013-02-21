# SconeServer (http://www.sconemad.com)
# automake file for simple module

AM_CPPFLAGS = -I$(top_srcdir)

pkglib_LTLIBRARIES = simple.la

simple_la_SOURCES = \
SimpleModule.cpp \
SimpleStreams.cpp

noinst_HEADERS = \
SimpleModule.h \
SimpleStreams.h

simple_la_LDFLAGS = -module -avoid-version

modconfdir = $(sysconfdir)/modules.d
modconf_DATA = simple.conf

EXTRA_DIST = $(modconf_DATA)
