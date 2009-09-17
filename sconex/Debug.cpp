/* SconeServer (http://www.sconemad.com)

Debugging support

Copyright (c) 2000-2005 Andrew Wedgbury <wedge@sconemad.com>

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

#include "sconex/Debug.h"
#include "sconex/Logger.h"
#include "sconex/Mutex.h"
namespace scx {

#define DEBUG_BREAKPOINT asm("int $3");

// Swtich on debug instance counting
#define DEBUG_INSTANCE_COUNT

//=============================================================================
DebugInstanceCounter::DebugInstanceCounter()
  : m_num(0),
    m_max(0)
{

}

//=============================================================================
void DebugInstanceCounter::reset()
{
  m_addrs.clear();
}

//=============================================================================
void DebugInstanceCounter::count_constructor(void* addr)
{
  m_addrs.push_back( std::pair<void*,int>(addr,m_max) );

  ++m_num;
  ++m_max;
}

//=============================================================================
void DebugInstanceCounter::count_destructor(void* addr)
{
  for (std::vector< std::pair<void*,int> >::iterator it = m_addrs.begin();
       it != m_addrs.end();
       ++it) {
    std::pair<void*,int>& p = *it;
    if (p.first == addr) {
      m_addrs.erase(it);
      break;
    }
  }

  --m_num;
}

//=============================================================================
int DebugInstanceCounter::get_num() const
{
  return m_num;
}

//=============================================================================
int DebugInstanceCounter::get_max() const
{
  return m_max;
}

//=============================================================================
std::string DebugInstanceCounter::get_deltas()
{
  std::ostringstream oss;
  for (std::vector< std::pair<void*,int> >::iterator it = m_addrs.begin();
       it != m_addrs.end();
       ++it) {
    std::pair<void*,int>& p = *it;
    oss << p.second << " ";
  }
  
  return oss.str();
}
  
Debug* Debug::s_debug = 0;
 
//=============================================================================
Debug* Debug::get()
{
  if (!s_debug) {
    s_debug = new Debug;
  }
  return s_debug;
}
  
//=============================================================================
void Debug::dbg_assert(
  bool test,
  const char* message,
  const char* file,
  int line
)
{
  if (test==false) {

    if (m_logger) {
      std::string filename(file);
      std::string::size_type ns = filename.find_last_of("/\\");
      if (ns != std::string::npos) {
        filename = filename.substr(ns+1);
      }
      std::ostringstream oss;
      oss << "(" <<  filename << ":" << line << ") "
          << message << " (ASSERTION)";
      m_logger->log(oss.str(),Logger::Debug);
    }
    
    if (m_stop_on_assert) {
      // Issue a debug breakpoint if required
      abort();
    }
    
  }
}

//=============================================================================
void Debug::log(
  const char* message,
  const char* file,
  int line
)
{
  if (m_logger) {
    std::string filename(file);
    std::string::size_type ns = filename.find_last_of("/\\");
    if (ns != std::string::npos) {
      filename = filename.substr(ns+1);
    }
    std::ostringstream oss;
    oss << "(" <<  filename << ":" << line << ") "
        << message;
        
    m_logger->log(oss.str(),Logger::Debug);
  }
}

//=============================================================================
void Debug::count_constuctor(const std::string& class_name, void* addr)
{
#ifdef DEBUG_INSTANCE_COUNT
  m_mutex->lock();

  // Place breakpoints for particular instance construction
  if (class_name == "Arg") {
    // if (m_inst_counts[class_name].get_max() == 2433) DEBUG_BREAKPOINT;
  }

  m_inst_counts[class_name].count_constructor(addr);
  m_mutex->unlock();
#endif
}

//=============================================================================
void Debug::count_destuctor(const std::string& class_name, void* addr)
{
#ifdef DEBUG_INSTANCE_COUNT
  m_mutex->lock();
  m_inst_counts[class_name].count_destructor(addr);
  m_mutex->unlock();
#endif
}

//=============================================================================
void Debug::reset_counters()
{
#ifdef DEBUG_INSTANCE_COUNT
  m_mutex->lock();
  for (InstanceCounterMap::iterator it = m_inst_counts.begin();
       it != m_inst_counts.end();
       ++it) {
    ((*it).second).reset();
  }
  m_mutex->unlock();
#endif
}

//=============================================================================
void Debug::get_counters(InstanceCounterMap& counters)
{
#ifdef DEBUG_INSTANCE_COUNT
  m_mutex->lock();
  counters = m_inst_counts;
  m_mutex->unlock();
#endif
}

//=============================================================================
void Debug::set_logger(Logger* logger)
{
  m_logger = logger;
}

//=============================================================================
Debug::Debug()
  : m_logger(0),
    m_mutex(new Mutex()),
    m_stop_on_assert(false)
{

}

};

