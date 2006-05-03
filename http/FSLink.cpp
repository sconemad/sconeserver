/* SconeServer (http://www.sconemad.com)

HTTP filesystem link node

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

#include "http/FSLink.h"
namespace http {

//=============================================================================
FSLink::FSLink(
  const std::string& name,
  const scx::FilePath& target
) : FSDirectory(name),
    m_target(target)
{
  DEBUG_COUNT_CONSTRUCTOR(FSLink);
}
	
//=============================================================================
FSLink::~FSLink()
{
  DEBUG_COUNT_DESTRUCTOR(FSLink);
}

//=============================================================================
std::string FSLink::url() const
{
  return FSNode::url();
}

//=============================================================================
scx::FilePath FSLink::path() const
{
  return m_target;
}

};
