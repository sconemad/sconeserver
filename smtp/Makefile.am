# SconeServer (http://www.sconemad.com)
# automake file for smtp module

AM_CPPFLAGS = -I$(top_srcdir)

pkglib_LTLIBRARIES = smtp.la

smtp_la_SOURCES = \
SMTPClient.cpp \
SMTPModule.cpp

noinst_HEADERS = \
SMTPClient.h \
SMTPModule.h

smtp_la_LDFLAGS = -module -avoid-version

modconfdir = $(sysconfdir)/modules.d
modconf_DATA = smtp.conf

EXTRA_DIST = $(modconf_DATA)
