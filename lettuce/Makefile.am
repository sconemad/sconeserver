# SconeServer (http://www.sconemad.com)
# automake file for lettuce module

AM_CPPFLAGS = -I$(top_srcdir)

pkglib_LTLIBRARIES = lettuce.la

lettuce_la_SOURCES = \
LettuceModule.cpp \
LettuceCommandStream.cpp \
LettuceMediaStream.cpp

lettuce_la_LDFLAGS = -module -avoid-version

noinst_HEADERS = \
LettuceModule.h \
LettuceCommandStream.h \
LettuceMediaStream.h
