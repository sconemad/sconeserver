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
namespace scx {

//=============================================================================
DebugInstanceCounter::DebugInstanceCounter()
  : m_num(0),
    m_max(0)
{

}

//=============================================================================
void DebugInstanceCounter::count_constructor()
{
  ++m_num;
  ++m_max;
}

//=============================================================================
void DebugInstanceCounter::count_destructor()
{
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
void Debug::count_constuctor(const std::string& class_name)
{
  m_inst_counts[class_name].count_constructor();
}

//=============================================================================
void Debug::count_destuctor(const std::string& class_name)
{
  m_inst_counts[class_name].count_destructor();
}

//=============================================================================
const std::map<std::string,DebugInstanceCounter>& Debug::get_counters() const
{
  return m_inst_counts;
}

//=============================================================================
void Debug::set_logger(Logger* logger)
{
  m_logger = logger;
}

//=============================================================================
Debug::Debug()
  : m_logger(0),
    m_stop_on_assert(false)
{

}

};

