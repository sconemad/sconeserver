cmake_minimum_required(VERSION 2.8)
project(ip)
include(../sconeserver.cmake)

add_library(ip MODULE 
  IPModule.cpp
  IPSocketAddress.cpp IPSocketAddress.h)

sconeserver_module(ip)
