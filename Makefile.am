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

MODULES = router http mime simple ssl stat exec forward tftp test testbuilder lettuce sconesite dbsql
SUBDIRS = sconex doc $(MODULES)

docdir=$(datadir)/doc/$(PACKAGE)-$(VERSION)
doc_DATA = AUTHORS COPYING INSTALL NEWS README THANKS TODO

sysconf_DATA = sconeserver.conf.example

EXTRA_DIST = \
$(sysconf_DATA) \
autogen.sh
