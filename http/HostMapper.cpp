/* SconeServer (http://www.sconemad.com)

HTTP Host mapper

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


#include "http/HostMapper.h"
#include "http/Host.h"
#include "http/HTTPModule.h"
namespace http {

//=========================================================================
HostMapper::HostMapper(HTTPModule& module)
  : m_module(module)
{

}

//=========================================================================
HostMapper::~HostMapper()
{
  for (std::map<std::string,Host*>::const_iterator it =
         m_hosts.begin();
       it != m_hosts.end();
       ++it) {
    delete (*it).second;
  }
}

//=============================================================================
Host* HostMapper::host_lookup(
  const std::string& name
)
{
  int bailout=100;
  std::string::size_type idot;
  std::string key=name;

  while (--bailout > 0) {
  
    std::map<std::string,std::string>::const_iterator it = m_hostmap.find(key);
    if (it != m_hostmap.end()) {
      std::string id = (*it).second;
      std::map<std::string,Host*>::const_iterator it2 = m_hosts.find(id);
      if (it2 != m_hosts.end()) {
        Host* h = (*it2).second;
        return h;
      }
      log(std::string("Lookup failure: '") + name + 
        "' maps to unknown host '" + id + "'");
      return 0; // maps to unknown host?
    }      

    if (key.size()<=0 || key=="*") {
      log(std::string("Lookup failure: '") + name + "'");
      return 0; // No match
    }

    if (key[0]=='*') {
      idot = key.find(".",2);
    } else {
      idot = key.find_first_of(".");
    }
    
    if (idot==key.npos) {
      key="*";
    } else {
      key = "*" + key.substr(idot);
    }
    
  }
  log(std::string("Lookup failure (BAILOUT): '") + name + "'");
  return 0; //Bailed out
}

//=============================================================================
std::string HostMapper::name() const
{
  return "HOST MAPPER";
}

//=============================================================================
scx::Arg* HostMapper::arg_lookup(
  const std::string& name
)
{
  // Methods
  
  if ("add" == name ||
      "remove" == name ||
      "map" == name) {
    return new scx::ArgObjectFunction(new scx::ArgObject(this),name);
  }

  // Properties
  
  if ("list" == name) {
    std::ostringstream oss;
    std::map<std::string,Host*>::const_iterator it = m_hosts.begin();
    while (it != m_hosts.end()) {
      oss << (*it).first << "\n";
      it++;
    }
    return new scx::ArgString(oss.str());
  }

  if ("lsmap" == name) {
    std::ostringstream oss;
    std::map<std::string,std::string>::const_iterator it = m_hostmap.begin();
    while (it != m_hostmap.end()) {
      oss << "'" << (*it).first << "' -> " << (*it).second << "\n";
      it++;
    }
    return new scx::ArgString(oss.str());
  }

  // Sub-objects

  std::map<std::string,Host*>::const_iterator it = m_hosts.find(name);
  if (it != m_hosts.end()) {
    Host* h = (*it).second;
    return new scx::ArgObject(h);
  }      

  return ArgObjectInterface::arg_lookup(name);
}

//=============================================================================
scx::Arg* HostMapper::arg_resolve(const std::string& name)
{
  scx::Arg* a = SCXBASE ArgObjectInterface::arg_resolve(name);
  if (a==0 || (dynamic_cast<scx::ArgError*>(a))!=0) {
    delete a;
    return m_module.arg_resolve(name);
  }
  return a;
}

//=============================================================================
scx::Arg* HostMapper::arg_function(
  const std::string& name,
  scx::Arg* args
)
{
  scx::ArgList* l = dynamic_cast<scx::ArgList*>(args);

  if ("add" == name) {
    const scx::ArgString* a_host =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_host) {
      return new scx::ArgError("add() No name specified");
    }
    std::string s_hostname = a_host->get_string();
    
    const scx::ArgString* a_path =
      dynamic_cast<const scx::ArgString*>(l->get(1));
    if (!a_path) {
      return new scx::ArgError("add() No path specified");
    }

    std::map<std::string,Host*>::const_iterator it = m_hosts.find(s_hostname);
    if (it != m_hosts.end()) {
      return new scx::ArgError("add() Host already exists");
    }
        
    log("Adding host '" + s_hostname + "' dir '" + a_path->get_string() + "'");
    scx::FilePath path = m_module.get_conf_path() + a_path->get_string();
    m_hosts[s_hostname] = new Host(m_module,*this,s_hostname,path.path());
    return 0;
  }

  if ("remove" == name) {
    const scx::ArgString* a_host =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_host) {
      return new scx::ArgError("remove() No name specified");
    }
    std::string s_hostname = a_host->get_string();

    std::map<std::string,Host*>::iterator it = m_hosts.find(s_hostname);
    if (it == m_hosts.end()) {
      return new scx::ArgError("remove() Host not found");
    }
    
    log("Removing host '" + s_hostname + "'");
    delete (*it).second;
    m_hosts.erase(it);
    return 0;
  }
  
  if ("map" == name) {
    const scx::ArgString* a_pattern =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_pattern) {
      return new scx::ArgError("map() No pattern specified");
    }
    std::string s_pattern = a_pattern->get_string();
    
    const scx::ArgString* a_target =
      dynamic_cast<const scx::ArgString*>(l->get(1));
    if (!a_target) {
      return new scx::ArgError("map() No target specified");
    }
    std::string s_target = a_target->get_string();

    log("Mapping host pattern '" + s_pattern + "' to ID '" + s_target + "'"); 
    m_hostmap[s_pattern] = s_target;
    return 0;
  }
    
  return ArgObjectInterface::arg_function(name,args);
}

};
