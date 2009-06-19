/* SconeServer (http://www.sconemad.com)

CGI Response stream

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

#include "CGIResponseStream.h"

#include "http/MessageStream.h"

//=========================================================================
CGIResponseStream::CGIResponseStream(
  http::MessageStream* http_msg
) 
  : scx::LineBuffer("exec:cgi-response"),
    m_http_msg(http_msg),
    m_done_headers(false)
{
  DEBUG_ASSERT(http_msg,"Invalid http message passed");

  enable_event(scx::Stream::Readable,true);
}

//=========================================================================
CGIResponseStream::~CGIResponseStream()
{

}

//=========================================================================
scx::Condition CGIResponseStream::event(scx::Stream::Event e)
{
  if (m_done_headers) {
    return scx::Ok;
  }
  
  if (e == scx::Stream::Readable) {
    scx::Condition c;
    std::string line;
    
    while ( (c=tokenize(line)) == scx::Ok ) {

      if (line.empty()) {
        m_done_headers = true;
        enable_event(scx::Stream::Readable,false);
        return scx::Ok;
      }

      std::string::size_type i = line.find_first_of(":");
      if (i != std::string::npos) {
      
        std::string name = std::string(line,0,i);
        std::string value;
        
        if (i < line.length()) {
          i = line.find_first_not_of(" ",i+1);
          if (i != std::string::npos) {
            value = std::string(line,i);
          }
        }
        m_http_msg->get_response().set_header(name,value);
      }

    }
    return scx::Wait;
  }
  
  return scx::Ok;
}
