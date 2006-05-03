/* SconeServer (http://www.sconemad.com)

sconex global header

Copyright (c) 2000-2004 Andrew Wedgbury <wedge@sconemad.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (see the file COPYING); if not, write to the
Free Software Foundation, Inc.,
59 Temple Place - Suite 330, Boston, MA  02111-1307, USA */

#ifndef sconex_h
#define sconex_h

#ifdef HAVE_CONFIG_H
#  include "config.h"
#else
#  include "config_win.h"
#endif

#ifndef WIN32

   // POSIX specifics
#  define SCONEX_API
#  define BOOL int
#  define TRUE 1
#  define FALSE 0
#  define SOCKET int
#  define SOCKET_ERROR -1
#  define _MAX_PATH 1024
#  define DIR_SEPARATOR "/"
#  define SCXBASE scx::

#else

   // WIN32 specifics
#  pragma warning (disable : 4786)
#  pragma warning (disable : 4251)
#  define WIN32_LEAN_AND_MEAN
#  define _WIN32_WINNT 0x0400
#  define WINVER 0x0400
#  define socklen_t int
#  define DIR_SEPARATOR "\\"
#  define SCXBASE
#  define uint32_t u_long

   // Standard WIN32 headers
#  define HAVE_WINDOWS_H
#  define HAVE_WINSOCK_H
#  define HAVE_IO_H
//#  define HAVE_SYS_STAT_H
//#  define HAVE_SYS_TYPES_H
//#  define HAVE_FCNTL_H

   // WIN32 DLL import/export
#  ifdef SCONEX_EXPORTS 
#    define SCONEX_API __declspec(dllexport)
#  else
#    define SCONEX_API __declspec(dllimport)
#  endif

#endif

// Standard headers

#ifdef HAVE_ARPA_INET_H
#  include <arpa/inet.h>
#endif

#ifdef HAVE_ERRNO_H
#  include <errno.h>
   extern int errno;
#endif

#ifdef HAVE_FCNTL_H
#  include <fcntl.h>
#endif

#ifdef HAVE_IO_H
#  include <io.h>
#endif

#ifdef HAVE_MALLOC_H
#  include <malloc.h>
#endif

#ifdef HAVE_MATH_H
#  include <math.h>
#endif

#ifdef HAVE_NETDB_H
#  include <netdb.h>
#endif

#ifdef HAVE_NETINET_IN_H
#  include <netinet/in.h>
#endif

#ifdef HAVE_PTHREAD_H
#  include <pthread.h>
#endif

#ifdef HAVE_SIGNAL_H
#  include <signal.h>
#endif

#ifdef HAVE_STDIO_H
#  include <stdio.h>
#endif

#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif

#ifdef HAVE_SYS_IOCTL_H
#  include <sys/ioctl.h>
#endif

#ifdef HAVE_SYS_SIGNAL_H
#  include <sys/signal.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
#  include <sys/socket.h>
#endif

#ifdef HAVE_SYS_STAT_H
#  include <sys/stat.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#ifdef HAVE_WINDOWS_H
#  include <windows.h>
#endif

#ifdef HAVE_WINSOCK_H
#  include <winsock.h>
#endif

// iostream

#ifdef HAVE_STRING
#  include <string>
#endif

#ifdef HAVE_IOSTREAM
#  include <iostream>
#endif

#ifdef HAVE_IOMANIP
#  include <iomanip>
#endif

#ifdef HAVE_FSTREAM
#  include <fstream>
#endif

#ifdef HAVE_SSTREAM
#  include <sstream>
#endif

// STL

#ifdef HAVE_MAP
#  include <map>
#endif

#ifdef HAVE_LIST
#  include <list>
#endif

#ifdef HAVE_QUEUE
#  include <queue>
#endif

#ifdef HAVE_STACK
#  include <stack>
#endif

#ifdef HAVE_SET
#  include <set>
#endif

// sconex Debugging
#include "sconex/Debug.h"


namespace scx {

class VersionTag;
class Date;
  
SCONEX_API VersionTag& version();
// Get sconex version number

SCONEX_API std::string& build_type();
// Get sconex build type (arch-platform-etc)

SCONEX_API Date& build_time();
// Get sconex build time

};
#endif
