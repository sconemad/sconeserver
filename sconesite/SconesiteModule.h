/* SconeServer (http://www.sconemad.com)

Sconesite Module

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

#ifndef sconesiteModule_h
#define sconesiteModule_h

#include <sconex/Module.h>
#include <sconex/Job.h>
#include "Profile.h"

//=============================================================================
// SconesiteModule - A content management module for SconeServer
//
class SconesiteModule : public scx::Module,
                        public scx::Provider<scx::Stream> {
public:

  SconesiteModule();
  virtual ~SconesiteModule();
  
  virtual std::string info() const;
  
  void refresh();
  Profile* lookup_profile(const std::string& name);

  // Module methods
  virtual scx::ScriptRef* script_op(const scx::ScriptAuth& auth,
				    const scx::ScriptRef& ref,
				    const scx::ScriptOp& op,
				    const scx::ScriptRef* right=0);

  virtual scx::ScriptRef* script_method(const scx::ScriptAuth& auth,
					const scx::ScriptRef& ref,
					const std::string& name,
					const scx::ScriptRef* args);

  // Provider<Stream> method
  virtual void provide(const std::string& type,
		       const scx::ScriptRef* args,
		       scx::Stream*& object);
protected:
  
private:

  typedef HASH_TYPE<std::string,Profile::Ref*> ProfileMap;
  ProfileMap m_profiles;

  scx::JobID m_job;
  
};

//=============================================================================
// RunTimer - Microsecond precision timer for profiling
//
class RunTimer {
public:

  RunTimer() : m_start_time(0), m_stop_time(0) {};
  
  void start() { m_start_time = current_time(); };
  void stop() { m_stop_time = current_time(); };
  
  long int get_run_time() { return (m_stop_time - m_start_time); };

protected:
  
  long int current_time() {
    timeval tv;
    gettimeofday(&tv,0);
    return (tv.tv_sec*1000000) + tv.tv_usec;
  };

  long int m_start_time;
  long int m_stop_time;

};


#endif

