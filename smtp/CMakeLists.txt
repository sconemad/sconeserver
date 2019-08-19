cmake_minimum_required(VERSION 2.8)
project(smtp)
include(../sconeserver.cmake)

set(SRCS
  SMTPClient.cpp
  SMTPModule.cpp)

set(HDRS
  SMTPClient.h
  SMTPModule.h)

add_library(smtp MODULE ${SRCS} ${HDRS})

sconeserver_module(smtp)
