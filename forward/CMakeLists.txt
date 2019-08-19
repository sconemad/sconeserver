cmake_minimum_required(VERSION 2.8)
project(forward)
include(../sconeserver.cmake)

add_library(forward MODULE 
  ForwardModule.cpp ForwardModule.h)

sconeserver_module(forward)
