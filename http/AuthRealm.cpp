/* SconeServer (http://www.sconemad.com)

HTTP Authentication

Copyright (c) 2000-2013 Andrew Wedgbury <wedge@sconemad.com>

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
#include <sconex/Log.h>

namespace http {

#define LOG(msg) scx::Log("http").submit(msg);

//=============================================================================
AuthRealm::AuthRealm(HTTPModule* module)
  : m_module(module), 
    m_hash(scx::PasswordHash::create("",0))
{
  DEBUG_COUNT_CONSTRUCTOR(AuthRealm);
  m_parent = module;
}

//=============================================================================
AuthRealm::~AuthRealm()
{
  DEBUG_COUNT_DESTRUCTOR(AuthRealm);
}

//=============================================================================
void AuthRealm::set_name(const std::string& name)
{
  m_name = name;
}

//=============================================================================
scx::ScriptRef* AuthRealm::authenticate(const std::string& username,
					const std::string& password)
{
  std::string hash = lookup_hash(username);
  if (hash.empty()) return 0;
  
  bool rehash = false;
  if (!m_hash.object()->verify(password, hash, rehash)) return 0;
  
  scx::ScriptRef* data = lookup_data(username);
  if (data == 0) {
    // If this realm doesn't implement user data, then simply set the data
    // to the username string, this should keep everyone happy.
    data = scx::ScriptString::new_ref(username);
  }

  if (rehash) {
    hash = m_hash.object()->rehash(password);
    if (!hash.empty() && update_hash(username,hash)) {
      LOG("Re-hashed password for user " + username + 
	  " using " + m_hash.object()->get_string());
    }
  }

  return data;
}

//=============================================================================
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

    // Properties
    if ("hash" == name) return m_hash.ref_copy();

    // Methods
    if ("auth" == name ||
	"set_hash" == name ||
	"chpass" == name) {
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
      return scx::ScriptError::new_ref("No username specified");

    const scx::ScriptString* a_pass =
      scx::get_method_arg<scx::ScriptString>(args,1,"password");
    if (!a_pass)
      return scx::ScriptError::new_ref("No password specified");

    return authenticate(a_user->get_string(),a_pass->get_string());
  }

  if ("set_hash" == name) {
    if (!auth.admin()) return scx::ScriptError::new_ref("Not permitted");

    const scx::ScriptString* a_hash =
      scx::get_method_arg<scx::ScriptString>(args,0,"hash");
    if (!a_hash)
      return scx::ScriptError::new_ref("No hash specified");

    scx::PasswordHash* hash =
      scx::PasswordHash::create(a_hash->get_string(),0);

    if (!hash)
      return scx::ScriptError::new_ref("Unknown hash");
    
    m_hash = scx::PasswordHash::Ref(hash);
    return 0;
  }

  if ("chpass" == name) {
    if (!auth.trusted()) return scx::ScriptError::new_ref("Not permitted");

    const scx::ScriptString* a_user =
      scx::get_method_arg<scx::ScriptString>(args,0,"username");
    if (!a_user)
      return scx::ScriptError::new_ref("No username specified");

    const scx::ScriptString* a_pass =
      scx::get_method_arg<scx::ScriptString>(args,1,"password");
    if (!a_pass)
      return scx::ScriptError::new_ref("No password specified");

    const scx::ScriptString* a_oldpass =
      scx::get_method_arg<scx::ScriptString>(args,2,"old_password");
    if (a_oldpass) {
      // Check the old password if specified
      std::string oldhash = lookup_hash(a_user->get_string());
      if (oldhash.empty())
	return scx::ScriptError::new_ref("Invalid credentials");
      bool rehash = false;
      if (!m_hash.object()->verify(a_oldpass->get_string(), 
				   oldhash, rehash))
	return scx::ScriptError::new_ref("Invalid credentials");

    } else if (!auth.admin()) {
      // Only admin is allowed to change passwords without specifying the
      // old password
      return scx::ScriptError::new_ref("No old password specified");
    }

    // Rehash the new password and update
    std::string hash = m_hash.object()->rehash(a_pass->get_string());
    if (hash.empty()) 
      return scx::ScriptError::new_ref("Failed to hash password");

    if (!update_hash(a_user->get_string(),hash)) 
      return scx::ScriptError::new_ref("Failed to update password");

    return 0;
  }

  return scx::ScriptObject::script_method(auth,ref,name,args);
}

//=============================================================================
bool AuthRealm::update_hash(const std::string& username,
			    const std::string& hash)
{
  return false;
}

//=============================================================================
scx::ScriptRef* AuthRealm::lookup_data(const std::string& username)
{
  return 0;
}


//=============================================================================
AuthRealmManager::AuthRealmManager(HTTPModule* module) 
  : m_module(module)
{
  m_parent = m_module;

  // Register standard realm types
  register_realm_type("htpasswd",this);
  register_realm_type("db",this);
}

//=============================================================================
AuthRealmManager::~AuthRealmManager()
{
  for (AuthRealmMap::const_iterator it = m_realms.begin();
       it != m_realms.end();
       ++it) {
    delete it->second;
  }
}

//=============================================================================
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
    AuthRealm* realm = m_realm_types.provide(a_type->get_string(), a_args);
    if (!realm)
      return scx::ScriptError::new_ref("Could not create realm");

    LOG("Adding realm '" + s_realm + "'");
    realm->set_name(s_realm);
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
    
    LOG("Removing realm '" + s_realm + "'");
    delete (*it).second;
    m_realms.erase(it);
    
    return 0;
  }

  return scx::ScriptObject::script_method(auth,ref,name,args);
}

//=============================================================================
void AuthRealmManager::register_realm_type(const std::string& type,
					   scx::Provider<AuthRealm>* factory)
{
  m_realm_types.register_provider(type,factory);
}

//=============================================================================
void AuthRealmManager::unregister_realm_type(const std::string& type,
					     scx::Provider<AuthRealm>* factory)
{
  m_realm_types.unregister_provider(type,factory);
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
      LOG("No htpasswd file specified");
      return;
    }
 
    object = new AuthRealmHtpasswd(m_module,a_path->get_string());

  } else if (type == "db") {
    object = new AuthRealmDB(m_module,args);
  }
}


};
