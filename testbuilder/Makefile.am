# SconeServer (http://www.sconemad.com)
# automake file for testbuilder module

AM_CPPFLAGS = -I$(top_srcdir)

pkglib_LTLIBRARIES = testbuilder.la

testbuilder_la_SOURCES = \
TestBuilderControlStream.cpp \
TestBuilderModule.cpp \
BuildProfile.cpp \
Build.cpp \
BuildProcessStream.cpp \
BuildStep.cpp

noinst_HEADERS = \
TestBuilderControlStream.h \
TestBuilderModule.h \
BuildProfile.h \
Build.h \
BuildProcessStream.h \
BuildStep.h

testbuilder_la_LDFLAGS = -module -avoid-version

sysconf_DATA = testbuilder.conf.example

EXTRA_DIST = $(sysconf_DATA) \
scripts/cvs \
scripts/make \
scripts/remove \
scripts/svn \
scripts/tarball
