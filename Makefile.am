# SconeServer (http://www.sconemad.com)
# automake file for sconeserver daemon and project

AM_CPPFLAGS = \
-I$(top_srcdir) \
-DCONF_PATH=\"$(sysconfdir)\" \
-DMOD_PATH=\"$(pkglibdir)\" \
-DVAR_PATH=\"$(localstatedir)\"

bin_PROGRAMS = sconed
sconed_SOURCES = main.cpp
sconed_LDADD = $(top_builddir)/sconex/libsconex.la

noinst_HEADERS = \
res/resource.h

MODULES = router http mime simple ssl stat exec test testbuilder
SUBDIRS = sconex doc $(MODULES)

docdir=$(datadir)/doc/$(PACKAGE)-$(VERSION)
doc_DATA = AUTHORS COPYING INSTALL NEWS README THANKS TODO

sysconf_DATA = sconeserver.conf

EXTRA_DIST = \
$(sysconf_DATA) \
autogen.sh \
sconeserver.dsw \
sconed.dsp \
config_win.h \
res/sconeserver.rc \
res/app.ico 