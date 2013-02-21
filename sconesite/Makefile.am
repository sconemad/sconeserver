# SconeServer (http://www.sconemad.com)

AM_CPPFLAGS = -I$(top_srcdir)

if BUILD_SCONESITE
  pkglib_LTLIBRARIES = sconesite.la
endif

sconesite_la_SOURCES = \
SconesiteModule.cpp \
SconesiteStream.cpp \
Article.cpp \
ArticleHeading.cpp \
Context.cpp \
Profile.cpp \
RenderMarkup.cpp \
Template.cpp \
WikiTextArticleBody.cpp \
XMLArticleBody.cpp \
XMLDoc.cpp

sconesiteincludedir = $(includedir)/sconesite

sconesiteinclude_HEADERS = \
SconesiteModule.h \
SconesiteStream.h \
Article.h \
ArticleBody.h \
ArticleHeading.h \
Context.h \
Profile.h \
RenderMarkup.h \
Template.h \
WikiTextArticleBody.h \
XMLArticleBody.h \
XMLDoc.h

sconesite_la_LDFLAGS = -module -avoid-version
sconesite_la_LIBADD = -lxml2

modconfdir = $(sysconfdir)/modules.d
modconf_DATA = sconesite.conf

EXTRA_DIST = $(modconf_DATA)
