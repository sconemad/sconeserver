# SconeServer (http://www.sconemad.com)
# automake file for image module

AM_CPPFLAGS = -I$(top_srcdir)

if BUILD_IMAGE
  pkglib_LTLIBRARIES = image.la
endif

image_la_SOURCES = \
ImageModule.cpp 

image_la_CXXFLAGS = $(IMAGE_CFLAGS)
image_la_LDFLAGS = $(IMAGE_LDFLAGS) -module -avoid-version
image_la_LIBADD = $(IMAGE_LIBS)

modconfdir = $(sysconfdir)/modules.d
modconf_DATA = image.conf

EXTRA_DIST = $(modconf_DATA)
