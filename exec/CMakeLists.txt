cmake_minimum_required(VERSION 2.8)
project(exec)
include(../sconeserver.cmake)

add_library(exec MODULE 
  ExecModule.cpp ExecModule.h
  ExecStream.cpp ExecModule.h
  CGIResponseStream.cpp CGIResponseStream.h)
sconeserver_module(exec)
