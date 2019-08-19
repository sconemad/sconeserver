cmake_minimum_required(VERSION 2.8)
include(../sconeserver.cmake)
project(markdown)

# This may be a slight hack...
set(CMARK_TESTS OFF CACHE BOOL "Build cmark tests and enable testing")
set(CMARK_SHARED OFF CACHE BOOL "Build shared libcmark library")
add_subdirectory(cmark)
file(COPY ${CMAKE_CURRENT_BINARY_DIR}/cmark/src/cmark_export.h
  DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_BINARY_DIR}/cmark/src/cmark_version.h
  DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_library(markdown MODULE 
  MarkdownDoc.cpp
  MarkdownModule.cpp)
include_directories(markdown ${CMAKE_CURRENT_SOURCE_DIR}/cmark/src)
target_link_libraries(markdown libcmark_static)

sconeserver_module(markdown)
