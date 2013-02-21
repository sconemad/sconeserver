# SconeServer (http://www.sconemad.com)
# automake file for sconeserver daemon and project

AM_CPPFLAGS = \
-I$(top_srcdir) \
-DCONF_PATH=\"$(sysconfdir)\" \
-DMOD_PATH=\"$(libdir)/sconeserver\" \
-DVAR_PATH=\"$(localstatedir)\"

bin_PROGRAMS = sconed
sconed_SOURCES = main.cpp
sconed_LDADD = $(top_builddir)/sconex/libsconex.la

MODULES = \
bluetooth \
exec \
forward \
http \
image \
ip \
ip6 \
local \
location \
maths \
mime \
mysql \
rss \
sconesite \
server \
simple \
smtp \
sqlite \
ssl \
stat \
tftp \
test \
testbuilder

EXPERIMENTAL = \
lettuce \
ui

SUBDIRS = sconex examples $(MODULES) $(EXPERIMENTAL)

docdir=$(datadir)/doc/$(PACKAGE)
doc_DATA = AUTHORS COPYING ChangeLog INSTALL NEWS README THANKS TODO

sysconf_DATA = sconeserver.conf

dist_man_MANS = sconed.1

EXTRA_DIST = \
$(sysconf_DATA) \
autogen.sh \
$(dist_man_MANS) \
debian/rules \
debian/control \
debian/changelog \
debian/postrm
