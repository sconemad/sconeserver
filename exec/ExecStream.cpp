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

//=========================================================================
char* new_c_str(const std::string& str)
{
  int len = str.size()+1;
  char* c_str = new char[len];
  memcpy(c_str,str.c_str(),len);
  return c_str;
}

//=========================================================================
ExecStream::ExecStream(
  ExecModule& module,
  scx::ArgList* args
) 
  : scx::Stream("exec"),
    m_module(module),
    m_args(args),
    m_pid(0)
{

}

//=========================================================================
ExecStream::~ExecStream()
{
  kill(m_pid,SIGKILL);
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
  const int max_args = 64;
  std::string prog = "";
  int bytes_readable = -1;

  // Construct the environment vector for the new program
  char* exec_envp[max_args];
  int nenv = 0;

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
    m_args->give(new scx::ArgString(node->url()));

    msg->set_header("Content-Type","text/html");
    msg->set_header("Connection","close");

    exec_envp[nenv++] = new_c_str("SERVER_NAME="+uri.get_host());

    std::ostringstream oss; oss << uri.get_port();
    exec_envp[nenv++] = new_c_str("SERVER_PORT="+oss.str());

    std::ostringstream oss2; oss2 << "/" << uri.get_path();
    exec_envp[nenv++] = new_c_str("SCRIPT_NAME="+oss2.str());

    exec_envp[nenv++] = new_c_str("QUERY_STRING="+uri.get_query());
    exec_envp[nenv++] = new_c_str("REQUEST_METHOD="+req.get_method());

    const std::string& ctype = req.get_header("Content-Type");
    if (!ctype.empty()) {
      exec_envp[nenv++] = new_c_str("CONTENT_TYPE="+ctype);
    }

    const std::string& clength = req.get_header("Content-Length");
    if (!clength.empty()) {
      exec_envp[nenv++] = new_c_str("CONTENT_LENGTH="+clength);
      bytes_readable = atoi(clength.c_str());
    } else {
      bytes_readable = 0;
    }

    const std::string& cookie = req.get_header("Cookie");
    if (!cookie.empty()) {
      exec_envp[nenv++] = new_c_str("HTTP_COOKIE="+cookie);
    }
    
    m_module.log("Spawning CGI process '" + prog + "'");
    
  } else if (m_args->size() > 0) {

    prog = m_args->get(0)->get_string();
    m_module.log("Spawning process '" + prog + "'");
  }

  // Mark the end of the environment vector
  exec_envp[nenv]=0;
  
  // Construct the argument vector for the new program
  char* exec_args[max_args];
  int narg = 0;
  
  for (int i=0; i<m_args->size(); ++i) {
    const std::string str = m_args->get(i)->get_string();
    exec_args[narg++] = new_c_str(str);
  }
  exec_args[narg] = 0;

  // Create sconeserver <--> program socketpair
  scx::StreamSocket* sock_scx = 0;
  scx::StreamSocket* sock_exec = 0;
  scx::StreamSocket::pair(sock_scx,sock_exec,"exec",prog);

  // Save these so we can use them after the fork - eliminating the need
  // to call any scx framework methods.
  int fd_scx = sock_scx->fd();
  int fd_exec = sock_exec->fd();
  char* c_prog = new_c_str(prog);
  
  // Create a new process
  m_pid = fork();

  if (m_pid != 0) {
    // In the original process

    // Cleanup exec-related stuff we don't need
    for (int i=0; i<narg; ++i) {
      char* cstr = exec_args[i];
      delete[] cstr;
    }
    for (int i=0; i<nenv; ++i) {
      char* cstr = exec_envp[i];
      delete[] cstr;
    }
    delete[] c_prog;

    // Close the other end of the socketpair
    delete sock_exec;

    if (m_pid < 0) {
      DEBUG_LOG("Fork failed");
      delete sock_scx;
      return false;
    }

    // Add debug logging to exec stream
    //    sock_scx->add_stream(new scx::StreamDebugger("exec"));

    // Add stream to interpret and pass through HTTP headers
    if (msg) {
      CGIResponseStream* crs = new CGIResponseStream(msg);
      crs->add_module_ref(m_module.ref());
      sock_scx->add_stream(crs);
    }
  
    // Create sconeserver <-- program transfer stream
    scx::StreamTransfer* xfer = new scx::StreamTransfer(sock_scx);
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
      sock_scx->add_stream(xfer2);
    }
    
    // Add sconeserver end of the socketpair to Kernel table
    scx::Kernel::get()->connect(sock_scx,0);
    
    return true;
  }

  // - - - - - - - - - - - - - - - - - - - - - - 
  // In the new forked process

  //  ::setuid(99);
  
  // Close sconeserver end of the socketpair  
  ::close(fd_scx);
  
  // Duplicate standard descriptors on to our end of the socketpair
  ::dup2(fd_exec,0);
  ::dup2(fd_exec,1);

  // Launch the program
  ::execve(c_prog,exec_args,exec_envp);
  
  // If we get here, something's gone wrong
  _exit(1);

  // Assume the brace position
  return false;
}
