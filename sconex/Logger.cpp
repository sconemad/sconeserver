/* SconeServer (http://www.sconemad.com)

Sconex logger

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

#include "sconex/Logger.h"
#include "sconex/Date.h"
namespace scx {

//=============================================================================
Logger::Logger(const std::string& name)
{
  DEBUG_COUNT_CONSTRUCTOR(Logger);
  if (m_file.open(name.c_str(),File::Write | File::Append | File::Create) !=
      Ok) {
    std::cerr << "Unable to open log file '" << name
              << "' - will log to stdout\n";
    return;
  }
  log("-----------------------------------------------------------------",
      Info);
}

//=============================================================================
Logger::~Logger()
{
  m_file.close();
  DEBUG_COUNT_DESTRUCTOR(Logger);
}

//=============================================================================
void Logger::log(
  const std::string& message,
  Level level
)
{
  int na=0;
  std::string lcode;
  switch (level) {
    case Logger::Error:    lcode = "E"; break;
    case Logger::Warning:  lcode = "W"; break;
    case Logger::Info:     lcode = "i"; break;
    case Logger::Debug:    lcode = "d"; break;
  }
  std::string entry = Date::now(true).code() + " " +
    lcode + " " + message + "\n";
  if (m_file.is_open()) {
    m_file.write(entry.c_str(),entry.size(),na);
  } else {
    std::cout << entry;
  }
}

};
