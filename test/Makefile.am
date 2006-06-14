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
arg/forloop.test \
site/html/index.html \
site/html/favicon.ico \
site/html/echo.pl \
site/html/env.pl \
site/html/getform.pl \
site/html/postform.pl \
site/html/largetext.pl \
site/html_secure/index.html \
scripts/echo \
scripts/http-robust \
scripts/localterm \
scripts/stats \
scripts/testbuild
