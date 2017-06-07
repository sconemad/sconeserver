/* SconeServer (http://www.sconemad.com)

Buffering stream

Copyright (c) 2000-2014 Andrew Wedgbury <wedge@sconemad.com>

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

#include <sconex/StreamBuffer.h>
namespace scx {

//=============================================================================
StreamBuffer::StreamBuffer(
  int read_buffer_size,
  int write_buffer_size
)
  : Stream("buffer"),
    m_read_buffer(read_buffer_size),
    m_write_buffer(write_buffer_size)
{
  DEBUG_COUNT_CONSTRUCTOR(StreamBuffer);

  DEBUG_ASSERT(
    read_buffer_size >= 0,
    "StreamBuffer() Buffer size invalid");
  DEBUG_ASSERT(
    read_buffer_size <= StreamBuffer_MAX_BUFFER,
    "StreamBuffer() Buffer size too large");
  
  DEBUG_ASSERT(
    write_buffer_size >= 0,
    "StreamBuffer() Buffer size invalid");
  DEBUG_ASSERT(
    write_buffer_size <= StreamBuffer_MAX_BUFFER,
    "StreamBuffer() Buffer size too large");
}
	
//=============================================================================
StreamBuffer::~StreamBuffer()
{
  DEBUG_ASSERT(
    m_write_buffer.used() == 0,
    "~StreamBuffer() Write buffer still contains data");

  DEBUG_COUNT_DESTRUCTOR(StreamBuffer);
}

//=============================================================================
Condition StreamBuffer::read(void* buffer,int n,int& na)
{
  na = 0;

  if (m_read_buffer.used()) {
    na += m_read_buffer.pop_to(buffer,n);
    if (na == n) {
      return Ok;
    }
  }

  int left = n-na;
  if (left >= m_read_buffer.free()) {
    int nr = 0;
    Condition c = Stream::read((char*)buffer+na,left,nr);
    if (nr > 0) na += nr;
    return c; 
  }

  int nr = 0;
  Condition c = Stream::read(m_read_buffer.tail(),m_read_buffer.free(),nr);
  if (nr > 0) {
    m_read_buffer.push(nr);
    na += m_read_buffer.pop_to((char*)buffer+na,left);
  }
  return c;
}

//=============================================================================
Condition StreamBuffer::write(
  const void* buffer, // data to be written
  int n,              // number of bytes in buffer to be written
  int& na             // receives number of bytes actually accepted
)
{
  DEBUG_ASSERT(n>0,"write() Zero bytes specified");
  DEBUG_ASSERT(buffer!=0,"write() Null buffer passed");

  na = 0;
  
  // Compact the buffer if there is too much wasted space
  if (m_write_buffer.wasted() > m_write_buffer.used()) {
    m_write_buffer.compact();
  }
  
  // See if the request can be buffered
  if (n <= m_write_buffer.free()) {
    na = m_write_buffer.push_from(buffer,n);
    enable_event(Stream::Writeable,true);
    return Ok;
  } 

  if (m_write_buffer.used() == 0) {
    // Buffer is empty so request must be > max buffer, try and write through
    return write_through(buffer,n,na);
  } 

  // Top up the buffer
  na = m_write_buffer.push_from(buffer,n);
  enable_event(Stream::Writeable,true);

  // Try and write out the whole buffer
  int nw=0;
  Condition c = Stream::write(m_write_buffer.head(),m_write_buffer.used(),nw);
  if (nw<=0) {
    // Something went wrong
    return c;
    
  } else {
    m_write_buffer.pop(nw);
  }

  // Can the rest be buffered
  int left = n-na;
  if (left <= m_write_buffer.free()) {
    na += m_write_buffer.push_from((char*)buffer+na,left);
    return Ok;
  }

  // Cannot completely buffer the rest so try and write through
  return write_through((char*)buffer+na,left,na);
}

//=============================================================================
Condition StreamBuffer::event(Event e)
{
  switch (e) {
  
    case Stream::Closing: {
      // Don't let the stream dissapear if we haven't finished
      // sending the write buffer.
      if (m_write_buffer.used() > 0) {
        enable_event(Stream::Writeable,true);
        return Wait;
      }
    } break;
    
    case Stream::Writeable: {
      int n = m_write_buffer.used();
      if (n > 0) {
	int nw = 0;
	Condition c = Stream::write(m_write_buffer.head(),n,nw);
	if (nw>0) m_write_buffer.pop(nw);
      
	if (c==scx::Error) {
	  // Went wrong
	  return scx::Error;
	  
	} else if (nw<n) {
	  // Wrote some of the buffer
	  enable_event(Stream::Writeable,true);
	  return scx::Wait;
	}
      }
      
      // Wrote everything, cancel writeable notifications now
      enable_event(Stream::Writeable,false);
    } break;

    default:
      break;
  }
    
  return Ok;
}

//=============================================================================
bool StreamBuffer::has_readable() const
{
  return (m_read_buffer.used() > 0);
}

//=============================================================================
Condition StreamBuffer::write_through(
  const void* buffer,
  int n,
  int& na
)
{
  //
  // Try and write the buffer straight thru to the underlying pipe
  //
  int nw=0;
  Condition c = Stream::write(buffer,n,nw);
  na+=nw;

  if (nw<=0) {
    //
    // Wrote nothing - return the error
    // 
    return c;

  } else if (nw==n) {
    //
    // Wrote it all just fine
    //
    return Ok;
  }

  //
  // Wrote something - try buffering the rest
  //
  int left = n-nw;
  if (left <= m_write_buffer.free()) {
    na += m_write_buffer.push_from((char*)buffer+nw,left);
  }

  return Ok;
}

//=============================================================================
std::string StreamBuffer::stream_status() const
{
  std::ostringstream oss;
  oss << "r:" << m_read_buffer.status_string()
      <<  " w:" << m_write_buffer.status_string();
  return oss.str();
}

};
