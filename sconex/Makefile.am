# SconeServer (http://www.sconemad.com)
# automake file for sconex library

AM_CPPFLAGS = -I$(top_srcdir)

lib_LTLIBRARIES = libsconex.la

libsconex_la_SOURCES = \
sconex.cpp \
utils.cpp \
Arg.cpp \
ArgObject.cpp \
ArgProc.cpp \
ArgScript.cpp \
ArgStatement.cpp \
Buffer.cpp \
ConfigFile.cpp \
ConfigStream.cpp \
Console.cpp \
Date.cpp \
Debug.cpp \
Descriptor.cpp \
DescriptorThread.cpp \
File.cpp \
FileDir.cpp \
FilePath.cpp \
FileStat.cpp \
Kernel.cpp \
LineBuffer.cpp \
ListenerSocket.cpp \
Logger.cpp \
MemFile.cpp \
MimeType.cpp \
Module.cpp \
ModuleLoader.cpp \
ModuleLoaderDLL.cpp \
ModuleRef.cpp \
Multiplexer.cpp \
Mutex.cpp \
Process.cpp \
Response.cpp \
Socket.cpp \
SocketAddress.cpp \
Stream.cpp \
StreamBuffer.cpp \
StreamDebugger.cpp \
StreamSocket.cpp \
StreamTokenizer.cpp \
StreamTransfer.cpp \
TermBuffer.cpp \
Thread.cpp \
Time.cpp \
UnitTester.cpp \
Uri.cpp \
User.cpp \
VersionTag.cpp

sconexincludedir = $(includedir)/sconex

sconexinclude_HEADERS = \
sconex.h \
utils.h \
Arg.h \
ArgObject.h \
ArgProc.h \
ArgScript.h \
ArgStatement.h \
Buffer.h \
ConfigFile.h \
ConfigStream.h \
Console.h \
Date.h \
Debug.h \
Descriptor.h \
DescriptorThread.h \
File.h \
FileDir.h \
FilePath.h \
FileStat.h \
Kernel.h \
LineBuffer.h \
ListenerSocket.h \
Logger.h \
MemFile.h \
MimeType.h \
Module.h \
ModuleInterface.h \
ModuleLoader.h \
ModuleLoaderDLL.h \
ModuleRef.h \
Multiplexer.h \
Mutex.h \
Process.h \
Response.h \
Socket.h \
SocketAddress.h \
Stream.h \
StreamBuffer.h \
StreamDebugger.h \
StreamSocket.h \
StreamTokenizer.h \
StreamTransfer.h \
TermBuffer.h \
Thread.h \
Time.h \
UnitTester.h \
Uri.h \
User.h \
VersionTag.h

libsconex_la_LDFLAGS = -version-info 0:0:0


check_PROGRAMS = utest


utest_SOURCES = \
utest.cpp \
Buffer_ut.cpp \
LineBuffer_ut.cpp \
MemFile_ut.cpp \
MimeType_ut.cpp \
TimeDate_ut.cpp \
Uri_ut.cpp \
VersionTag_ut.cpp

utest_LDADD = libsconex.la


TESTS = utest

