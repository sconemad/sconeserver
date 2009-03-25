# SconeServer (http://www.sconemad.com)
# automake file for http module

SUBDIRS = dirindex getfile errorpage

AM_CPPFLAGS = -I$(top_srcdir)

pkglib_LTLIBRARIES = http.la

http_la_SOURCES = \
ConnectionStream.cpp \
DocRoot.cpp \
HeaderTable.cpp \
Host.cpp \
HostMapper.cpp \
MessageStream.cpp \
Request.cpp \
ResponseStream.cpp \
HTTPModule.cpp \
Status.cpp

httpincludedir = $(includedir)/http

httpinclude_HEADERS = \
ConnectionStream.h \
DocRoot.h \
HeaderTable.h \
Host.h \
HostMapper.h \
Request.h \
ResponseStream.h \
MessageStream.h \
http.h \
HTTPModule.h \
Status.h

http_la_LDFLAGS = -module -avoid-version

sysconf_DATA = http.conf

EXTRA_DIST = $(sysconf_DATA)
