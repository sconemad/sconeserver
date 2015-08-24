# SconeServer (http://www.sconemad.com)
# automake file for sconex library

AM_CPPFLAGS = -I$(top_srcdir)

lib_LTLIBRARIES = libsconex.la

libsconex_la_SOURCES = \
sconex.cpp \
utils.cpp \
Base64.cpp \
Buffer.cpp \
ConfigFile.cpp \
ConfigStream.cpp \
Console.cpp \
Database.cpp \
DatagramChannel.cpp \
DatagramMultiplexer.cpp \
DatagramSocket.cpp \
Date.cpp \
Debug.cpp \
Descriptor.cpp \
Digest.cpp \
File.cpp \
FileDir.cpp \
FilePath.cpp \
FileStat.cpp \
GzipStream.cpp \
Job.cpp \
Kernel.cpp \
LineBuffer.cpp \
ListenerSocket.cpp \
Log.cpp \
Logger.cpp \
MemFile.cpp \
MimeHeader.cpp \
MimeType.cpp \
Module.cpp \
ModuleLoader.cpp \
Multiplexer.cpp \
Mutex.cpp \
NullFile.cpp \
Password.cpp \
Process.cpp \
RegExp.cpp \
Response.cpp \
ScriptBase.cpp \
ScriptContext.cpp \
ScriptEngine.cpp \
ScriptExpr.cpp \
ScriptStatement.cpp \
ScriptTypes.cpp \
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
Base64.h \
Buffer.h \
ConfigFile.h \
ConfigStream.h \
Console.h \
Database.h \
DatagramChannel.h \
DatagramMultiplexer.h \
DatagramSocket.h \
Date.h \
Debug.h \
Descriptor.h \
Digest.h \
File.h \
FileDir.h \
FilePath.h \
FileStat.h \
GzipStream.h \
IOBase.h \
Job.h \
Kernel.h \
LineBuffer.h \
ListenerSocket.h \
Logger.h \
MemFile.h \
MimeHeader.h \
MimeType.h \
Module.h \
ModuleInterface.h \
ModuleLoader.h \
Multiplexer.h \
Mutex.h \
NullFile.h \
Password.h \
Process.h \
Provider.h \
RegExp.h \
Response.h \
ScriptBase.h \
ScriptContext.h \
ScriptEngine.h \
ScriptExpr.h \
ScriptStatement.h \
ScriptTypes.h \
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

libsconex_la_LDFLAGS = -version-info 1:0:0

check_PROGRAMS = utest

utest_SOURCES = \
utest.cpp \
Buffer_ut.cpp \
FilePath_ut.cpp \
LineBuffer_ut.cpp \
MemFile_ut.cpp \
MimeHeader_ut.cpp \
MimeType_ut.cpp \
Password_ut.cpp \
ScriptTypes_ut.cpp \
TimeDate_ut.cpp \
Uri_ut.cpp \
utils_ut.cpp \
VersionTag_ut.cpp

utest_LDADD = libsconex.la

TESTS = utest

