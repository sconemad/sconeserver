cmake_minimum_required(VERSION 2.8)
project(sqlite)
include(../sconeserver.cmake)

set(SRCS
  SQLiteModule.cpp 
  SQLiteProfile.cpp 
  SQLiteQuery.cpp) 

set(HDRS
  SQLiteModule.h
  SQLiteProfile.h
  SQLiteQuery.h)

find_library(SQLITE_LIBRARY NAMES sqlite3)
find_path(SQLITE_INCLUDE_DIR NAMES sqlite3.h
  PATHS /usr/include)
include_directories(${SQLITE_INCLUDE_DIR})

add_library(sqlite MODULE ${SRCS} ${HDRS})
target_link_libraries(sqlite ${SQLITE_LIBRARY})

sconeserver_module(sqlite)
