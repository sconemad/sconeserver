/* SconeServer (http://www.sconemad.com)

Build Step

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

#ifndef testbuilderBuildStep_h
#define testbuilderBuildStep_h

#include "TestBuilderModule.h"
#include "BuildProfile.h"
#include "Build.h"

#include <sconex/Date.h>
#include <sconex/Mutex.h>

namespace scx { class Process; class StreamTokenizer; };
class BuildProcessStream;

//#########################################################################
class BuildStep {

public:

  BuildStep(
    TestBuilderModule& module,
    const std::string& name,
    const std::string& command
  );
  // Create a new build step

  BuildStep(
    TestBuilderModule& module,
    const scx::FilePath& dir,
    int version,
    scx::StreamTokenizer* parser
  );
  // Load an existing build step from parser

  ~BuildStep();

  bool save(scx::Descriptor& d);
  // Save this build step
  
  bool proceed();
  // Proceed with this build step
  // Returns true if the step is finished

  bool abort();
  // Abort the build step

  void set_dir(const scx::FilePath& dir);
  // Set working directory for this build step

  const std::string& get_name() const;
  // Get the name of this step
  
  Build::State get_state() const;
  // Get the current state of this build step

  void process_exited(int code);
  // Notification that the process has exited

  const scx::Date& get_start_time() const;
  // Get the start time

  const scx::Date& get_finish_time() const;
  // Get the finish time
  
private:

  bool launch();
  // Launch the build command

  TestBuilderModule& m_module;

  std::string m_name;
  std::string m_command;
  scx::FilePath m_dir;
  Build::State m_state;
  int m_exitcode;

  BuildProcessStream* m_process_stream;
  scx::Mutex m_process_mutex;
  
  scx::Date m_start_time;
  scx::Date m_finish_time;
};

#endif
