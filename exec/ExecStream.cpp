/* SconeServer (http://www.sconemad.com)

External program execution Stream

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

#include "ExecStream.h"
#include "ExecModule.h"
#include "CGIResponseStream.h"

#include "http/MessageStream.h"
#include "http/FSNode.h"
#include "http/Request.h"

#include "sconex/Kernel.h"
#include "sconex/FilePath.h"
#include "sconex/StreamSocket.h"
#include "sconex/StreamTransfer.h"
#include "sconex/StreamDebugger.h"
#include "sconex/Process.h"

//=========================================================================
ExecStream::ExecStream(
  ExecModule& module,
  scx::ArgList* args
) 
  : scx::Stream("exec"),
    m_module(module),
    m_args(args),
    m_process(0)
{

}

//=========================================================================
ExecStream::~ExecStream()
{
  if (m_process) {
    m_process->kill();
    delete m_process;
  }
}

//=========================================================================
scx::Condition ExecStream::event(scx::Stream::Event e)
{
  if (e == scx::Stream::Opening) {
    if (!spawn_process()) {
      return scx::Close;
    }
  }
  
  return scx::Ok;
}

//=========================================================================
bool ExecStream::spawn_process()
{
  std::string prog;
  int bytes_readable = -1;

  http::MessageStream* msg = 
    dynamic_cast<http::MessageStream*>(find_stream("http:message"));
  if (msg) {
    // We're part of an HTTP request chain
    const http::Request& req = msg->get_request();
    const http::FSNode* node = msg->get_node();
    const scx::Uri& uri = req.get_uri();

    delete m_args;
    m_args = new scx::ArgList();

    prog = node->path().path();
    m_process = new scx::Process(prog);
      
    m_args->give(new scx::ArgString(node->url()));

    msg->set_header("Content-Type","text/html");
    msg->set_header("Connection","close");

    m_process->set_env("SERVER_NAME",uri.get_host());

    std::ostringstream oss; oss << uri.get_port();
    m_process->set_env("SERVER_PORT",oss.str());

    std::ostringstream oss2; oss2 << "/" << uri.get_path();
    m_process->set_env("SCRIPT_NAME",oss2.str());

    m_process->set_env("QUERY_STRING",uri.get_query());
    m_process->set_env("REQUEST_METHOD",req.get_method());

    const std::string& ctype = req.get_header("Content-Type");
    if (!ctype.empty()) {
      m_process->set_env("CONTENT_TYPE",ctype);
    }

    const std::string& clength = req.get_header("Content-Length");
    if (!clength.empty()) {
      m_process->set_env("CONTENT_LENGTH",clength);
      bytes_readable = atoi(clength.c_str());
    } else {
      bytes_readable = 0;
    }

    const std::string& cookie = req.get_header("Cookie");
    if (!cookie.empty()) {
      m_process->set_env("HTTP_COOKIE",cookie);
    }
    
    m_module.log("Spawning CGI process '" + prog + "'");
    
  } else if (m_args->size() > 0) {

    prog = m_args->get(0)->get_string();
    m_process = new scx::Process(prog);
    m_module.log("Spawning process '" + prog + "'");
  }

  // Set arguments
  for (int i=0; i<m_args->size(); ++i) {
    m_process->add_arg( m_args->get(i)->get_string() );
  }

  // Launch the new process and connect socket
  if (!m_process->launch()) {
    DEBUG_LOG("Failed to launch process");
    return false;
  }

  //  DEBUG_ASSERT(sock,"spawn_process() Bad socket from Process::launch");
  
  // Add debug logging to exec stream
  // sock->add_stream(new scx::StreamDebugger("exec"));

  // Add stream to interpret and pass through HTTP headers
  if (msg) {
    CGIResponseStream* crs = new CGIResponseStream(msg);
    crs->add_module_ref(m_module.ref());
    m_process->add_stream(crs);
  }
  
  // Create sconeserver <-- program transfer stream
  scx::StreamTransfer* xfer = new scx::StreamTransfer(m_process);
  xfer->set_close_when_finished(true);
  endpoint().add_stream(xfer);
    
  // Create sconeserver --> program transfer stream if required
  const int MAX_READ_BUFFER = 65536;
  if (bytes_readable > MAX_READ_BUFFER || bytes_readable < 0) {
    bytes_readable = MAX_READ_BUFFER;
  }
  if (bytes_readable > 0) {
    scx::StreamTransfer* xfer2 =
      new scx::StreamTransfer(&endpoint(),bytes_readable);
    m_process->add_stream(xfer2);
  }
  
  // Add socket to Kernel table
  scx::Kernel::get()->connect(m_process,0);
  m_process = 0;
  
  return true;
}
