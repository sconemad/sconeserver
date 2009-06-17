/* SconeServer (http://www.sconemad.com)

http Host

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


#include "http/Host.h"
#include "http/HostMapper.h"
#include "http/DocRoot.h"
#include "http/MessageStream.h"
#include "http/Request.h"
#include "http/AuthRealm.h"
#include "sconex/ConfigFile.h"
namespace http {

//=========================================================================
Host::Host(
  HTTPModule& module,
  HostMapper& mapper,
  const std::string id,
  const std::string hostname,
  const scx::FilePath& dir
) : m_module(module),
    m_mapper(mapper),
    m_id(id),
    m_hostname(hostname),
    m_dir(dir)
{
  m_realms = new AuthRealmManager(m_module,*this);
}

//=========================================================================
Host::~Host()
{
  for (std::map<std::string,DocRoot*>::const_iterator it =
         m_docroots.begin();
       it != m_docroots.end();
       ++it) {
    delete (*it).second;
  }
  
  delete m_realms;
}

//=========================================================================
int Host::init()
{
  scx::ConfigFile config(m_dir + "host.conf");
  scx::ArgObject* ctx = new scx::ArgObject(this);
  int err = config.load(ctx);
  return err;
}

//=========================================================================
bool Host::connect_request(scx::Descriptor* endpoint, MessageStream& message)
{
  std::string profile = message.get_request().get_profile();
  DocRoot* docroot = get_docroot(profile);
  if (docroot == 0) {
    // Unknown profile
    m_module.log("Unknown profile '" + profile +
                 "' for host '" + m_hostname + "'",
                 scx::Logger::Error);
    message.set_status(http::Status::NotFound);
    return false;
  }

  message.set_docroot(docroot);
  return docroot->connect_request(endpoint,message);
}

//=========================================================================
const std::string Host::get_id() const
{
  return m_id;
}

//=========================================================================
const std::string Host::get_hostname() const
{
  return m_hostname;
}

//=========================================================================
DocRoot* Host::get_docroot(const std::string& profile)
{
  std::map<std::string,DocRoot*>::const_iterator it =
    m_docroots.find(profile);
  if (it != m_docroots.end()) {
    return (*it).second;
  }

  return 0;
}

//=========================================================================
AuthRealm* Host::lookup_realm(const std::string& realm)
{
  return m_realms->lookup_realm(realm);
}

//=========================================================================
std::string Host::name() const
{
  std::ostringstream oss;
  oss << "HOST:" << get_id();
  return oss.str();
}

//=============================================================================
scx::Arg* Host::arg_lookup(
  const std::string& name
)
{
  // Methods
  
  if ("add" == name ||
      "remove" == name) {
    return new scx::ArgObjectFunction(new scx::ArgObject(this),name);
  }

  // Properties
  
  if ("list" == name) {
    std::ostringstream oss;
    for (std::map<std::string,DocRoot*>::const_iterator it =
           m_docroots.begin();
         it != m_docroots.end();
         ++it) {
      oss << (*it).first << "\n";
    }
    return new scx::ArgString(oss.str());
  }

  // Sub-objects

  if ("realms" == name) {
    return new scx::ArgObject(m_realms);
  }

  std::map<std::string,DocRoot*>::const_iterator it = m_docroots.find(name);
  if (it != m_docroots.end()) {
    DocRoot* r = (*it).second;
    return new scx::ArgObject(r);
  }      

  return ArgObjectInterface::arg_lookup(name);
}

//=============================================================================
scx::Arg* Host::arg_resolve(const std::string& name)
{
  scx::Arg* a = SCXBASE ArgObjectInterface::arg_resolve(name);
  if (a==0 || (dynamic_cast<scx::ArgError*>(a)!=0)) {
    delete a;
    return m_mapper.arg_resolve(name);
  }
  return a;
}

//=============================================================================
scx::Arg* Host::arg_function(
  const std::string& name,
  scx::Arg* args
)
{
  scx::ArgList* l = dynamic_cast<scx::ArgList*>(args);

  if ("add" == name) {
    const scx::ArgString* a_profile =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_profile) {
      return new scx::ArgError("add() No profile name specified");
    }
    std::string s_profile = a_profile->get_string();
    
    const scx::ArgString* a_path =
      dynamic_cast<const scx::ArgString*>(l->get(1));
    if (!a_path) {
      return new scx::ArgError("add() No path specified");
    }

    std::map<std::string,DocRoot*>::const_iterator it =
      m_docroots.find(s_profile);
    if (it != m_docroots.end()) {
      return new scx::ArgError("add() Profile already exists");
    }
        
    scx::FilePath path = m_dir + a_path->get_string();
    log("Adding profile '" + s_profile + "' dir '" +
        path.path() + "'");
    m_docroots[s_profile] = new DocRoot(m_module,*this,s_profile,path.path());
    return 0;
  }

  if ("remove" == name) {
    const scx::ArgString* a_profile =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_profile) {
      return new scx::ArgError("remove() No profile name specified");
    }
    std::string s_profile = a_profile->get_string();

    std::map<std::string,DocRoot*>::iterator it =
      m_docroots.find(s_profile);
    if (it == m_docroots.end()) {
      return new scx::ArgError("remove() Profile not found");
    }
        
    log("Removing profile '" + s_profile + "'");
    delete (*it).second;
    m_docroots.erase(it);
    return 0;
  }
    
  return ArgObjectInterface::arg_function(name,args);
}

};
