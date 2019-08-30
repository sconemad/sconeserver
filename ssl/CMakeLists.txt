cmake_minimum_required(VERSION 2.8)
project(ssl)
include(../sconeserver.cmake)
include(FindOpenSSL)

set(SRCS
  CryptoDigests.cpp
  openssl-compat.c
  SSLChannel.cpp 
  SSLModule.cpp 
  SSLStream.cpp)

set(HDRS
  CryptoDigests.h
  openssl-compat.h
  SSLChannel.h
  SSLModule.h 
  SSLStream.h) 

include_directories(${OPENSSL_INCLUDE_DIR})

add_library(ssl MODULE ${SRCS} ${HDRS})
target_link_libraries(ssl ${OPENSSL_LIBRARIES})

sconeserver_module(ssl)
