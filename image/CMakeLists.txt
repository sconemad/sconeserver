cmake_minimum_required(VERSION 2.8)
project(image)
include(../sconeserver.cmake)

find_package(ImageMagick COMPONENTS Magick++)
include_directories(${ImageMagick_INCLUDE_DIRS})

add_library(image MODULE 
  ImageModule.cpp)
target_link_libraries(image ${ImageMagick_LIBRARIES})

sconeserver_module(image)
