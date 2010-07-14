# SconeServer (http://www.sconemad.com)
# automake file for sconesite module

AM_CPPFLAGS = -I$(top_srcdir)

if BUILD_SCONESITE
  SUBDIRS = image rss
  pkglib_LTLIBRARIES = sconesite.la
endif

sconesite_la_SOURCES = \
SconesiteModule.cpp \
SconesiteStream.cpp \
ArgFile.cpp \
Article.cpp \
ArticleBody.cpp \
Context.cpp \
Profile.cpp \
RenderMarkup.cpp \
Template.cpp \
XMLDoc.cpp

sconesiteincludedir = $(includedir)/http/sconesite

sconesiteinclude_HEADERS = \
SconesiteModule.h \
SconesiteStream.h \
ArgFile.h \
Article.h \
ArticleBody.h \
Context.h \
Profile.h \
RenderMarkup.h \
Template.h \
XMLDoc.h

sconesite_la_LDFLAGS = -module -avoid-version
sconesite_la_LIBADD = -lxml2

sysconf_DATA = sconesite.conf.example

EXTRA_DIST = $(sysconf_DATA)
