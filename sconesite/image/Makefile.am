# SconeServer (http://www.sconemad.com)
# automake file for sconesite image module

AM_CPPFLAGS = -I$(top_srcdir)

if BUILD_SCONESITE_IMAGE
  pkglib_LTLIBRARIES = image.la
endif

image_la_SOURCES = \
ImageModule.cpp 

image_la_LDFLAGS = $(SCONESITE_IMAGE_LDFLAGS) -module -avoid-version
image_la_LIBS = $(SCONESITE_IMAGE_LIBS)

