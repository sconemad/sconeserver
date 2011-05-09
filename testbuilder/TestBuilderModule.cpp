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


#include "TestBuilderModule.h"
#include "BuildProfile.h"
#include "Build.h"
#include "BuildStep.h"

#include <sconex/ModuleInterface.h>
#include <sconex/Module.h>
#include <sconex/Stream.h>
#include <sconex/ScriptTypes.h>
#include <sconex/Date.h>
#include <sconex/FileDir.h>
#include <sconex/File.h>

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
  
  for (ProfileMap::iterator it_p =
         m_profiles.begin();
       it_p != m_profiles.end();
       it_p++) {
    delete it_p->second;
  }

  for (BuildList::iterator it_b = m_builds.begin();
       it_b != m_builds.end();
       it_b++) {
    delete (*it_b);
  }
}

//=========================================================================
std::string TestBuilderModule::info() const
{
  return "Test Builder";
}

//=============================================================================
void* TestBuilderModule::run()
{
  // Load builds from disk
  m_builds_mutex.lock();
  scx::FileDir dir(m_dir + "builds");
  while (dir.next()) {
    std::string id = dir.name();
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

    if (should_exit()) {
      break;
    }

    int nrun = 0;
    
    m_builds_mutex.lock();
    for (BuildList::iterator it = m_builds.begin();
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
scx::ScriptRef* TestBuilderModule::script_op(const scx::ScriptAuth& auth,
					     const scx::ScriptRef& ref,
					     const scx::ScriptOp& op,
					     const scx::ScriptRef* right)
{
  if (scx::ScriptOp::Lookup == op.type()) {
    std::string name = right->object()->get_string();

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
      return new scx::ScriptMethodRef(ref,name);
    }      

    // Properties
    if ("max_running" == name) return scx::ScriptInt::new_ref(m_max_running);
    if ("source_methods" == name) return get_source_methods();
    if ("profiles" == name) return get_profiles();
    if ("builds" == name) return get_builds();
    if ("buildstats" == name) return get_buildstats();
    
    // Sub-objects
    ProfileMap::const_iterator it = m_profiles.find(name);
    if (it != m_profiles.end()) {
      return it->second->ref_copy(ref.reftype());
    }
  }
  
  return scx::Module::script_op(auth,ref,op,right);
}

//=============================================================================
scx::ScriptRef* TestBuilderModule::script_method(const scx::ScriptAuth& auth,
						 const scx::ScriptRef& ref,
						 const std::string& name,
						 const scx::ScriptRef* args)
{
  //  scx::ScriptList* l = dynamic_cast<scx::ScriptList*>(args);
  
  if (!auth.trusted()) return scx::ScriptError::new_ref("Not permitted");

  if ("add" == name) {
    const scx::ScriptString* a_name =
      scx::get_method_arg<scx::ScriptString>(args,0,"name");
    if (!a_name) return scx::ScriptError::new_ref("Profile name must be specified");
    std::string s_name = a_name->get_string();

    if (!add_profile(s_name))
      return scx::ScriptError::new_ref("Profile '" + s_name + "' already exists");

    return new scx::ScriptRef(lookup_profile(s_name));
  }
  
  if ("remove" == name) {
    const scx::ScriptString* a_name =
      scx::get_method_arg<scx::ScriptString>(args,0,"name");
    if (!a_name)
      return scx::ScriptError::new_ref("Profile name must be specified");
    std::string s_name = a_name->get_string();

    if (!remove_profile(s_name))
      return scx::ScriptError::new_ref("Profile '" + s_name + "' does not exist");
    return 0;
  }
  
  if ("submit_build" == name) {
    const scx::ScriptString* a_profile =
      scx::get_method_arg<scx::ScriptString>(args,0,"profile");
    if (!a_profile)
      return scx::ScriptError::new_ref("Profile must be specified");
    std::string s_profile = a_profile->get_string();
    std::string id = submit_build(s_profile);
    if (id.empty())
      return scx::ScriptError::new_ref("Could not submit build");
    return scx::ScriptString::new_ref(id);
  }

  if ("abort_build" == name) {
    const scx::ScriptString* a_build =
      scx::get_method_arg<scx::ScriptString>(args,0,"build");
    if (!a_build)
      return scx::ScriptError::new_ref("Build ID must be specified");
    std::string s_id = a_build->get_string();
    if (!abort_build(s_id))
      return scx::ScriptError::new_ref("Could not abort build");
    return 0;
  }

  if ("remove_build" == name) {
    const scx::ScriptString* a_build =
      scx::get_method_arg<scx::ScriptString>(args,0,"build");
    if (!a_build)
      return scx::ScriptError::new_ref("Build ID must be specified");
    std::string s_id = a_build->get_string();
    if (!remove_build(s_id))
      return scx::ScriptError::new_ref("Could not remove build");
    return 0;
  }

  if ("set_dir" == name) {
    const scx::ScriptString* a_dir =
      scx::get_method_arg<scx::ScriptString>(args,0,"value");
    if (!a_dir)
      return scx::ScriptError::new_ref("Directory must be specified");
    m_dir = a_dir->get_string();
    return 0;
  }

  if ("set_max_running" == name) {
    const scx::ScriptInt* a_max = 
      scx::get_method_arg<scx::ScriptInt>(args,0,"value");
    if (!a_max)
      return scx::ScriptError::new_ref("Value must be specified");
    int i_max = a_max->get_int();
    if (i_max <=0)
      return scx::ScriptError::new_ref("Value must be >= 0");
    m_max_running = i_max;
    return 0;
  }

  if ("set_build_user" == name) {
    const scx::ScriptString* a_user = 
      scx::get_method_arg<scx::ScriptString>(args,0,"value");
    if (!a_user)
      return scx::ScriptError::new_ref("Username must be specified");
    m_build_user = scx::User(a_user->get_string());

    return 0;
  }

  if ("add_source_method" == name) {
    const scx::ScriptString* a_name =
      scx::get_method_arg<scx::ScriptString>(args,0,"method");
    if (!a_name)
      return scx::ScriptError::new_ref("Name must be specified");
    std::string s_name = a_name->get_string();

    const scx::ScriptString* a_desc =
      scx::get_method_arg<scx::ScriptString>(args,1,"description");
    if (!a_desc)
      return scx::ScriptError::new_ref("Description must be specified");
    m_source_methods[s_name] = a_desc->get_string();
    return 0;
  }

  if ("load_profiles" == name) {
    scx::FilePath path = m_dir + "profiles.conf";
    if (!load_config_file(path))
      return scx::ScriptError::new_ref("Error loading profiles");
    return 0;
  }
  
  if ("save_profiles" == name) {
    if (!save_profiles())
      return scx::ScriptError::new_ref("Error saving profiles");
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
  
  return scx::Module::script_method(auth,ref,name,args);
}

//=============================================================================
BuildProfile* TestBuilderModule::lookup_profile(const std::string& name)
{
  ProfileMap::const_iterator it = m_profiles.find(name);
  if (it == m_profiles.end()) {
    return 0;
  }
  return it->second->object();
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
  ProfileMap::iterator it = m_profiles.find(profile);
  if (it == m_profiles.end()) {
    return "";
  }
  
  BuildProfile* profile_obj = it->second->object();
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
  for (BuildList::iterator it = m_builds.begin();
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
  for (BuildList::iterator it = m_builds.begin();
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
  ProfileMap::const_iterator it = m_profiles.find(profile);
  if (it != m_profiles.end()) {
    return false;
  }
   
  m_profiles[profile] = 
    new BuildProfile::Ref(new BuildProfile(*this,profile));
  return true;
}

//=============================================================================
bool TestBuilderModule::remove_profile(const std::string& profile)
{
  ProfileMap::iterator it = m_profiles.find(profile);
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
  for (ProfileMap::const_iterator it = m_profiles.begin();
       it != m_profiles.end();
       it++) {
    BuildProfile* profile = it->second->object();
    profile->save(file);
  }
  return true;
}

//=============================================================================
scx::ScriptRef* TestBuilderModule::get_profiles()
{
  scx::ScriptList::Ref* l1 = 
    new scx::ScriptList::Ref(new scx::ScriptList());
  for (ProfileMap::const_iterator it = m_profiles.begin();
       it != m_profiles.end();
       it++) {
    l1->object()->give( scx::ScriptString::new_ref(it->first) );
  }
  return l1;
}

//=============================================================================
scx::ScriptRef* TestBuilderModule::get_source_methods()
{
  scx::ScriptList::Ref* l1 = 
    new scx::ScriptList::Ref(new scx::ScriptList());
  for (SourceMethodMap::const_iterator it = m_source_methods.begin();
       it != m_source_methods.end();
       it++) {
    scx::ScriptList::Ref* l2 = 
      new scx::ScriptList::Ref(new scx::ScriptList());
    l2->object()->give( scx::ScriptString::new_ref(it->first) );
    l2->object()->give( scx::ScriptString::new_ref(it->second) );
    l1->object()->give(l2);
  }
  return l1;
}

//=============================================================================
scx::ScriptRef* TestBuilderModule::get_builds()
{
  m_builds_mutex.lock();
  std::ostringstream oss;
  for (BuildList::reverse_iterator it = m_builds.rbegin();
       it != m_builds.rend();
       it++) {
    Build* build = (*it);
    oss << build->get_id() << " "
	<< build->get_profile() << " "
	<< Build::get_state_str(build->get_state()) << "\n";
  }
  m_builds_mutex.unlock();
  return scx::ScriptString::new_ref(oss.str());
}

//=============================================================================
scx::ScriptRef* TestBuilderModule::get_buildstats()
{
  m_builds_mutex.lock();
  scx::ScriptList::Ref* l1 = 
    new scx::ScriptList::Ref(new scx::ScriptList());
  for (BuildList::reverse_iterator it = m_builds.rbegin();
       it != m_builds.rend();
       it++) {
    Build* build = (*it);
    
    scx::ScriptList::Ref* l2 = 
      new scx::ScriptList::Ref(new scx::ScriptList());
    l2->object()->give( scx::ScriptString::new_ref(build->get_profile()) );
    l2->object()->give( scx::ScriptString::new_ref(build->get_id()) );
    l2->object()->give( scx::ScriptString::new_ref(
      Build::get_state_str(build->get_state())) );
    
    scx::ScriptList::Ref* l3 = 
      new scx::ScriptList::Ref(new scx::ScriptList());
    for (Build::StepList::const_iterator it_s =
	   build->get_steps().begin();
	 it_s != build->get_steps().end();
	 it_s++) {
      BuildStep* step = (*it_s);
      scx::ScriptList::Ref* l4 = 
	new scx::ScriptList::Ref(new scx::ScriptList());
      Build::State state = step->get_state();
      l4->object()->give( scx::ScriptString::new_ref(step->get_name()) );
      
      if (state == Build::Unstarted) {
	l4->object()->give( scx::ScriptString::new_ref("") );
      } else {
	l4->object()->give( 
	  scx::ScriptString::new_ref(step->get_start_time().code()) );
      }
      
      if (state == Build::Unstarted || state == Build::Aborted) {
	l4->object()->give( scx::ScriptString::new_ref("") );
      } else if (state == Build::Running) {
	scx::Time t = scx::Date::now() - step->get_start_time();
	l4->object()->give( scx::ScriptString::new_ref(t.string() + "+") );
      } else {
	scx::Time t = step->get_finish_time() - step->get_start_time();
	l4->object()->give( scx::ScriptString::new_ref(t.string()) );
      }
      
      l4->object()->give( 
	scx::ScriptString::new_ref(Build::get_state_str(state)) );
      l3->object()->give(l4);
    }
    l2->object()->give(l3);
    l1->object()->give(l2);
  }
  m_builds_mutex.unlock();
  return l1;
}
