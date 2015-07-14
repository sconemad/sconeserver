/* SconeServer (http://www.sconemad.com)

Sconex log interface

Copyright (c) 2000-2013 Andrew Wedgbury <wedge@sconemad.com>

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

#include <sconex/Log.h>
#include <sconex/Logger.h>
#include <sconex/ScriptTypes.h>
namespace scx {
  
//=============================================================================
Log::Log(const std::string& category)
  : m_category(category), m_data(0)
{
}

//=============================================================================
Log::~Log()
{
  if (m_data) {
    for (LogData::iterator it = m_data->begin(); it != m_data->end(); ++it) {
      delete it->second;
    }
    delete m_data;
  }
}

//=============================================================================
Log& Log::attach(const std::string& name, ScriptRef* value)
{
  if (!m_data) m_data = new LogData();
  (*m_data)[name] = value;
  return *this;
}

//=============================================================================
Log& Log::attach(const std::string& name, const std::string& value)
{
  if (!m_data) m_data = new LogData();
  (*m_data)[name] = ScriptString::new_ref(value);
  return *this;
}

//=============================================================================
Log& Log::attach(const std::string& name, int value)
{
  if (!m_data) m_data = new LogData();
  (*m_data)[name] = ScriptInt::new_ref(value);
  return *this;
}
  
//=============================================================================
void Log::submit(const std::string& msg)
{
  Logger::get()->log(m_category, msg, m_data);
  m_data = 0;
}

};
