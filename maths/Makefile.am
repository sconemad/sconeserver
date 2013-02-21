# SconeServer (http://www.sconemad.com)

AM_CPPFLAGS = -I$(top_srcdir)

if BUILD_MATHS
  pkglib_LTLIBRARIES = maths.la
endif

maths_la_SOURCES = \
MathsModule.cpp \
Num.cpp

noinst_HEADERS = \
MathsModule.h \
Num.h 

maths_la_LDFLAGS = -module -avoid-version
maths_la_LIBADD = $(MATHS_LIBADD)

modconfdir = $(sysconfdir)/modules.d
modconf_DATA = maths.conf

check_PROGRAMS = utest

utest_SOURCES = \
utest.cpp \
Num_ut.cpp 

utest_LDADD = $(MATHS_LIBADD) maths.la $(top_builddir)/sconex/libsconex.la

TESTS = utest

EXTRA_DIST = $(modconf_DATA)
