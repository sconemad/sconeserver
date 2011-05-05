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

#ifndef scxLogger_h
#define scxLogger_h

#include "sconex/sconex.h"
namespace scx {

class File;
class Mutex;

//=============================================================================
class SCONEX_API Logger {

public:

  Logger(const std::string& name);
  // Construct logger
  
  virtual ~Logger();

  enum Level { Error, Warning, Info, Debug };
  
  virtual void log(
    const std::string& message,
    Level level
  );
  // Write message to the log

protected:

  File* m_file;
  Mutex* m_mutex;

private:
	
};

};
#endif
