# SconeServer (http://www.sconemad.com)

AM_CPPFLAGS = -I$(top_srcdir)

if BUILD_SERVER
  pkglib_LTLIBRARIES = server.la
endif

server_la_SOURCES = \
ServerModule.cpp \
ConnectionChain.cpp \
ServerDatagramMultiplexer.cpp \
ServerListener.cpp

noinst_HEADERS = \
ServerModule.h \
ConnectionChain.h \
ServerDatagramMultiplexer.h \
ServerListener.h

server_la_LDFLAGS = -module -avoid-version

modconfdir = $(sysconfdir)/modules.d
modconf_DATA = server.conf

EXTRA_DIST = $(modconf_DATA)
