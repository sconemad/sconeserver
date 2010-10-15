# SconeServer (http://www.sconemad.com)
# automake file for sconeserver daemon and project

AM_CPPFLAGS = \
-I$(top_srcdir) \
-DCONF_PATH=\"$(sysconfdir)\" \
-DMOD_PATH=\"$(libdir)\" \
-DVAR_PATH=\"$(localstatedir)\"

bin_PROGRAMS = sconed
sconed_SOURCES = main.cpp
sconed_LDADD = $(top_builddir)/sconex/libsconex.la

MODULES = router http mime simple ssl stat exec forward tftp test testbuilder lettuce dbsql smtp
SUBDIRS = sconex examples $(MODULES)

docdir=$(datadir)/doc/$(PACKAGE)
doc_DATA = AUTHORS COPYING ChangeLog INSTALL NEWS README THANKS TODO

sysconf_DATA = sconeserver.conf.example

EXTRA_DIST = \
$(sysconf_DATA) \
autogen.sh \
debian/rules \
debian/control \
debian/changelog \
debian/postrm
