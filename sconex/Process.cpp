/* SconeServer (http://www.sconemad.com)

Process

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

#include "sconex/Process.h"
#include "sconex/utils.h"
namespace scx {

//============================================================================
Process::Process(
  const std::string& exe
)	
  : m_pid(-1),
    m_exe(exe),
    m_launched(false)
{
  DEBUG_COUNT_CONSTRUCTOR(Process);
}
  
//============================================================================
Process::~Process()
{
  DEBUG_COUNT_DESTRUCTOR(Process);
}

//============================================================================
void Process::add_arg(const std::string& arg)
{
  DEBUG_ASSERT(!m_launched,"add_arg() Process already launched");
  
  m_args.push_back(arg);
}

//============================================================================
void Process::set_env(const std::string& name, const std::string& value)
{
  DEBUG_ASSERT(!m_launched,"set_env() Process already launched");
  
  m_env[name] = value;
}

//============================================================================
bool Process::launch(StreamSocket*& sock)
{
  DEBUG_ASSERT(!m_launched,"launch() Process already launched");
  
#ifdef WIN32
  std::string cmd_line;
  
  
  STARTUPINFO start_info;
  ZeroMemory(&start_info,sizeof(STARTUPINFO));
  start_info.cb = sizeof(STARTUPINFO);
  start_info.dwFlags = STARTF_USESTDHANDLES;
  start_info.wShowWindow = SW_SHOWDEFAULT;
  start_info.hStdInput;
  start_info.hStdOutput;
  start_info.hStdError;
  
  PROCESS_INFORMATION proc_info;
  
  if (0 == CreateProcess(
        m_exe.c_str(),
        LPTSTR lpCommandLine,
        NULL,
        NULL,
        FALSE,
        DETACHED_PROCESS,
        LPVOID lpEnvironment,
        NULL,
        &start_info,
        &proc_info)) {
    // failed
    return false;
  }
  
  // Save process id
  m_pid = proc_info.dwProcessId;
  
  // Don't need these handles any more
  CloseHandle(proc_info.hThread);
  CloseHandle(proc_info.hProcess);
  
#else

  // Construct the argument vector
  char* exec_args[256];
  int narg = 0;
  for (std::list<std::string>::const_iterator it = m_args.begin();
       it != m_args.end();
       it++) {
    exec_args[narg++] = new_c_str(*it);
  }
  exec_args[narg] = 0;

  // Construct the environment vector
  char* exec_envp[256];
  int nenv = 0;
  for (std::map<std::string,std::string>::const_iterator it2 = m_env.begin();
       it2 != m_env.end();
       it2++) {
    std::string entry = (*it2).first + "=" + (*it2).second;
    exec_envp[nenv++] = new_c_str(entry);
  }
  exec_envp[nenv]=0;

  // Create sconeserver <--> program socketpair
  scx::StreamSocket* sock_exec = 0;
  scx::StreamSocket::pair(sock,sock_exec,"exec",m_exe);

  // Save these so we can use them after the fork - eliminating the need
  // to call any scx framework methods.
  int fd_scx = sock->fd();
  int fd_exec = sock_exec->fd();
  char* c_prog = new_c_str(m_exe);

  // Fork the process
  m_pid = ::fork();
  
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
      return false;
    }

    
  } else {
    // ****** Child process ******

    // Close sconeserver end of the socketpair  
    ::close(fd_scx);
    
    // Duplicate standard descriptors on to our end of the socketpair
    ::dup2(fd_exec,0);
    ::dup2(fd_exec,1);

    // Launch the program
    ::execve(c_prog,exec_args,exec_envp);
    
    // If we get here, something's gone wrong
    _exit(1);
  }
  
#endif
  
  m_launched = true;
  return true;
}

//============================================================================
bool Process::kill()
{
  return (0 == ::kill(m_pid,SIGKILL));
}

};
