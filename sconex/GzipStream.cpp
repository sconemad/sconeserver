/* SconeServer (http://www.sconemad.com)

Compression/decompression stream using Gzip

Copyright (c) 2000-2015 Andrew Wedgbury <wedge@sconemad.com>

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

#include <sconex/GzipStream.h>
namespace scx {

// Uncomment to enable debug logging
//#define GZIP_DEBUG_LOG(m) STREAM_DEBUG_LOG(m)

#ifndef GZIP_DEBUG_LOG
#  define GZIP_DEBUG_LOG(m)
#endif

//=============================================================================
GzipStream::GzipStream(
  int read_buffer_size,
  int write_buffer_size
) : Stream("gzip"),
    m_read_in(read_buffer_size),
    m_read_out(read_buffer_size),
    m_read_zs(0),
    m_write_in(write_buffer_size),
    m_write_out(write_buffer_size),
    m_write_zs(0)
{
  DEBUG_COUNT_CONSTRUCTOR(GzipStream);

  DEBUG_ASSERT(
    read_buffer_size >= 0,
    "GzipStream() Buffer size invalid");
  DEBUG_ASSERT(
    read_buffer_size <= GzipStream_MAX_BUFFER,
    "GzipStream() Buffer size too large");
  
  DEBUG_ASSERT(
    write_buffer_size >= 0,
    "GzipStream() Buffer size invalid");
  DEBUG_ASSERT(
    write_buffer_size <= GzipStream_MAX_BUFFER,
    "GzipStream() Buffer size too large");

  if (read_buffer_size > 0) {
    m_read_zs = new z_stream;
    memset(m_read_zs,0,sizeof(z_stream));
    inflateInit2(m_read_zs, 32+15);
  }

  if (write_buffer_size > 0) {
    m_write_zs = new z_stream;
    memset(m_write_zs,0,sizeof(z_stream));
    deflateInit2(m_write_zs,
                 Z_DEFAULT_COMPRESSION,Z_DEFLATED,
                 16+15,8,Z_DEFAULT_STRATEGY);
  }
}
	
//=============================================================================
GzipStream::~GzipStream()
{
  DEBUG_ASSERT(
    m_read_in.used() == 0,
    "~GzipStream() Read input buffer still contains data");
  DEBUG_ASSERT(
    m_read_out.used() == 0,
    "~GzipStream() Read output buffer still contains data");
  DEBUG_ASSERT(
    m_write_in.used() == 0,
    "~GzipStream() Write input buffer still contains data");
  DEBUG_ASSERT(
    m_write_out.used() == 0,
    "~GzipStream() Write output buffer still contains data");

  if (m_read_zs) {
    inflateEnd(m_read_zs);
    delete m_read_zs;
  }
  
  if (m_write_zs) {
    deflateEnd(m_write_zs);
    delete m_write_zs;
  }
  
  DEBUG_COUNT_DESTRUCTOR(GzipStream);
}

//=============================================================================
Condition GzipStream::read(void* buffer,int n,int& na)
{
  if (!m_read_zs) return Stream::read(buffer,n,na);
  DEBUG_ASSERT(n>0,"write() Zero bytes specified");
  DEBUG_ASSERT(buffer!=0,"write() Null buffer passed");
  na = 0;
  Condition c = Ok;

  // Can the request be fullfilled by the buffer?
  if (n <= m_read_out.used()) {
    na = m_read_out.pop_to(buffer, n);
    return Ok;
  }

  m_read_in.compact();
  if (m_read_in.free()) {
    int nr = 0;
    c = Stream::read(m_read_in.tail(),m_read_in.free(),nr);
    if (nr > 0) m_read_in.push(nr);
  }

  inflate_buffer(Z_FULL_FLUSH);

  n = std::min(n, m_read_out.used());
  if (n > 0) {
    na = m_read_out.pop_to(buffer,n);
    return Ok;
  }

  if (m_read_in.used()) {
    return Wait;
  }

  return c;
}

//=============================================================================
Condition GzipStream::write(const void* buffer,int n,int& na)
{
  if (!m_write_zs) return Stream::write(buffer,n,na);
  DEBUG_ASSERT(n>0,"write() Zero bytes specified");
  DEBUG_ASSERT(buffer!=0,"write() Null buffer passed");
  na = 0;

  // Ensure there is enough space in the input buffer
  if (n > m_write_in.free()) {
    m_write_in.compact();
    if (n > m_write_in.free()) {
      int extra = n - m_write_in.free();
      DEBUG_LOG("Increasing input buffer by " << extra);
      m_write_in.resize(m_write_in.size() + extra);
      //XXX could make this more efficient?
    }    
  } 

  na = m_write_in.push_from(buffer,n);

  deflate_buffer(Z_FULL_FLUSH);

  // Attempt to write the data from the output buffer
  int nw = 0;
  Condition c = Stream::write(m_write_out.head(),m_write_out.used(),nw);
  if (nw>0) m_write_out.pop(nw);
  enable_event(Stream::Writeable,m_write_out.used() > 0);
  return c;
}

//=============================================================================
Condition GzipStream::event(Event e)
{
  switch (e) {
  
    case Stream::Closing: {
      if (!m_write_zs) break;

      if (m_write_in.used() > 0) {
        deflate_buffer(Z_FULL_FLUSH);
      } else {
        deflate_buffer(Z_FINISH);
      }

      // Don't let the stream dissapear if there is any data left to process.
      if (m_write_in.used() > 0 || m_write_out.used() > 0) {
        enable_event(Stream::Writeable,true);
	return Wait;
      }
    } break;
    
    case Stream::Writeable: {
      if (m_write_in.used() > 0) {
        deflate_buffer(Z_FULL_FLUSH);
      }
      int n = m_write_out.used();
      if (n > 0) {
	int nw = 0;
	Condition c = Stream::write(m_write_out.head(),n,nw);
	if (nw>0) m_write_out.pop(nw);
      
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
bool GzipStream::has_readable() const
{
  return m_read_out.used();
}

//=============================================================================
std::string GzipStream::stream_status() const
{
  std::ostringstream oss;
  if (m_read_zs) {
    oss << "ri:" << m_read_in.status_string()
        << " ro:" << m_read_out.status_string();
  }
  if (m_write_zs) {
    if (m_read_zs) oss << " ";
    oss << "wi:" << m_write_in.status_string()
        << " wo:" << m_write_out.status_string();
  }
  return oss.str();
}

//=============================================================================
int GzipStream::inflate_buffer(int flush)
{
  int prev_in = m_read_zs->total_in;
  m_read_zs->next_in = (Bytef*)m_read_in.head();
  m_read_zs->avail_in = m_read_in.used();
  
  int prev_out = m_read_zs->total_out;
  m_read_zs->next_out = (Bytef*)m_read_out.tail();
  m_read_zs->avail_out = m_read_out.free();

  int ret = inflate(m_read_zs,flush);

  int in = m_read_zs->total_in - prev_in;
  if (in) m_read_in.pop(in);

  int out = m_read_zs->total_out - prev_out;
  if (out) m_read_out.push(out);

  GZIP_DEBUG_LOG("inflate(" << flush << "): " << ret <<
                 " in:" << in << " out:" << out);
  return ret;
}

  //=============================================================================
int GzipStream::deflate_buffer(int flush)
{
  int prev_in = m_write_zs->total_in;
  m_write_zs->next_in = (Bytef*)m_write_in.head();
  m_write_zs->avail_in = m_write_in.used();
  
  int prev_out = m_write_zs->total_out;
  m_write_zs->next_out = (Bytef*)m_write_out.tail();
  m_write_zs->avail_out = m_write_out.free();

  int ret = deflate(m_write_zs,flush);

  int in = m_write_zs->total_in - prev_in;
  if (in) m_write_in.pop(in);

  int out = m_write_zs->total_out - prev_out;
  if (out) m_write_out.push(out);

  GZIP_DEBUG_LOG("deflate(" << flush << "): " << ret <<
                 " in:" << in << " out:" << out);
  return ret;
}
  
};
