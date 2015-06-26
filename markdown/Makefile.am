# SconeServer (http://www.sconemad.com)
# automake file for markdown module

AM_CPPFLAGS = -I$(top_srcdir) $(MARKDOWN_CFLAGS)

if BUILD_MARKDOWN
  pkglib_LTLIBRARIES = markdown.la
endif

markdown_la_SOURCES = \
MarkdownDoc.cpp \
MarkdownModule.cpp

markdown_la_LDFLAGS = $(MARKDOWN_LDFLAGS) -module -avoid-version
markdown_la_LIBADD = $(MARKDOWN_LIBS)

modconfdir = $(sysconfdir)/modules.d
modconf_DATA = markdown.conf

EXTRA_DIST = $(modconf_DATA)
