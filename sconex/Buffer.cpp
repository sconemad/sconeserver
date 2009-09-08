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

#include "sconex/Buffer.h"
namespace scx {

//=============================================================================
Buffer::Buffer(
  int buffer_size
)
  : m_buffer(new char[buffer_size]),
    m_size(buffer_size),
    m_head(0),
    m_tail(0)
{
  DEBUG_COUNT_CONSTRUCTOR(Buffer);
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
  if (new_size <= used()) {
    return false;
  }

  // Allocate a new buffer and copy over the data
  char* new_buffer = new char[new_size];
  memcpy(new_buffer, &m_buffer[m_head], used());

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
std::string Buffer::status_string() const
{
  std::ostringstream oss;
  oss << wasted() << "|" << used() << "|" << free();
  return oss.str();
}


};
