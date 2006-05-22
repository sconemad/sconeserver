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


#include "TestBuilderModule.h"
#include "TestBuilderControlStream.h"
#include "BuildProfile.h"
#include "Build.h"
#include "BuildStep.h"

#include "sconex/ModuleInterface.h"
#include "sconex/Module.h"
#include "sconex/Arg.h"
#include "sconex/Stream.h"
#include "sconex/Date.h"
#include "sconex/FileDir.h"

SCONESERVER_MODULE(TestBuilderModule);

//=========================================================================
TestBuilderModule::TestBuilderModule(
)
  : scx::Module("testbuilder",scx::version()),
    m_build_user("nobody"),
    m_max_running(1)
{

}

//=========================================================================
TestBuilderModule::~TestBuilderModule()
{
  stop();
  
  for (std::map<std::string,BuildProfile*>::iterator it_p =
         m_profiles.begin();
       it_p != m_profiles.end();
       it_p++) {
    delete (*it_p).second;
  }

  for (std::list<Build*>::iterator it_b = m_builds.begin();
       it_b != m_builds.end();
       it_b++) {
    delete (*it_b);
  }
}

//=========================================================================
std::string TestBuilderModule::info() const
{
  return "Copyright (c) 2000-2006 Andrew Wedgbury\n"
         "Test Builder\n";
}

//=========================================================================
bool TestBuilderModule::connect(
  scx::Descriptor* endpoint,
  scx::ArgList* args
)
{
  TestBuilderControlStream* s = new TestBuilderControlStream(*this);
  s->add_module_ref(ref());
  
  endpoint->add_stream(s);
  return true;
}

//=============================================================================
void* TestBuilderModule::run()
{
  // Load builds from disk
  m_builds_mutex.lock();
  scx::FileDir dir(m_dir + "builds");
  while (dir.next()) {
    std::string id = dir.name().path();
    Build* build = new Build(*this);
    if (build->load(id)) {
      m_builds.push_back(build);
    } else {
      delete build;
    }
  }
  m_builds_mutex.unlock();
  
  while (true) {
    
    sleep(1);

    int nrun = 0;
    
    m_builds_mutex.lock();
    for (std::list<Build*>::iterator it = m_builds.begin();
         it != m_builds.end();
       it++) {
      Build* build = (*it);
      switch (build->get_state()) {
        
        case Build::Unstarted:
          // Don't start build if we're already running the max allowed
          if (nrun >= m_max_running) break;
          
          // Fall through to running state...
          
        case Build::Running:
          ++nrun;
          // Proceed with the build
          build->proceed();
          break;
          
        case Build::Passed:
        case Build::Failed:
        case Build::Aborted:
          // Build is finished
          break;
          
      }
    }
    m_builds_mutex.unlock();
  }
  
  return 0;
}

//=============================================================================
scx::Arg* TestBuilderModule::arg_lookup(const std::string& name)
{
  // Methods

  if ("add" == name ||
      "remove" == name ||
      "submit_build" == name ||
      "abort_build" == name ||
      "remove_build" == name ||
      "set_dir" == name ||
      "set_max_running" == name ||
      "set_build_user" == name ||
      "add_source_method" == name ||
      "load_profiles" == name ||
      "save_profiles" == name ||
      "start" == name ||
      "stop" == name) {
    return new scx::ArgObjectFunction(
      new scx::ArgModule(ref()),name);
  }      

  // Properties
  
  if ("max_running" == name) {
    return new scx::ArgInt(m_max_running);
  }

  if ("source_methods" == name) {
    scx::ArgList* l1 = new scx::ArgList();
    for (std::map<std::string,std::string>::const_iterator it = 
	   m_source_methods.begin();
	 it != m_source_methods.end();
	 it++) {
      scx::ArgList* l2 = new scx::ArgList();
      l2->give( new scx::ArgString((*it).first) );
      l2->give( new scx::ArgString((*it).second) );
      l1->give(l2);
    }
    return l1;
  }

  if ("profiles" == name) {
    scx::ArgList* l1 = new scx::ArgList();
    for (std::map<std::string,BuildProfile*>::const_iterator it = 
	   m_profiles.begin();
	 it != m_profiles.end();
	 it++) {
      l1->give( new scx::ArgString((*it).first) );
    }
    return l1;
  }

  if ("builds" == name) {
    std::ostringstream oss;
    for (std::list<Build*>::reverse_iterator it = m_builds.rbegin();
	 it != m_builds.rend();
	 it++) {
      Build* build = (*it);
      oss << build->get_id() << " "
          << build->get_profile() << " "
          << Build::get_state_str(build->get_state()) << "\n";
    }
    return new scx::ArgString(oss.str());
  }

  if ("buildstats" == name) {
    m_builds_mutex.lock();
    scx::ArgList* l1 = new scx::ArgList();
    for (std::list<Build*>::reverse_iterator it = m_builds.rbegin();
	 it != m_builds.rend();
	 it++) {
      Build* build = (*it);
      
      scx::ArgList* l2 = new scx::ArgList();
      l2->give( new scx::ArgString(build->get_profile()) );
      l2->give( new scx::ArgString(build->get_id()) );
      l2->give( new scx::ArgString(Build::get_state_str(build->get_state())) );

      scx::ArgList* l3 = new scx::ArgList();
      for (std::list<BuildStep*>::const_iterator it_s =
             build->get_steps().begin();
           it_s != build->get_steps().end();
           it_s++) {
        BuildStep* step = (*it_s);
        scx::ArgList* l4 = new scx::ArgList();
        Build::State state = step->get_state();
        l4->give( new scx::ArgString(step->get_name()) );

        if (state == Build::Unstarted) {
          l4->give( new scx::ArgString("") );
        } else {
          l4->give( new scx::ArgString(step->get_start_time().code()) );
        }

        if (state == Build::Unstarted || state == Build::Aborted) {
          l4->give( new scx::ArgString("") );
        } else if (state == Build::Running) {
          scx::Time t = scx::Date::now() - step->get_start_time();
          l4->give( new scx::ArgString(t.string() + "+") );
        } else {
          scx::Time t = step->get_finish_time() - step->get_start_time();
          l4->give( new scx::ArgString(t.string()) );
        }

        l4->give( new scx::ArgString(Build::get_state_str(state)) );
        l3->give(l4);
      }
      l2->give(l3);
      l1->give(l2);
    }
    m_builds_mutex.unlock();
    return l1;
  }
  
  // Sub-objects
  
  std::map<std::string,BuildProfile*>::const_iterator it = 
    m_profiles.find(name);
  if (it != m_profiles.end()) {
    return new scx::ArgObject((*it).second);
  }
  
  return SCXBASE Module::arg_lookup(name);
}

//=============================================================================
scx::Arg* TestBuilderModule::arg_function(
  const std::string& name,
  scx::Arg* args
)
{
  scx::ArgList* l = dynamic_cast<scx::ArgList*>(args);
  
  if ("add" == name) {
    const scx::ArgString* a_name =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_name) {
      return new scx::ArgError("testbuilder::add() Name must be specified");
    }
    std::string s_name = a_name->get_string();

    if (!add_profile(s_name)) {
      return new scx::ArgError("testbuilder::add() Profile '" + s_name +
                               "' already exists");
    }
    return 0;
  }
  
  if ("remove" == name) {
    const scx::ArgString* a_name =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_name) {
      return new scx::ArgError("testbuilder::remove() Name must be specified");
    }
    std::string s_name = a_name->get_string();

    if (!remove_profile(s_name)) {
      return new scx::ArgError("testbuilder::remove() Profile '" + s_name +
                               "' does not exist");
    }
    return 0;
  }
  
  if ("submit_build" == name) {
    const scx::ArgString* a_profile =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_profile) {
      return new scx::ArgError("testbuilder::submit() "
                               "Profile must be specified");
    }
    std::string s_profile = a_profile->get_string();
    std::string id = submit_build(s_profile);
    if (id.empty()) {
      return new scx::ArgError("testbuilder::submit() "
                               "Could not submit build");
    }
    return new scx::ArgString(id);
  }

  if ("abort_build" == name) {
    const scx::ArgString* a_build =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_build) {
      return new scx::ArgError("testbuilder::abort_build() "
                               "Build ID must be specified");
    }
    std::string s_id = a_build->get_string();
    if (!abort_build(s_id)) {
      return new scx::ArgError("testbuilder::abort_build() "
                               "Could not abort build");
    }
    return 0;
  }

  if ("remove_build" == name) {
    const scx::ArgString* a_build =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_build) {
      return new scx::ArgError("testbuilder::remove_build() "
                               "Build ID must be specified");
    }
    std::string s_id = a_build->get_string();
    if (!remove_build(s_id)) {
      return new scx::ArgError("testbuilder::remove_build() "
                               "Could not remove build");
    }
    return 0;
  }

  if ("set_dir" == name) {
    const scx::ArgString* a_dir =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_dir) {
      return new scx::ArgError("testbuilder::set_dir() "
                               "Directory must be specified");
    }
    m_dir = a_dir->get_string();
    return 0;
  }

  if ("set_max_running" == name) {
    const scx::ArgInt* a_max = dynamic_cast<const scx::ArgInt*>(l->get(0));
    if (!a_max) {
      return new scx::ArgError("testbuilder::set_max_running() "
                               "Value must be specified");
    }
    int i_max = a_max->get_int();
    if (i_max <=0) {
      return new scx::ArgError("testbuilder::set_max_running() "
                               "Value must be >= 0");
    }
    m_max_running = i_max;
    return 0;
  }

  if ("set_build_user" == name) {
    const scx::ArgString* a_user = dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_user) {
      return new scx::ArgError("testbuilder::set_build_user() "
                               "Username must be specified");
    }
    m_build_user = scx::User(a_user->get_string());

    return 0;
  }

  if ("add_source_method" == name) {
    const scx::ArgString* a_name =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_name) {
      return new scx::ArgError("testbuilder::add_source_method() "
                               "Name must be specified");
    }
    std::string s_name = a_name->get_string();

    const scx::ArgString* a_desc =
      dynamic_cast<const scx::ArgString*>(l->get(1));
    if (!a_desc) {
      return new scx::ArgError("testbuilder::add_source_method() "
                               "Description must be specified");
    }
    
    m_source_methods[s_name] = a_desc->get_string();
    return 0;
  }

  if ("load_profiles" == name) {
    scx::FilePath path = m_dir + "profiles.conf";
    if (!load_config_file(path)) {
      return new scx::ArgError("load_profiles() Error occured");
    }
    return 0;
  }
  
  if ("save_profiles" == name) {
    if (!save_profiles()) {
      return new scx::ArgError("testbuilder::save_profiles() "
                               "Error saving profiles");
    }
    return 0;
  }

  if ("start" == name) {
    start();
    return 0;
  }

  if ("stop" == name) {
    stop();
    return 0;
  }
  
  return SCXBASE Module::arg_function(name,args);
}

//=============================================================================
BuildProfile* TestBuilderModule::lookup_profile(
  const std::string& name
)
{
  std::map<std::string,BuildProfile*>::const_iterator it =
    m_profiles.find(name);
  if (it == m_profiles.end()) {
    return 0;
  }
  return (*it).second;
}

//=============================================================================
const scx::FilePath& TestBuilderModule::get_dir() const
{
  return m_dir;
}

//=============================================================================
const scx::User& TestBuilderModule::get_build_user() const
{
  return m_build_user;
}

//=============================================================================
std::string TestBuilderModule::submit_build(const std::string& profile)
{
  std::map<std::string,BuildProfile*>::iterator it = 
    m_profiles.find(profile);
  if (it == m_profiles.end()) {
    return "";
  }
  
  BuildProfile* profile_obj = (*it).second;
  Build* build = profile_obj->create_build(m_dir);
  
  if (!build) {
    return "";
  }
  
  std::string id = build->get_id();
  
  // Add build to array
  m_builds_mutex.lock();
  m_builds.push_back(build);
  m_builds_mutex.unlock();

  return id;
}

//=============================================================================
bool TestBuilderModule::abort_build(const std::string& id)
{
  scx::MutexLocker locker(m_builds_mutex);
  for (std::list<Build*>::iterator it = m_builds.begin();
       it != m_builds.end();
       it++) {
    Build* build = (*it);
    if (build->get_id() == id) {
      return build->abort();
    }
  }
  return false;
}

//=============================================================================
bool TestBuilderModule::remove_build(const std::string& id)
{
  scx::MutexLocker locker(m_builds_mutex);
  for (std::list<Build*>::iterator it = m_builds.begin();
       it != m_builds.end();
       it++) {
    Build* build = (*it);
    if (build->get_id() == id) {
      build->remove();
      delete build;
      m_builds.erase(it);
      return true;
    }
  }
  return false;
}

//=============================================================================
bool TestBuilderModule::add_profile(const std::string& profile)
{
  std::map<std::string,BuildProfile*>::const_iterator it = 
    m_profiles.find(profile);
  if (it != m_profiles.end()) {
    return false;
  }
   
  m_profiles[profile] = new BuildProfile(*this,profile);
  return true;
}

//=============================================================================
bool TestBuilderModule::remove_profile(const std::string& profile)
{
  std::map<std::string,BuildProfile*>::iterator it = 
    m_profiles.find(profile);
  if (it == m_profiles.end()) {
    return false;
  }

  delete (*it).second;
  m_profiles.erase(it);

  return true;
}

//=============================================================================
bool TestBuilderModule::save_profiles()
{
  scx::FilePath path = m_dir + "profiles.conf";
  scx::File file;
  if (scx::Ok != file.open(path,scx::File::Write | scx::File::Create)) {
    return false;
  }
  for (std::map<std::string,BuildProfile*>::const_iterator it = 
         m_profiles.begin();
       it != m_profiles.end();
       it++) {
    BuildProfile* profile = (*it).second;
    profile->save(file);
  }
  return true;
}
