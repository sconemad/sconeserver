/* SconeServer (http://www.sconemad.com)

Terminal buffer - tokenizer to split lines

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

#ifndef scxTermBuffer_h
#define scxTermBuffer_h

#include "sconex/sconex.h"
#include "sconex/Stream.h"

#include <termios.h>

namespace scx {

//=============================================================================
class SCONEX_API TermBuffer : public Stream {

public:

  TermBuffer(
    const std::string& stream_name
  );

  virtual ~TermBuffer();

  virtual Condition read(void* buffer,int n,int& na);

  virtual Condition event(Stream::Event e);
  
protected:

  void erase_line();

private:

  termios m_saved_termios;
  std::string m_prompt;
  std::string m_line;

  typedef std::vector<std::string> HistoryVec;
  HistoryVec m_history;
  unsigned int m_history_pos;
  bool m_history_add;
};

};
#endif
