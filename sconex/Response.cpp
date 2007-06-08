/* SconeServer (http://www.sconemad.com)

Simple response stream

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

#include "sconex/Response.h"
namespace scx {

//=============================================================================
Response::Response(
  const std::string& text
) : StreamBuffer(0,text.length())
{
  DEBUG_COUNT_CONSTRUCTOR(Response);

  m_write_buffer.push_string(text);

  enable_event(Stream::Writeable,true);
}

//=============================================================================
Response::Response(
  const void* buffer,
  int n
) : StreamBuffer(0,n)
{
  DEBUG_COUNT_CONSTRUCTOR(Response);

  m_write_buffer.push_from(buffer,n);

  enable_event(Stream::Writeable,true);
}
  
//=============================================================================
Response::~Response()
{
  DEBUG_COUNT_DESTRUCTOR(Response);
}

//=============================================================================
Condition Response::event(Stream::Event e)
{
  Condition c = StreamBuffer::event(e);

  if ((e == Stream::Writeable) && c == scx::Ok) {
    return scx::Close;
  }
 
  return c;
}

//=============================================================================
std::string Response::stream_status() const
{
  return m_write_buffer.status_string();
}

};

