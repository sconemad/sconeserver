cmake_minimum_required(VERSION 2.8)
project(simple)
include(../sconeserver.cmake)

set(SRCS
  SimpleModule.cpp
  SimpleStreams.cpp)

set(HDRS
  SimpleModule.h
  SimpleStreams.h)

add_library(simple MODULE ${SRCS} ${HDRS})

sconeserver_module(simple)
