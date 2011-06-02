# SconeServer (http://www.sconemad.com)
# automake file for http module

SUBDIRS = dirindex getfile errorpage

AM_CPPFLAGS = -I$(top_srcdir)

pkglib_LTLIBRARIES = http.la

http_la_SOURCES = \
AuthRealm.cpp \
AuthRealmHtpasswd.cpp \
AuthRealmDB.cpp \
Client.cpp \
ConnectionStream.cpp \
DocRoot.cpp \
Host.cpp \
HostMapper.cpp \
HTTPModule.cpp \
MessageStream.cpp \
PartialResponseStream.cpp \
Request.cpp \
Response.cpp \
ResponseStream.cpp \
Session.cpp \
Status.cpp

httpincludedir = $(includedir)/sconex/http

httpinclude_HEADERS = \
AuthRealm.h \
AuthRealmHtpasswd.h \
AuthRealmDB.h \
Client.h \
ConnectionStream.h \
DocRoot.h \
Host.h \
HostMapper.h \
http.h \
HTTPModule.h \
MessageStream.h \
PartialResponseStream.h \
Request.h \
Response.h \
ResponseStream.h \
Session.h \
Status.h

http_la_LDFLAGS = -module -avoid-version

sysconf_DATA = http.conf.example

EXTRA_DIST = $(sysconf_DATA)
