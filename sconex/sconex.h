/* SconeServer (http://www.sconemad.com)

sconex global header

Copyright (c) 2000-2011 Andrew Wedgbury <wedge@sconemad.com>

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
#endif

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

// Standard headers

#ifdef HAVE_ARPA_INET_H
#  include <arpa/inet.h>
#endif

#ifdef HAVE_ERRNO_H
#  include <errno.h>
#  ifndef errno
     extern int errno;
#  endif
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

#ifdef HAVE_STRING_H
#  include <string.h>
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

#ifdef HAVE_TR1_UNORDERED_MAP
#  include <tr1/unordered_map>
#  define HASH_TYPE std::tr1::unordered_map
#else
#  if HAVE_EXT_HASH_MAP
#    include <ext/hash_map>
#    define HASH_TYPE __gnu_cxx::hash_map
     namespace __gnu_cxx
     {
       template<> struct hash<std::string> 
       {
         size_t operator()(const std::string& x) const
         {
           return hash<const char*>()(x.c_str());
         }
       };
     }
#  else
#    ifdef HAVE_MAP
#      define HASH_TYPE std::map
#    else
#      error "No std::map type found!"
#    endif
#  endif
#endif

// PCRE

#ifdef HAVE_PCRE_H
#  include <pcre.h>
#endif

// RTTI

#ifdef HAVE_TYPEINFO
#  include <typeinfo>
#endif

#ifdef HAVE_CXXABI_H
#  include <cxxabi.h>
#endif

// OS

#ifdef HAVE_SYS_UTSNAME_H
#  include <sys/utsname.h>
#endif

#ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
#endif

#ifdef HAVE_CRYPT_H
#  include <crypt.h>
#endif

// sconex Debugging
#include "sconex/Debug.h"

// Standard character constants
#define CRLF "\r\n"

namespace scx {

class VersionTag;
class Date;
  
// Get sconex version number
SCONEX_API VersionTag& version();

// Get copyright message
SCONEX_API const std::string& sconeserver_copyright();

// Get sconex build type (arch-platform-etc)
SCONEX_API const std::string& build_type();

// Get sconex build time
SCONEX_API Date& build_time();

};
#endif
