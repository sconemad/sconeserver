cmake_minimum_required(VERSION 2.8)
project(mime)
include(../sconeserver.cmake)

add_library(mime MODULE 
  MIMEModule.cpp)

sconeserver_module(mime)
