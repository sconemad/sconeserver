# SconeServer (http://www.sconemad.com)
# automake file for router bluetooth module

AM_CPPFLAGS = -I$(top_srcdir)

if BUILD_ROUTER_BLUETOOTH
  pkglib_LTLIBRARIES = bluetooth.la
endif

bluetooth_la_SOURCES = \
BluetoothModule.cpp \
BluetoothSocketAddress.cpp

noinst_HEADERS = \
BluetoothSocketAddress.h

bluetooth_la_LDFLAGS = -module -avoid-version
bluetooth_la_LIBADD = -lbluetooth

