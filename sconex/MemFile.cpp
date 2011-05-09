/* SconeServer (http://www.sconemad.com)

Sconex memory file

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

#include <sconex/MemFile.h>
#include <sconex/File.h>
#include <sconex/Stream.h>
namespace scx {

//=============================================================================
MemFile::MemFile(const MemFileBuffer* buffer)
  : m_buffer(buffer),
    m_pos(0)
{
  DEBUG_COUNT_CONSTRUCTOR(MemFile);
  m_state = Connected;
}

//=============================================================================
MemFile::~MemFile()
{
  close();
  DEBUG_COUNT_DESTRUCTOR(MemFile);
}

//=============================================================================
void MemFile::close()
{
  m_state = Closed;
}

//=============================================================================
long MemFile::tell() const
{
  return m_pos;
}

//=============================================================================
long MemFile::size() const
{
  return m_buffer->get_buffer()->used();
}

//=============================================================================
int MemFile::event_create()
{
  return 0;
}

//=============================================================================
int MemFile::fd()
{
  return -1;
}

//=============================================================================
Condition MemFile::endpoint_read(void* buffer,int n,int& na)
{
  if (m_state != Connected) {
    return scx::Error;
  }

  int max = m_buffer->get_buffer()->used();
  if (m_pos >= max) {
    return scx::End;
  }
  
  int left = max - m_pos;
  if (n > left) {
    n = left;
  }
    
  const char* head = (const char*)m_buffer->get_buffer()->head();
  memcpy(buffer,head+m_pos,n);
  
  na = n;
  m_pos += na;
  
  return scx::Ok;
}

//=============================================================================
Condition MemFile::endpoint_write(const void* buffer,int n,int& na)
{
  DESCRIPTOR_DEBUG_LOG("endpoint_write() cannot write to MemFile!");
  return scx::Error;
}


//=============================================================================
MemFileBuffer::MemFileBuffer(int buffer_size)
{
  m_buffer = new Buffer(buffer_size);
}

//=============================================================================
MemFileBuffer::MemFileBuffer(File& file)
{
  int buffer_size = file.size();
  
  m_buffer = new Buffer(buffer_size);
  int na;
  file.read(m_buffer->tail(),buffer_size,na);
}
    
//=============================================================================
MemFileBuffer::~MemFileBuffer()
{
  delete m_buffer;
}

//=============================================================================
const Buffer* MemFileBuffer::get_buffer() const
{
  return m_buffer;
}

//=============================================================================
Buffer* MemFileBuffer::get_buffer()
{
  return m_buffer;
}

};
