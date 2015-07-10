# SconeServer (http://www.sconemad.com)
# automake file for markdown module

AM_CPPFLAGS = -I$(top_srcdir) -Icmark/src -Icmark/build/src

if BUILD_MARKDOWN
  pkglib_LTLIBRARIES = markdown.la
  SUBDIRS = cmark
endif

markdown_la_SOURCES = \
MarkdownDoc.cpp \
MarkdownModule.cpp

markdown_la_LDFLAGS = -module -avoid-version
markdown_la_LIBADD = cmark/build/src/libcmark.a

modconfdir = $(sysconfdir)/modules.d
modconf_DATA = markdown.conf

EXTRA_DIST = $(modconf_DATA)
