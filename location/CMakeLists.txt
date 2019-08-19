cmake_minimum_required(VERSION 2.8)
project(location)
include(../sconeserver.cmake)

find_library(LIBGPS_LIBRARY NAMES gps)

add_library(location MODULE 
  LocationModule.cpp)
target_link_libraries(location ${LIBGPS_LIBRARY})

sconeserver_module(location)
