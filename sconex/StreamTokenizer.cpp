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

#include <sconex/StreamTokenizer.h>
namespace scx {

//=============================================================================
StreamTokenizer::StreamTokenizer(
  const std::string& stream_name,
  int buffer_size
)
  : Stream(stream_name),
    m_buffer(buffer_size),
    m_overflow(false),
    m_line(0)
{
  DEBUG_COUNT_CONSTRUCTOR(StreamTokenizer);
}

//=============================================================================
StreamTokenizer::~StreamTokenizer()
{
  DEBUG_COUNT_DESTRUCTOR(StreamTokenizer);
}

//=============================================================================
Condition StreamTokenizer::tokenize(std::string& token)
{
  bool prev_overflow=m_overflow;
  m_overflow=false;
  int pre_skip=0;
  int token_len=0;
  int post_skip=0;

  bool found_token = next_token(m_buffer,pre_skip,token_len,post_skip);
  if (!found_token) {
    
    m_buffer.compact();
    int avail = m_buffer.free();
    if (avail==0) {
      m_overflow=true;
      token = std::string();
      return scx::Error;
    }

    int nr=0;
    Condition c = Stream::read(m_buffer.tail(),avail,nr);
    enable_event(Stream::SendReadable,m_buffer.used()); 
    if (nr <= 0) {
      // No more bytes available from source
      token = std::string();
      return c;
    }
    m_buffer.push(nr);

    pre_skip=0;
    token_len=0;
    post_skip=0;
    found_token = next_token(m_buffer,pre_skip,token_len,post_skip);
    
    if (!found_token) {
      if (m_buffer.free() > 0) {
        // No more bytes available right now
        token = std::string();
        return scx::Wait;
      } else {
        // Buffer overflow
        m_overflow=true;
        token_len=m_buffer.used();
	return scx::Error;
      }
    }
  }

  if (m_line > 0) {
    // Perform line number tracking
    std::string whole((char*)m_buffer.head(),pre_skip + token_len + post_skip);
    std::string::size_type s = 0;
    while (true) {
      s = whole.find("\n",s);
      if (s == std::string::npos) {
	break;
      }
      ++s;
      ++m_line;
    }
  }

  token = std::string((char*)m_buffer.head()+pre_skip,token_len);
  m_buffer.pop(pre_skip+token_len+post_skip);
  enable_event(Stream::SendReadable,m_buffer.used()); 

  if (prev_overflow) {
    if (found_token) {
      m_overflow=false;
    }
    return scx::Error;
  }

  return scx::Ok;  
}

//=============================================================================
Condition StreamTokenizer::read(void* buffer,int n,int& na)
{
  na = 0;

  if (m_buffer.used()) {
    na += m_buffer.pop_to(buffer,n);
    if (na == n) {
      enable_event(Stream::SendReadable,m_buffer.used()); 
      return Ok;
    }
  }

  int left = n-na;
  if (left >= m_buffer.free()) {
    int nr = 0;
    Condition c = Stream::read((char*)buffer+na,left,nr);
    if (nr > 0) na += nr;
    enable_event(Stream::SendReadable,m_buffer.used());
    return c;
  }

  int nr = 0;
  Condition c = Stream::read(m_buffer.tail(),m_buffer.free(),nr);
  if (nr > 0) {
    m_buffer.push(nr);
    na += m_buffer.pop_to((char*)buffer+na,left);
  }

  enable_event(Stream::SendReadable,m_buffer.used()); 
  return c;
}

//=============================================================================
std::string StreamTokenizer::stream_status() const
{
  std::ostringstream oss;
  oss << "tok:" << m_buffer.status_string();
  if (m_overflow) oss << " OF!";
  return oss.str();
}

//=============================================================================
bool StreamTokenizer::overflow()
{
  return m_overflow;
}

};
