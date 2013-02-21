# SconeServer (http://www.sconemad.com)
# automake file for test module

AM_CPPFLAGS = -I$(top_srcdir)

pkglib_LTLIBRARIES = test.la

test_la_SOURCES = \
TestModule.cpp

noinst_HEADERS = \
TestModule.h

test_la_LDFLAGS = -module -avoid-version

modconfdir = $(sysconfdir)/modules.d
modconf_DATA = test.conf

check_PROGRAMS = scones

scones_SOURCES = \
scones.cpp

scones_LDADD = $(top_builddir)/sconex/libsconex.la


TESTS = runscripttests.sh

EXTRA_DIST = $(modconf_DATA) \
runscripttests.sh \
sconescript/forloop.test \
sconescript/list.test \
sconescript/ops.test \
sconescript/regexp.test \
sconescript/short.test \
sconescript/sub.test \
sconescript/var.test \
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

DISTCLEANFILES = \
scones.log
