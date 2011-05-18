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


#include "BuildStep.h"
#include "BuildProcessStream.h"
#include <sconex/Process.h>
#include <sconex/FilePath.h>
#include <sconex/StreamTransfer.h>
#include <sconex/StreamTokenizer.h>
#include <sconex/File.h>
#include <sconex/Kernel.h>
#include <sconex/User.h>

//=========================================================================
BuildStep::BuildStep(
  TestBuilderModule& module,
  const std::string& name,
  const std::string& command
) : m_module(module),
    m_name(name),
    m_command(command),
    m_state(Build::Unstarted),
    m_exitcode(1),
    m_process_stream(0)
{

}

//=========================================================================
BuildStep::BuildStep(
  TestBuilderModule& module,
  const scx::FilePath& dir,
  int version,
  scx::StreamTokenizer* parser
) : m_module(module),
    m_dir(dir),
    m_process_stream(0)
{
  std::string token;
  int i = 0;
  bool done = false;
  while (!done && parser->tokenize(token) == scx::Ok) {
    switch (++i) {
      case 1: m_name = token; break;
      case 2: m_command = token; break;
      case 3: m_state = (Build::State)atoi(token.c_str()); break;
      case 4: m_start_time = scx::Date(token); break;
      case 5: m_finish_time = scx::Date(token);
        done = true;
        break;
    }
  }

  if (m_state == Build::Running) {
    m_state = Build::Aborted;
  }
}

//=========================================================================
BuildStep::~BuildStep()
{
  // Notify the process stream
  m_process_mutex.lock();
  if (m_process_stream) m_process_stream->cancel();
  m_process_mutex.unlock();
}

//=========================================================================
bool BuildStep::save(scx::Descriptor& d)
{
  std::ostringstream oss;
  m_process_mutex.lock();
  oss << "1\n"
      << m_name << "\n"
      << m_command << "\n"
      << m_state << "\n"
      << m_start_time.string() << "\n"
      << m_finish_time.string() << "\n";
  m_process_mutex.unlock();
  d.write(oss.str());
  return true;
}

//=========================================================================
bool BuildStep::proceed()
{
  switch (m_state) {
    
    case Build::Unstarted: {
      if (launch()) {
        m_state = Build::Running;
      } else {
        m_state = Build::Failed;
        return true;
      }
    }
    case Build::Running:
      {
        scx::MutexLocker locker(m_process_mutex);
        if (m_process_stream != 0) {
          break;
        }
      }
      if (m_exitcode == 0) {
        m_state = Build::Passed;
      } else {
        m_state = Build::Failed;
      }
      
    case Build::Passed:
    case Build::Failed: 
    case Build::Aborted: 
      return true;
  }
  return false;
}

//=========================================================================
bool BuildStep::abort()
{
  m_process_mutex.lock();
  if (m_process_stream) {
    m_process_stream->cancel();
    m_process_stream = 0;
  }
  m_process_mutex.unlock();
  m_state = Build::Aborted;
  return true;
}

//=========================================================================
void BuildStep::set_dir(const scx::FilePath& dir)
{
  m_dir = dir;
}

//=========================================================================
const std::string& BuildStep::get_name() const
{
  return m_name;
}

//=========================================================================
Build::State BuildStep::get_state() const
{
  return m_state;
}

//=========================================================================
void BuildStep::process_exited(int code)
{
  m_process_mutex.lock();
  m_finish_time = scx::Date::now();
  m_exitcode = code;
  m_process_stream = 0;
  m_process_mutex.unlock();
}

//=========================================================================
const scx::Date& BuildStep::get_start_time() const
{
  return m_start_time;
}

//=========================================================================
const scx::Date& BuildStep::get_finish_time() const
{
  return m_finish_time;
}

//=========================================================================
bool BuildStep::launch()
{
  scx::Process* process = 0;

  std::ostringstream oss_log;
  oss_log << "---------------------------------------"
          << "---------------------------------------\n"
          << " SconeServer testbuilder log file\n"
          << "\n";

  process = new scx::Process();
  process->parse_command_line(m_command);
  oss_log << " Command: " << m_command << "\n";

  oss_log << "\n"
          << " Working dir: " << m_dir.path() << "\n"
          << "---------------------------------------"
          << "---------------------------------------\n"
          << "\n";

  if (!process) {
    DEBUG_LOG("Process not created");
    // Process wasn't created for some reason?
    return false;
  }

  process->set_dir(m_dir);
  process->set_user(m_module.get_build_user());
  m_start_time = scx::Date::now();
  m_finish_time = m_start_time;
  bool err = process->launch();

  scx::File* file = new scx::File();
  if (file->open(m_dir + scx::FilePath("testbuilder_" + m_name + ".log"),
                 scx::File::Write | scx::File::Append | scx::File::Create)
      != scx::Ok) {
    DEBUG_LOG("Cannot open log file");
    delete process;
    return false;
  }
  scx::Kernel::get()->connect(file,0);
  file->write(oss_log.str());

  m_process_mutex.lock();
  m_process_stream = new BuildProcessStream(&m_module,this);
  process->add_stream(m_process_stream);
  
  scx::StreamTransfer* xfer = new scx::StreamTransfer(process);
  xfer->set_close_when_finished(true);
  file->add_stream(xfer);
  
  scx::Kernel::get()->connect(process,0);
  m_process_mutex.unlock();

  return err;
}
