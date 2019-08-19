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

#include <arpa/inet.h>

#include <errno.h>
#ifndef errno
  extern int errno;
#endif

#include <fcntl.h>
#include <malloc.h>
#include <math.h>
#include <limits.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

#include <map>
#include <list>
#include <vector>
#include <queue>
#include <stack>
#include <set>

#if defined(HAVE_UNORDERED_MAP)
#  include <unordered_map>
#  define HASH_TYPE std::unordered_map
#elif defined(HAVE_TR1_UNORDERED_MAP)
#  include <tr1/unordered_map>
#  define HASH_TYPE std::tr1::unordered_map
#elif defined(HAVE_EXT_HASH_MAP)
#  include <ext/hash_map>
#  define HASH_TYPE __gnu_cxx::hash_map
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
#elif defined(HAVE_MAP)
#  define HASH_TYPE std::map
#else
#  error "No std::map type found!"
#endif

// PCRE

#ifdef HAVE_PCRE_H
#  include <pcre.h>
#else
#  ifdef HAVE_PCRE_PCRE_H
#    include <pcre/pcre.h>
#  endif
#endif

// RTTI

#  include <typeinfo>
#  include <cxxabi.h>

// OS

#  include <sys/utsname.h>
#  include <sys/time.h>
#  include <crypt.h>

// sconex Debugging
#include <sconex/Debug.h>

// Standard character constants
#define CRLF "\r\n"

namespace scx {

class VersionTag;
class Date;
  
// Get sconex version number
SCONEX_API VersionTag& version();

// Get copyright message
SCONEX_API const std::string& sconex_copyright();

// Get sconex build type (arch-platform-etc)
SCONEX_API const std::string& build_type();

// Get sconex build time
SCONEX_API Date& build_time();

};
#endif
