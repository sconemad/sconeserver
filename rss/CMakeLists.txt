cmake_minimum_required(VERSION 2.8)
project(rss)
include(../sconeserver.cmake)

find_library(XML2_LIBRARY NAMES xml2)
find_path(XML2_INCLUDE_DIR NAMES libxml/parser.h
  PATHS /usr/include/libxml2)
include_directories(${XML2_INCLUDE_DIR})

add_library(rss MODULE 
  RSSModule.cpp RSSModule.h
  Feed.cpp Feed.h)
target_link_libraries(rss ${XML2_LIBRARY})

sconeserver_module(rss)
