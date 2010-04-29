# SconeServer (http://www.sconemad.com)
# automake file for tftp module

AM_CPPFLAGS = -I$(top_srcdir)

pkglib_LTLIBRARIES = tftp.la

tftp_la_SOURCES = \
TFTPModule.cpp \
TFTPStream.cpp \
TFTPProfile.cpp

noinst_HEADERS = \
TFTPModule.h \
TFTPStream.h \
TFTPProfile.h

tftp_la_LDFLAGS = -module -avoid-version

sysconf_DATA = tftp.conf.example

EXTRA_DIST = $(sysconf_DATA)
