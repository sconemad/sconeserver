cmake_minimum_required(VERSION 2.8)
project(test)
include(../sconeserver.cmake)

set(SRCS
  TestModule.cpp)

set(HDRS
  TestModule.h)

add_library(test MODULE ${SRCS} ${HDRS})

sconeserver_module(test)
