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

#include "http/MessageStream.h"
#include "http/ConnectionStream.h"
#include "http/Request.h"
#include "http/HostMapper.h"
#include "http/Host.h"
#include "http/DocRoot.h"

#include "sconex/sconex.h"
#include "sconex/Buffer.h"
#include "sconex/VersionTag.h"
#include "sconex/Date.h"
#include "sconex/StreamSocket.h"

namespace http {
  
const char* CRLF = "\r\n";

//=============================================================================
MessageStream::MessageStream(
  HTTPModule& module,
  ConnectionStream& httpstream,
  Request* request
)
  : scx::Stream("http:message"),
    m_module(module),
    m_httpstream(httpstream),
    m_request(request),
    m_status(Status::Ok),
    m_headers_sent(false),
    m_error_response(false),
    m_buffer(0),
    m_write_chunked(false),
    m_write_remaining(0),
    m_finished(false),
    m_bytes_written(0),
    m_bytes_read(0),
    m_bytes_readable(-1)
{
  // Set HTTP version to match request
  m_version = request->get_version();

  // Add standard headers
  set_header("Server","SconeServer/" + scx::version().get_string());
  set_header("Date",scx::Date::now().string());
}

//=============================================================================
MessageStream::~MessageStream()
{
  delete m_request;
  delete m_buffer;
}

//=============================================================================
scx::Condition MessageStream::event(scx::Stream::Event e)
{
  switch (e) {

    case scx::Stream::Opening: {
      if (!connect_request_module(false)) {
        return scx::Close;
      }
    } break;
    
    case scx::Stream::Closing: {
    
      if (!m_headers_sent) {
        if (!m_buffer) {
          if (!m_error_response) {
            m_error_response = true;
            if (connect_request_module(true)) {
              // Cancel shutdown for now
              return scx::End;
            }
          }
          build_header();
          m_finished = true;
        }
        return write_header();
      }
      
      if (m_write_chunked) {
        DEBUG_ASSERT(m_write_remaining==0,"event(Closing) Written incomplete chunk");
        std::ostringstream oss;
        oss << CRLF << 0 << CRLF << CRLF;
        Stream::write(oss.str());
        
      } else {
        std::string slen = m_headers.get("Content-Length");
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
          return scx::End;
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
  if (m_bytes_readable < 0) {
    m_bytes_readable = 0;
    // See how much should be read
    const std::string& clength = m_request->get_header("Content-Length");
    if (!clength.empty()) {
      m_bytes_readable = atoi(clength.c_str());
    }
  }
  
  if (m_bytes_read >= m_bytes_readable) {
    // Read all that is allowed, finish
    na = 0;
    return scx::Close;
  }

  if ((n + m_bytes_read) > m_bytes_readable) {
    STREAM_DEBUG_LOG("Trying to read more than Content-Length, truncating");
    DEBUG_LOG("n=" << n <<
              ", read=" << m_bytes_read <<
              ", readable=" << m_bytes_readable);
    n = m_bytes_readable - m_bytes_read;
  }

  scx::Condition c = Stream::read(buffer,n,na);

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
  oss << m_status.code();
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
void MessageStream::set_version(const scx::VersionTag& version)
{
  m_version = version;
}

//=============================================================================
const scx::VersionTag& MessageStream::get_version() const
{
  return m_version;
}

//=============================================================================
void MessageStream::set_status(const Status& status)
{
  m_status = status;
}

//=============================================================================
const Status& MessageStream::get_status() const
{
  return m_status;
}

//=============================================================================
void MessageStream::set_header(
  const std::string& name,
  const std::string& value
)
{
  m_headers.set(name,value);
}

//=============================================================================
std::string MessageStream::get_header(const std::string& name) const
{
  return m_headers.get(name);
}

//=============================================================================
const Request& MessageStream::get_request() const
{
  return *m_request;
}

//=============================================================================
void MessageStream::set_path(const scx::FilePath& path)
{
  m_path = path;
}

//=============================================================================
const scx::FilePath& MessageStream::get_path() const
{
  return m_path;
}

//=============================================================================
void MessageStream::set_docroot(DocRoot* docroot)
{
  m_docroot = docroot;
}

//=============================================================================
DocRoot* MessageStream::get_docroot()
{
  return m_docroot;
}

//=============================================================================
Host* MessageStream::get_host()
{
  return m_host;
}

//=============================================================================
bool MessageStream::connect_request_module(bool error)
{
  const scx::Uri& uri = m_request->get_uri();
  
  // Log request
  std::ostringstream oss;
  oss << m_httpstream.get_num_connection() << "-"
      << m_httpstream.get_num_request();
  const std::string& id = oss.str();
  
  const scx::StreamSocket* sock =
    dynamic_cast<const scx::StreamSocket*>(&endpoint());
  const scx::SocketAddress* addr = sock->get_remote_addr();

  if (!error) {
    m_module.log(id + " " + addr->get_string() + " " +
                 m_request->get_method() + " " + uri.get_string());
    
    const std::string& referer = m_request->get_header("Referer");
    if (!referer.empty()) {
      m_module.log(id + " Referer: " + referer);
    }
    
    const std::string& useragent = m_request->get_header("User-Agent");
    if (!useragent.empty()) {
      m_module.log(id + " User-Agent: " + useragent);
    }
  }
  
  // Lookup host object
  m_host = m_module.get_host_mapper().host_lookup(uri.get_host());
  if (m_host==0) {
    // This is bad, user should have setup a default host
    m_module.log(id + " Unknown host '" + uri.get_host() + "'",
                 scx::Logger::Error);
    set_status(http::Status::NotFound);
    return false;
  }
  
  return m_host->connect_request(&endpoint(),*this);
}
  
//=============================================================================
bool MessageStream::build_header()
{
  // Should persistant connections be used
  bool persist = (m_version > scx::VersionTag(1,0));
  if (m_request->get_header("CONNECTION") == "close") {
    m_headers.set("Connection","close");
    persist = false;
  }

  // Can a body be sent with this response type
  bool body = (m_status.code() >= 200 &&
	       m_status.code() != 204 &&
	       m_status.code() != 304);
  
  if (body) {
    // Do we know the content length
    if (m_headers.get("Content-Length").empty()) {
      if (persist) {
	// Use chunked encoding
	m_write_chunked = true;
	m_write_remaining = -1;
	m_headers.set("Transfer-Encoding","chunked");
	m_headers.erase("Connection");
      }
    }

  } else {
    m_headers.erase("Content-Length");

  }

  // Tell the connection stream
  m_httpstream.set_persist(persist);
  
  // Write the header into the buffer
  std::string str = "HTTP/" + m_version.get_string() + " " + 
    m_status.string() + CRLF +
    m_headers.get_all() + CRLF;
  
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
