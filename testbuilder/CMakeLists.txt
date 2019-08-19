cmake_minimum_required(VERSION 2.8)
project(testbuilder)
include(../sconeserver.cmake)

set(SRCS
  Build.cpp
  BuildProcessStream.cpp
  BuildProfile.cpp
  BuildStep.cpp
  TestBuilderControlStream.cpp
  TestBuilderModule.cpp)

set(HDRS
  Build.h
  BuildProcessStream.h
  BuildProfile.h
  BuildStep.h
  TestBuilderControlStream.h
  TestBuilderModule.h)

add_library(testbuilder MODULE ${SRCS} ${HDRS})

sconeserver_module(testbuilder)
