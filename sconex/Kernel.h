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

#ifndef scxKernel_h
#define scxKernel_h

#include <sconex/Module.h>
#include <sconex/Multiplexer.h>
#include <sconex/Job.h>

namespace scx {

//=============================================================================
class SCONEX_API Kernel : public Module {
public:

  // Create a new sconex kernel for this app - must be called once at startup
    static Kernel* create(const std::string& appname, 
			  const VersionTag& version = scx::version());

  // Get the kernel object singleton
  static Kernel* get();
  
  virtual ~Kernel();
  
  virtual std::string info() const;

  virtual int init();
  virtual bool close();

  // Set/get configuration path
  void set_conf_path(const FilePath& path);
  FilePath get_conf_path() const;
  void set_autoload_config(bool autoload);

  // Run the kernel event loop.
  // Returns either when the multiplexer runs out of descriptors, or a shutdown
  // or restart is requested.
  // A return value of 0 indicates a restart has been requested.
  // If console is true, spawns a configuration console on stdin/stdout.
  int run(bool console);

  void connect_config_console();

  // Add a descriptor job to the kernel
  virtual bool connect(Descriptor* d);

  // Add a job to the kernel
  JobID add_job(Job* job);

  // End a job
  bool end_job(JobID jobid);

  virtual ScriptRef* script_op(const ScriptAuth& auth,
			       const ScriptRef& ref,
			       const ScriptOp& op,
			       const ScriptRef* right);
  
  virtual ScriptRef* script_method(const ScriptAuth& auth,
				   const ScriptRef& ref,
				   const std::string& name,
				   const ScriptRef* args);
  
  // Access system info
  const std::string& get_system_nodename() const;
  const std::string& get_system_version() const;
  const std::string& get_system_hardware() const;
  
protected:

  Kernel(const std::string& appname, const VersionTag& version);

private:

  enum State { Init, Run, Restart, Shutdown };
  State m_state;

  Multiplexer m_spinner;

  FilePath m_conf_path;
  bool m_autoload_config;
  
  std::string m_system_nodename;
  std::string m_system_version;
  std::string m_system_hardware;
  
  static ScriptRefTo<Kernel>* s_kernel;
  
};

};
  
#endif
