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
#include "BuildProfile.h"
#include "Build.h"

#include "sconex/ModuleInterface.h"
#include "sconex/Module.h"
#include "sconex/Arg.h"
#include "sconex/Stream.h"
#include "sconex/TimeDate.h"

SCONESERVER_MODULE(TestBuilderModule);

//=========================================================================
TestBuilderModule::TestBuilderModule(
)
  : scx::Module("testbuilder",scx::version())
{
  start();
}

//=========================================================================
TestBuilderModule::~TestBuilderModule()
{
  //  stop();
  
  for (std::map<std::string,BuildProfile*>::iterator it = 
	 m_profiles.begin();
       it != m_profiles.end();
       it++) {
    delete (*it).second;
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
  //  StatStream* s = new StatStream(*this,channel->get_string());
  //  s->add_module_ref(ref());
  
  //  endpoint->add_stream(s);
  //  return true;
  return false;
}

//=============================================================================
void* TestBuilderModule::run()
{
  while (true) {
    sleep(10);

    DEBUG_LOG("HELLO FROM THE THREAD");

  }
  
  return 0;
}

//=============================================================================
scx::Arg* TestBuilderModule::arg_lookup(const std::string& name)
{
  // Methods

  if ("add" == name ||
      "remove" == name) {
    return new scx::ArgObjectFunction(
      new scx::ArgModule(ref()),name);
  }      

  // Properties
  
  if ("list" == name) {
    std::ostringstream oss;
    for (std::map<std::string,BuildProfile*>::const_iterator it = 
	   m_profiles.begin();
	 it != m_profiles.end();
	 it++) {
      oss << (*it).first << "\n";
    }
    return new scx::ArgString(oss.str());
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

    // Check profile doesn't already exist
    std::map<std::string,BuildProfile*>::const_iterator it = 
      m_profiles.find(name);
    if (it != m_profiles.end()) {
      return new scx::ArgError("testbuilder::add() Profile '" + s_name +
                               "' already exists");
    }
   
    m_profiles[s_name] = new BuildProfile(s_name);
    return 0;
  }
  
  if ("remove" == name) {
    const scx::ArgString* a_name =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_name) {
      return new scx::ArgError("stat::remove() Name must be specified");
    }
    std::string s_name = a_name->get_string();

    // Remove channel
    std::map<std::string,BuildProfile*>::iterator it = 
      m_profiles.find(name);
    if (it == m_profiles.end()) {
      return new scx::ArgError("testbuilder::remove() Profile '" + s_name +
                               "' does not exist");
    }

    delete (*it).second;
    m_profiles.erase(it);

    return 0;
  }

  return SCXBASE Module::arg_function(name,args);
}
