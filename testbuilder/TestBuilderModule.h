/* SconeServer (http://www.sconemad.com)

Test Builder Module

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

#ifndef testbuilderModule_h
#define testbuilderModule_h

#include "BuildProfile.h"

#include "sconex/Module.h"
#include "sconex/Thread.h"
#include "sconex/Mutex.h"
#include "sconex/User.h"

class Build;

//=========================================================================
// TestBuilderModule - A sconeserver-based build and test system
//
class TestBuilderModule : public scx::Module, 
                          public scx::Thread {
public:

  TestBuilderModule();
  virtual ~TestBuilderModule();

  virtual std::string info() const;

  // Thread methods  
  virtual void* run();
  
  // ScriptObject methods  
  virtual scx::ScriptRef* script_op(const scx::ScriptAuth& auth,
				    const scx::ScriptRef& ref,
				    const scx::ScriptOp& op,
				    const scx::ScriptRef* right=0);

  virtual scx::ScriptRef* script_method(const scx::ScriptAuth& auth,
					const scx::ScriptRef& ref,
					const std::string& name,
					const scx::ScriptRef* args);

  BuildProfile* lookup_profile(const std::string& name);

  typedef scx::ScriptRefTo<TestBuilderModule> Ref;

  // Get the testbuilder directory
  const scx::FilePath& get_dir() const;

  // Get the user to run builds as
  const scx::User& get_build_user() const;
  
  // Submit a new build using the specified profile
  // Returns the created build ID
  std::string submit_build(const std::string& profile);

  // Abort build specified by id
  bool abort_build(const std::string& id);

  // Remove build specified by id
  bool remove_build(const std::string& id);
  
  // Add/remove a profile
  bool add_profile(const std::string& profile);
  bool remove_profile(const std::string& profile);

  // Save profiles into a file
  bool save_profiles();
  
  scx::ScriptRef* get_profiles();
  scx::ScriptRef* get_source_methods();
  scx::ScriptRef* get_builds();
  scx::ScriptRef* get_buildstats();

private:

  // Test build profiles
  typedef std::map<std::string,BuildProfile::Ref*> ProfileMap;
  ProfileMap m_profiles;

  // Source methods
  typedef std::map<std::string,std::string> SourceMethodMap;
  SourceMethodMap m_source_methods;
  
  // Test builds
  typedef std::list<Build*> BuildList;
  BuildList m_builds;
  
  // Mutex for accessing build array
  scx::Mutex m_builds_mutex;

  // Root directory for test builds
  scx::FilePath m_dir;

  // User to run builds as
  scx::User m_build_user;

  // Maximum number of running builds allowed
  int m_max_running;
  
};

#endif

