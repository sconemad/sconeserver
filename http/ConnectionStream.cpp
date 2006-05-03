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
#include "http/HostMapper.h"
#include "http/Host.h"
#include "http/DocRoot.h"

#include "sconex/StreamSocket.h"
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
) : scx::LineBuffer("http",1024),
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
      endpoint().reset_timeout(scx::Time(15));
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
void ConnectionStream::set_persist(bool persist)
{
  m_persist = persist;
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
	return scx::End;
      }
      m_seq = http_Body;
      return scx::Ok;
      
    } else if (m_seq == http_Request) { // REQUEST STAGE

      delete m_request;
      m_request = new Request();
      if (m_request->parse_request(line,0!=find_stream("ssl"))) {
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

//=============================================================================
bool ConnectionStream::process_request(Request*& request)
{
  DEBUG_ASSERT(request,"ConnectionStream::process_request() NULL request object");

  const scx::Uri& uri = request->get_uri();
  
  // Log request
  std::ostringstream oss;
  oss << m_num_connection << "-" << m_num_request;
  const std::string& id = oss.str();
  
  const scx::StreamSocket* sock =
    dynamic_cast<const scx::StreamSocket*>(&endpoint());
  const scx::SocketAddress* addr = sock->get_remote_addr();

  m_module.log(id + " " + addr->get_string() + " " +
               request->get_method() + " " + uri.get_string());

  const std::string& referer = request->get_header("Referer");
  if (!referer.empty()) {
    m_module.log(id + " Referer: " + referer);
  }

  const std::string& useragent = request->get_header("User-Agent");
  if (!useragent.empty()) {
    m_module.log(id + " User-Agent: " + useragent);
  }
  
  // Lookup host object
  Host* host = m_module.get_host_mapper().host_lookup(uri.get_host());
  if (host==0) {
    // This is bad, user should have setup a default host
    m_module.log(id + " Unknown host '" + uri.get_host() + "'",
                 scx::Logger::Error);
    delete request;
    request = 0;
    return false;
  }

  // Lookup resource node
  std::string path = uri.get_path();
  if (path.empty()) path = "/";
  DocRoot* docroot = host->get_docroot(m_profile);
  if (docroot==0) {
    // Profile is unknown within this host, can't do anything
    m_module.log(id + " Unknown profile '" + m_profile +
                 "' for host '" + uri.get_host() + "'",
                 scx::Logger::Error);
    delete request;
    request = 0;
    return false;
  }

  const FSNode* node = docroot->lookup(path);
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
    modname = docroot->lookup_mod("!");
  }

  // Construct args
  scx::ArgList args;

  // Lookup module
  scx::ModuleRef ref = m_module.get_module(modname.c_str());
  if (!ref.valid()) {
    m_module.log("No module found to handle request",scx::Logger::Error);
    request = 0;
    return false;
  }

  // Create and add the message stream
  MessageStream* msg = new MessageStream(*this,request);
  msg->add_module_ref(m_module.ref());
  msg->set_node(node);
  endpoint().add_stream(msg);

  request = 0;

  // Connect module
  return ref.module()->connect(&endpoint(),&args);
}

};
