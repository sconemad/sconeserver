cmake_minimum_required(VERSION 2.8)
project(bluetooth)
include(../sconeserver.cmake)


find_package(PkgConfig)
pkg_check_modules(PC_LIBBLUETOOTH bluez)
find_library(LIBBLUETOOTH_LIBRARIES NAMES bluetooth
  PATHS ${PC_LIBBLUETOOTH_LIBDIR} ${PC_LIBBLUETOOTH_LIBRARY_DIRS})

add_library(bluetooth MODULE 
  BluetoothModule.cpp
  BluetoothSocketAddress.cpp BluetoothSocketAddress.h)
target_link_libraries(bluetooth ${LIBBLUETOOTH_LIBRARIES})

sconeserver_module(bluetooth)
