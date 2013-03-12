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
#include "http/Request.h"

#include <sconex/Kernel.h>
#include <sconex/FilePath.h>
#include <sconex/StreamSocket.h>
#include <sconex/StreamTransfer.h>
#include <sconex/StreamDebugger.h>
#include <sconex/Process.h>
#include <sconex/User.h>
#include <sconex/Log.h>

//=========================================================================
ExecStream::ExecStream(ExecModule* module,
		       scx::ScriptList* args) 
  : scx::Stream("exec"),
    m_module(module),
    m_args(args),
    m_process(0),
    m_cgi_mode(false),
    m_launched(0)
{

}

//=========================================================================
ExecStream::~ExecStream()
{
  if (m_process) {
    m_process->kill();
    delete m_process;
  }
  delete m_args;
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

  http::MessageStream* msg = GET_HTTP_MESSAGE();
  if (msg) {
    // We're part of an HTTP request chain
    m_cgi_mode = true;
    const http::Request& req = msg->get_request();
    const scx::Uri& uri = req.get_uri();

    delete m_args;
    m_args = new scx::ScriptList();

    prog = req.get_path().path();
    m_process = new scx::Process(prog);

    std::string::size_type is = prog.find_last_of("/");
    if (is != std::string::npos) {
      std::string wd = prog.substr(0,is);
      m_process->set_dir(wd);
    }
      
    m_args->give(scx::ScriptString::new_ref(uri.get_string()));

    msg->get_response().set_header("Content-Type","text/html");
    msg->get_response().set_header("Connection","close");

    // Other environmnt variables required to fully comply with CGI spec:
    // AUTH_TYPE
    // DOCUMENT_ROOT
    // GATEWAY_INTERFACE (CGI/1.1)
    // HTTP_ACCEPT
    // HTTP_FROM (obsolete)
    // HTTP_REFERER
    // HTTP_USER_AGENT
    // PATH_INFO
    // PATH_TRANSLATED (local path incl doc root)
    // REMOTE_ADDR (ip)
    // REMOTE_HOST (hotname)
    // REMOTE_IDENT (obsolete)
    // SERVER_PROTOCOL (HTTP/1.1)
    // SERVER_SOFTWARE (NCSA/1.5)
    // SERVER_ADMIN (email)
    
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
    
    msg->log("Spawning CGI process '" + prog + "'");
    
  } else if (m_args->size() > 0) {

    prog = m_args->get(0)->object()->get_string();
    m_process = new scx::Process(prog);
    scx::Log("exec").submit("Spawning process '" + prog + "'");
  }

  // Set arguments
  for (int i=0; i<m_args->size(); ++i) {
    m_process->add_arg( m_args->get(i)->object()->get_string() );
  }

  m_process->set_user(m_module.object()->get_exec_user());
  
  // Launch the new process and connect socket
  if (!m_process->launch()) {
    m_launched = -1;
    DEBUG_LOG("Failed to launch process");
    return false;
  }
  m_launched = 1;
  
  // Add debug logging to exec stream
  // sock->add_stream(new scx::StreamDebugger("exec"));

  // Add stream to interpret and pass through HTTP headers
  if (msg) {
    CGIResponseStream* crs = new CGIResponseStream(m_module.object(),msg);
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
  scx::Kernel::get()->connect(m_process);
  m_process = 0;
  
  return true;
}

//=========================================================================
std::string ExecStream::stream_status() const
{
  std::ostringstream oss;
  if (m_cgi_mode) oss << "CGI ";
  if (m_launched > 0) oss << "LAUNCHED "; 
  if (m_launched < 0) oss << "FAILED "; 

  if (m_args->size() > 0) {
    oss << m_args->get(0)->object()->get_string();
  } else {
    oss << "(no exe)";
  }
  return oss.str();
}
