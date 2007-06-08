/* SconeServer (http://www.sconemad.com)

Line buffer - tokenizer to split lines

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

#include "sconex/TermBuffer.h"
namespace scx {

//=============================================================================
TermBuffer::TermBuffer(
  const std::string& stream_name
)
  : Stream(stream_name),
    m_prev(0)
{
  DEBUG_COUNT_CONSTRUCTOR(TermBuffer);
}

//=============================================================================
TermBuffer::~TermBuffer()
{
  DEBUG_COUNT_DESTRUCTOR(TermBuffer);
}

//=============================================================================
Condition TermBuffer::read(void* buffer,int n,int& na)
{
  Condition c = Stream::read(buffer,n,na);
  int wna;
  Stream::write(buffer,na,wna);
  return c;
}

//=============================================================================
Condition TermBuffer::event(Stream::Event e)
{
  switch (e) {
    
    case Stream::Opening: { // OPENING
      int fd = endpoint().fd();

      // Save the terminal settings
      tcgetattr(fd,&m_saved_termios);

      // Setup new terminal settings
      termios new_termios = m_saved_termios;
      new_termios.c_lflag &= (~ICANON);
      new_termios.c_lflag &= (~ECHO);
      new_termios.c_cc[VTIME] = 0;
      new_termios.c_cc[VMIN] = 1;
      tcsetattr(fd,TCSANOW,&new_termios);

    } break;
   
    case Stream::Closing: { // CLOSING

      // Restore the saved terminal settings
      tcsetattr(endpoint().fd(),TCSANOW,&m_saved_termios);

    } return Close;
    
    case Stream::Readable: { // READABLE
      
    } break;
    
    default:
      break;
    
  }

  return Ok;
}

};
