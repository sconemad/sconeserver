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
#include <sconex/Uri.h>
#include <sconex/LineBuffer.h>
#include <sconex/User.h>
#include <sconex/File.h>
#include <sconex/ScriptTypes.h>

namespace http {

//=========================================================================
HTTPUser::HTTPUser(HTTPModule& module,
		   const std::string& username,
		   const std::string& password,
		   const scx::FilePath& path)
  : m_module(module),
    m_username(username),
    m_password(password)
{
  DEBUG_COUNT_CONSTRUCTOR(HTTPUser);
  m_parent = &m_module;
  //  load();
}

//=========================================================================
HTTPUser::~HTTPUser()
{
  //  save();
  DEBUG_COUNT_DESTRUCTOR(HTTPUser);
}

//=========================================================================
bool HTTPUser::authorised(const std::string& password)
{
  if (m_password == "!system") {
    return scx::User(m_username).verify_password(password);
  }  
    
  struct crypt_data data;
  memset(&data,0,sizeof(data));
  data.initialized = 0;
  std::string check = crypt_r(password.c_str(),
			      m_password.c_str(),
			      &data);

  // could use this if crypt_r not available:
  //  std::string check = crypt(password.c_str(), m_password.c_str());

  return (check == m_password);
}

//=========================================================================
std::string HTTPUser::get_string() const
{
  //  return m_username;
  return scx::ScriptMap::get_string();
}

//=========================================================================
scx::ScriptRef* HTTPUser::script_op(const scx::ScriptAuth& auth,
				    const scx::ScriptRef& ref,
				    const scx::ScriptOp& op,
				    const scx::ScriptRef* right)
{
  return scx::ScriptMap::script_op(auth,ref,op,right);
}

//=========================================================================
scx::ScriptRef* HTTPUser::script_method(const scx::ScriptAuth& auth,
					const scx::ScriptRef& ref,
					const std::string& name,
					const scx::ScriptRef* args)
{
  return scx::ScriptMap::script_method(auth,ref,name,args);
}


//=========================================================================
AuthRealm::AuthRealm(
  HTTPModule& module,
  const std::string name,
  const scx::FilePath& path
) : m_module(module),
    m_name(name),
    m_path(path)
{
  DEBUG_COUNT_CONSTRUCTOR(AuthRealm);
  m_parent = &m_module;
}

//=========================================================================
AuthRealm::~AuthRealm()
{
  for (UserMap::const_iterator it = m_users.begin();
       it != m_users.end();
       ++it) {
    delete it->second;
  }
  DEBUG_COUNT_DESTRUCTOR(AuthRealm);
}

//=========================================================================
HTTPUser* AuthRealm::lookup_user(const std::string& username)
{
  UserMap::iterator it = m_users.find(username);
  if (it != m_users.end()) {
    return it->second->object();
  }
  return 0;
}

//=========================================================================
bool AuthRealm::authorised(const std::string& username,
			   const std::string& password)
{
  refresh();
  scx::MutexLocker locker(m_mutex);
  UserMap::const_iterator it = m_users.find(username);
  if (it != m_users.end()) {
    HTTPUser* user = it->second->object();
    return user->authorised(password);
  }
  return false;
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
    if (!a_user) {
      return scx::ScriptError::new_ref("auth() No username specified");
    }
    std::string s_user = a_user->get_string();

    const scx::ScriptString* a_pass =
      scx::get_method_arg<scx::ScriptString>(args,1,"password");
    if (!a_pass) {
      return scx::ScriptError::new_ref("auth() No password specified");
    }
    std::string s_pass = a_pass->get_string();

    refresh();

    scx::MutexLocker locker(m_mutex);
    UserMap::const_iterator it = m_users.find(s_user);
    if (it == m_users.end()) {
      return scx::ScriptError::new_ref("auth() User does not exist");
    }
    
    HTTPUser::Ref* user = it->second;
    if (!user->object()->authorised(s_pass)) {
      return scx::ScriptError::new_ref("auth() Invalid password");
    }
    
    return user->ref_copy(ref.reftype());
  }

  return scx::ScriptObject::script_method(auth,ref,name,args);
}

//=========================================================================
void AuthRealm::refresh()
{
  scx::FilePath path(m_path + "htpasswd");
  scx::FileStat stat(path);
  if (stat.is_file()) {
    if (m_modtime != stat.time()) {
      scx::MutexLocker locker(m_mutex);

      m_users.clear();
      scx::File file;
      if (scx::Ok == file.open(path,scx::File::Read)) {
	scx::LineBuffer* tok = new scx::LineBuffer("");
	file.add_stream(tok);
	std::string line;
	while (scx::Ok == tok->tokenize(line)) {
	  std::string::size_type i = line.find_first_of(":");
	  std::string username;
	  std::string password;
	  username = line.substr(0,i);
          ++i;
	  password = line.substr(i);
	  if (!username.empty() && !password.empty()) {
	    scx::FilePath path(m_path + username);
	    HTTPUser* user = new HTTPUser(m_module,username,password,path);
	    m_users[username] = new HTTPUser::Ref(user);
	  }
	}
	file.close();
	m_modtime = stat.time();
      }
    }
  }
}


//=========================================================================
AuthRealmManager::AuthRealmManager(
  HTTPModule& module
) : m_module(module)
{
  m_parent = &m_module;
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
    if (!a_realm) {
      return scx::ScriptError::new_ref("add() No realm specified");
    }
    std::string s_realm = a_realm->get_string();

    AuthRealmMap::const_iterator it = m_realms.find(s_realm);
    if (it != m_realms.end()) {
      return scx::ScriptError::new_ref("add() Realm already exists");
    }

    const scx::ScriptString* a_path =
      scx::get_method_arg<scx::ScriptString>(args,1,"path");
    if (!a_path) {
      return scx::ScriptError::new_ref("add() No path specified");
    }
    std::string s_path = a_path->get_string();
    
    log("Adding realm '" + s_realm + "'");
    AuthRealm* realm = new AuthRealm(m_module,s_realm,s_path);
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

};
