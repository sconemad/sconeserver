cmake_minimum_required(VERSION 2.8)
project(maths)
include(../sconeserver.cmake)

set(SRCS
  MathsFloat.cpp
  MathsInt.cpp
  MathsModule.cpp)
  
set(HDRS
  MathsFloat.h
  MathsInt.h
  MathsModule.h)

find_library(GMP_LIBRARIES NAMES gmp PATHS $ENV{GMPDIR} ${LIB_INSTALL_DIR})

#find_library(GMP_LIBRARY NAMES gmp)
find_path(GMP_INCLUDE_DIR NAMES gmp.h
  PATHS /usr/include)

#find_library(MPFR_LIBRARY NAMES mpfr)
find_path(MPFR_INCLUDE_DIR NAMES mpfr.h
  PATHS /usr/include)
include_directories(${MPFR_INCLUDE_DIR} ${GMP_INCLUDE_DIR})

add_library(maths MODULE ${SRCS} ${HDRS})
target_link_libraries(maths ${GMP_LIBRARY} ${MPFR_LIBRARY})

sconeserver_module(maths)

#Num_ut.cpp
#utest.cpp
