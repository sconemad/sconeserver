# SconeServer (http://www.sconemad.com)
# automake file for http module

SUBDIRS = dirindex getfile errorpage

AM_CPPFLAGS = -I$(top_srcdir)

pkglib_LTLIBRARIES = http.la

http_la_SOURCES = \
AuthRealm.cpp \
ConnectionStream.cpp \
DocRoot.cpp \
Host.cpp \
HostMapper.cpp \
HTTPModule.cpp \
MessageStream.cpp \
Request.cpp \
Response.cpp \
ResponseStream.cpp \
Session.cpp \
Status.cpp

httpincludedir = $(includedir)/http

httpinclude_HEADERS = \
AuthRealm.h \
ConnectionStream.h \
DocRoot.h \
Host.h \
HostMapper.h \
http.h \
HTTPModule.h \
MessageStream.h \
Request.h \
Response.h \
ResponseStream.h \
Session.h \
Status.h

http_la_LDFLAGS = -module -avoid-version

sysconf_DATA = http.conf

EXTRA_DIST = $(sysconf_DATA)
