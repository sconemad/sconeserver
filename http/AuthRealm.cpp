/* SconeServer (http://www.sconemad.com)

http Authorisation Realm

Copyright (c) 2000-2009 Andrew Wedgbury <wedge@sconemad.com>

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


#include "http/AuthRealm.h"
#include "http/HTTPModule.h"
#include "http/MessageStream.h"
#include "http/Request.h"
#include "sconex/Uri.h"
#include "sconex/LineBuffer.h"
namespace http {

//=========================================================================
AuthRealm::AuthRealm(
  HTTPModule& module,
  const std::string name,
  const scx::FilePath& path
) : m_module(module),
    m_name(name),
    m_path(path)
{

}

//=========================================================================
AuthRealm::~AuthRealm()
{

}

//=========================================================================
const std::string& AuthRealm::get_realm() const
{
  return m_name;
}

//=========================================================================
bool AuthRealm::authorised(const std::string& user, const std::string& pass)
{
  refresh();
  scx::MutexLocker locker(m_mutex);
  UserMap::const_iterator it = m_users.find(user);
  if (it != m_users.end()) {
    return (pass == it->second);
  }
  return false;
}

//=========================================================================
std::string AuthRealm::name() const
{
  std::ostringstream oss;
  oss << "AUTHREALM:" << get_realm();
  return oss.str();
}

//=============================================================================
scx::Arg* AuthRealm::arg_resolve(const std::string& name)
{
  scx::Arg* a = SCXBASE ArgObjectInterface::arg_resolve(name);
  if (a==0 || (dynamic_cast<scx::ArgError*>(a)!=0)) {
    delete a;
    return m_module.arg_resolve(name);
  }
  return a;
}

//=============================================================================
scx::Arg* AuthRealm::arg_lookup(const std::string& name)
{
  // Methods
  if ("add" == name ||
      "auth" == name) {
    return new scx::ArgObjectFunction(new scx::ArgObject(this),name);
  }

  return SCXBASE ArgObjectInterface::arg_lookup(name);
}

//=============================================================================
scx::Arg* AuthRealm::arg_function(
  const scx::Auth& auth,
  const std::string& name,
  scx::Arg* args
)
{
  scx::ArgList* l = dynamic_cast<scx::ArgList*>(args);

  if ("add" == name) {
    if (!auth.admin()) return new scx::ArgError("Not permitted");
    const scx::ArgString* a_user =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_user) {
      return new scx::ArgError("add() No user specified");
    }
    std::string s_user = a_user->get_string();

    const scx::ArgString* a_pass =
      dynamic_cast<const scx::ArgString*>(l->get(1));
    if (!a_pass) {
      return new scx::ArgError("add() No pass specified");
    }
    std::string s_pass = a_pass->get_string();

    log("Adding user '" + s_user + "'");
    m_mutex.lock();
    m_users[s_user] = s_pass;
    m_mutex.unlock();

    return 0;
  }

  if ("auth" == name) {
    if (!auth.trusted()) return new scx::ArgError("Not permitted");
    const scx::ArgString* a_user =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_user) {
      return new scx::ArgError("add() No user specified");
    }
    std::string s_user = a_user->get_string();

    const scx::ArgString* a_pass =
      dynamic_cast<const scx::ArgString*>(l->get(1));
    if (!a_pass) {
      return new scx::ArgError("add() No pass specified");
    }
    std::string s_pass = a_pass->get_string();

    return new scx::ArgInt(authorised(s_user,s_pass));
  }

  return SCXBASE ArgObjectInterface::arg_function(auth,name,args);
}

//=========================================================================
void AuthRealm::refresh()
{
  scx::FileStat stat(m_path);
  if (stat.is_file()) {
    if (m_modtime != stat.time()) {
      scx::MutexLocker locker(m_mutex);

      m_users.clear();
      scx::File file;
      if (scx::Ok == file.open(m_path,scx::File::Read)) {
	scx::LineBuffer* tok = new scx::LineBuffer("");
	file.add_stream(tok);
	std::string line;
	while (scx::Ok == tok->tokenize(line)) {
	  std::string::size_type i = line.find_first_of(" ");
	  std::string username;
	  std::string password;
	  username = line.substr(0,i);
	  i = line.find_first_not_of(" ",i);
	  password = line.substr(i);
	  if (!username.empty() && !password.empty()) {
	    m_users[username] = password;
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
    return it->second;
  }
  return 0;
}

//=========================================================================
std::string AuthRealmManager::name() const
{
  std::ostringstream oss;
  oss << "AUTHREALM MANAGER";
  return oss.str();
}

//=============================================================================
scx::Arg* AuthRealmManager::arg_resolve(const std::string& name)
{
  scx::Arg* a = SCXBASE ArgObjectInterface::arg_resolve(name);
  if (a==0 || (dynamic_cast<scx::ArgError*>(a)!=0)) {
    delete a;
    return m_module.arg_resolve(name);
  }
  return a;
}

//=============================================================================
scx::Arg* AuthRealmManager::arg_lookup(const std::string& name)
{
  // Methods
  if ("add" == name ||
      "remove" == name) {
    return new scx::ArgObjectFunction(new scx::ArgObject(this),name);
  }

  // Sub-objects
  
  if ("list" == name) {
    scx::ArgList* list = new scx::ArgList();
    for (AuthRealmMap::const_iterator it = m_realms.begin();
	 it != m_realms.end();
	 ++it) {
      list->give(new scx::ArgObject(it->second));
    }
    return list;
  }
  
  AuthRealmMap::const_iterator it = m_realms.find(name);
  if (it != m_realms.end()) {
    AuthRealm* r = it->second;
    return new scx::ArgObject(r);
  }
  
  return SCXBASE ArgObjectInterface::arg_lookup(name);
}

//=============================================================================
scx::Arg* AuthRealmManager::arg_function(
  const scx::Auth& auth,
  const std::string& name,
  scx::Arg* args
)
{
  scx::ArgList* l = dynamic_cast<scx::ArgList*>(args);

  if (!auth.admin()) return new scx::ArgError("Not permitted");

  if ("add" == name) {
    const scx::ArgString* a_realm =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_realm) {
      return new scx::ArgError("add_realm() No realm specified");
    }
    std::string s_realm = a_realm->get_string();

    AuthRealmMap::const_iterator it = m_realms.find(s_realm);
    if (it != m_realms.end()) {
      return new scx::ArgError("add_realm() Realm already exists");
    }

    const scx::ArgString* a_path =
      dynamic_cast<const scx::ArgString*>(l->get(1));
    if (!a_path) {
      return new scx::ArgError("add_realm() No path specified");
    }
    std::string s_path = a_path->get_string();
    
    log("Adding realm '" + s_realm + "'");
    m_realms[s_realm] = new AuthRealm(m_module,s_realm,s_path);

    return 0;
  }

  if ("remove" == name) {
    const scx::ArgString* a_realm =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_realm) {
      return new scx::ArgError("add_realm() No realm specified");
    }
    std::string s_realm = a_realm->get_string();

    AuthRealmMap::iterator it = m_realms.find(s_realm);
    if (it == m_realms.end()) {
      return new scx::ArgError("add_realm() Realm does not exist");
    }
    
    log("Removing realm '" + s_realm + "'");
    delete (*it).second;
    m_realms.erase(it);
    
    return 0;
  }

  return SCXBASE ArgObjectInterface::arg_function(auth,name,args);
}

};
