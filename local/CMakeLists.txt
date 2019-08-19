cmake_minimum_required(VERSION 2.8)
project(local)
include(../sconeserver.cmake)

add_library(local MODULE 
  LocalModule.cpp
  LocalSocketAddress.cpp LocalSocketAddress.h)

sconeserver_module(local)
