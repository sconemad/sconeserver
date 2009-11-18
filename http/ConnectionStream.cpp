/* SconeServer (http://www.sconemad.com)

HTTP Connection Stream

Copyright (c) 2000-2005 Andrew Wedgbury <wedge@sconemad.com>

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

#include "http/HTTPModule.h"
#include "http/ConnectionStream.h"
#include "http/Request.h"
#include "http/MessageStream.h"
#include "http/Status.h"

#include "sconex/Logger.h"
#include "sconex/Response.h"
#include "sconex/VersionTag.h"
#include "sconex/utils.h"
namespace http {

int connection_count=0;

//=============================================================================
ConnectionStream::ConnectionStream(
  HTTPModule& module,
  const std::string& profile
) : scx::LineBuffer("http:connection",1024),
    m_module(module),
    m_request(0),
    m_profile(profile),
    m_seq(http_Request),
    m_num_connection(0),
    m_num_request(0)
{
  enable_event(scx::Stream::Readable,true);
}

//=============================================================================
ConnectionStream::~ConnectionStream()
{
  delete m_request;
}

//=============================================================================
scx::Condition ConnectionStream::event(scx::Stream::Event e)
{
  switch (e) {
    
    case scx::Stream::Opening: { // OPENING
      m_num_connection = (++connection_count);
    } break;

    case scx::Stream::Closing: { // CLOSING
      if (m_persist && m_seq == http_Body) {
        m_seq = http_Request;
        scx::Condition c = process_input();
        if (c != scx::Close) {
          return scx::End;
        }
      }
    } return scx::Ok;
    
    case scx::Stream::Readable: { // READABLE
      if (m_seq != http_Body) {
        return process_input();
      }
    } break;

    default:
      break;
  }
  
  return scx::Ok;
}

//=============================================================================
std::string ConnectionStream::stream_status() const
{
  std::ostringstream oss;
  oss << scx::StreamTokenizer::stream_status()
      << " " << m_num_connection << "-" << m_num_request
      << " prf:" << m_profile
      << " seq:";
  switch (m_seq) {
    case http_Request: oss << "REQUEST"; break;
    case http_Headers: oss << "HEADERS"; break;
    case http_Body: oss << "BODY"; break;
    default: oss << "UNKNOWN!"; break;
  }
  return oss.str();
}

//=============================================================================
void ConnectionStream::set_persist(bool persist)
{
  m_persist = persist;
}

//=============================================================================
bool ConnectionStream::get_persist() const
{
  return m_persist;
}

//=============================================================================
int ConnectionStream::get_num_connection() const
{
  return m_num_connection;
}

//=============================================================================
int ConnectionStream::get_num_request() const
{
  return m_num_request;
}

//=============================================================================
scx::Condition ConnectionStream::process_input()
{
  scx::Condition c;
  std::string line;
  
  while ( (c=tokenize(line)) == scx::Ok ) {

    if (line.empty() && m_request) { // ACTION STAGE
      
      if (!process_request(m_request)) {
	// Something went badly wrong, send a simple response
	Status status(Status::NotImplemented);
	std::string str = std::string("HTTP/1.0 " + status.string() +
				      "\r\n\r\n");
	endpoint().add_stream( new scx::Response(str) );
        m_seq = http_Request;
	return scx::End;
      }
      m_seq = http_Body;
      return scx::Ok;
      
    } else if (m_seq == http_Request) { // REQUEST STAGE

      delete m_request;
      std::ostringstream oss;
      oss << m_num_connection << "." << (m_num_request+1);
      m_request = new Request(m_profile,oss.str());
      if (m_request->parse_request(line,0!=find_stream("ssl"))) {
	m_seq = http_Headers;
	++m_num_request;
      }
      
    } else if (m_seq == http_Headers) { // HEADER STAGE
      
      m_request->parse_header(line);
    }
    
    endpoint().reset_timeout();
  }
  
  if (c == scx::End && m_seq == http_Request) {
    return scx::Close;
  }

  return c;
}

//=============================================================================
bool ConnectionStream::process_request(Request*& request)
{
  DEBUG_ASSERT(request,"ConnectionStream::process_request() NULL request object");

  // Create and add the message stream
  MessageStream* msg = new MessageStream(m_module,*this,request);
  msg->add_module_ref(m_module.ref());
  endpoint().add_stream(msg);

  request = 0;

  return true;
}

};
