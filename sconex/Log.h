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

#ifndef scxLog_h
#define scxLog_h

#include <sconex/sconex.h>
#include <sconex/ScriptBase.h>
namespace scx {

typedef std::map<std::string, ScriptRef*> LogData;

//=============================================================================
// Log - Submits log entries to the system logger
//
class SCONEX_API Log {
public:

  // Create a log entry submitter for the specified category
  Log(const std::string& category);
  ~Log();

  // Attach some data to the log entry
  Log& attach(const std::string& name, ScriptRef* value);
  Log& attach(const std::string& name, const std::string& value);

  // Submit the log entry, specifying a message
  // This may be called multiple times on the same instance, to submit logs
  // for the same category. However, any attached data is not preserved
  // between calls and must be reattached if required.
  void submit(const std::string& msg);
  
private:

  std::string m_category;
  LogData* m_data;

};

};
#endif
