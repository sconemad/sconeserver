cmake_minimum_required(VERSION 2.8)
project(tuesdayonly)
include(../../sconeserver.cmake)

add_library(tuesdayonly MODULE TuesdayOnly.cpp)

sconeserver_module(tuesdayonly)
