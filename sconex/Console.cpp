/* SconeServer (http://www.sconemad.com)

SconeX console descriptor(s)

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

#include "sconex/Console.h"
namespace scx {

//=============================================================================
Console::Console()
{
  set_blocking(false);
  m_state = Connected;
}

//=============================================================================
Console::~Console()
{

}

//=============================================================================
void Console::close()
{

}

//=============================================================================
std::string Console::describe() const
{
  return Descriptor::describe() + " (console)";
}
 
//=============================================================================
int Console::fd()
{
  return fileno(stdin);
}

//=============================================================================
// Read n bytes from file into buffer
//
Condition Console::endpoint_read(void* buffer,int n,int& na)
{
  // Perform the read
  na = ::read(fileno(stdin),buffer,n);

  if (na > 0) {
    // Some, if not all requested, bytes were read
    return scx::Ok;
    
  } else if (na == 0) {
    // End of file reached
    return scx::End;

  } else if (error() == Descriptor::Wait) {
    // No data available right now, but not an error as such
    na=0;
    return scx::Wait; 
  }

  // Fatal error occured
  na = 0;
  DESCRIPTOR_DEBUG_LOG("Console::endpoint_read() error");
  return scx::Error;
}

//=============================================================================
// Write n bytes from file into buffer
//
Condition Console::endpoint_write(const void* buffer,int n,int& na)
{
  na = ::write(fileno(stdout),buffer,n);
  
  if (na > 0 || n==na) {
    // Written some or all of the data ok
    return scx::Ok;

  } else if (error() == Descriptor::Wait) {
    // Cannot write right now
    na=0;
    return scx::Wait;
  }

  // Fatal error occured
  na = 0;
  DESCRIPTOR_DEBUG_LOG("Console::endpoint_write() error");
  return scx::Error;
}

};
