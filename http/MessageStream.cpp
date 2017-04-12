/* SconeServer (http://www.sconemad.com)

HTTP Message Stream

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

#include <http/MessageStream.h>
#include <http/ConnectionStream.h>
#include <http/Request.h>
#include <http/Response.h>
#include <http/HostMapper.h>
#include <http/Host.h>

#include <sconex/sconex.h>
#include <sconex/Buffer.h>
#include <sconex/VersionTag.h>
#include <sconex/Date.h>
#include <sconex/StreamSocket.h>
#include <sconex/Log.h>
#include <sconex/utils.h>

namespace http {
  
//=============================================================================
MessageStream::MessageStream(HTTPModule* module,
			     ConnectionStream& httpstream,
			     Request* request)
  : scx::Stream("http:message"),
    m_module(module),
    m_httpstream(httpstream),
    m_request(request),
    m_response(new Response()),
    m_error_response(false),
    m_transparent(false),
    m_bytes_read(0),
    m_bytes_readable(-1),
    m_buffer(0),
    m_headers_sent(false),
    m_bytes_written(0),
    m_write_chunked(false),
    m_write_remaining(0),
    m_finished(false)
{
  // Set HTTP version to match request
  m_response.object()->set_version(request->get_version());

  // Add standard headers
  m_response.object()->set_header("Server",
    "SconeServer/" + scx::version().get_string());
  m_response.object()->set_header("Date",scx::Date::now().string());
}

//=============================================================================
MessageStream::~MessageStream()
{
  delete m_buffer;
}

//=============================================================================
scx::Condition MessageStream::event(scx::Stream::Event e)
{
  switch (e) {

    case scx::Stream::Opening: {
      if (m_bytes_readable < 0) {
        m_bytes_readable = 0;
        // See how much should be read for this message
        const std::string& clength = 
	  m_request.object()->get_header("Content-Length");
        if (!clength.empty()) {
          m_bytes_readable = atoi(clength.c_str());
        }
      }
  
      if (!handle_request()) {
        return scx::Close;
      }
    } break;
    
    case scx::Stream::Closing: {
    
      if (!m_headers_sent) {
        if (m_buffer) {
          return write_header();
        } else {
          send_simple_response(m_response.object()->get_status());
          return scx::Wait;
        }
      }

      DEBUG_ASSERT(m_bytes_readable == m_bytes_read,"event(Closing) Message not read");
      
      if (m_write_chunked) {
        DEBUG_ASSERT(m_write_remaining==0,"event(Closing) Written incomplete chunk");
        std::ostringstream oss;
        oss << CRLF << 0 << CRLF << CRLF;
        Stream::write(oss.str());
        
      } else {
        std::string slen = m_response.object()->get_header("Content-Length");
        if (!slen.empty()) {
          int len = atoi(slen.c_str());
          DEBUG_ASSERT(m_bytes_written==len,"event(Closing) Incomplete message");
        }      
      }
    } break;

    case scx::Stream::Writeable: { // WRITEABLE
      if (!m_headers_sent && m_buffer) {
        write_header();
      }
      if (m_headers_sent) {
        enable_event(scx::Stream::Writeable,false);
        if (m_finished) {
          DEBUG_LOG("Doing the end thing");
          return scx::End; //WHY?
        }
      }
    } break;

    default:
      break;
  }
  
  return scx::Ok;
}

//=========================================================================
scx::Condition MessageStream::read(void* buffer,int n,int& na)
{
  if (m_transparent) return Stream::read(buffer,n,na);

  if (m_bytes_read >= m_bytes_readable) {
    // Read all that is allowed, finish
    na = 0;
    return scx::End;
  }

  if ((n + m_bytes_read) > m_bytes_readable) {
    //    STREAM_DEBUG_LOG("Trying to read more than Content-Length, truncating");
    //    STREAM_DEBUG_LOG("n=" << n <<
    //                     ", read=" << m_bytes_read <<
    //                     ", readable=" << m_bytes_readable);
    n = m_bytes_readable - m_bytes_read;
  }

  scx::Condition c = Stream::read(buffer,n,na);
  //  STREAM_DEBUG_LOG("MessageStream::read(" << n << ") returned " << c << " na=" << na);

  // Update counter
  m_bytes_read += na;
  DEBUG_ASSERT(m_bytes_read <= m_bytes_readable,"Read more than is allowed");

  return c;
}

//=============================================================================
scx::Condition MessageStream::write(const void* buffer,int n,int& na)
{
  scx::Condition c = scx::Ok;

  if (!m_headers_sent) {
    if (!m_buffer) {
      build_header();
    }
    write_header();
  }

  if (m_transparent) return Stream::write(buffer,n,na);
  
  if (m_headers_sent) {
    if (m_write_chunked) {
      if (m_write_remaining <= 0) {
	std::ostringstream oss;
	oss << (m_write_remaining==0 ? CRLF : "") 
	    << std::setbase(16) << n << CRLF;

	Stream::write(oss.str());
	m_write_remaining = n;
      }
      
      if (n==0) {
	m_finished = true;
	return c;
      }
    }

    c = Stream::write(buffer,n,na);

    m_bytes_written += na;
    m_write_remaining -= na;
  }

  return c;
}

//=============================================================================
std::string MessageStream::stream_status() const
{
  std::ostringstream oss;
  oss << m_response.object()->get_status().code();
  if (m_headers_sent) oss << " HDRS";
  if (m_write_chunked) {
    oss << " chunk-rem:" << m_write_remaining;
  }
  if (m_finished) oss << " FIN";
  oss << " w:" << m_bytes_written;
  oss << " r:" << m_bytes_read << "/" << m_bytes_readable;
  if (m_buffer) oss << " buf:" << m_buffer->status_string() << " ";
  return oss.str();
}

//=============================================================================
void MessageStream::send_continue()
{
  Stream::write("HTTP/1.0 100 Continue\r\n\r\n");
}

//=============================================================================
void MessageStream::send_simple_response(Status status)
{
  m_response.object()->set_status(status);
  int na=0;
  if (status.has_body()) {
    std::ostringstream oss;
    oss << "<html>"
        << "<body>"
        << "<h1>" << status.desc() << "</h1>"
        << "<hr>"
        << "<address>" << "SconeServer/" << scx::version().get_string() << "</address>"
        << "</body>"
        << "</html>";
    std::string str = oss.str();
    m_response.object()->set_header("Content-Type","text/html");
    write(str.c_str(), str.length(), na);
  } else {
    write("", 0, na);
  }
}

//=============================================================================
void MessageStream::set_transparent()
{
  m_transparent = true;
  build_header();
  write_header();
}

//=============================================================================
void MessageStream::add_stream(scx::Stream* stream)
{
  endpoint().add_stream(stream);
}
  
//=============================================================================
HTTPModule& MessageStream::get_module()
{
  return *m_module.object();
}

//=============================================================================
void MessageStream::log(const std::string& message)
{
  scx::Log("http").
    attach("id", m_request.object()->get_id()).
    submit(message);
}

//=============================================================================
const Request& MessageStream::get_request() const
{
  return *m_request.object();
}

//=============================================================================
Response& MessageStream::get_response()
{
  return *m_response.object();
}

//=============================================================================
bool MessageStream::handle_request()
{
  // Pass through to host mapper for connection to appropriate host object
  HostMapper& mapper = m_module.object()->get_hosts();
  bool success = mapper.connect_request(this,
                                        *m_request.object(),
                                        *m_response.object());
  scx::Log log("http");
  log.attach("id", m_request.object()->get_id());
  log.attach("uri", m_request.object()->get_uri().get_string());
  
  const scx::StreamSocket* sock =
    dynamic_cast<const scx::StreamSocket*>(&endpoint());
  const scx::SocketAddress* addr = sock->get_remote_addr();
  log.attach("peer", addr->get_string());
  
  log.attach("referer", m_request.object()->get_header("Referer"));
  log.attach("user-agent", m_request.object()->get_header("User-Agent"));
  const Host* host = m_request.object()->get_host();
  if (host) {
    log.attach("host", host->get_id());
  }
  Session* session = m_request.object()->get_session();
  if (session) {
    log.attach("session", session->get_id());
  }
  log.submit(m_request.object()->get_method());

  return success;
}
  
//=============================================================================
bool MessageStream::build_header()
{
  // Should a persistant connection be used?
  // Used by default for HTTP versions greater than 1.0
  bool persist = (m_response.object()->get_version() > scx::VersionTag(1,0));
  if (persist) {
    // Check if either the request or response have specified 
    // Connection: close, if so then respect this and switch off persistant 
    // connections.
    std::string req_con = 
      m_request.object()->get_header("Connection"); scx::strlow(req_con);
    std::string resp_con = 
      m_response.object()->get_header("Connection"); scx::strlow(resp_con);
    if (req_con == "close") {
      // Copy to response
      m_response.object()->set_header("Connection","close");
      persist = false;
    } else if (resp_con == "close") {
      persist = false;
    }
  }

  if (m_response.object()->get_status().has_body()) {
    // Do we know the content length
    if (m_response.object()->get_header("Content-Length").empty()) {
      if (persist) {
	// Use chunked encoding
	m_write_chunked = true;
	m_write_remaining = -1;
	m_response.object()->set_header("Transfer-Encoding","chunked");
	m_response.object()->remove_header("Connection");
      }
    }

  } else {
    m_response.object()->remove_header("Content-Length");

  }
  
  // Tell the connection stream
  m_httpstream.set_persist(persist);

  // Build the response header and place in buffer
  std::string str = m_response.object()->build_header_string();
  m_buffer = new scx::Buffer(str.length());
  m_buffer->push_string(str);

  enable_event(scx::Stream::Writeable,true);
  return true;
}

//=============================================================================
scx::Condition MessageStream::write_header()
{
  int na = 0;
  scx::Condition c = Stream::write(m_buffer->head(),m_buffer->used(),na);
  m_buffer->pop(na);
  
  if (m_buffer->used() == 0) {
    // Finished sending header
    delete m_buffer;
    m_buffer = 0;
    m_headers_sent = true;
  }

  return c;
}

};
