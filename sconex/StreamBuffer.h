/* SconeServer (http://www.sconemad.com)

Buffering stream

Implements a dual FIFO I/O buffer for streams, with read and 
write/flush operations.

       Read Operation                  Write Operation
Stream     Buffer     Stream    Stream      Buffer     Stream
----------------------------    -----------------------------
 Read -4-> |####| -1-> Read      Write -1-> |#---|
           |-###| -1-> Read      Write -1-> |##--|
           |--##| -1-> Read      Write -1-> |###-|
           |---#| -1-> Read      Write -1-> |####| -4-> Write
 Read -4-> |####| -1-> Read      Write -1-> |#---|
           |-###| -1-> Read      Write -1-> |##--| 
           |--##| -1-> Read      Flush ---> |----| -2-> Write
           |---#| -1-> Read      
(In this example both read and write buffers are 4 bytes)

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

#ifndef scxStreamBuffer_h
#define scxStreamBuffer_h

#include <sconex/sconex.h>
#include <sconex/Stream.h>
#include <sconex/Buffer.h>
namespace scx {

#define StreamBuffer_MAX_BUFFER (10*1048576)
#define StreamBuffer_DEFAULT_BUFFER 1024

//=============================================================================
class SCONEX_API StreamBuffer : public Stream {

public:

  StreamBuffer(
    int read_buffer_size = StreamBuffer_DEFAULT_BUFFER,
    int write_buffer_size = StreamBuffer_DEFAULT_BUFFER
  );

  virtual ~StreamBuffer();

  virtual Condition read(void* buffer,int n,int& na);
  virtual Condition write(const void* buffer,int n,int& na);

  virtual Condition event(Event e);

  virtual std::string stream_status() const;
  
protected:

  Condition write_through(const void* buffer,int n,int& na);

  Buffer m_read_buffer;
  Buffer m_write_buffer;

private:

};

};
#endif
