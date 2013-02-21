# SconeServer (http://www.sconemad.com)
# automake file for ssl module

AM_CPPFLAGS = -I$(top_srcdir)

if BUILD_SSL
  pkglib_LTLIBRARIES = ssl.la
endif

ssl_la_SOURCES = \
SSLModule.cpp \
SSLStream.cpp \
SSLChannel.cpp

noinst_HEADERS = \
SSLModule.h \
SSLStream.h \
SSLChannel.h

ssl_la_LDFLAGS = -module -avoid-version
ssl_la_LIBADD = $(SSL_LIBADD)

modconfdir = $(sysconfdir)/modules.d
modconf_DATA = ssl.conf

EXTRA_DIST = $(modconf_DATA)
