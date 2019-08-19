cmake_minimum_required(VERSION 2.8)
project(mysql)
include(../sconeserver.cmake)

find_library(MYSQLCLIENT_LIBRARY NAMES mysqlclient)
find_path(MYSQL_INCLUDE_DIR NAMES mysql.h
  PATHS /usr/include/mysql)
include_directories(${MYSQL_INCLUDE_DIR})

add_library(mysql MODULE 
  MySqlModule.cpp MySqlModule.h
  MySqlProfile.cpp MySqlProfile.h
  MySqlQuery.cpp MySqlQuery.h)
target_link_libraries(mysql ${MYSQLCLIENT_LIBRARY})

sconeserver_module(mysql)
