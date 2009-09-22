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
std::string ansi(const std::string& code) {
  return std::string("\x1b[")+code;
}

//=============================================================================
TermBuffer::TermBuffer(
  const std::string& stream_name
)
  : Stream(stream_name),
    m_prompt(ansi("33m") + "sconeserver" + ansi("31m") + "> " + ansi("00m"))

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
  char ch;
  Condition c = Stream::read(&ch,1,na);
  int nw;

  if (c == Wait) {
    enable_event(Stream::Writeable,true);
    return c;
  }

  c = Wait;
  na = 0;

  if (isgraph(ch) || ch==' ') {
    m_line += ch;
    Stream::write(ansi("32m"));
    Stream::write(&ch,1,nw);
    Stream::write(ansi("00m"));

  } else {
    switch (ch) {
    
    case 0x1b: // Arrow key
      Stream::read(&ch,1,na);
      Stream::read(&ch,1,na);
      break;
      
    case 0x7f: // Backspace
      if (m_line.length() > 0) {
	m_line.erase(m_line.length()-1,1);
	ch = '\b';
	Stream::write(&ch,1,nw);
	Stream::write(" ");
	Stream::write(&ch,1,nw);
      }
      break;
      
    case 0x0a: // Enter
      m_line += ch;
      na = m_line.length();
      memcpy(buffer,m_line.data(),na);
      m_line = "";
      c = Ok;
      Stream::write(ansi("31m")+";"+ansi("00m")+"\n");
      break;

    }
  }

  //  std::ostringstream oss;
  //  oss << "<" << m_line << ">";
  //  Stream::write(oss.str());

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

      Stream::write(ansi("31m") + "*" + ansi("33m") + " SconeServer configuration console " + ansi("31m") + "*" + ansi("00m") + "\n\n");
      enable_event(Stream::Writeable,true);
    } break;
   
    case Stream::Closing: { // CLOSING

      // Restore the saved terminal settings
      tcsetattr(endpoint().fd(),TCSANOW,&m_saved_termios);

    } return Close;
    
    case Stream::Readable: { // READABLE
      
    } break;

    case Stream::Writeable: { // WRITEABLE
      Stream::write(m_prompt);
      enable_event(Stream::Writeable,false);
    } break;
    
    default:
      break;
    
  }

  return Ok;
}

};
