/* SconeServer (http://www.sconemad.com)

HTTP Client

Copyright (c) 2000-2011 Andrew Wedgbury <wedge@sconemad.com>

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

#include <http/Client.h>
#include <http/MessageStream.h>
#include <http/Status.h>

#include <sconex/Logger.h>
#include <sconex/Response.h>
#include <sconex/VersionTag.h>
#include <sconex/utils.h>
#include <sconex/StreamSocket.h>
#include <sconex/Kernel.h>
#include <sconex/ScriptTypes.h>
#include <sconex/ScriptExpr.h>
namespace http {

//=============================================================================
Client::Client(HTTPModule* module,
               const std::string& method,
               const scx::Uri& url)
  : m_module(module),
    m_request(new Request("client","")),
    m_response(new Response()),
    m_error(false)
{
  DEBUG_COUNT_CONSTRUCTOR(HTTPClient);
  m_request.object()->set_method(method);
  m_request.object()->set_uri(url);
}

//=============================================================================
Client::Client(const Client& c)
  : ScriptObject(c),
    m_module(c.m_module),
    m_request(c.m_request),
    m_response(c.m_response),
    m_response_data(c.m_response_data),
    m_error(c.m_error)
{
  DEBUG_COUNT_CONSTRUCTOR(HTTPClient);
}

//=============================================================================
Client::~Client()
{
  DEBUG_COUNT_DESTRUCTOR(HTTPClient);
}

//=============================================================================
scx::ScriptObject* Client::new_copy() const
{
  return new Client(*this);
}

//=============================================================================
std::string Client::get_string() const
{
  return m_request.object()->get_method() + " " + 
         m_request.object()->get_uri().get_string();
}
  
//=============================================================================
int Client::get_int() const
{
  return !m_error;
}

//=============================================================================
scx::ScriptRef* Client::script_op(const scx::ScriptAuth& auth,
				  const scx::ScriptRef& ref,
				  const scx::ScriptOp& op,
				  const scx::ScriptRef* right)
{
  if (op.type() == scx::ScriptOp::Lookup) {
    const std::string name = right->object()->get_string();

    if (name == "run") 
      return new scx::ScriptMethodRef(ref,name);

    if (name == "request") return m_request.ref_copy(ref.reftype());
    if (name == "response") return m_response.ref_copy(ref.reftype());
    if (name == "data") return scx::ScriptString::new_ref(m_response_data);
  }    

  return scx::ScriptObject::script_op(auth,ref,op,right);
}

//=============================================================================
scx::ScriptRef* Client::script_method(const scx::ScriptAuth& auth,
				      const scx::ScriptRef& ref,
				      const std::string& name,
				      const scx::ScriptRef* args)
{
  if ("run" == name) {
    const scx::ScriptObject* a_data = 
      scx::get_method_arg<scx::ScriptObject>(args,0,"data");
    std::string data = (a_data ? a_data->get_string() : "");
    
    bool success = run(data);
    return scx::ScriptInt::new_ref(success);
  }

  return scx::ScriptObject::script_method(auth,ref,name,args);
}

//=============================================================================
bool Client::run(const std::string& request_data)
{
  bool proxy = false;
  scx::Uri addr_url = m_module.object()->get_client_proxy();
  if (addr_url.get_int()) {
    // Connect to proxy
    proxy = true;
  } else {
    // Connect direct to host given in uri
    addr_url = m_request.object()->get_uri();
  }
  
  // Lookup the ip module and use to create an IP4 socket address
  scx::SocketAddress* addr = 0;    
  scx::Module::Ref ip = scx::Kernel::get()->get_module("ip");
  if (!ip.valid()) {
    DEBUG_LOG("IP address support not available");
    return false;
  }

  scx::ScriptList::Ref args(new scx::ScriptList());
  args.object()->give( scx::ScriptString::new_ref(addr_url.get_host()) );
  args.object()->give( scx::ScriptInt::new_ref(addr_url.get_port()) );

  scx::ScriptObject* addr_obj = scx::ScriptExpr::create_object("IPAddr",&args);
  addr = dynamic_cast<scx::SocketAddress*>(addr_obj);

  if (addr == 0) {
    DEBUG_LOG("Unable to create socket address");
    return false;
  }
  
  // Create the socket  
  scx::StreamSocket* sock = new scx::StreamSocket();

  // Set idle timeout
  sock->set_timeout(scx::Time(m_module.object()->get_idle_timeout()));
  
  // Is this a secure http connection?
  if (m_request.object()->get_uri().get_scheme() == "https") {
    
    // If a proxy is in use, add a connect stream to setup the tunnel
    if (proxy) {
      sock->add_stream( new ProxyConnectStream(m_module.object(),
					       *m_request.object()) );
    }
    
    // Connect an SSL stream to this socket
    scx::ScriptList::Ref args(new scx::ScriptList());
    args.object()->give( scx::ScriptString::new_ref("client") );
    scx::Stream* ssl = scx::Stream::create_new("ssl",&args);
    if (!ssl) {
      delete sock;
      delete addr;
      DEBUG_LOG("SSL support unavailable or not configured");
      return false;
    }
    sock->add_stream(ssl);
  }

  // Add client stream
  ClientStream* cs = new ClientStream(m_module.object(),this,
				      *m_request.object(),
				      request_data,
				      *m_response.object(),
				      m_response_data);
  sock->add_stream(cs);

  // Start the socket connection
  scx::Condition err = sock->connect(addr);
  if (err != scx::Ok && err != scx::Wait) {
    delete sock;
    delete addr;
    DEBUG_LOG("Unable to initiate connection");
    return false;
  }
  delete addr;

  // Give to the kernel for async processing
  if (!scx::Kernel::get()->connect(sock)) {
    delete sock;
    DEBUG_LOG("System failure");
    return false;
  }

  // Now wait for the completion signal
  m_mutex.lock();
  m_complete.wait(m_mutex);
  m_mutex.unlock();

  return !m_error;
}

//=============================================================================
const Response& Client::get_response() const
{
  return *m_response.object();
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
ClientStream::ClientStream(HTTPModule* module,
			   Client* client,
			   Request& request,
			   const std::string& request_data,
			   Response& response,
			   std::string& response_data)
  : scx::LineBuffer("http:client",1024),
    m_module(module),
    m_client(client),
    m_request(request),
    m_request_data(request_data),
    m_response(response),
    m_response_data(response_data),
    m_seq(Send),
    m_buffer(0)
{
  build_request(m_request.get_method(),m_request.get_uri());
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
    
    case scx::Stream::Writeable: { // WRITEABLE
      if (m_seq == Send) {
        int na = 0;
        scx::Condition c = Stream::write(m_buffer->head(),m_buffer->used(),na);
	if (c != scx::Ok && c != scx::Wait) {
	  return c;
	}
        m_buffer->pop(na);
        
        if (m_buffer->used() == 0) {
          // Finished sending request
          delete m_buffer;
          m_buffer = 0;
          std::string method = m_request.get_method();
	  m_seq = RecieveResponse;
	  enable_event(scx::Stream::Writeable,false);
	  enable_event(scx::Stream::Readable,true);
        } else {
	  return scx::Wait;
	}
      }

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
            endpoint().reset_timeout();
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
          endpoint().reset_timeout();
	}
	if (c != scx::Ok && c != scx::Wait) {
	  enable_event(scx::Stream::Readable,false);
	  m_seq = End;
	  m_client->event_complete(false);
	  return c;
	}
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
  return scx::StreamTokenizer::read(buffer,n,na);
}

//=============================================================================
scx::Condition ClientStream::write(const void* buffer,int n,int& na)
{
  return scx::StreamTokenizer::write(buffer,n,na);
}

//=============================================================================
std::string ClientStream::stream_status() const
{
  std::ostringstream oss;
  oss << scx::StreamTokenizer::stream_status()
      << " seq:";
  switch (m_seq) {
    case Send: oss << "SEND"; break;
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
    m_request.set_header("Content-Type","application/x-www-form-urlencoded");
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


//=============================================================================
ProxyConnectStream::ProxyConnectStream(HTTPModule* module,
				       Request& request) 
  : scx::LineBuffer("http:proxy-connect",1024),
    m_module(module),
    m_request(request),
    m_seq(Send)
{
  enable_event(scx::Stream::Writeable,true);
}

//=============================================================================
ProxyConnectStream::~ProxyConnectStream()
{

}

//=============================================================================
scx::Condition ProxyConnectStream::event(scx::Stream::Event e)
{
  switch (e) {
    
    case scx::Stream::Opening: { // OPENING
      if (m_seq != Established) {
	return scx::Wait;
      }
    } break;

    case scx::Stream::Writeable: { // WRITEABLE

      if (m_seq == Send) {
	scx::Uri url = m_request.get_uri();
	std::ostringstream oss;
	oss << "CONNECT " << url.get_host() << ":" << url.get_port() << " HTTP/1.1\r\n\r\n";
	write(oss.str());
	m_seq = RecieveResponse;
	enable_event(scx::Stream::Writeable,false);
	enable_event(scx::Stream::Readable,true);
      }

    } break;

    case scx::Stream::Readable: { // READABLE
      std::string line;

      if (m_seq == RecieveResponse) {
        if (scx::Ok == tokenize(line)) {
          if (m_response.parse_response(line)) {
	    // Check the status code to see if the proxy accepted
	    if (m_response.get_status().code() != Status::Ok) {
	      DEBUG_LOG("PROXY CONNECT FAILED: " << line);
	      return scx::Error;
	    }
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
	    // Tunnel is established, our job is done!
	    m_seq = Established;
	    enable_event(scx::Stream::Readable,false);
	    break;
          } else {
            if (!m_response.parse_header(line)) {
              // went wrong!
	      return scx::Error;
            }
            endpoint().reset_timeout();
          }
        }
      }

    } break;

    default: 
      break;
  }
  
  return scx::Ok;
}

//=============================================================================
std::string ProxyConnectStream::stream_status() const
{
  std::ostringstream oss;
  oss << scx::StreamTokenizer::stream_status()
      << " seq:";
  switch (m_seq) {
    case Send: oss << "SEND"; break;
    case RecieveResponse: oss << "RECV-RESP"; break;
    case RecieveHeaders: oss << "RECV-HEADERS"; break;
    case Established: oss << "ESTABLISHED"; break;
    default: oss << "UNKNOWN!"; break;
  }
  return oss.str();
}
  
};
