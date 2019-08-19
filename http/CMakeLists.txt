cmake_minimum_required(VERSION 2.8)
project(http)
include(../sconeserver.cmake)

set(SRCS
  AuthRealm.cpp
  AuthRealmHtpasswd.cpp
  AuthRealmDB.cpp
  Client.cpp
  ConnectionStream.cpp
  DirIndex.cpp
  GetFile.cpp
  Handler.cpp
  Host.cpp
  HostMapper.cpp
  HTTPModule.cpp
  MessageStream.cpp
  PartialResponseStream.cpp
  Request.cpp
  Response.cpp
  ResponseStream.cpp
  Session.cpp
  Status.cpp
  WebSocket.cpp)

set(HDRS
  AuthRealm.h
  AuthRealmHtpasswd.h
  AuthRealmDB.h
  Client.h
  ConnectionStream.h
  DirIndex.h
  GetFile.h
  Handler.h
  Host.h
  HostMapper.h
  http.h
  HTTPModule.h
  MessageStream.h
  PartialResponseStream.h
  Request.h
  Response.h
  ResponseStream.h
  Session.h
  Status.h
  WebSocket.h)

add_library(http MODULE ${SRCS} ${HDRS})
sconeserver_module(http)

install(FILES ${HDRS} DESTINATION ${INC_PATH}/http)
