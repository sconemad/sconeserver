/* SconeServer (http://www.sconemad.com)

HTTP Client

Copyright (c) 2000-2010 Andrew Wedgbury <wedge@sconemad.com>

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

#include "http/Client.h"
#include "http/MessageStream.h"
#include "http/Status.h"

#include "sconex/Logger.h"
#include "sconex/Response.h"
#include "sconex/VersionTag.h"
#include "sconex/utils.h"
#include "sconex/StreamSocket.h"
#include "sconex/Kernel.h"
namespace http {

//=============================================================================
Client::Client(HTTPModule& module)
  : m_module(module),
    m_stream(0),
    m_error(false)
{

}

//=============================================================================
Client::~Client()
{

}

//=============================================================================
Status Client::run(
  const std::string& method,
  const scx::Uri& url,
  const std::string& post_data)
{
  // Lookup the router.ip module and use to create an IP4 socket address
  scx::SocketAddress* addr = 0;    
  scx::ModuleRef router = scx::Kernel::get()->get_module("router");
  if (router.valid()) {
    scx::ModuleRef ip = router.module()->get_module("ip");
    if (ip.valid()) {
      scx::ArgList args;
      args.give( new scx::ArgString(url.get_host()) );
      args.give( new scx::ArgInt(url.get_port()) );
      scx::Arg* ret = ip.module()->arg_method(scx::Auth::Trusted,"addr",&args);
      addr = dynamic_cast<scx::SocketAddress*>(ret);
    }
  }

  // Create the socket  
  scx::StreamSocket* sock = new scx::StreamSocket();

  // Add SSL stream if its a secure http url
  if (url.get_scheme() == "https") {
    scx::ModuleRef ssl = scx::Kernel::get()->get_module("ssl");
    scx::ArgList args;
    args.give( new scx::ArgString("client") );
    ssl.module()->connect(sock,&args);
  }

  // Add client stream
  ClientStream* cs = new ClientStream(m_module,this,
				      method,url,post_data,
				      m_response,m_response_data);
  sock->add_stream(cs);

  // Start the socket connection and give to the kernel for async processing
  scx::Condition err = sock->connect(addr);
  scx::Kernel::get()->connect(sock,0);

  // Now wait for the completion signal
  m_mutex.lock();
  m_complete.wait(m_mutex);
  m_mutex.unlock();

  delete addr;

  return m_error ? Status((Status::Code)0) : m_response.get_status();
}

//=============================================================================
const Response& Client::get_response() const
{
  return m_response;
}

//=============================================================================
const std::string& Client::get_response_data() const
{
  return m_response_data;
}

//=============================================================================
void Client::event_complete(bool error)
{
  m_mutex.lock();

  m_error = error;
  m_complete.signal();

  m_mutex.unlock();
}


//=============================================================================
ClientStream::ClientStream(
  HTTPModule& module,
  Client* client,
  const std::string& method,
  const scx::Uri& url,
  const std::string& post_data,
  Response& response,
  std::string& response_data
) : scx::LineBuffer("http:client",1024),
    m_module(module),
    m_client(client),
    m_request("client",""),
    m_request_data(post_data),
    m_response(response),
    m_response_data(response_data),
    m_seq(SendHeaders),
    m_buffer(0)
{
  build_request(method,url);
  enable_event(scx::Stream::Writeable,true);
}

//=============================================================================
ClientStream::~ClientStream()
{
  delete m_buffer;
  if (m_seq != End) {
    m_client->event_complete(true);
  }
}

//=============================================================================
scx::Condition ClientStream::event(scx::Stream::Event e)
{
  switch (e) {
    
    case scx::Stream::Opening: { // OPENING

    } break;

    case scx::Stream::Closing: { // CLOSING

    } break;
    
    case scx::Stream::Readable: { // READABLE
      std::string line;

      if (m_seq == RecieveResponse) {
        if (scx::Ok == tokenize(line)) {
          if (m_response.parse_response(line)) {
	    DEBUG_LOG("HTTP RESPONSE: " << line);
            m_seq = RecieveHeaders;
          } else {
            // went wrong!
	    return scx::Error;
          }
        }
      }

      if (m_seq == RecieveHeaders) {
        while (scx::Ok == tokenize(line)) {
          if (line.empty()) {
	    std::string method = m_request.get_method();
	    if (method != "HEAD") {
	      // Response has a body
	      m_seq = RecieveBody;
	    } else {
	      enable_event(scx::Stream::Readable,false);
	      m_seq = End;
	      m_client->event_complete(false);
	      return scx::End;
	    }
	    break;
          } else {
            if (!m_response.parse_header(line)) {
              // went wrong!
	      return scx::Error;
            }
          }
        }
      }

      if (m_seq == RecieveBody) {
	char buffer[1024];
	int na=0;
	scx::Condition c = scx::StreamTokenizer::read(buffer,1023,na);
	if (na > 0) {
	  buffer[na] = '\0';
	  m_response_data += buffer;
	}
	if (c != scx::Ok && c != scx::Wait) {
	  enable_event(scx::Stream::Readable,false);
	  m_seq = End;
	  m_client->event_complete(false);
	  return c;
	}
      }

      //      if (m_seq != RecieveBody) {
      //        return scx::Wait;
      //      }
      
    } break;

    case scx::Stream::Writeable: { // WRITEABLE

      if (m_seq == SendHeaders) {
        int na = 0;
        scx::Condition c = Stream::write(m_buffer->head(),m_buffer->used(),na);
        m_buffer->pop(na);
        
        if (m_buffer->used() == 0) {
          // Finished sending request
          delete m_buffer;
          m_buffer = 0;
          std::string method = m_request.get_method();
	  //          if (method == "POST" ||
	  //              method == "PUT") {
            // Request requires a body
	  //            m_seq = SendBody;
	  //          } else {
            m_seq = RecieveResponse;
	    enable_event(scx::Stream::Writeable,false);
            enable_event(scx::Stream::Readable,true);
	    //          }
        } else {
	  return scx::Wait;
	}
      }

      if (m_seq == SendBody) {
	Stream::write(m_request_data);
	m_seq = RecieveResponse;
	enable_event(scx::Stream::Writeable,false);
	enable_event(scx::Stream::Readable,true);
      }

    } break;

    default: 
      break;
  }
  
  return scx::Ok;
}

//=============================================================================
scx::Condition ClientStream::read(void* buffer,int n,int& na)
{
  if (m_seq == RecieveBody) {
    return scx::StreamTokenizer::read(buffer,n,na);
  }
  na = 0;
  return scx::Wait;
}

//=============================================================================
scx::Condition ClientStream::write(const void* buffer,int n,int& na)
{
  if (m_seq == SendBody) {
    return scx::StreamTokenizer::write(buffer,n,na);
  }
  na = 0;
  return scx::Wait;
}

//=============================================================================
std::string ClientStream::stream_status() const
{
  std::ostringstream oss;
  oss << scx::StreamTokenizer::stream_status()
      << " seq:";
  switch (m_seq) {
    case SendHeaders: oss << "SEND-HEADERS"; break;
    case SendBody: oss << "SEND-BODY"; break;
    case RecieveResponse: oss << "RECV-RESP"; break;
    case RecieveHeaders: oss << "RECV-HEADERS"; break;
    case RecieveBody: oss << "RECV-BODY"; break;
    case End: oss << "END"; break;
    default: oss << "UNKNOWN!"; break;
  }
  return oss.str();
}

//=============================================================================
void ClientStream::build_request(const std::string& method, const scx::Uri& url)
{
  if (m_buffer) {
    // Clear existing buffer
    delete m_buffer;
    m_buffer = 0;
  }

  // Set version to HTTP/1.1
  m_request.set_version(scx::VersionTag(1,1));

  // Set method and url
  m_request.set_method(method);
  m_request.set_uri(url);

  // Fill in the host header from the url
  m_request.set_header("Host",url.get_host());

  // Identify ourselves
  m_request.set_header("User-Agent","SconeServer/" + scx::version().get_string());

  // Set content length for the request body if the method requires it
  int body_len = 0;
  if (method == "POST" || method == "PUT") {
    body_len = m_request_data.length();
    std::ostringstream oss;
    oss << body_len;
    m_request.set_header("Content-Length",oss.str());
  }

  // Other headers
  m_request.set_header("Connection","Close");

  // Push the request into a buffer ready for sending
  std::string hdrs = m_request.build_header_string();
  m_buffer = new scx::Buffer(hdrs.length() + body_len);
  m_buffer->push_string(hdrs);
  if (body_len > 0) {
    m_buffer->push_string(m_request_data);
  }
}
  
};
