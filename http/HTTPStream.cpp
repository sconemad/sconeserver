/* SconeServer (http://www.sconemad.com)

HTTP Stream

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
#include "http/HTTPStream.h"
#include "http/Request.h"
#include "http/MessageStream.h"
#include "http/Status.h"
#include "http/HostMapper.h"
#include "http/Host.h"

#include "sconex/StreamSocket.h"
#include "sconex/Logger.h"
#include "sconex/Response.h"
#include "sconex/VersionTag.h"
#include "sconex/utils.h"
namespace http {

int connection_count=0;

//=============================================================================
HTTPStream::HTTPStream(
  HTTPModule& module
) : scx::LineBuffer("http",1024),
    m_module(module),
    m_request(0),
    m_seq(http_Request),
    m_num_connection(0),
    m_num_request(0)
{
  set_event_mask(scx::Stream::Readable);
}

//=============================================================================
HTTPStream::~HTTPStream()
{
  delete m_request;
}

//=============================================================================
scx::Condition HTTPStream::event(int type)
{
  // OPENED
  if (type & scx::Stream::Opened) {
    endpoint().reset_timeout(scx::Time(15));
    m_num_connection = (++connection_count);
  }
  
  // READABLE
  if (type & scx::Stream::Readable) {
    scx::Condition c;
    std::string line;

    while ( (c=tokenize(line)) == scx::Ok ) {
      
      if (line.empty() && m_request) { // ACTION STAGE

        if (!process_request(m_request)) {
          // Something went badly wrong, send a simple response
          m_module.log("No module defined to handle request",scx::Logger::Error);
          Status status(Status::NotImplemented);
          std::string str = std::string("HTTP/1.0 " + status.string() +
                                        "\r\n\r\n");
          endpoint().add_stream( new scx::Response(str) );
          return scx::End;
        }
        m_seq = http_Request;
        return scx::Ok;

      } else if (m_seq == http_Request) { // REQUEST STAGE

        delete m_request;
        m_request = new Request();
	if (m_request->parse_request(line)) {
	  m_seq = http_Headers;
	  ++m_num_request;
	}

      } else if (m_seq == http_Headers) { // HEADER STAGE

        m_request->parse_header(line);
      }

      endpoint().reset_timeout(scx::Time(15));
    }

    if (c == scx::End && m_seq == http_Request) {
      return scx::Close;
    }

    return c;
  }
  
  return scx::Ok;
}

//=============================================================================
bool HTTPStream::process_request(Request*& request)
{
  DEBUG_ASSERT(request,"HTTPStream::process_request() NULL request object");

  const scx::Uri& uri = request->get_uri();
  
  // Log request
  std::ostringstream oss;
  const scx::StreamSocket& sock = (const scx::StreamSocket&)endpoint();
  const scx::SocketAddress* addr = sock.get_remote_addr();
  oss << m_num_connection << "/" << m_num_request << " "
      << addr->get_string() << ": "
      << "\"" << uri.get_string() << "\" "
//    << "\"" << request->get_header("REFERER") << "\" "
      << "\"" << request->get_header("USER-AGENT") << "\" ";
  m_module.log(oss.str());

  // Lookup host object
  Host* host = m_module.get_host_mapper().host_lookup(uri.get_host());
  if (host==0) {
    // This is bad, user should have setup a default host
    delete request;
    request = 0;
    return false;
  }

  // Lookup resource node
  std::string path = uri.get_path();
  if (path.empty()) path = "/";
  const FSNode* node = host->lookup(path);
  request->set_node(node);
  std::string modname;
  if (node) {
    if (node->type() == FSNode::Directory) {
      // Use the directory module
      modname = ((FSDirectory*)node)->lookup_mod(".");
    } else {
      modname = node->parent()->lookup_mod(node->name());
    }
  } else {
    // Use the error module
    modname = host->lookup_mod("!");
  }

  // Construct args
  scx::ArgList args;
  args.give(request);
  
  // Reformat into args for post commands
  if (request->get_method() == "POST") {
    std::string len = request->get_header("CONTENT-LENGTH");
    int l = atoi(len.c_str());
    char* buffer = new char[l+1];
    int na=0;
    Stream::read(buffer,l,na);
    if  (na>0) {
      buffer[na]='\0';
      args.give(new scx::ArgString(buffer));
    }
    delete[] buffer;
  }

  // Lookup module
  scx::ModuleRef ref = m_module.get_module(modname.c_str());
  if (!ref.valid()) {
    request = 0;
    return false;
  }

  // Create and add the message stream
  MessageStream* msg = new MessageStream(request->get_version());
  endpoint().add_stream(msg);

  request = 0;

  // Connect module
  return ref.module()->connect(&endpoint(),&args);
}

};
