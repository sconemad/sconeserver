cmake_minimum_required(VERSION 2.8)
project(sconex)
include(../sconeserver.cmake)

find_library(PCRE_LIBRARY NAMES pcre)

set(SRCS
  sconex.cpp
  utils.cpp
  Base64.cpp
  Buffer.cpp
  ConfigFile.cpp
  ConfigStream.cpp
  Console.cpp
  Database.cpp
  DatagramChannel.cpp
  DatagramMultiplexer.cpp
  DatagramSocket.cpp
  Date.cpp
  Debug.cpp
  Descriptor.cpp
  Digest.cpp
  File.cpp
  FileDir.cpp
  FilePath.cpp
  FileStat.cpp
  GzipStream.cpp
  Job.cpp
  Kernel.cpp
  LineBuffer.cpp
  ListenerSocket.cpp
  Log.cpp
  Logger.cpp
  MemFile.cpp
  MimeHeader.cpp
  MimeType.cpp
  Module.cpp
  ModuleLoader.cpp
  Multiplexer.cpp
  Mutex.cpp
  NullFile.cpp
  Password.cpp
  Process.cpp
  RegExp.cpp
  Response.cpp
  ScriptBase.cpp
  ScriptContext.cpp
  ScriptEngine.cpp
  ScriptExpr.cpp
  ScriptStatement.cpp
  ScriptTypes.cpp
  Socket.cpp
  SocketAddress.cpp
  Stream.cpp
  StreamBuffer.cpp
  StreamDebugger.cpp
  StreamSocket.cpp
  StreamTokenizer.cpp
  StreamTransfer.cpp
  TermBuffer.cpp
  Thread.cpp
  Time.cpp
  Uri.cpp
  User.cpp
  VersionTag.cpp)

file(GLOB HDRS *.h)

add_library(sconex SHARED ${SRCS} ${HDRS})
set_target_properties(sconex PROPERTIES
  OUTPUT_NAME "sconex"
  SOVERSION 1.0.0)
target_include_directories(sconex PRIVATE . .. ${CMAKE_CURRENT_BINARY_DIR}/..)
target_link_libraries(sconex pthread dl crypt z ${PCRE_LIBRARY})

install(TARGETS sconex DESTINATION ${LIB_PATH})
install(FILES ${HDRS} DESTINATION ${INC_PATH}/sconex)

add_executable(utest
  utest.cpp
  UnitTester.cpp
  Buffer_ut.cpp
  FilePath_ut.cpp
  LineBuffer_ut.cpp
  MemFile_ut.cpp
  MimeHeader_ut.cpp
  MimeType_ut.cpp
  Password_ut.cpp
  ScriptTypes_ut.cpp
  TimeDate_ut.cpp
  Uri_ut.cpp
  utils_ut.cpp
  VersionTag_ut.cpp)

target_link_libraries(utest sconex)
target_include_directories(utest PRIVATE . .. ${CMAKE_CURRENT_BINARY_DIR}/..)

