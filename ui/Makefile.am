# SconeServer (http://www.sconemad.com)

AM_CPPFLAGS = -I$(top_srcdir)

if BUILD_UI
  pkglib_LTLIBRARIES = ui.la
endif

ui_la_SOURCES = \
UIModule.cpp 

noinst_HEADERS = \
UIModule.h

ui_la_LDFLAGS = -module -avoid-version
ui_la_LIBADD = $(UI_LIBADD)
