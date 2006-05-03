/* SconeServer (http://www.sconemad.com)

Tokenizing stream buffer base class

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

#ifndef scxStreamTokenizer_h
#define scxStreamTokenizer_h

#include "sconex/sconex.h"
#include "sconex/Stream.h"
#include "sconex/Buffer.h"
namespace scx {

#define StreamTokenizer_MAX_BUFFER (10*1048576)
#define StreamTokenizer_DEFAULT_BUFFER 1024

//=============================================================================
class SCONEX_API StreamTokenizer : public Stream {

public:

  StreamTokenizer(
    const std::string& stream_name,
    int buffer_size = StreamTokenizer_DEFAULT_BUFFER
  );

  virtual ~StreamTokenizer();

  Condition tokenize(std::string& token);
  
  virtual Condition read(void* buffer,int n,int& na);
  virtual Condition write(const void* buffer,int n,int& na);

  bool overflow();
  // Has the buffer overflown?

protected:

  virtual bool next_token(
    const Buffer& buffer,
    int& pre_skip,
    int& length,
    int& post_skip
  ) =0;
  // Find the next token from the current buffer head
  // Sets the buffer head to the start of the token and
  // returns the token length or -1 if not found
  
private:

  Buffer m_buffer;
  bool m_overflow;

};

};
#endif
