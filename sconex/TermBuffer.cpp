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

#include <sconex/TermBuffer.h>
#include <sconex/Kernel.h>
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
    m_prompt(ansi("01;33m") + "sconeserver" + ansi("01;31m") + "> " + ansi("00m")),
    m_history_pos(0),
    m_history_add(true)

{

}

//=============================================================================
TermBuffer::~TermBuffer()
{

}

//=============================================================================
Condition TermBuffer::read(void* buffer,int n,int& na)
{
  char ch[130];
  Condition c = Stream::read(ch,128,na);
  int nw=na;

  if (c == Wait) {
    enable_event(Stream::Writeable,true);
    return c;
  }

  ch[na] = '\0';
  c = Wait;
  na = 0;

  if (isgraph(ch[0]) || ch[0]==' ') {
    m_history_add = true;
    m_line += ch;
    Stream::write(ansi("01;32m"));
    Stream::write(ch);
    Stream::write(ansi("00m"));

  } else {
    /*
    for (int i=0; i<nw; ++i) {
      char buf[8];
      sprintf(buf,"<%02x> ",ch[i]);
      Stream::write(buf);
    }
    */
    switch (ch[0]) {

    case CEOT: 
      m_line = "exit\n";
      na = m_line.length();
      memcpy(buffer,m_line.data(),na);
      c = Ok;
      Stream::write(ansi("01;31m")+"exit;"+ansi("00m")+"\n");
      m_line = "";
      break;
      
    case '\e': // Escape sequence
      if (ch[1] == 0x5b) {
	switch (ch[2]) {
	  case 0x41: { // UP
	    --m_history_pos;
	    if (m_history_pos < 0) m_history_pos = 0;
	    erase_line();
	    m_line = "";
	    if (m_history_pos >= 0 && m_history_pos < m_history.size()) {
	      m_line = m_history[m_history_pos];
	    }
	    Stream::write(ansi("00;32m") + m_line + ansi("00m"));
	    m_history_add = false;
	  } break;

	  case 0x42: { // DOWN
	    ++m_history_pos;
	    if (m_history_pos > m_history.size()) m_history_pos = m_history.size();
	    erase_line();
	    m_line = "";
	    if (m_history_pos >= 0 && m_history_pos < m_history.size()) {
	      m_line = m_history[m_history_pos];
	    }
	    Stream::write(ansi("00;32m") + m_line + ansi("00m"));
	    m_history_add = false;
	  } break;
	}
      }
      break;
      
    case CERASE: // Backspace
      m_history_add = true;
      if (m_line.length() > 0) {
	m_line.erase(m_line.length()-1,1);
	ch[0] = '\b';
	Stream::write(ch,1,nw);
	Stream::write(" ");
	Stream::write(ch,1,nw);
      }
      break;
      
    case '\n': // Enter
      if (m_line.length() > 0 && m_line[0] == '!') { // Pick history item
	std::string snum = m_line.substr(1);
	unsigned int num = atoi(snum.c_str());
	erase_line();
	m_line = "";
	if (num >= 0 && num < m_history.size()) m_line = m_history[num];
	Stream::write("\r"+m_prompt);
	Stream::write(ansi("00;32m") + m_line + ansi("00m"));
	m_history_add = false;
	break;
      }
	
      if (m_history_add && !m_line.empty()) {
	if (m_history.size() == 0 || m_history.back() != m_line) {
	  m_history.push_back(m_line);
	  m_history_pos = m_history.size();
	}
      } else {
	++m_history_pos;
      }

      if (m_line == "h" || m_line == "history") { // View history
	Stream::write("\n");
	int i=0;
	std::ostringstream oss;
	for (HistoryVec::const_iterator it = m_history.begin();
	     it != m_history.end();
	     ++it) {
	  oss << "  " << (i++) << "  " << (*it) << "\n";
	}
	oss << "\n";
	Stream::write(oss.str());
	enable_event(Stream::Writeable,true);

      } else { // Execute line
	m_line += ch;
	na = m_line.length();
	memcpy(buffer,m_line.data(),na);
	c = Ok;
	Stream::write(ansi("01;31m")+";"+ansi("00m")+"\n");
      }
      m_line = "";
      break;
    }
  }

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

      Kernel* k = Kernel::get();

      // Draw some coloured ascii art!
      std::string m = " o";
      Stream::write("\n "+ansi("01;31m")+m+ansi("00;33m")+m+ansi("01;37m")+m+ansi("00;33m")+m);
      Stream::write("\n "+ansi("00;33m")+m+ansi("01;31m")+m+ansi("01;31m")+m+ansi("00;33m")+m);
      Stream::write(ansi("01;33m") + "  " + k->name() + "-" + k->version().get_string());
      Stream::write("\n "+ansi("01;31m")+m+ansi("01;37m")+m+ansi("00;33m")+m+ansi("01;31m")+m);
      Stream::write(ansi("01;31m") + "  " + k->copyright());
      Stream::write("\n "+ansi("00;33m")+m+ansi("00;33m")+m+ansi("01;31m")+m+ansi("00;33m")+m);
      Stream::write("\n");
      Stream::write("\n");
      Stream::write("\n");
      Stream::write(ansi("00m"));

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

//=============================================================================
void TermBuffer::erase_line()
{
  char buf[128];
  unsigned int i;
  Stream::write("\r"+m_prompt);
  for (i=0; i<m_line.length(); ++i) { buf[i] = ' '; } buf[i] = '\0';
  Stream::write(buf);
  for (i=0; i<m_line.length(); ++i) { buf[i] = '\b'; } buf[i] = '\0';
  Stream::write(buf);
}

};
