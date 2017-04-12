/* SconeServer (http://www.sconemad.com)

HTTP Message Stream

Manages the sending of an individual message within a HTTP connection,
prepending headers where appropriate.

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

#ifndef httpMessageStream_h
#define httpMessageStream_h

#include <http/HTTPModule.h>
#include <http/Status.h>
#include <http/Request.h>
#include <http/Response.h>
#include <sconex/Stream.h>
#include <sconex/VersionTag.h>
#include <sconex/FilePath.h>
#include <sconex/MimeHeader.h>
namespace scx { class Buffer; };

namespace http {

class ConnectionStream;
class Host;

// Macro to find the current http::MessageStream from within another stream
#define GET_HTTP_MESSAGE() dynamic_cast<http::MessageStream*>(find_stream("http:message"))
  
//=============================================================================
class HTTP_API MessageStream : public scx::Stream {
public:

  MessageStream(HTTPModule* module,
		ConnectionStream& httpstream,
		Request* request);
  
  virtual ~MessageStream();

  virtual scx::Condition event(scx::Stream::Event e);

  virtual scx::Condition read(void* buffer,int n,int& na);
  virtual scx::Condition write(const void* buffer,int n,int& na);

  virtual std::string stream_status() const;
 
  void send_continue();
  void send_simple_response(Status status);
  void set_transparent();

  void add_stream(scx::Stream* stream);
  
  HTTPModule& get_module();

  // Log message with id
  void log(const std::string& message);

  // Request:
  const Request& get_request() const;

  // Response:
  Response& get_response();
  
private:

  bool handle_request();
  
  bool build_header();
  scx::Condition write_header();
  
  HTTPModule::Ref m_module;
  ConnectionStream& m_httpstream;
  Request::Ref m_request;
  Response::Ref m_response;

  bool m_error_response;
  bool m_transparent;

  // Read
  int m_bytes_read;
  int m_bytes_readable;

  // Write
  scx::Buffer* m_buffer;
  bool m_headers_sent;
  int m_bytes_written;
  bool m_write_chunked;
  int m_write_remaining;
  bool m_finished;
};

};
#endif
