# SconeServer (http://www.sconemad.com)
# automake file for testbuilder module

AM_CPPFLAGS = -I$(top_srcdir)

pkglib_LTLIBRARIES = testbuilder.la

testbuilder_la_SOURCES = \
TestBuilderModule.cpp \
BuildProfile.cpp \
Build.cpp

noinst_HEADERS = \
TestBuilderModule.h \
BuildProfile.h \
Build.h

testbuilder_la_LDFLAGS = -module -avoid-version

sysconf_DATA = testbuilder.conf

EXTRA_DIST = $(sysconf_DATA) testbuilder.dsp
