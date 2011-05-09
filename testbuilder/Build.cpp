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


#include "Build.h"
#include "BuildStep.h"
#include <sconex/FilePath.h>
#include <sconex/LineBuffer.h>
#include <sconex/Process.h>
#include <sconex/File.h>

const char TESTBUILDER_STAT_FILE[] = "testbuilder.stat";

//=========================================================================
Build::Build(
  TestBuilderModule& module
) : m_module(module),
    m_state(Unstarted)
{

}

//=========================================================================
Build::Build(
  TestBuilderModule& module,
  const std::string& profile
) : m_module(module),
    m_state(Unstarted),
    m_profile(profile)
{
  m_id = scx::Date::now().dcode();
  m_dir = m_module.get_dir() + "builds" + m_id;

  // Create working directory for this build and set ownership
  scx::FilePath::mkdir(m_dir,true,0770);
  scx::FilePath::chown(m_dir,m_module.get_build_user());
}

//=========================================================================
Build::~Build()
{
  for (StepList::iterator it = m_steps.begin();
       it != m_steps.end();
       it++) {
    delete (*it);
  }
}

//=========================================================================
bool Build::load(const std::string& id)
{
  m_id = id;
  m_dir = m_module.get_dir() + "builds" + m_id;

  scx::FilePath stat_filename = m_dir + TESTBUILDER_STAT_FILE;
  scx::File file;
  if (!scx::FileStat(stat_filename).is_file() ||
      file.open(stat_filename,scx::File::Read) != scx::Ok) {
    return false;
  }

  scx::LineBuffer* parser = new scx::LineBuffer("build-parser");
  file.add_stream(parser);

  std::string token;
  int i = 0;
  int version;
  while (parser->tokenize(token) == scx::Ok) {
    switch (++i) {
      case 1: /* text header */ break;
      case 2: version = (State)atoi(token.c_str()); break;
      case 3: m_state = (State)atoi(token.c_str()); break;
      case 4: m_profile = token; break;
      case 5: m_dir = token; break;
      default: {
        int version = (State)atoi(token.c_str());
        add_step( new BuildStep(m_module,m_dir,version,parser) );
      }
    }
  }

  if (m_state == Running) {
    m_state = Aborted;
  }

  return true;
}

//=========================================================================
bool Build::save()
{
  scx::FilePath stat_filename = m_dir + TESTBUILDER_STAT_FILE;
  scx::File file;
  if (file.open(stat_filename,scx::File::Write | scx::File::Create) != scx::Ok) {
    return false;
  }

  std::ostringstream oss;
  oss << "// TestBuilder build state file\n"
      << "1\n"
      << ((int)m_state) << "\n"
      << m_profile << "\n"
      << m_dir.path() << "\n";
  file.write(oss.str());

  for (StepList::const_iterator it = m_steps.begin();
       it != m_steps.end();
       it++) {
    BuildStep* step = (*it);
    if (!step->save(file)) {
      return false;
    }
  }

  return true;
}

//=========================================================================
bool Build::proceed()
{
  for (StepList::const_iterator it = m_steps.begin();
       it != m_steps.end();
       it++) {
    BuildStep* step = (*it);
    switch (step->get_state()) {

      case Unstarted: 
        if (it == m_steps.begin()) {
          m_state = Running;
          log("STARTED (using profile " + m_profile + ")");
        }
        // Setup and run the next build step
        step->set_dir(m_dir);
        log("Step '" + step->get_name() +  "' STARTED");

        // Fall through to running state...
        
      case Running:
        if (!step->proceed()) {
          // Stay on this step for now
          save();
          return false;
        }
        log("Step '" + step->get_name() +  "' " +
            get_state_str(step->get_state()));
        if (step->get_state() == Passed) {
          // Proceed to next step
          save();
          break;
        }

        // Fall through to failed/aborted state...

      case Failed:
      case Aborted:
        m_state = step->get_state();
        log(get_state_str(m_state));
        save();
        return true;

      case Passed:
        // Proceed to next step
        break;
    }       

  }

  m_state = Passed;
  log(get_state_str(m_state));
  save();
  return true;
}

//=========================================================================
bool Build::abort()
{
  if (m_state != Running) {
    return false;
  }
  
  m_state = Aborted;
  for (StepList::const_iterator it = m_steps.begin();
       it != m_steps.end();
       it++) {
    BuildStep* step = (*it);
    if (step->get_state() == Running) {
      step->abort();
    }
  }
  save();
  return true;
}

//=========================================================================
bool Build::remove()
{
  abort();

  scx::FilePath script = m_module.get_dir() + "scripts" + "remove";
  scx::Process process(script.path());
  process.add_arg(script.path());
  process.add_arg(m_dir.path());
  process.set_dir(m_module.get_dir());
  process.set_detatched(true);
  return (process.launch());
}

//=========================================================================
void Build::add_step(BuildStep* step)
{
  m_steps.push_back(step);
}

//=========================================================================
Build::State Build::get_state() const
{
  return m_state;
}

//=========================================================================
std::string Build::get_state_str(State state)
{
  switch (state) {
    case Unstarted: return "UNSTARTED";
    case Running: return "RUNNING";
    case Passed: return "PASSED";
    case Failed: return "FAILED";
    case Aborted: return "ABORTED";
  }
  return "UNKNOWN";
}

//=========================================================================
void Build::log(const std::string& message)
{
  m_module.log("{BUILD:" + m_id + "} " + message);
}
