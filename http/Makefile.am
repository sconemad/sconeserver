# SconeServer (http://www.sconemad.com)
# automake file for http module

AM_CPPFLAGS = -I$(top_srcdir)

pkglib_LTLIBRARIES = http.la

http_la_SOURCES = \
AuthRealm.cpp \
AuthRealmHtpasswd.cpp \
AuthRealmDB.cpp \
Client.cpp \
ConnectionStream.cpp \
DirIndex.cpp \
GetFile.cpp \
Handler.cpp \
Host.cpp \
HostMapper.cpp \
HTTPModule.cpp \
MessageStream.cpp \
PartialResponseStream.cpp \
Request.cpp \
Response.cpp \
ResponseStream.cpp \
Session.cpp \
Status.cpp \
WebSocket.cpp

httpincludedir = $(includedir)/http

httpinclude_HEADERS = \
AuthRealm.h \
AuthRealmHtpasswd.h \
AuthRealmDB.h \
Client.h \
ConnectionStream.h \
DirIndex.h \
GetFile.h \
Handler.h \
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
Status.h \
WebSocket.h

http_la_LDFLAGS = -module -avoid-version

modconfdir = $(sysconfdir)/modules.d
modconf_DATA = http.conf

EXTRA_DIST = $(modconf_DATA)
