/* SconeServer (http://www.sconemad.com)

MySQL Database Module

Copyright (c) 2000-2010 Andrew Wedgbury <wedge@sconemad.com>

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


#include "MySqlModule.h"
#include "MySqlProfile.h"
#include "MySqlQuery.h"
#include <sconex/ScriptTypes.h>

namespace mysql {

SCONEX_MODULE(MySqlModule);

//=========================================================================
MySqlModule::MySqlModule(
) : scx::Module("mysql",scx::version())
{
  scx::Database::register_provider("MySQL",this);
}

//=========================================================================
MySqlModule::~MySqlModule()
{
  scx::Database::unregister_provider("MySQL",this);
}

//=========================================================================
std::string MySqlModule::info() const
{
  return "SQL Database interface";
}

//=========================================================================
int MySqlModule::init()
{
  return Module::init();
}

//=========================================================================
bool MySqlModule::close()
{
  if (!scx::Module::close()) return false;
  
  for (MySqlProfileMap::const_iterator it = m_profiles.begin();
       it != m_profiles.end();
       ++it) {
    delete it->second;
  }
  m_profiles.clear();
  return true;
}

//=========================================================================
MySqlProfile* MySqlModule::lookup_profile(const std::string& profile)
{
  MySqlProfileMap::const_iterator it = m_profiles.find(profile);
  if (it != m_profiles.end()) {
    return it->second->object();
  }

  return 0;
}

//=========================================================================
MySqlQuery* MySqlModule::new_query(const std::string& profile,
				   const std::string& query)
{
  MySqlProfile* p = lookup_profile(profile);
  if (!p) return 0;

  return new MySqlQuery(p,query);
}

//=============================================================================
scx::ScriptRef* MySqlModule::script_op(const scx::ScriptAuth& auth,
				       const scx::ScriptRef& ref,
				       const scx::ScriptOp& op,
				       const scx::ScriptRef* right)
{
  if (op.type() == scx::ScriptOp::Lookup) {
    const std::string name = right->object()->get_string();
    
    // Methods
    if ("add" == name) {
      return new scx::ScriptMethodRef(ref,name);
    }

    // Sub-objects
    MySqlProfile* profile = lookup_profile(name);
    if (profile) {
      return new MySqlProfile::Ref(profile);
    }
  }

  return scx::Module::script_op(auth,ref,op,right);
}

//=============================================================================
scx::ScriptRef* MySqlModule::script_method(const scx::ScriptAuth& auth,
					   const scx::ScriptRef& ref,
					   const std::string& name,
					   const scx::ScriptRef* args)
{
  if (name == "add") {
    if (!auth.admin()) return scx::ScriptError::new_ref("Not permitted");

    const scx::ScriptString* a_profile = 
      scx::get_method_arg<scx::ScriptString>(args,0,"name");
    if (!a_profile) 
      return scx::ScriptError::new_ref("add() No profile name specified");
    std::string s_profile = a_profile->get_string();
    
    // Check profile doesn't already exist
    MySqlProfileMap::const_iterator it = m_profiles.find(s_profile);
    if (it != m_profiles.end()) 
      return scx::ScriptError::new_ref("add() Profile already exists");
        
    scx::Database::Ref* profile = 0;
    provide("MySQL",args,profile);
    if (!profile)
      return scx::ScriptError::new_ref("add() Invalid database parameters");

    return profile;
  }

  return scx::Module::script_method(auth,ref,name,args);
}

//=========================================================================
void MySqlModule::provide(const std::string& type,
			  const scx::ScriptRef* args,
			  scx::Database::Ref*& object)
{
  const scx::ScriptString* a_profile = 
    scx::get_method_arg<scx::ScriptString>(args,0,"profile");
  if (!a_profile) {
    DEBUG_LOG("No profile specified");
    return;
  }
  std::string s_profile = a_profile->get_string();

  // Look for the specified profile
  MySqlProfile* profile = lookup_profile(s_profile);

  if (!profile) {
    // Does not match an existing profile, create a new one
    
    const scx::ScriptString* a_db =
      scx::get_method_arg<scx::ScriptString>(args,1,"database");
    if (!a_db) {
      DEBUG_LOG("No database specified");
      return;
    }
    
    const scx::ScriptString* a_user =
      scx::get_method_arg<scx::ScriptString>(args,2,"username");
    if (!a_user) {
      DEBUG_LOG("No username specified");
      return;
    }
    
    const scx::ScriptString* a_pass =
      scx::get_method_arg<scx::ScriptString>(args,3,"password");
    if (!a_pass) {
      DEBUG_LOG("No password specified");
      return;
    }

    profile = new MySqlProfile(this,
			       s_profile,
			       a_db->get_string(),
			       a_user->get_string(),
			       a_pass->get_string());

    m_profiles[s_profile] = new MySqlProfile::Ref(profile);
    log("Adding profile '" + s_profile + 
	"' db '" + a_db->get_string() + 
	"' user '" + a_user->get_string() + "'");
  }

  object = new scx::Database::Ref(profile);
}

  
};
