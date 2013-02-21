# SconeServer (http://www.sconemad.com)
# automake file for forward module

AM_CPPFLAGS = -I$(top_srcdir)

pkglib_LTLIBRARIES = forward.la

forward_la_SOURCES = \
ForwardModule.cpp

noinst_HEADERS = \
ForwardModule.h 

forward_la_LDFLAGS = -module -avoid-version

modconfdir = $(sysconfdir)/modules.d
modconf_DATA = forward.conf

EXTRA_DIST = $(modconf_DATA)
