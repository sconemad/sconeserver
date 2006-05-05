# SconeServer (http://www.sconemad.com)
# automake file for mime module

AM_CPPFLAGS = -I$(top_srcdir)

pkglib_LTLIBRARIES = mime.la

mime_la_SOURCES = \
MIMEModule.cpp

noinst_HEADERS = \
MIMEModule.h

mime_la_LDFLAGS = -module -avoid-version

sysconf_DATA = mime.conf

EXTRA_DIST = $(sysconf_DATA) mime.dsp
