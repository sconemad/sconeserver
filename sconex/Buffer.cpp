/* SconeServer (http://www.sconemad.com)

Generic byte buffer

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

#include <sconex/Buffer.h>
namespace scx {

//=============================================================================
Buffer::Buffer(int buffer_size)
  : m_buffer(new char[buffer_size]),
    m_size(buffer_size),
    m_head(0),
    m_tail(0)
{
  DEBUG_COUNT_CONSTRUCTOR(Buffer);
  memset(m_buffer, 0, m_size);
}

//=============================================================================
Buffer::Buffer(const Buffer& c)
  : m_buffer(new char[c.m_size]),
    m_size(c.m_size),
    m_head(c.m_head),
    m_tail(c.m_tail)
{
  DEBUG_COUNT_CONSTRUCTOR(Buffer);
  memset(m_buffer, 0, m_size);
  memcpy(head(), c.head(), used());
}

//=============================================================================
Buffer::~Buffer()
{
  delete[] m_buffer;
  DEBUG_COUNT_DESTRUCTOR(Buffer);
}

//=============================================================================
const void* Buffer::head() const
{
  return &m_buffer[m_head];
}

//=============================================================================
void* Buffer::head()
{
  return &m_buffer[m_head];
}

//=============================================================================
void* Buffer::tail()
{
  return &m_buffer[m_tail];
}

//=============================================================================
const void* Buffer::tail() const
{
  return &m_buffer[m_tail];
}

//=============================================================================
void Buffer::push(int n)
{
  m_tail += n;
  DEBUG_ASSERT(m_tail <= m_size,"push() Buffer overflow");
  DEBUG_ASSERT(m_head <= m_tail,"push() Buffer overflow");
}

//=============================================================================
void Buffer::pop(int n)
{
  m_head += n;
  DEBUG_ASSERT(m_head <= m_size,"pop() Buffer underflow");
  DEBUG_ASSERT(m_head <= m_tail,"pop() Buffer underflow");
  if (m_head == m_tail) {
    compact();
  }
}

//=============================================================================
int Buffer::pop_to(void* a, int n)
{
  int na = (n > used() ? used() : n);
  memcpy(a, &m_buffer[m_head], na);
  pop(na);
  return na;
}

//=============================================================================
int Buffer::push_from(const void* a, int n)
{
  int na = (n > free() ? free() : n);
  memcpy(&m_buffer[m_tail], a, na);
  push(na);
  return na;
}

//=============================================================================
int Buffer::push_string(const std::string& s)
{
  int n = s.length();
  int na = (n > free() ? free() : n);
  memcpy(&m_buffer[m_tail], s.c_str(), na);
  push(na);
  return na;
}

//=============================================================================
int Buffer::insert_from(const void* a, int p, int n)
{
  DEBUG_ASSERT(p <= used(),"insert_from() Bad position");
  // Pack down if we are running out of space
  if (n > free()) {
    compact();
  }
  DEBUG_ASSERT(n <= free(),"insert_from() Buffer overflow");

  int toend = used() - p;
  if (toend > 0) {
    memmove(&m_buffer[m_head+p+n], &m_buffer[m_head+p], toend);
  }
  m_tail += n;

  // Copy into the space we made
  memcpy(&m_buffer[m_head+p], a, n);
  
  return n;
}

//=============================================================================
int Buffer::remove(int p, int n)
{
  DEBUG_ASSERT(p <= used(),"remove() Buffer underflow");
  DEBUG_ASSERT(p+n <= used(),"remove() Buffer underflow");

  int toend = used() - p - n;
  DEBUG_ASSERT(toend >= 0,"remove() Buffer underflow");
  if (toend > 0) {
    memmove(&m_buffer[m_head+p], &m_buffer[m_head+p+n], toend);
  }
  m_tail -= n;
  return n;
}

//=============================================================================
int Buffer::size() const
{
  return m_size;
}

//=============================================================================
int Buffer::free() const
{
  return m_size - m_tail;
}

//=============================================================================
int Buffer::used() const
{
  return m_tail - m_head;
}

//=============================================================================
int Buffer::wasted() const
{
  return m_head;
}
 
//=============================================================================
void Buffer::compact()
{
  if (m_head > 0) {
    if (used() == 0) {
      m_head = m_tail = 0;
    } else {
      // Pack the buffer down against 0 so there is no wasted space
      memmove(m_buffer, head(), used());
      m_tail -= m_head;
      m_head = 0;
    }
  }
}

//=============================================================================
bool Buffer::resize(int new_size)
{
  if (new_size <= size()) {
    return false;
  }

  // Allocate a new buffer and copy over the data
  char* new_buffer = new char[new_size];
  if (used() > 0) memcpy(new_buffer, &m_buffer[m_head], used());

  // Delete the old buffer and replace with new one
  delete[] m_buffer;
  m_buffer = new_buffer;

  // Update pointers
  m_tail -= m_head;
  m_head = 0;
  m_size = new_size;

  return true;
}

//=============================================================================
bool Buffer::ensure_free(int reqd)
{
  if (free() >= reqd)
    return false;
  if (free() + wasted() >= reqd) {
    compact();
    return false;
  }
  int extra = reqd - free() - wasted();
  return resize(size() + extra);
}
  
//=============================================================================
std::string Buffer::status_string() const
{
  std::ostringstream oss;
  oss << wasted() << "|" << used() << "|" << free();
  return oss.str();
}

//=============================================================================
BufferReader::BufferReader(Buffer& buffer, bool net)
  : m_buffer(buffer),
    m_net(net),
    m_pos(0)
{
}

//=============================================================================
BufferReader::~BufferReader()
{
}

//=============================================================================
void BufferReader::done()
{
  m_buffer.pop(m_pos);
  m_pos = 0;
}

//=============================================================================
uint8_t BufferReader::read_u8()
{
  return *(uint8_t*)next(1);
}

//=============================================================================
uint16_t BufferReader::read_u16()
{
  uint16_t a = *(uint16_t*)next(2);
  return m_net ? be16toh(a) : a;
}

//=============================================================================
uint32_t BufferReader::read_u32()
{
  uint32_t a = *(uint32_t*)next(4);
  return m_net ? be32toh(a) : a;
}

//=============================================================================
uint64_t BufferReader::read_u64()
{
  uint64_t a = *(uint64_t*)next(8);
  return m_net ? be64toh(a) : a;
}

//=============================================================================
void BufferReader::read_bytes(char* v, int n)
{
  void* p = next(n);
  memcpy(v, p, n);
}
  
//=============================================================================
void* BufferReader::next(int n)
{
  if (m_pos + n > m_buffer.used()) throw Underflow();
  void* a = (uint8_t*)m_buffer.head() + m_pos;
  m_pos += n;
  return a;
}


//=============================================================================
BufferWriter::BufferWriter(Buffer& buffer, bool net)
  : m_buffer(buffer),
    m_net(net),
    m_pos(0)
{
}

//=============================================================================
BufferWriter::~BufferWriter()
{
}

//=============================================================================
void BufferWriter::done()
{
  m_buffer.push(m_pos);
  m_pos = 0;
}

//=============================================================================
void BufferWriter::write_u8(uint8_t v)
{
  *(uint8_t*)next(1) = v;
}
  
//=============================================================================
void BufferWriter::write_u16(uint16_t v)
{
  *(uint16_t*)next(2) = m_net ? htobe16(v) : v;
}
  
//=============================================================================
void BufferWriter::write_u32(uint32_t v)
{
  *(uint32_t*)next(4) = m_net ? htobe32(v) : v;
}
  
//=============================================================================
void BufferWriter::write_u64(uint64_t v)
{
  *(uint64_t*)next(8) = m_net ? htobe64(v) : v;
}
 
//=============================================================================
void BufferWriter::write_bytes(const char* v, int n)
{
  void* p = next(n);
  memcpy(p, v, n);
}

//=============================================================================
void* BufferWriter::next(int n)
{
  if (m_pos + n > m_buffer.free()) throw Overflow();
  void* a = (uint8_t*)m_buffer.tail() + m_pos;
  m_pos += n;
  return a;
}
  
};
