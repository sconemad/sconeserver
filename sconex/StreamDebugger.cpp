/* SconeServer (http://www.sconemad.com)

SconeX stream pipe with debug output

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

#include "sconex/StreamDebugger.h"
#include "sconex/Date.h"
#include "sconex/Mutex.h"
namespace scx {

static Mutex* s_debugger_mutex = 0;
static int s_debugger_index = 0;

//=============================================================================
StreamDebugger::StreamDebugger(
  const std::string& name
) : Stream("debug")
{
  DEBUG_COUNT_CONSTRUCTOR(StreamDebugger);

  if (s_debugger_mutex == 0) {
    s_debugger_mutex = new Mutex;
  }
  
  MutexLocker locker(*s_debugger_mutex);
  ++s_debugger_index;

  std::ostringstream oss;
  oss << "dbg." << name << "." 
      << std::setfill('0') << std::setw(5) << s_debugger_index;
  
  m_file.open(oss.str(),File::Write | File::Create | File::Truncate);

  write_header("OPENED");
}

//=============================================================================
StreamDebugger::~StreamDebugger()
{
  DEBUG_COUNT_DESTRUCTOR(StreamDebugger);
  
  write_header("CLOSED");
  m_file.close();
}

//=============================================================================
Condition StreamDebugger::read(void* buffer,int n,int& na)
{
  na=0;
  Condition c = Stream::read(buffer,n,na);

  std::ostringstream oss;
  oss << "READ " << na << "(" << n << ") " << get_cond_str(c);
  write_header(oss.str());

  if (na > 0) {
    int na2;
    m_file.write(buffer,na,na2);
  }
  
  return c;
}

//=============================================================================
Condition StreamDebugger::write(const void* buffer,int n,int& na)
{
  na=0;
  Condition c = Stream::write(buffer,n,na);

  std::ostringstream oss;
  oss << "WRITE " << na << "/" << n << " " << get_cond_str(c);
  write_header(oss.str());

  if (na > 0) {
    int na2;
    m_file.write(buffer,na,na2);
  }
  
  return c;
}

//=============================================================================
std::string StreamDebugger::get_cond_str(Condition c)
{
  switch (c) {
    case scx::Ok:    return "OK";
    case scx::Wait:  return "WAIT";
    case scx::End:   return "END";
    case scx::Close: return "CLOSE";
    case scx::Error: return "ERROR";
  }
  return "UNKNOWN";
}

//=============================================================================
void StreamDebugger::write_header(const std::string& message)
{
  std::ostringstream oss;
  oss << "\n----< " << Date::now().code() << " " << message << " >----\n";
  m_file.write(oss.str());
}

//=============================================================================
std::string StreamDebugger::stream_status() const
{
  return m_file.path().path();
}


};

