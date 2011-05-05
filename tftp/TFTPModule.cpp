/* SconeServer (http://www.sconemad.com)

Trivial File Transfer Protocol (TFTP) module

See RFC1350

Copyright (c) 2000-2004 Andrew Wedgbury <wedge@sconemad.com>

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


#include "TFTPModule.h"
#include "TFTPStream.h"
#include "TFTPProfile.h"

#include "sconex/ModuleInterface.h"
#include "sconex/ScriptTypes.h"

SCONESERVER_MODULE(TFTPModule);

//=========================================================================
TFTPModule::TFTPModule()
  : scx::Module("tftp",scx::version())
{
  scx::Stream::register_stream("tftp",this);
}

//=========================================================================
TFTPModule::~TFTPModule()
{  
  scx::Stream::unregister_stream("tftp",this);

  for (ProfileMap::const_iterator it = m_profiles.begin();
       it != m_profiles.end();
       it++) {
    delete it->second;
  }
}

//=========================================================================
std::string TFTPModule::info() const
{
  return "Trivial File Transfer Protocol (TFTP)";
}

//=========================================================================
int TFTPModule::init()
{
  return Module::init();
}

//=========================================================================
TFTPProfile* TFTPModule::find_profile(const std::string& name)
{
  ProfileMap::const_iterator it = m_profiles.find(name);
  
  if (it != m_profiles.end()) {
    return it->second->object();
  }
  
  return 0;
}

//=============================================================================
scx::ScriptRef* TFTPModule::script_op(const scx::ScriptAuth& auth,
				       const scx::ScriptRef& ref,
				       const scx::ScriptOp& op,
				       const scx::ScriptRef* right)
{
  if (op.type() == scx::ScriptOp::Lookup) {
    const std::string name = right->object()->get_string();

    // Methods
    if ("add" == name ||
	"remove" == name) {
      return new scx::ScriptMethodRef(ref,name);
    }      

    // Properties
    if ("list" == name) {
      scx::ScriptList* list = new scx::ScriptList();
      for (ProfileMap::const_iterator it = m_profiles.begin();
	   it != m_profiles.end();
	   ++it) {
	list->give(scx::ScriptString::new_ref((*it).first));
      }
      return new scx::ScriptRef(list);
    }

    // Sub-objects
    TFTPProfile* profile = find_profile(name);
    if (profile) {
      return new scx::ScriptRef(profile,ref.reftype());
    }
  }

  return scx::Module::script_op(auth,ref,op,right);
}

//=============================================================================
scx::ScriptRef* TFTPModule::script_method(const scx::ScriptAuth& auth,
					  const scx::ScriptRef& ref,
					  const std::string& name,
					  const scx::ScriptRef* args)
{
  if (!auth.admin()) return scx::ScriptError::new_ref("Not permitted");

  if ("add" == name) {
    const scx::ScriptString* a_name = 
      scx::get_method_arg<scx::ScriptString>(args,0,"name");
    if (!a_name) 
      return scx::ScriptError::new_ref("Name must be specified");
    std::string s_name = a_name->get_string();

    const scx::ScriptString* a_path = 
      scx::get_method_arg<scx::ScriptString>(args,1,"path");
    if (!a_path) 
      return scx::ScriptError::new_ref("Path must be specified");
    scx::FilePath p_path = scx::FilePath(a_path->get_string());

    // Check profile doesn't already exist
    if (find_profile(s_name))
      return scx::ScriptError::new_ref("Profile '" + s_name + 
				       "' already exists");

    TFTPProfile* profile = new TFTPProfile(*this,s_name,p_path);
    m_profiles[s_name] = new TFTPProfile::Ref(profile);
    return new TFTPProfile::Ref(profile);
  }
  
  if ("remove" == name) {
    const scx::ScriptString* a_name = 
      scx::get_method_arg<scx::ScriptString>(args,0,"name");
    if (!a_name) 
      return scx::ScriptError::new_ref("Name must be specified");
    std::string s_name = a_name->get_string();

    // Remove profile
    ProfileMap::iterator it = m_profiles.find(s_name);
    if (it == m_profiles.end())
      return scx::ScriptError::new_ref("Profile '" + s_name + 
				       "' does not exist");
    delete it->second;
    m_profiles.erase(it);
    return 0;
  }

  return scx::Module::script_method(auth,ref,name,args);
}

//=========================================================================
void TFTPModule::provide(const std::string& type,
			 const scx::ScriptRef* args,
			 scx::Stream*& object)
{
  const scx::ScriptString* profile =
    scx::get_method_arg<scx::ScriptString>(args,0,"profile");
  if (!profile) {
    log("No TFTP profile specified, aborting connection");
    return;
  }

  object = new TFTPStream(*this,profile->get_string());
  object->add_module_ref(this);
}

