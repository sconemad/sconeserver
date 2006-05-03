/* SconeServer (http://www.sconemad.com)

sconex global functions

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

#include "sconex/sconex.h"
#include "sconex/VersionTag.h"
#include "sconex/TimeDate.h"
namespace scx {

//=========================================================================
VersionTag& version()
{
  static VersionTag s_version(PACKAGE_VERSION);
  return s_version;
}

//=========================================================================
std::string& build_type()
{
  static std::string s_build_type = TARGET;
  return s_build_type;
}

//=========================================================================
Date& build_time()
{
  static Date s_build_time(__DATE__" "__TIME__,true);
  return s_build_time;
}
  
};
