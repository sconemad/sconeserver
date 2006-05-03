/* SconeServer (http://www.sconemad.com)

Debugging support

Copyright (c) 2000-2006 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef scxDebug_h
#define scxDebug_h

// THIS TURNS DEBUGGING ON AND OFF
#define _DEBUG


#include "sconex/sconex.h"
namespace scx {

class Logger;

#ifdef _DEBUG

// Debugging macros
  
#define DEBUG_ASSERT(test,message) \
  scx::Debug::get()->dbg_assert(test,message,__FILE__,__LINE__);
// Log debug message if test fails

#define DEBUG_LOG(message) \
  { \
    std::ostringstream oss; \
    oss << message; \
    scx::Debug::get()->log(oss.str().c_str(),__FILE__,__LINE__); \
  }                                                            
// Log debug message

#define DEBUG_COUNT_CONSTRUCTOR(class_name) \
  scx::Debug::get()->count_constuctor(#class_name);
// Increment nstance count for this class
  
#define DEBUG_COUNT_DESTRUCTOR(class_name) \
  scx::Debug::get()->count_destuctor(#class_name);
// Decrement instance count for this class

#define DESCRIPTOR_DEBUG_LOG(message) \
  { \
    std::ostringstream oss; \
    oss << "[" << uid() << "] " << message; \
    scx::Debug::get()->log(oss.str().c_str(),__FILE__,__LINE__); \
  }                                                            
// Log debug message with descriptor uid

#define STREAM_DEBUG_LOG(message) \
  { \
    std::ostringstream oss; \
    oss << "[" << endpoint().uid() << "] " \
        << message; \
    scx::Debug::get()->log(oss.str().c_str(),__FILE__,__LINE__); \
  }                                                            
// Log debug message with descriptor uid
  
#else

// Evaluate to nothing outside debug mode
 
#define DEBUG_ASSERT(test,message);
#define DEBUG_LOG(message);
#define DEBUG_COUNT_CONSTRUCTOR(class_name);
#define DEBUG_COUNT_DESTRUCTOR(class_name);

#define DESCRIPTOR_DEBUG_LOG(message);
#define STREAM_DEBUG_LOG(message);

#endif

  
//=============================================================================
class SCONEX_API DebugInstanceCounter {

public:

  DebugInstanceCounter();

  void count_constructor();
  void count_destructor();

  int get_num() const;
  int get_max() const;

private:
  
  int m_num;
  int m_max;
};

  
//=============================================================================
class SCONEX_API Debug {

public:

  static Debug* get();
  // Get the debug instance

  void dbg_assert(
    bool test,
    const char* message,
    const char* file,
    int line
  );
  // Log debug message if test fails

  void log(
    const char* message,
    const char* file,
    int line
  );
  // Log debug message

  void count_constuctor(const std::string& class_name);
  void count_destuctor(const std::string& class_name);
  // Count instance construction/destruction

  const std::map<std::string,DebugInstanceCounter>& get_counters() const;
  // Get the instance counters
  
  void set_logger(Logger* logger);
  // Set logging class for the debugger to use
  
private:

  Logger* m_logger;
  std::map<std::string,DebugInstanceCounter> m_inst_counts;

  bool m_stop_on_assert;

  Debug();
  static Debug* s_debug;
  
};
  
};

#endif
