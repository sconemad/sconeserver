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

sysconf_DATA = server.conf.example

EXTRA_DIST = $(sysconf_DATA)
