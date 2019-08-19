cmake_minimum_required(VERSION 2.8)
project(stat)
include(../sconeserver.cmake)

set(SRCS
  StatChannel.cpp
  StatStream.cpp
  StatModule.cpp)

set(HDRS
  StatChannel.h
  StatStream.h
  StatModule.h)

add_library(stat MODULE ${SRCS} ${HDRS})

sconeserver_module(stat)
