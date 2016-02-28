/* SconeServer (http://www.sconemad.com)

Generic byte buffer

For example, here's a 16 byte buffer containing 5 bytes of data:

 |---------------------size----------------------|   size=16
                                                     
  00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15    head=3
 +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+   tail=8
 |  |  |  |AA|BB|CC|DD|EE|  |  |  |  |  |  |  |  |   
 +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+   wasted=head=3
           ^-head         ^-tail                     used=tail-head=5
                                                     free=size-tail=8
 |-wasted-|----used------|---------free----------|   
                                                     

Copyright (c) 2000-2006 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef scxBuffer_h
#define scxBuffer_h

#include <sconex/sconex.h>
namespace scx {

//=============================================================================
class SCONEX_API Buffer {

public:

  Buffer(int buffer_size);
  Buffer(const Buffer& c);
  ~Buffer();

  // Get a pointer to the buffer head - the start of the data
  // Don't copy more than used() from head()
  const void* head() const;
  void* head();

  // Get a pointer to the buffer tail - one past the end of the data
  // Don't copy more than free() to tail
  const void* tail() const;
  void* tail();

  void push(int n);
  void pop(int n);

  int pop_to(void* a, int n);
  int push_from(const void* a, int n);
  int push_string(const std::string& s);

  // Insert n bytes into the buffer at position p
  int insert_from(const void* a, int p, int n);
  
  // Remove n bytes from the buffer from position p
  int remove(int p, int n);
  
  int size() const;
  int free() const;
  int used() const;
  int wasted() const;

  // Rearrange buffer to remove any wasted space.
  void compact();

  // Resize buffer
  bool resize(int new_size);

  // Ensure at least reqd bytes are free in the buffer
  bool ensure_free(int reqd);
  
  // Get a string representing the status of this buffer, for debugging/info
  std::string status_string() const;
  
protected:

private:

  char* m_buffer;
  int m_size;
  int m_head;
  int m_tail;
  
};

  
//=============================================================================
class SCONEX_API BufferReader {
public:

  // Underflow is thrown when reading with not enough data available
  class Underflow {
  public:
    Underflow() {};
  };

  // Construct a BufferReader for the buffer.
  // net: convert multibyte values from network to host byte ordering.
  BufferReader(Buffer& buffer, bool net=true);
  
  ~BufferReader();

  // Pops read data from the buffer
  void done();
  
  uint8_t read_u8();
  uint16_t read_u16();
  uint32_t read_u32();
  uint64_t read_u64();

  // Read an exact number of bytes into v
  void read_bytes(char* v, int n);
  
private:

  void* next(int n);

  Buffer& m_buffer;
  bool m_net;
  int m_pos;
};

  
//=============================================================================
class SCONEX_API BufferWriter {
public:

  // Overflow is thrown when writing with not enough space available
  class Overflow {
  public:
    Overflow() {};
  };

  // Construct a BufferWriter for the buffer.
  // net: convert multibyte values from host to network byte ordering.
  BufferWriter(Buffer& buffer, bool net=true);
  
  ~BufferWriter();

  // Pushes read data to the buffer
  void done();
  
  void write_u8(uint8_t v);
  void write_u16(uint16_t v);
  void write_u32(uint32_t v);
  void write_u64(uint64_t v);

  // Write an exact number of bytes from v
  void write_bytes(const char* v, int n);
  
private:

  void* next(int n);

  Buffer& m_buffer;
  bool m_net;
  int m_pos;
};

};
#endif
