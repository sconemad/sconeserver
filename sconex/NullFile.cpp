/* SconeServer (http://www.sconemad.com)

Sconex null file

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

#include "sconex/NullFile.h"
#include "sconex/Stream.h"
namespace scx {

//=============================================================================
NullFile::NullFile()
{
  DEBUG_COUNT_CONSTRUCTOR(NullFile);
  m_state = Descriptor::Connected;
  m_virtual_events = (1<<Stream::Writeable);
}

//=============================================================================
NullFile::~NullFile()
{
  close();
  DEBUG_COUNT_DESTRUCTOR(NullFile);
}

//=============================================================================
void NullFile::close()
{

}

//=============================================================================
int NullFile::event_create()
{
  return 0;
}

//=============================================================================
std::string NullFile::describe() const
{
  return std::string("Null File");
}

//=============================================================================
int NullFile::fd()
{
  return -1;
}

//=============================================================================
Condition NullFile::endpoint_read(void* buffer,int n,int& na)
{
  // Read nothing
  na = 0;
  return scx::End;
}

//=============================================================================
Condition NullFile::endpoint_write(const void* buffer,int n,int& na)
{
  // Write (and discard) everything
  na = n;
  return scx::Ok;
}

};
