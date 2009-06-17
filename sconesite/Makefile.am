# SconeServer (http://www.sconemad.com)
# automake file for sconesite module

AM_CPPFLAGS = -I$(top_srcdir)

pkglib_LTLIBRARIES = sconesite.la

sconesite_la_SOURCES = \
SconesiteArticle.cpp \
SconesiteStream.cpp \
SconesiteModule.cpp

noinst_HEADERS = \
SconesiteArticle.h \
SconesiteStream.h \
SconesiteModule.h

sconesite_la_LDFLAGS = -module -avoid-version

sysconf_DATA = sconesite.conf

EXTRA_DIST = $(sysconf_DATA)
