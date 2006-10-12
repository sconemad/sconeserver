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

#include "sconex/LineBuffer.h"
namespace scx {

//=============================================================================
LineBuffer::LineBuffer(
  const std::string& stream_name,
  int buffer_size
)
  : StreamTokenizer(stream_name,buffer_size),
    m_prev_char('\0')
{
  DEBUG_COUNT_CONSTRUCTOR(LineBuffer);
}

//=============================================================================
LineBuffer::~LineBuffer()
{
  DEBUG_COUNT_DESTRUCTOR(LineBuffer);
}

//=============================================================================
bool LineBuffer::next_token(
  const Buffer& buffer,
  int& pre_skip,
  int& length,
  int& post_skip
)
{
  int i=0;
  int max = buffer.used();

  for (i=pre_skip; i<max; ++i) {
    char c = *((char*)buffer.head()+i);

    if (m_prev_char == '\r' && c == '\n') {
      ++pre_skip;
      m_prev_char = '\0';
      continue;
    }
    
    if (c=='\r' || c=='\n') {
      ++post_skip;
      if (i+1<max) {
        char cn = *((char*)buffer.head()+i+1);
        if (c=='\r' && cn=='\n') {
          ++post_skip;
        }
      } else {
        // next char could be a rogue one - save for later check
        m_prev_char = c;
      }
      break;
    }
    
    ++length;
  }

  return (post_skip > 0);
}

};
