/* SconeServer (http://www.sconemad.com)

SQL Database Module

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


#include "DbSqlModule.h"
#include "DbSqlProfile.h"

namespace dbsql {

SCONESERVER_MODULE(DbSqlModule);

//=========================================================================
DbSqlModule::DbSqlModule(
) : scx::Module("dbsql",scx::version())
{

}

//=========================================================================
DbSqlModule::~DbSqlModule()
{
  for (DbSqlProfileMap::const_iterator it = m_profiles.begin();
       it != m_profiles.end();
       ++it) {
    delete it->second;
  }
}

//=========================================================================
std::string DbSqlModule::info() const
{
  return "SQL Database interface";
}

//=========================================================================
int DbSqlModule::init()
{
  return Module::init();
}

//=============================================================================
scx::Arg* DbSqlModule::arg_lookup(const std::string& name)
{
  // Methods

  if ("add" == name) {
    return new_method(name);
  }

  // Sub-objects

  DbSqlProfile* profile = lookup_profile(name);
  if (profile) {
    return new scx::ArgObject(profile);
  }

  return SCXBASE Module::arg_lookup(name);
}

//=============================================================================
scx::Arg* DbSqlModule::arg_method(
  const scx::Auth& auth,
  const std::string& name,
  scx::Arg* args
)
{
  scx::ArgList* l = dynamic_cast<scx::ArgList*>(args);

  if (!auth.admin()) return new scx::ArgError("Not permitted");

  if (name == "add") {
    const scx::ArgString* a_profile = dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_profile) return new scx::ArgError("add() No profile name specified");
    std::string s_profile = a_profile->get_string();
    
    const scx::ArgString* a_db = dynamic_cast<const scx::ArgString*>(l->get(1));
    if (!a_db) return new scx::ArgError("add() No database specified");

    const scx::ArgString* a_user = dynamic_cast<const scx::ArgString*>(l->get(2));
    if (!a_user) return new scx::ArgError("add() No username specified");

    const scx::ArgString* a_pass = dynamic_cast<const scx::ArgString*>(l->get(3));
    if (!a_pass) return new scx::ArgError("add() No password specified");

    // Check profile doesn't already exist
    DbSqlProfileMap::const_iterator it = m_profiles.find(s_profile);
    if (it != m_profiles.end()) return new scx::ArgError("add() Profile already exists");
        
    DbSqlProfile* profile = 0;
    try {
      profile = new DbSqlProfile(*this,
				 s_profile,
				 a_db->get_string(),
				 a_user->get_string(),
				 a_pass->get_string());
    } catch (...) {
      log("Failed to connect profile '" + s_profile + "'");
    }

    if (profile) {
      m_profiles[s_profile] = profile;
      log("Adding profile '" + s_profile + 
	  "' db '" + a_db->get_string() + 
	  "' user '" + a_user->get_string() + "'");
    }
    return 0;
  }

  return SCXBASE Module::arg_method(auth,name,args);
}

//=========================================================================
bool DbSqlModule::connect(
  scx::Descriptor* endpoint,
  scx::ArgList* args
)
{
  return true;
}

//=========================================================================
DbSqlProfile* DbSqlModule::lookup_profile(const std::string& profile)
{
  DbSqlProfileMap::const_iterator it = m_profiles.find(profile);
  if (it != m_profiles.end()) {
    return it->second;
  }

  return 0;
}

};
