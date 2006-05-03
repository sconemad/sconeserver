# SconeServer (http://www.sconemad.com)
# automake file for http dirindex module

AM_CPPFLAGS = -I$(top_srcdir)

pkglib_LTLIBRARIES = dirindex.la

dirindex_la_SOURCES = \
DirIndexModule.cpp 

dirindex_la_LDFLAGS = -module -avoid-version

EXTRA_DIST = dirindex.dsp
