cmake_minimum_required(VERSION 2.8)
project(rot13)
include(../../sconeserver.cmake)

add_library(rot13 MODULE 
  Rot13Module.cpp Rot13Module.h
  Rot13Stream.cpp Rot13Stream.h)

sconeserver_module(rot13)
