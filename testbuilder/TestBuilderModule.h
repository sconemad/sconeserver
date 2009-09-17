/* SconeServer (http://www.sconemad.com)

Test Builder Module

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

#ifndef testbuilderModule_h
#define testbuilderModule_h

#include "sconex/Module.h"
#include "sconex/Thread.h"
#include "sconex/Mutex.h"
#include "sconex/User.h"

class BuildProfile;
class Build;

//#########################################################################
class TestBuilderModule : public scx::Module, public scx::Thread {

public:

  TestBuilderModule();
  virtual ~TestBuilderModule();

  virtual std::string info() const;
  
  virtual bool connect(
    scx::Descriptor* endpoint,
    scx::ArgList* args
  );

  virtual void* run();
  
  virtual scx::Arg* arg_lookup(const std::string& name);
  virtual scx::Arg* arg_function(const std::string& name,scx::Arg* args);

  BuildProfile* lookup_profile(const std::string& name);

  const scx::FilePath& get_dir() const;
  // Get the testbuilder directory

  const scx::User& get_build_user() const;
  // Get the user to run builds as
  
  std::string submit_build(const std::string& profile);
  // Submit a new build using the specified profile
  // Returns the created build ID

  bool abort_build(const std::string& id);
  // Abort build specified by id

  bool remove_build(const std::string& id);
  // Remove build specified by id
  
  bool add_profile(const std::string& profile);
  bool remove_profile(const std::string& profile);
  // Add/remove a profile

  bool save_profiles();
  // Save profiles into a file
  
protected:
  
private:

  typedef std::map<std::string,BuildProfile*> ProfileMap;
  ProfileMap m_profiles;
  // Test build profiles

  typedef std::map<std::string,std::string> SourceMethodMap;
  SourceMethodMap m_source_methods;
  // Source methods
  
  typedef std::list<Build*> BuildList;
  BuildList m_builds;
  // Test builds
  
  scx::Mutex m_builds_mutex;
  // Mutex for accessing build array

  scx::FilePath m_dir;
  // Root directory for test builds

  scx::User m_build_user;
  // User to run builds as

  int m_max_running;
  // Maximum number of running builds allowed
  
};

#endif

