/* SconeServer (http://www.sconemad.com)

Test Build

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

#ifndef testbuilderBuild_h
#define testbuilderBuild_h

#include "TestBuilderModule.h"
#include "BuildProfile.h"

class BuildStep;

//#########################################################################
class Build {

public:

  Build(TestBuilderModule& module);
  // Create a new build for loading

  Build(
    TestBuilderModule& module,
    const std::string& profile
  );
  // Create a new build for running

  ~Build();

  bool load(const std::string& id);
  // Load an existing build given by id

  bool save();
  // Save build's current state
  
  bool proceed();
  // Proceed with the build

  bool abort();
  // Abort the build

  bool remove();
  // Remove the build 
  
  void add_step(BuildStep* step);
  // Adds (and takes ownership) of the build step
  
  enum State { Unstarted, Running, Passed, Failed, Aborted };
  State get_state() const;
  // Get the current state of the build

  const std::string& get_profile() const { return m_profile; };
  const std::string& get_id() const { return m_id; };
  const std::list<BuildStep*>& get_steps() const { return m_steps; };

  static std::string get_state_str(State state);
  
private:

  void log(const std::string& message);
  
  TestBuilderModule& m_module;

  State m_state;
  std::string m_profile;
  std::string m_id;
  scx::FilePath m_dir;
  
  std::list<BuildStep*> m_steps;
  
};

#endif
