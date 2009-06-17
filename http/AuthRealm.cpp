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
#include "http/Host.h"
#include "http/MessageStream.h"
#include "http/Request.h"
#include "sconex/Uri.h"
namespace http {

//=========================================================================
AuthRealm::AuthRealm(
  HTTPModule& module,
  Host& host,
  const std::string name
) : m_module(module),
    m_host(host),
    m_name(name)
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
bool AuthRealm::authorised(const std::string& user, const std::string& pass) const
{
  std::map<std::string,std::string>::const_iterator it = m_users.find(user);
  if (it != m_users.end()) {
    return (pass == (*it).second);
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
    return m_host.arg_resolve(name);
  }
  return a;
}

//=============================================================================
scx::Arg* AuthRealm::arg_lookup(const std::string& name)
{
  // Methods
  if ("add" == name) {
    return new scx::ArgObjectFunction(new scx::ArgObject(this),name);
  }

  return SCXBASE ArgObjectInterface::arg_lookup(name);
}

//=============================================================================
scx::Arg* AuthRealm::arg_function(
  const std::string& name,
  scx::Arg* args
)
{
  scx::ArgList* l = dynamic_cast<scx::ArgList*>(args);

  if ("add" == name) {
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
    m_users[s_user] = s_pass;

    return 0;
  }

  return 0;
}


//=========================================================================
AuthRealmManager::AuthRealmManager(
  HTTPModule& module,
  Host& host
) : m_module(module),
    m_host(host)
{

}

//=========================================================================
AuthRealmManager::~AuthRealmManager()
{
  for (std::map<std::string,AuthRealm*>::const_iterator it =
         m_realms.begin();
       it != m_realms.end();
       ++it) {
    delete (*it).second;
  }
}

//=========================================================================
AuthRealm* AuthRealmManager::lookup_realm(const std::string& name)
{
  std::map<std::string,AuthRealm*>::const_iterator it = m_realms.find(name);
  if (it != m_realms.end()) {
    return (*it).second;
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
    return m_host.arg_resolve(name);
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
  
  std::map<std::string,AuthRealm*>::const_iterator it = m_realms.find(name);
  if (it != m_realms.end()) {
    AuthRealm* r = (*it).second;
    return new scx::ArgObject(r);
  }
  
  return SCXBASE ArgObjectInterface::arg_lookup(name);
}

//=============================================================================
scx::Arg* AuthRealmManager::arg_function(
  const std::string& name,
  scx::Arg* args
)
{
  scx::ArgList* l = dynamic_cast<scx::ArgList*>(args);

  if ("add" == name) {
    const scx::ArgString* a_realm =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_realm) {
      return new scx::ArgError("add_realm() No realm specified");
    }
    std::string s_realm = a_realm->get_string();

    std::map<std::string,AuthRealm*>::const_iterator it = m_realms.find(s_realm);
    if (it != m_realms.end()) {
      return new scx::ArgError("add_realm() Realm already exists");
    }
    
    log("Adding realm '" + s_realm + "'");
    m_realms[s_realm] = new AuthRealm(m_module,m_host,s_realm);

    return 0;
  }

  if ("remove" == name) {
    const scx::ArgString* a_realm =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_realm) {
      return new scx::ArgError("add_realm() No realm specified");
    }
    std::string s_realm = a_realm->get_string();

    std::map<std::string,AuthRealm*>::iterator it = m_realms.find(s_realm);
    if (it == m_realms.end()) {
      return new scx::ArgError("add_realm() Realm does not exist");
    }
    
    log("Removing realm '" + s_realm + "'");
    delete (*it).second;
    m_realms.erase(it);
    
    return 0;
  }

  return 0;
}

};
