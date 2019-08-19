cmake_minimum_required(VERSION 2.8)
project(tftp)
include(../sconeserver.cmake)

set(SRCS
  TFTPModule.cpp
  TFTPProfile.cpp
  TFTPStream.cpp)

set(HDRS
  TFTPModule.h
  TFTPProfile.h
  TFTPStream.h)

add_library(tftp MODULE ${SRCS} ${HDRS})

sconeserver_module(tftp)
