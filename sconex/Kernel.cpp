/* SconeServer (http://www.sconemad.com)

sconex kernel

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

#include <sconex/Kernel.h>
#include <sconex/ConfigStream.h>
#include <sconex/ScriptEngine.h>
#include <sconex/TermBuffer.h>
#include <sconex/Console.h>
#include <sconex/Logger.h>
#include <sconex/Debug.h>
#include <sconex/User.h>
#include <sconex/Process.h>

namespace scx {

ScriptRefTo<Kernel>* Kernel::s_kernel = 0;

#define LOG(msg) Log("kernel").submit(msg);

//===========================================================================
void signal_restart(int sig)
{
  LOG(std::string("Restarting due to signal: ") + strsignal(sig));
  Kernel::get()->restart();
}

//===========================================================================
void signal_shutdown(int sig)
{
  LOG(std::string("Shutting down due to signal: ") + strsignal(sig));
  Kernel::get()->shutdown();
}

//=============================================================================
Kernel* Kernel::create(const std::string& appname,
		       const VersionTag& version)
{
  DEBUG_ASSERT(!s_kernel,"Sconex kernel already created");

  scx::Process::init();

  s_kernel = new ScriptRefTo<Kernel>( new Kernel(appname, version) );
  return s_kernel->object();
}

//=============================================================================
Kernel* Kernel::get()
{
  DEBUG_ASSERT(s_kernel,"Sconex kernel not created");
  return s_kernel->object();
}

//=============================================================================
Kernel::~Kernel()
{

}

//=============================================================================
std::string Kernel::info() const
{
  return "A modular, scriptable application framework";
}

//=============================================================================
int Kernel::init() 
{
  Logger::init(get_var_path());

  // Install signal handlers for restart and shutdown
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = signal_restart;
  sigaction(SIGHUP,&sa,0);
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = signal_shutdown;
  sigaction(SIGTERM,&sa,0);

  LOG("Init " + name() + "-" + version().get_string());
  LOG("Built for " + scx::build_type() + " on " + scx::build_time().code());

  if (m_autoload_config) {
    load_config_file(m_conf_path + std::string(name() + ".conf"));
  }
  
  Module::init();
  
  return 0;
}

//=============================================================================
bool Kernel::close()
{
  Logger::get()->set_async(false);

  LOG("Stopping threads");
  m_spinner.close();

  const int max_retries = 10;
  int retry = 0;
  do {
    if (++retry <= max_retries) {
      std::ostringstream oss;
      oss << "Unloading modules (attempt " << retry << ")";
      LOG(oss.str());
    } else {
      LOG("Could not unload all modules");
      return false;
    }
  } while (!Module::close());

  return true;
}

//=============================================================================
void Kernel::set_conf_path(const FilePath& path)
{
  m_conf_path = path;
}

//=============================================================================
FilePath Kernel::get_conf_path() const
{
  return m_conf_path;
}

//=============================================================================
void Kernel::set_autoload_config(bool autoload)
{
  m_autoload_config = autoload;
}
  
//=============================================================================
int Kernel::run(bool console)
{
  int err = init();
  if (err) {
    return err;
  }

  if (console) {
    connect_config_console();
  }

  m_state = Run;
  LOG("Init complete, entering scheduler loop");
  
  while (true) {

    if (m_spinner.spin() < 0) {
      LOG("Descriptor table empty, shutting down");
      break;
    }
    
    if (m_state == Shutdown) {
      break;
    }

    if (m_state == Restart) {
      close();
      LOG("Restarting");
      return 0;
    }
  }

  close();
  LOG("Exiting");
  return 1;
}

//=============================================================================
void Kernel::connect_config_console()
{
  Console* c = new Console();
  c->add_stream( new TermBuffer("term") );
  c->add_stream( new ConfigStream(new ScriptRef(this),true) );
  //  c->add_stream( new ScriptEngineExec(ScriptAuth::Admin,
  //  				      new ScriptRef(this),
  //  				      "console") );
  connect(c);
}

//=============================================================================
bool Kernel::connect(Descriptor* d)
{
  return (0 != add_job(new DescriptorJob(d)));
}

//=============================================================================
JobID Kernel::add_job(Job* job)
{
  return m_spinner.add_job(job);
}

//=============================================================================
bool Kernel::end_job(JobID jobid)
{
  if (m_state != Run) {
    return false;
  }
  return m_spinner.end_job(jobid);
}

//=============================================================================
void Kernel::restart()
{
  if (m_state == Run) {
    m_state = Restart;
  }
}

//=============================================================================
void Kernel::shutdown()
{
  if (m_state == Run) {
    m_state = Shutdown;
  }
}

//=============================================================================
ScriptRef* Kernel::script_op(const ScriptAuth& auth,
			     const ScriptRef& ref,
			     const ScriptOp& op,
			     const ScriptRef* right)
{
  if (ScriptOp::Lookup == op.type()) {
    std::string name = right->object()->get_string();

    // Methods
    if ("restart" == name ||
	"shutdown" == name ||
	"set_user" == name ||
	"set_thread_pool" == name ||
	"set_latency" == name ||
	"enable_jobs" == name) {
      return new ScriptMethodRef(ref,name);
    }      

    // Properties
    if ("kernel" == name) 
      return ref.ref_copy();
    if ("logger" == name)
      return new Logger::Ref(Logger::get());
    if ("jobs" == name) 
      return ScriptString::new_ref(m_spinner.describe());
    if ("root" == name) 
      return ScriptInt::new_ref(geteuid() == 0);
    if ("pid" == name) 
      return ScriptInt::new_ref(getpid());
    if ("thread_pool" == name) 
      return ScriptInt::new_ref(m_spinner.get_num_threads());
    if ("latency" == name) 
      return ScriptInt::new_ref(m_spinner.get_latency());
    if ("system_nodename" == name) 
      return ScriptString::new_ref(get_system_nodename());
    if ("system_version" == name) 
      return ScriptString::new_ref(get_system_version());
    if ("system_hardware" == name) 
      return ScriptString::new_ref(get_system_hardware());
  }
  
  return Module::script_op(auth,ref,op,right);
}

//=============================================================================
ScriptRef* Kernel::script_method(const ScriptAuth& auth,
				 const ScriptRef& ref,
				 const std::string& name,
				 const ScriptRef* args)
{
  if ("restart" == name) {
    if (!auth.admin()) return ScriptError::new_ref("Not permitted");
    LOG("Restart requested via command");
    restart();
    return 0;
  }

  if ("shutdown" == name) {
    if (!auth.admin()) return ScriptError::new_ref("Not permitted");
    LOG("Shutdown requested via command");
    shutdown();
    return 0;
  }

  if ("set_user" == name) {
    if (!auth.admin()) return ScriptError::new_ref("Not permitted");
    User user;
    const ScriptString* a_name = get_method_arg<ScriptString>(args,0,"name");
    const ScriptInt* a_uid = get_method_arg<ScriptInt>(args,0,"id");

    if (a_name) {
      if (!user.set_user_name(a_name->get_string())) {
        return ScriptError::new_ref("set_user() Unknown username '" +
                            a_name->get_string() + "'");
      }
    } else if (a_uid) {
      if (!user.set_user_id(a_uid->get_int())) {
        return ScriptError::new_ref("set_user() Unknown user id '" +
                            a_uid->get_string() + "'");
      }
    } else {
      return ScriptError::new_ref("set_user() Invalid argument");
    }

    std::ostringstream oss;
    oss << "Setting effective user and group ids to " << 
      user.get_user_id() << ":" << user.get_group_id();
    LOG(oss.str());

    if (!user.set_effective()) {
      return ScriptError::new_ref("set_user() Unable to set user/group ids");
    }
    return 0;
  }

  if ("set_thread_pool" == name) {
    if (!auth.admin()) return ScriptError::new_ref("Not permitted");
    const ScriptInt* a_threads = get_method_arg<ScriptInt>(args,0,"threads");
    if (!a_threads) 
      return ScriptError::new_ref("set_thread_pool() Must specify number of threads");
    int n_threads = a_threads->get_int();
    if (n_threads < 0)
      return ScriptError::new_ref("set_thread_pool() Must specify >= 0 threads");

    std::ostringstream oss;
    oss << "Setting thread pool to " << n_threads
        << (n_threads ? "" : " (multiplexed mode)");
    LOG(oss.str());

    m_spinner.set_num_threads((unsigned int)n_threads);
    return 0;
  }

  if ("set_latency" == name) {
    if (!auth.admin()) return ScriptError::new_ref("Not permitted");
    const ScriptInt* a_latency = get_method_arg<ScriptInt>(args,0,"latency");
    if (!a_latency)
      return ScriptError::new_ref("set_latency() Must specify latency time");

    int n_latency = a_latency->get_int();
    if (n_latency < 0)
      return ScriptError::new_ref("set_latency() Latency must be >= 0");

    std::ostringstream oss;
    oss << "Setting latency to " << n_latency;
    LOG(oss.str());

    m_spinner.set_latency((long)n_latency);
    return 0;
  }

  if ("enable_jobs" == name) {
    if (!auth.admin()) return ScriptError::new_ref("Not permitted");
    const ScriptInt* a_enable = get_method_arg<ScriptInt>(args,0,"enable");
    if (!a_enable) 
      return ScriptError::new_ref("enable_jobs() Enable flag not specified");

    if (a_enable->get_int()) {
      LOG("Enabling non-descriptor jobs");
      m_spinner.enable_jobs(true);
    } else {
      LOG("Disabling non-descriptor jobs");
      m_spinner.enable_jobs(false);
    }
    return 0;
  }

  return Module::script_method(auth,ref,name,args);
}

//=============================================================================
const std::string& Kernel::get_system_nodename() const
{
  return m_system_nodename;
}

//=============================================================================
const std::string& Kernel::get_system_version() const
{
  return m_system_version;
}

//=============================================================================
const std::string& Kernel::get_system_hardware() const
{
  return m_system_hardware;
}


//=============================================================================
Kernel::Kernel(const std::string& appname, const VersionTag& version)
  : Module(appname,version),
    m_state(Init),
    m_spinner(),
    m_conf_path(),
    m_autoload_config(false),
    m_system_nodename(),
    m_system_version(),
    m_system_hardware()
{
  struct utsname sysinf;
  if (::uname(&sysinf) != -1) {
    m_system_nodename = sysinf.nodename;
    m_system_version = std::string(sysinf.sysname) + " " + sysinf.release;
    m_system_hardware = sysinf.machine;
  }
}

};
