# SconeServer (http://www.sconemad.com)
# automake file for test module

AM_CPPFLAGS = -I$(top_srcdir)

pkglib_LTLIBRARIES = test.la

test_la_SOURCES = \
TestModule.cpp

noinst_HEADERS = \
TestModule.h

test_la_LDFLAGS = -module -avoid-version

check_PROGRAMS = argtest
argtest_SOURCES = \
ArgTest.cpp
argtest_LDADD = $(top_builddir)/sconex/libsconex.la

EXTRA_DIST = \
test.dsp \
arg/forloop.test \
site/index.html \
site/favicon.ico \
site/echo.pl \
site/env.pl \
site/getform.pl \
site/postform.pl \
site/largetext.pl \
scripts/echo \
scripts/http-robust \
scripts/localterm \
scripts/stats \
scripts/testbuild
