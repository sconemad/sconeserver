/* SconeServer (http://www.sconemad.com)

SconeServer kernel

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

#ifndef scxKernel_h
#define scxKernel_h

#include "sconex/Module.h"
#include "sconex/Multiplexer.h"
#include "sconex/Job.h"

namespace scx {

//=============================================================================
class SCONEX_API Kernel : public Module {

public:

  static Kernel* get();
  
  virtual ~Kernel();
  
  virtual std::string info() const;

  virtual int init();
  virtual void close();

  int run();

  void connect_config_console();

  virtual void set_logger(Logger* logger);
  
  // Connect descriptor
  virtual bool connect(
    Descriptor* d,
    ScriptRef* args
  );

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

  Kernel();

private:

  enum State { Init, Run, Restart, Shutdown };
  State m_state;

  Multiplexer m_spinner;

  std::string m_system_nodename;
  std::string m_system_version;
  std::string m_system_hardware;
  
  static ScriptRefTo<Kernel>* s_kernel;
  
};

};
  
#endif
