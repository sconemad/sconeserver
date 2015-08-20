# SconeServer (http://www.sconemad.com)

AM_CPPFLAGS = -I$(top_srcdir)

if BUILD_SCONESITE
  pkglib_LTLIBRARIES = sconesite.la
endif

sconesite_la_SOURCES = \
SconesiteModule.cpp \
SconesiteStream.cpp \
Article.cpp \
Context.cpp \
Document.cpp \
Heading.cpp \
Profile.cpp \
RenderMarkup.cpp \
Template.cpp \
TemplateManager.cpp \
XMLDoc.cpp

sconesiteincludedir = $(includedir)/sconesite

sconesiteinclude_HEADERS = \
SconesiteModule.h \
SconesiteStream.h \
Article.h \
Context.h \
Document.h \
Heading.h \
Profile.h \
RenderMarkup.h \
Template.h \
TemplateManager.h \
XMLDoc.h

sconesite_la_LDFLAGS = -module -avoid-version
sconesite_la_LIBADD = -lxml2

modconfdir = $(sysconfdir)/modules.d
modconf_DATA = sconesite.conf

scsdatadir = $(datadir)/sconeserver/sconesite

tpldir = $(scsdatadir)/tpl
tpl_DATA = \
tpl/account.xml \
tpl/botdetect.xml \
tpl/common.xml \
tpl/normal.xml \
tpl/rss.xml \
tpl/shorturl.xml \
tpl/start.xml

testsitedir = $(scsdatadir)/test-site

testsitetpldir = $(testsitedir)/tpl
testsitetpl_DATA = \
test-site/tpl/common.xml

testsiteartdir = $(testsitedir)/art
testsiteart_DATA = \
test-site/art/article.xml

testsitestyledir = $(testsiteartdir)/style
testsitestyle_DATA = \
test-site/art/style/common.css \
test-site/art/style/mobile.css \
test-site/art/style/print.css

EXTRA_DIST = \
$(modconf_DATA) \
$(tpl_DATA) \
$(testsitetpl_DATA) \
$(testsiteart_DATA) \
$(testsitestyle_DATA)
