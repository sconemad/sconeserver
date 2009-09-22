# SconeServer (http://www.sconemad.com)
# automake file for sconesite module

AM_CPPFLAGS = -I$(top_srcdir)

pkglib_LTLIBRARIES = sconesite.la

sconesite_la_SOURCES = \
SconesiteModule.cpp \
SconesiteStream.cpp \
Article.cpp \
Context.cpp \
Profile.cpp \
RenderMarkup.cpp \
Template.cpp \
XMLDoc.cpp

noinst_HEADERS = \
SconesiteModule.h \
SconesiteStream.h \
Article.h \
Context.h \
Profile.h \
RenderMarkup.h \
Template.h \
XMLDoc.h

sconesite_la_LDFLAGS = -module -avoid-version
sconesite_la_LIBADD = -lxml2

sysconf_DATA = sconesite.conf

EXTRA_DIST = $(sysconf_DATA)
