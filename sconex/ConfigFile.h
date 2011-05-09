/* SconeServer (http://www.sconemad.com)

Configuration file

Loads and executes a SconeScript configuration file into a given context.

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

#ifndef scxConfigFile_h
#define scxConfigFile_h

#include <sconex/sconex.h>
#include <sconex/FilePath.h>
namespace scx {

class ScriptRef;
  
//=============================================================================
class SCONEX_API ConfigFile {

public:

  ConfigFile(const FilePath& filename);
  ~ConfigFile();

  // Execute configuration commands from this file in specified context
  bool load(ScriptRef* ctx);

protected:

  FilePath m_filename;

private:

};

};
#endif
