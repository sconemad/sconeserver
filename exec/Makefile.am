# SconeServer (http://www.sconemad.com)
# automake file for exec module

AM_CPPFLAGS = -I$(top_srcdir)

pkglib_LTLIBRARIES = exec.la

exec_la_SOURCES = \
ExecModule.cpp \
ExecStream.cpp \
CGIResponseStream.cpp

noinst_HEADERS = \
ExecModule.h \
ExecStream.h \
CGIResponseStream.h

exec_la_LDFLAGS = -module -avoid-version

#sysconf_DATA = ../etc/exec.conf

#EXTRA_DIST = exec.dsp
