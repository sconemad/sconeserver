# Microsoft Developer Studio Project File - Name="sconex" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=SCONEX - WIN32 RELEASE
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sconex.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sconex.mak" CFG="SCONEX - WIN32 RELEASE"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sconex - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "sconex - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sconex - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "."
# PROP Intermediate_Dir "."
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SCONEX_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I ".." /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SCONEX_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib ws2_32.lib /nologo /dll /machine:I386 /out:"../sconex.dll"
# SUBTRACT LINK32 /nodefaultlib

!ELSEIF  "$(CFG)" == "sconex - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "."
# PROP Intermediate_Dir "."
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SCONEX_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I ".." /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SCONEX_EXPORTS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib ws2_32.lib /nologo /dll /debug /machine:I386 /out:"../sconex.dll" /pdbtype:sept
# SUBTRACT LINK32 /nodefaultlib

!ENDIF 

# Begin Target

# Name "sconex - Win32 Release"
# Name "sconex - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Arg.cpp
# End Source File
# Begin Source File

SOURCE=.\ArgObject.cpp
# End Source File
# Begin Source File

SOURCE=.\ArgProc.cpp
# End Source File
# Begin Source File

SOURCE=.\ArgScript.cpp
# End Source File
# Begin Source File

SOURCE=.\ArgStatement.cpp
# End Source File
# Begin Source File

SOURCE=.\Buffer.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfigFile.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfigStream.cpp
# End Source File
# Begin Source File

SOURCE=.\Console.cpp
# End Source File
# Begin Source File

SOURCE=.\Date.cpp
# End Source File
# Begin Source File

SOURCE=.\Debug.cpp
# End Source File
# Begin Source File

SOURCE=.\Descriptor.cpp
# End Source File
# Begin Source File

SOURCE=.\File.cpp
# End Source File
# Begin Source File

SOURCE=.\FileDir.cpp
# End Source File
# Begin Source File

SOURCE=.\FilePath.cpp
# End Source File
# Begin Source File

SOURCE=.\FileStat.cpp
# End Source File
# Begin Source File

SOURCE=.\Kernel.cpp
# End Source File
# Begin Source File

SOURCE=.\LineBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\ListenerSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\Logger.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=.\MemFile.cpp
# End Source File
# Begin Source File

SOURCE=.\MimeType.cpp
# End Source File
# Begin Source File

SOURCE=.\Module.cpp
# End Source File
# Begin Source File

SOURCE=.\ModuleLoader.cpp
# End Source File
# Begin Source File

SOURCE=.\ModuleLoaderDLL.cpp
# End Source File
# Begin Source File

SOURCE=.\ModuleRef.cpp
# End Source File
# Begin Source File

SOURCE=.\Multiplexer.cpp
# End Source File
# Begin Source File

SOURCE=.\Mutex.cpp
# End Source File
# Begin Source File

SOURCE=.\Process.cpp
# End Source File
# Begin Source File

SOURCE=.\Response.cpp
# End Source File
# Begin Source File

SOURCE=.\sconex.cpp
# End Source File
# Begin Source File

SOURCE=.\Socket.cpp
# End Source File
# Begin Source File

SOURCE=.\SocketAddress.cpp
# End Source File
# Begin Source File

SOURCE=.\Stream.cpp
# End Source File
# Begin Source File

SOURCE=.\StreamBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\StreamDebugger.cpp
# End Source File
# Begin Source File

SOURCE=.\StreamSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\StreamTokenizer.cpp
# End Source File
# Begin Source File

SOURCE=.\StreamTransfer.cpp
# End Source File
# Begin Source File

SOURCE=.\Thread.cpp
# End Source File
# Begin Source File

SOURCE=.\Time.cpp
# End Source File
# Begin Source File

SOURCE=.\UnitTester.cpp
# End Source File
# Begin Source File

SOURCE=.\Uri.cpp
# End Source File
# Begin Source File

SOURCE=.\User.cpp
# End Source File
# Begin Source File

SOURCE=.\utils.cpp
# End Source File
# Begin Source File

SOURCE=.\VersionTag.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Arg.h
# End Source File
# Begin Source File

SOURCE=.\ArgObject.h
# End Source File
# Begin Source File

SOURCE=.\ArgProc.h
# End Source File
# Begin Source File

SOURCE=.\ArgScript.h
# End Source File
# Begin Source File

SOURCE=.\ArgStatement.h
# End Source File
# Begin Source File

SOURCE=.\Buffer.h
# End Source File
# Begin Source File

SOURCE=.\ConfigFile.h
# End Source File
# Begin Source File

SOURCE=.\ConfigStream.h
# End Source File
# Begin Source File

SOURCE=.\Console.h
# End Source File
# Begin Source File

SOURCE=.\Date.h
# End Source File
# Begin Source File

SOURCE=.\Debug.h
# End Source File
# Begin Source File

SOURCE=.\Descriptor.h
# End Source File
# Begin Source File

SOURCE=.\File.h
# End Source File
# Begin Source File

SOURCE=.\FileDir.h
# End Source File
# Begin Source File

SOURCE=.\FilePath.h
# End Source File
# Begin Source File

SOURCE=.\FileStat.h
# End Source File
# Begin Source File

SOURCE=.\Kernel.h
# End Source File
# Begin Source File

SOURCE=.\LineBuffer.h
# End Source File
# Begin Source File

SOURCE=.\ListenerSocket.h
# End Source File
# Begin Source File

SOURCE=.\Logger.h
# End Source File
# Begin Source File

SOURCE=.\MemFile.h
# End Source File
# Begin Source File

SOURCE=.\MimeType.h
# End Source File
# Begin Source File

SOURCE=.\Module.h
# End Source File
# Begin Source File

SOURCE=.\ModuleInterface.h
# End Source File
# Begin Source File

SOURCE=.\ModuleLoader.h
# End Source File
# Begin Source File

SOURCE=.\ModuleLoaderDLL.h
# End Source File
# Begin Source File

SOURCE=.\ModuleRef.h
# End Source File
# Begin Source File

SOURCE=.\Multiplexer.h
# End Source File
# Begin Source File

SOURCE=.\Mutex.h
# End Source File
# Begin Source File

SOURCE=.\Process.h
# End Source File
# Begin Source File

SOURCE=.\Response.h
# End Source File
# Begin Source File

SOURCE=.\sconex.h
# End Source File
# Begin Source File

SOURCE=.\Socket.h
# End Source File
# Begin Source File

SOURCE=.\SocketAddress.h
# End Source File
# Begin Source File

SOURCE=.\Stream.h
# End Source File
# Begin Source File

SOURCE=.\StreamBuffer.h
# End Source File
# Begin Source File

SOURCE=.\StreamDebugger.h
# End Source File
# Begin Source File

SOURCE=.\StreamSocket.h
# End Source File
# Begin Source File

SOURCE=.\StreamTokenizer.h
# End Source File
# Begin Source File

SOURCE=.\StreamTransfer.h
# End Source File
# Begin Source File

SOURCE=.\Thread.h
# End Source File
# Begin Source File

SOURCE=.\Time.h
# End Source File
# Begin Source File

SOURCE=.\UnitTester.h
# End Source File
# Begin Source File

SOURCE=.\Uri.h
# End Source File
# Begin Source File

SOURCE=.\User.h
# End Source File
# Begin Source File

SOURCE=.\utils.h
# End Source File
# Begin Source File

SOURCE=.\VersionTag.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
