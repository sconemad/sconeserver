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
#include "sconex/Mutex.h"

#ifdef HAVE_SYS_WAIT_H
#  include <sys/wait.h>
#endif

#ifdef HAVE_SYS_UIO_H
#  include <sys/uio.h>
#endif

namespace scx {

#ifndef WIN32

#ifndef CMSG_SPACE
#  if defined(_CMSG_DATA_ALIGN) && defined(_CMSG_HDR_ALIGN)
#    define CMSG_SPACE(len) \
       (_CMSG_DATA_ALIGN(len) + _CMSG_DATA_ALIGN(sizeof(struct cmsghdr)))
#    define CMSG_LEN(len) \
       (_CMSG_DATA_ALIGN(sizeof(struct cmsghdr)) + (len))
#  endif
#endif

// Uncomment to enable debug logging
//#define PROCESS_DEBUG_LOG(a) std::cerr <<"["<< getpid() <<"]" << a << "\n"
  
#ifndef PROCESS_DEBUG_LOG
#  define PROCESS_DEBUG_LOG(a)
#endif
  
#define PROXY_MAX_STR 8182
  
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
class ProxyPacket {

public:
  enum Type {
    Invalid,
    LaunchExe, LaunchArg,
    LaunchEnvName, LaunchEnvValue,
    LaunchDir, LaunchUser, LaunchGroup,
    LaunchReady,
    LaunchedOk, LaunchedError,
    CheckPid, DetatchPid,
    CheckedRunning, CheckedTerminated
  };

  ProxyPacket();

  void set_type(Type type);
  Type get_type();

  void set_str_value(const char* str_value);
  char* get_str_value();
  char* copy_str_value();

  void set_num_value(int num_value);
  int get_num_value();
  
  void set_fd(int fd);
  int get_fd();
  
  bool send(SOCKET s);
  bool recv(SOCKET s);

  std::string get_type_str(Type t);
  
private:
  struct {
    unsigned int size;
    int type;
    int num;
    char str[PROXY_MAX_STR];
  } m_data;
  int m_fd;
};
  
//============================================================================
ProxyPacket::ProxyPacket()
{
  m_data.type = Invalid;
  m_data.num = 0;
  m_data.str[0] = 0;
  m_fd = -1;
}

//============================================================================
void ProxyPacket::set_type(ProxyPacket::Type type)
{
  m_data.type = (int)type;
}

//============================================================================
ProxyPacket::Type ProxyPacket::get_type()
{
  return Type(m_data.type);
}

//============================================================================
void ProxyPacket::set_str_value(const char* str_value)
{
  strncpy(m_data.str,str_value,PROXY_MAX_STR);
}

//============================================================================
char* ProxyPacket::get_str_value()
{
  return m_data.str;
}

//============================================================================
char* ProxyPacket::copy_str_value()
{
  int l = strlen(m_data.str)+1;
  char* c = new char[l];
  strncpy(c,m_data.str,l);
  return c;
}

//============================================================================
void ProxyPacket::set_num_value(int num_value)
{
  m_data.num = num_value;
}

//============================================================================
int ProxyPacket::get_num_value()
{
  return m_data.num;
}

//============================================================================
void ProxyPacket::set_fd(int fd)
{
  m_fd = fd;
}

//============================================================================
int ProxyPacket::get_fd()
{
  return m_fd;
}

//============================================================================
bool ProxyPacket::send(SOCKET s)
{
  struct msghdr msg;
  struct iovec iov[1];
  unsigned int t = 0;
  int n;
  char* buf = (char*)&m_data;
  int err = 0;

#ifdef HAVE_MSGHDR_MSG_CONTROL
  int control_len = CMSG_SPACE(sizeof(int));
  char* control_buf = new char[control_len];
#endif

  m_data.size = (3 * sizeof(int)) + strlen(m_data.str) + 1;
  PROCESS_DEBUG_LOG("SEND {"
                    << m_data.size << ","
                    << get_type_str((Type)m_data.type) << ","
                    << m_data.num << ",'"
                    << m_data.str <<  "'}");
  while (t < m_data.size) {

    if (m_fd >= 0 && t == 0) {
      // Sending a file descriptor with the packet
#ifdef HAVE_MSGHDR_MSG_CONTROL
      msg.msg_controllen = control_len;
      msg.msg_control = control_buf;

      struct cmsghdr* cmptr = CMSG_FIRSTHDR(&msg);
      cmptr->cmsg_len = CMSG_LEN(sizeof(int));
      cmptr->cmsg_level = SOL_SOCKET;
      cmptr->cmsg_type = SCM_RIGHTS;
      *((int*) CMSG_DATA(cmptr)) = m_fd;
#else
      msg.msg_accrights = (caddr_t)&m_fd;
      msg.msg_accrightslen = sizeof(int);
#endif
      msg.msg_name = 0;
      msg.msg_namelen = 0;
      iov[0].iov_base = buf;
      iov[0].iov_len = m_data.size;
      msg.msg_iov = iov;
      msg.msg_iovlen = 1;
      
      n = sendmsg(s,&msg,0);

    } else {
      n = ::send(s,buf+t,m_data.size-t,0);
    }
    
    if (n > 0) {
      t += (unsigned int)n;
      
    } else if (n == 0) {
      err = 1;
      break;

    } else if (errno == EINTR) {
      PROCESS_DEBUG_LOG("ProxyPacket::recv() Interrupted");

    } else {
      PROCESS_DEBUG_LOG("ProxyPacket::send() Error when sending packet "
			<< errno);
      err = errno;
      break;
    }
  }

#ifdef HAVE_MSGHDR_MSG_CONTROL
  delete[] control_buf;
#endif

  return (err == 0);
}

//============================================================================
bool ProxyPacket::recv(SOCKET s)
{
  struct msghdr msg;
  struct iovec iov[1];
  unsigned int t = 0;
  int n;
  char* buf = (char*)&m_data;
  m_fd = -1;
  int err = 0;

#ifdef HAVE_MSGHDR_MSG_CONTROL
  int control_len = CMSG_SPACE(sizeof(int));
  char* control_buf = new char[control_len];
#else
  int recv_fd = -1;
#endif

  // First read the header
  unsigned int init_len = (3 * sizeof(int)) + 1;
  while (t < init_len) {

#ifdef HAVE_MSGHDR_MSG_CONTROL
    msg.msg_controllen = control_len;
    msg.msg_control = control_buf;
#else
    msg.msg_accrights = (caddr_t)&recv_fd;
    msg.msg_accrightslen = sizeof(int);
#endif

    msg.msg_name = 0;
    msg.msg_namelen = 0;
    iov[0].iov_base = buf+t;
    iov[0].iov_len = init_len-t;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    
    n = recvmsg(s,&msg,0);
    
    if (n > 0) {
      t += (unsigned int)n;

    } else if (n == 0) {
      err = 1;
      break;

    } else if (errno == EINTR) {
      PROCESS_DEBUG_LOG("ProxyPacket::recv() Interrupted");

    } else {
      PROCESS_DEBUG_LOG("ProxyPacket::recv() Error when recieving packet, errno=" << errno);
      err = errno;
      break;
    }
  
#ifdef HAVE_MSGHDR_MSG_CONTROL
    struct cmsghdr* cmptr = CMSG_FIRSTHDR(&msg);
    if (cmptr != 0 &&
	cmptr->cmsg_len == CMSG_LEN(sizeof(int)) &&
	m_fd == -1) {
      m_fd = *( (int*)CMSG_DATA(cmptr) );
    }
#else
    if (msg.msg_accrightslen == sizeof(int) &&
	m_fd == -1) {
      m_fd == recv_fd;
    }
#endif

  } // read header

#ifdef HAVE_MSGHDR_MSG_CONTROL
  delete[] control_buf;
#endif

  if (err != 0) {
    return false;
  }

  while (t < m_data.size) {
    n = ::recv(s,buf+t,m_data.size-t,0);
    if (n > 0) {
      t += (unsigned int)n;

    } else if (n == 0) {
      err = 1;
      break;

    } else if (errno == EINTR) {
      PROCESS_DEBUG_LOG("ProxyPacket::recv() Interrupted");

    } else {
      PROCESS_DEBUG_LOG("ProxyPacket::recv() Error when recieving packet, errno=" << errno);
      err = errno;
      break;
    }
  }
  PROCESS_DEBUG_LOG("RECV {"
                    << m_data.size << ","
                    << get_type_str((Type)m_data.type) << ","
                    << m_data.num << ",'"
                    << m_data.str <<  "'}");
  return (err == 0);
}

//============================================================================
std::string ProxyPacket::get_type_str(Type t)
{
  switch (t) {
    case Invalid:           return "INVALID";
    case LaunchExe:         return "LAUNCH:EXE"; 
    case LaunchArg:         return "LAUNCH:ARG"; 
    case LaunchEnvName:     return "LAUNCH:ENV-NAME"; 
    case LaunchEnvValue:    return "LAUNCH:ENV-VALUE"; 
    case LaunchDir:         return "LAUNCH:DIR"; 
    case LaunchUser:        return "LAUNCH:USER"; 
    case LaunchGroup:       return "LAUNCH:GROUP"; 
    case LaunchReady:       return "LAUNCH:READY"; 
    case LaunchedOk:        return "LAUNCHED:OK"; 
    case LaunchedError:     return "LAUNCHED:ERROR"; 
    case CheckPid:          return "CHECK"; 
    case DetatchPid:        return "DETATCH"; 
    case CheckedRunning:    return "RUNNING"; 
    case CheckedTerminated: return "TERMINATED"; 
  }
  return "UNKNOWN";
}


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
class Proxy {

public:
  Proxy();

  int run();

  void process_terminated(pid_t pid, int code);
  
private:

  void add_env(const char* name, const char* value);

  bool launch();
  
  void reset();

  ProxyPacket m_packet;
  // Current packet
  
  char* m_prog;
  // Executable name

  char* m_exec_args[256];
  int m_narg;
  // Argument vector and size
  
  char* m_exec_envp[256];
  int m_nenv;
  // Environment vector and size

  char* m_cur_env_name;
  // Current environment variable name

  char* m_dir;
  // Working directory

  uid_t m_uid;
  gid_t m_gid;
  // User/group to run process as
  
  typedef struct ProcStat {
    Process::RunState type;
    int code;
  };
  std::map<pid_t,ProcStat> m_stats;
  
};

static Proxy* s_proxy = 0;

//=========================================================================
void handleSIGCHLD(int i)
{  
  int stat;
  int pid;
  while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0) {
    PROCESS_DEBUG_LOG("SIGCHLD PID " << pid << " TERMINATED");
    if (s_proxy) s_proxy->process_terminated(pid,stat/256);
  }
}

//============================================================================
Proxy::Proxy()
  : m_prog(0),
    m_narg(0),
    m_nenv(0),
    m_cur_env_name(0),
    m_dir(0),
    m_uid(0),
    m_gid(0)
{

}

//============================================================================
int Proxy::run()
{
  // Install signal handler to catch exiting processes
  signal(SIGCHLD, handleSIGCHLD);

  while (true) {

    if (!m_packet.recv(1)) {
      PROCESS_DEBUG_LOG("PROXY TERMINATING");
      return 1;
    }
    
    switch (m_packet.get_type()) {

      case ProxyPacket::LaunchExe: {
        reset();
        m_prog = m_packet.copy_str_value();
      } break;

      case ProxyPacket::LaunchArg: {
        m_exec_args[m_narg++] = m_packet.copy_str_value();
      } break;

      case ProxyPacket::LaunchEnvName: {
        delete m_cur_env_name;
        m_cur_env_name = m_packet.copy_str_value();
      } break;
      
      case ProxyPacket::LaunchEnvValue: {
        add_env(m_cur_env_name,m_packet.get_str_value());
      } break;

      case ProxyPacket::LaunchDir: {
        m_dir = m_packet.copy_str_value();
      } break;

      case ProxyPacket::LaunchUser: {
        m_uid = m_packet.get_num_value();
      } break;

      case ProxyPacket::LaunchGroup: {
        m_gid = m_packet.get_num_value();
      } break;

      case ProxyPacket::LaunchReady: {
        launch();
        reset();
      } break;

      case ProxyPacket::CheckPid: {
        ProxyPacket packet;
        packet.set_type(ProxyPacket::CheckedRunning);
        std::map<pid_t,ProcStat>::iterator it =
          m_stats.find(m_packet.get_num_value());
        if (it != m_stats.end()) {
          ProcStat& stat = (*it).second;
            if (stat.type == Process::Terminated) {
              packet.set_type(ProxyPacket::CheckedTerminated);
              packet.set_num_value(stat.code);
              m_stats.erase(it);
            }
        }
        packet.send(0);
      } break;
      
      case ProxyPacket::DetatchPid: {
        std::map<pid_t,ProcStat>::iterator it =
          m_stats.find(m_packet.get_num_value());
        if (it != m_stats.end()) {
          ProcStat& stat = (*it).second;
          if (stat.type == Process::Running) {
            stat.type = Process::Detatched;
          } else {
            m_stats.erase(it);
          }
        }
      } break;
      
      default:
        break;
    }
  }

  return 0;
}

//============================================================================
void Proxy::process_terminated(pid_t pid, int code)
{
  std::map<pid_t,ProcStat>::iterator it = m_stats.find(pid);
  if (it != m_stats.end()) {
    ProcStat& stat = (*it).second;
    if (stat.type == Process::Running) {
      stat.type = Process::Terminated;
      stat.code = code;
    } else {
      m_stats.erase(it);
    }
  } else {
    ProcStat& stat = m_stats[pid];
    stat.type = Process::Terminated;
    stat.code = code;
  }
}
  
//============================================================================
void Proxy::add_env(const char* name, const char* value)
{
  int name_len = strlen(name);
  int value_len = strlen(value);
  
  char* str = new char[value_len + 1 + name_len + 1];
  
  strncpy(str,name,name_len+1);
  strncat(str,"=",2);
  strncat(str,value,value_len+1);
  
  m_exec_envp[m_nenv++] = str;
}

//============================================================================
bool Proxy::launch()
{
  // Terminate arg and env vectors
  m_exec_args[m_narg] = 0;
  m_exec_envp[m_nenv]=0;
  
  int d[2];
  if (socketpair(PF_UNIX,SOCK_STREAM,0,d) < 0) {
    PROCESS_DEBUG_LOG("Socketpair failed");
    return false;
  }
  
  // Fork the process
  pid_t pid = ::fork();
  
  if (pid != 0) { // ****** Original process ******
    
    // Close the other end of the socketpair
    ::close(d[0]);
    
    if (pid < 0) {
      PROCESS_DEBUG_LOG("Fork failed");
      return false;
    }

    // Initialise the stat entry if not already there
    std::map<pid_t,ProcStat>::iterator it = m_stats.find(pid);
    if (it == m_stats.end()) {
      ProcStat& stat = m_stats[pid];
      stat.type = Process::Running;
      stat.code = 0;
    }
    
    // Send the pid and socket to SconeServer
    ProxyPacket packet;
    packet.set_type(ProxyPacket::LaunchedOk);
    packet.set_num_value(pid);
    packet.set_fd(d[1]);
    packet.send(0);

    // Close the socket as its no longer ours
    ::close(d[1]);
    
  } else { // ****** Child process ******
    
    // Close sconeserver end of the socketpair  
    ::close(d[1]);

    // Set effective user and group ids
    if (m_uid > 0) ::seteuid(m_uid);
    if (m_gid > 0) ::setegid(m_gid);
    
    // Duplicate standard descriptors on to our end of the socketpair
    ::dup2(d[0],0);
    ::dup2(d[0],1);
    ::dup2(d[0],2);

    // Change to the specified working directory
    ::chdir(m_dir);
    
    // Launch the program
    ::execve(m_prog,m_exec_args,m_exec_envp);
    
    // If we get here, something's gone wrong
    _exit(1);
  }
  
  return true;
}

//============================================================================
void Proxy::reset()
{
  // Cleanup exec-related stuff we don't need
  for (int i=0; i<m_narg; ++i) {
    char* cstr = m_exec_args[i];
    delete[] cstr;
  }
  m_narg = 0;
  
  for (int i=0; i<m_nenv; ++i) {
    char* cstr = m_exec_envp[i];
    delete[] cstr;
  }
  m_nenv = 0;
  
  delete[] m_prog;
  m_prog = 0;

  delete[] m_dir;
  m_dir = 0;
}

pid_t Process::s_proxy_pid = -1;
int Process::s_proxy_sock = -1;
Mutex* Process::s_proxy_mutex = 0;
#endif
  
//============================================================================
void Process::init()
{
#ifndef WIN32
  DEBUG_ASSERT(s_proxy_pid==-1,"init() Already called");
  
  int d[2];
  if (socketpair(PF_UNIX,SOCK_STREAM,0,d) < 0) {
    DEBUG_LOG("init() socketpair failed");
    return;
  }
  
  // Fork the process
  s_proxy_pid = ::fork();

  if (s_proxy_pid != 0) { // ***** Original process ******

    // Close the child's end of the socketpair
    ::close(d[1]);

    // Save the socket to communicate with the proxy process
    s_proxy_sock = d[0];
  
    if (s_proxy_pid < 0) {
      DEBUG_LOG("init() fork failed");
    }

    s_proxy_mutex = new Mutex();
    
  } else {  // ****** Child process ******

    // Close the parent's end of the socketpair
    ::close(d[0]);

    ::dup2(d[1],0);
    ::dup2(d[1],1);

    // Open a log file and redirect stderr
/*
    int errfd = ::open("proxy.log",
		       O_WRONLY | O_CREAT | O_APPEND,
		       S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    ::dup2(errfd,2);
*/
    
    s_proxy = new Proxy();

    exit(s_proxy->run());
  }
#endif
}
  
//============================================================================
Process::Process(
  const std::string& exe
)	
  : m_pid(-1),
    m_exe(exe),
    m_runstate(Unstarted)
{
  DEBUG_COUNT_CONSTRUCTOR(Process);
}
  
//============================================================================
Process::~Process()
{
  if (m_runstate == Running) {
    kill();
  }

  if (m_runstate != Unstarted) {
    ProxyPacket packet;
    packet.set_type(ProxyPacket::DetatchPid);
    packet.set_num_value(m_pid);

    s_proxy_mutex->lock();
    packet.send(s_proxy_sock);
    s_proxy_mutex->unlock();
  }

  DEBUG_COUNT_DESTRUCTOR(Process);
}

//============================================================================
void Process::add_arg(const std::string& arg)
{
  DEBUG_ASSERT(m_runstate==Unstarted,"add_arg() Process already launched");
  
  m_args.push_back(arg);
}

//============================================================================
void Process::set_env(const std::string& name, const std::string& value)
{
  DEBUG_ASSERT(m_runstate==Unstarted,"set_env() Process already launched");
  
  m_env[name] = value;
}

//============================================================================
void Process::set_dir(const FilePath& dir)
{
  DEBUG_ASSERT(m_runstate==Unstarted,"set_dir() Process already launched");

  m_dir = dir;
}

//============================================================================
void Process::set_user(const User& user)
{
  DEBUG_ASSERT(m_runstate==Unstarted,"set_user() Process already launched");

  m_user = user;
}

//============================================================================
bool Process::launch()
{
  DEBUG_ASSERT(m_runstate != Running,"launch() Process already launched");
  
#ifdef WIN32
  // TODO: get this working!
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
  
#else // *NIX

  s_proxy_mutex->lock();
  
  ProxyPacket packet;
  packet.set_type(ProxyPacket::LaunchExe);
  packet.set_str_value(m_exe.c_str());
  packet.send(s_proxy_sock);

  for (std::list<std::string>::const_iterator it_a = m_args.begin();
       it_a != m_args.end();
       it_a++) {
    packet.set_type(ProxyPacket::LaunchArg);
    packet.set_str_value((*it_a).c_str());
    packet.send(s_proxy_sock);
  }
  
  for (std::map<std::string,std::string>::const_iterator it_e = m_env.begin();
       it_e != m_env.end();
       it_e++) {
    packet.set_type(ProxyPacket::LaunchEnvName);
    packet.set_str_value((*it_e).first.c_str());
    packet.send(s_proxy_sock);
    
    packet.set_type(ProxyPacket::LaunchEnvValue);
    packet.set_str_value((*it_e).second.c_str());
    packet.send(s_proxy_sock);    
  }

  packet.set_type(ProxyPacket::LaunchDir);
  packet.set_str_value(m_dir.path().c_str());
  packet.send(s_proxy_sock);

  packet.set_type(ProxyPacket::LaunchUser);
  packet.set_num_value(m_user.get_user_id());
  packet.send(s_proxy_sock);

  packet.set_type(ProxyPacket::LaunchGroup);
  packet.set_num_value(m_user.get_group_id());
  packet.send(s_proxy_sock);
  
  // Tell the proxy to launch
  packet.set_type(ProxyPacket::LaunchReady);
  packet.set_str_value("");
  packet.send(s_proxy_sock);

  // Recieve resulting pid and socket from proxy
  packet.recv(s_proxy_sock);
  if (packet.get_type() == ProxyPacket::LaunchedOk) {
    m_pid = packet.get_num_value();
    m_socket = packet.get_fd();
    event_create();
    m_state = Descriptor::Connected;

    m_addr_local = new AnonSocketAddress("process");
    m_addr_remote = new AnonSocketAddress(m_exe);
     
    PROCESS_DEBUG_LOG("Got pid " << m_pid << " and fd " 
                      << m_socket << " from process proxy");
  }

  s_proxy_mutex->unlock();
#endif

  if (m_runstate != Detatched) {
    m_runstate = Running;
  }
  return true;
}

//============================================================================
bool Process::kill()
{
#ifdef WIN32
  //TODO
#else 
  return (0 == ::kill(m_pid,SIGKILL));
#endif
}

//============================================================================
bool Process::is_running()
{
  return (m_runstate == Running);
}

//============================================================================
bool Process::get_exitcode(int& code)
{
  ProxyPacket packet;

  switch (m_runstate) {

    case Running:
      packet.set_type(ProxyPacket::CheckPid);
      packet.set_num_value(m_pid);

      s_proxy_mutex->lock();
      packet.send(s_proxy_sock);
      packet.recv(s_proxy_sock);
      s_proxy_mutex->unlock();

      switch (packet.get_type()) {
        case ProxyPacket::CheckedRunning:
          //          DEBUG_LOG("get_exitcode() for pid " << m_pid
          //                    << " still running");
          break;
        case ProxyPacket::CheckedTerminated: 
          m_exitcode = packet.get_num_value();
          m_runstate = Terminated;
          //          DEBUG_LOG("get_exitcode() for pid " << m_pid
          //                    << ": terminated with code " << m_exitcode);
          break;
        default:
          //          DEBUG_LOG("get_exitcode() for pid " << m_pid
          //                    << ": ERROR");
          break;
      }
      // Intentional fall through...
    case Terminated:
      code = m_exitcode;
      break;

    default:
      break;
  }
  
  return (m_runstate == Terminated);
}

//============================================================================
void Process::set_detatched(bool onoff)
{
  if (m_runstate == Unstarted ||
      m_runstate == Running) {
    m_runstate = Detatched;
  }
}


};
