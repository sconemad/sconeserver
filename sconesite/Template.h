/* SconeServer (http://www.sconemad.com)

Sconesite Template

Copyright (c) 2000-2009 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef sconesiteTemplate_h
#define sconesiteTemplate_h

#include <sconesite/XMLDoc.h>
#include <sconex/Stream.h>
#include <sconex/FilePath.h>
namespace scs {
  
class Profile;

//=========================================================================
class Template : public XMLDoc {

public:

  Template(const std::string& name,
           const scx::FilePath& path);

  ~Template();

  typedef scx::ScriptRefTo<Template> Ref;
  
};

};
#endif
