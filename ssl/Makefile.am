# SconeServer (http://www.sconemad.com)
# automake file for ssl module

AM_CPPFLAGS = -I$(top_srcdir)

if BUILD_SSL
  pkglib_LTLIBRARIES = ssl.la
endif

ssl_la_SOURCES = \
CryptoDigests.cpp \
SSLModule.cpp \
SSLStream.cpp \
SSLChannel.cpp \
openssl-compat.c

noinst_HEADERS = \
CryptoDigests.h \
SSLModule.h \
SSLStream.h \
SSLChannel.h \
openssl-compat.h

ssl_la_LDFLAGS = -module -avoid-version
ssl_la_LIBADD = $(SSL_LIBS)

modconfdir = $(sysconfdir)/modules.d
modconf_DATA = ssl.conf

EXTRA_DIST = $(modconf_DATA)
