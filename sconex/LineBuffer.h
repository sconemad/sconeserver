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

#ifndef scxLineBuffer_h
#define scxLineBuffer_h

#include <sconex/sconex.h>
#include <sconex/StreamTokenizer.h>
namespace scx {

//=============================================================================
class SCONEX_API LineBuffer : public StreamTokenizer {

public:

  LineBuffer(
    const std::string& stream_name,
    int buffer_size = StreamTokenizer_DEFAULT_BUFFER
  );

  virtual ~LineBuffer();

protected:

  virtual bool next_token(
    const Buffer& buffer,
    int& pre_skip,
    int& length,
    int& post_skip
  );

private:

  char m_prev_char;
  
};

};
#endif
