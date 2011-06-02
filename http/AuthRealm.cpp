/* SconeServer (http://www.sconemad.com)

HTTP Authorisation

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


#include <http/AuthRealm.h>
#include <http/HTTPModule.h>
#include <http/MessageStream.h>
#include <http/Request.h>
#include <http/AuthRealmHtpasswd.h>
#include <http/AuthRealmDB.h>

#include <sconex/ScriptTypes.h>

namespace http {

//=========================================================================
AuthRealm::AuthRealm(const std::string name)
  : m_name(name)
{
  DEBUG_COUNT_CONSTRUCTOR(AuthRealm);
}

//=========================================================================
AuthRealm::~AuthRealm()
{
  DEBUG_COUNT_DESTRUCTOR(AuthRealm);
}

//=========================================================================
std::string AuthRealm::get_string() const
{
  return m_name;
}

//=============================================================================
scx::ScriptRef* AuthRealm::script_op(const scx::ScriptAuth& auth,
				     const scx::ScriptRef& ref,
				     const scx::ScriptOp& op,
				     const scx::ScriptRef* right)
{
  if (op.type() == scx::ScriptOp::Lookup) {
    const std::string name = right->object()->get_string();

    // Methods
    if ("auth" == name) {
      return new scx::ScriptMethodRef(ref,name);
    }
  }

  return scx::ScriptObject::script_op(auth,ref,op,right);
}

//=============================================================================
scx::ScriptRef* AuthRealm::script_method(const scx::ScriptAuth& auth,
					 const scx::ScriptRef& ref,
					 const std::string& name,
					 const scx::ScriptRef* args)
{
  if ("auth" == name) {
    if (!auth.trusted()) return scx::ScriptError::new_ref("Not permitted");

    const scx::ScriptString* a_user =
      scx::get_method_arg<scx::ScriptString>(args,0,"username");
    if (!a_user)
      return scx::ScriptError::new_ref("auth() No username specified");

    const scx::ScriptString* a_pass =
      scx::get_method_arg<scx::ScriptString>(args,1,"password");
    if (!a_pass)
      return scx::ScriptError::new_ref("auth() No password specified");

    return authorised(a_user->get_string(),a_pass->get_string());
  }

  return scx::ScriptObject::script_method(auth,ref,name,args);
}



//=========================================================================
AuthRealmManager::AuthRealmManager(HTTPModule* module) 
  : m_module(module)
{
  m_parent = m_module;

  // Register standard realm types
  register_realm("htpasswd",this);
  register_realm("db",this);
}

//=========================================================================
AuthRealmManager::~AuthRealmManager()
{
  for (AuthRealmMap::const_iterator it = m_realms.begin();
       it != m_realms.end();
       ++it) {
    delete it->second;
  }
}

//=========================================================================
AuthRealm* AuthRealmManager::lookup_realm(const std::string& name)
{
  AuthRealmMap::const_iterator it = m_realms.find(name);
  if (it != m_realms.end()) {
    return it->second->object();
  }
  return 0;
}

//=============================================================================
scx::ScriptRef* AuthRealmManager::script_op(const scx::ScriptAuth& auth,
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
    
    // Sub-objects
    if ("list" == name) {
      scx::ScriptList* list = new scx::ScriptList();
      scx::ScriptRef* list_ref = new scx::ScriptRef(list);
      for (AuthRealmMap::const_iterator it = m_realms.begin();
	   it != m_realms.end();
	   ++it) {
	list->give(it->second->ref_copy(ref.reftype()));
      }
      return list_ref;
    }
    
    AuthRealmMap::const_iterator it = m_realms.find(name);
    if (it != m_realms.end()) {
      AuthRealm::Ref* r = it->second;
      return r->ref_copy(ref.reftype());
    }
  }
  
  return scx::ScriptObject::script_op(auth,ref,op,right);
}

//=============================================================================
scx::ScriptRef* AuthRealmManager::script_method(const scx::ScriptAuth& auth,
						const scx::ScriptRef& ref,
						const std::string& name,
						const scx::ScriptRef* args)
{
  if (!auth.admin()) return scx::ScriptError::new_ref("Not permitted");

  if ("add" == name) {
    const scx::ScriptString* a_realm =
      scx::get_method_arg<scx::ScriptString>(args,0,"name");
    if (!a_realm)
      return scx::ScriptError::new_ref("No realm specified");
    std::string s_realm = a_realm->get_string();

    AuthRealmMap::const_iterator it = m_realms.find(s_realm);
    if (it != m_realms.end())
      return scx::ScriptError::new_ref("Realm already exists");

    const scx::ScriptString* a_type =
      scx::get_method_arg<scx::ScriptString>(args,1,"type");
    if (!a_type)
      return scx::ScriptError::new_ref("No realm type specified");

    const scx::ScriptRef* a_args = scx::get_method_arg_ref(args,2,"args");
    if (!a_args)
      return scx::ScriptError::new_ref("No realm args specified");

    // Create the realm using the provider scheme
    AuthRealm* realm = m_providers.provide(a_type->get_string(), a_args);

    log("Adding realm '" + s_realm + "'");
    m_realms[s_realm] = new AuthRealm::Ref(realm);

    return new AuthRealm::Ref(realm);
  }

  if ("remove" == name) {
    const scx::ScriptString* a_realm =
      scx::get_method_arg<scx::ScriptString>(args,0,"name");
    if (!a_realm) {
      return scx::ScriptError::new_ref("add_realm() No realm specified");
    }
    std::string s_realm = a_realm->get_string();

    AuthRealmMap::iterator it = m_realms.find(s_realm);
    if (it == m_realms.end()) {
      return scx::ScriptError::new_ref("add_realm() Realm does not exist");
    }
    
    log("Removing realm '" + s_realm + "'");
    delete (*it).second;
    m_realms.erase(it);
    
    return 0;
  }

  return scx::ScriptObject::script_method(auth,ref,name,args);
}

//=============================================================================
void AuthRealmManager::register_realm(const std::string& type,
				      scx::Provider<AuthRealm>* factory)
{
  m_providers.register_provider(type,factory);
}

//=============================================================================
void AuthRealmManager::unregister_realm(const std::string& type,
					scx::Provider<AuthRealm>* factory)
{
  m_providers.unregister_provider(type,factory);
}

//=============================================================================
void AuthRealmManager::provide(const std::string& type,
			       const scx::ScriptRef* args,
			       AuthRealm*& object)
{
  if (type == "htpasswd") {
    const scx::ScriptString* a_path =
      scx::get_method_arg<scx::ScriptString>(args,0,"file");
    if (!a_path) {
      DEBUG_LOG("No htpasswd file specified");
      return;
    }
 
    object = new AuthRealmHtpasswd(m_module,a_path->get_string());

  } else if (type == "db") {
    object = new AuthRealmDB(m_module,args);
  }
}


};
