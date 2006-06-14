# SconeServer (http://www.sconemad.com)
# automake file for router module

SUBDIRS = ip ip6 local bluetooth

AM_CPPFLAGS = -I$(top_srcdir)

pkglib_LTLIBRARIES = router.la

router_la_SOURCES = \
RouterModule.cpp \
RouterChain.cpp \
RouterListener.cpp

noinst_HEADERS = \
RouterModule.h \
RouterChain.h \
RouterListener.h

router_la_LDFLAGS = -module -avoid-version

sysconf_DATA = router.conf

EXTRA_DIST = $(sysconf_DATA)
