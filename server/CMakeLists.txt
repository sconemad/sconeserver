cmake_minimum_required(VERSION 2.8)
project(server)
include(../sconeserver.cmake)

set(SRCS
  ConnectionChain.cpp
  ServerDatagramMultiplexer.cpp
  ServerListener.cpp
  ServerModule.cpp)

set(HDRS
  ConnectionChain.h
  ServerDatagramMultiplexer.h
  ServerListener.h
  ServerModule.h)

add_library(server MODULE ${SRCS} ${HDRS})

sconeserver_module(server)
