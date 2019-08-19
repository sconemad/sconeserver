cmake_minimum_required(VERSION 2.8)
project(sconesite)
include(../sconeserver.cmake)

find_library(XML2_LIBRARY NAMES xml2)
find_path(XML2_INCLUDE_DIR NAMES libxml/parser.h
  PATHS /usr/include/libxml2)
include_directories(${XML2_INCLUDE_DIR})

set(SRCS
  Article.cpp
  Context.cpp
  Document.cpp
  Heading.cpp
  Profile.cpp
  RenderMarkup.cpp
  SconesiteModule.cpp
  SconesiteStream.cpp
  Template.cpp
  TemplateManager.cpp
  XMLDoc.cpp)

set(HDRS
  Article.h
  Context.h
  Document.h
  Heading.h
  Profile.h
  RenderMarkup.h
  SconesiteModule.h
  SconesiteStream.h
  Template.h
  TemplateManager.h
  XMLDoc.h)

add_library(sconesite MODULE ${SRCS} ${HDRS})
target_link_libraries(sconesite ${XML2_LIBRARY})
sconeserver_module(sconesite)

install(FILES ${HDRS} DESTINATION ${INC_PATH}/sconesite)
install(DIRECTORY tpl DESTINATION ${DATA_PATH}/sconesite)
install(DIRECTORY test-site DESTINATION ${DATA_PATH}/sconesite)
