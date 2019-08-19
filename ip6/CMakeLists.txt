cmake_minimum_required(VERSION 2.8)
project(ip6)
include(../sconeserver.cmake)

add_library(ip6 MODULE 
  IP6Module.cpp
  IP6SocketAddress.cpp IP6SocketAddress.h)

sconeserver_module(ip6)
