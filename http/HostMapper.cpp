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
#include "http/Request.h"
#include "http/Response.h"
namespace http {

//=========================================================================
HostMapper::HostMapper(HTTPModule& module)
  : m_module(module)
{

}

//=========================================================================
HostMapper::~HostMapper()
{
  for (HostMap::const_iterator it = m_hosts.begin();
       it != m_hosts.end();
       ++it) {
    delete it->second;
  }
}

//=============================================================================
bool HostMapper::connect_request(scx::Descriptor* endpoint, Request& request, Response& response)
{
  const scx::Uri& uri = request.get_uri();
  const std::string& host = uri.get_host();
  std::string mapped_host;
  bool redirect = false;
  
  if (lookup(m_redirects,host,mapped_host)) {
    // Redirect
    redirect = true;

  } else if (lookup(m_aliases,host,mapped_host)) {
    // Alias map

  } else {
    // This is bad, user should have setup a default host
    m_module.log("Unknown host '" + uri.get_host() + "'",scx::Logger::Error);
    response.set_status(http::Status::NotFound);
    response.set_header("Content-Type","text/html");
    endpoint->write("<html><head></head><body><h1>Host not found</h1></body></html>");
    return false;
  }

  HostMap::const_iterator it = m_hosts.find(mapped_host);
  if (it == m_hosts.end()) {
    log(std::string("Lookup failure: '") + host + "' maps to unknown host '" + mapped_host + "'");
    response.set_status(http::Status::NotFound);
    response.set_header("Content-Type","text/html");
    endpoint->write("<html><head></head><body><h1>Host not found</h1></body></html>");
    return false;
  }
  Host* h = it->second;

  if (redirect) {
    // Redirect to host's uri
    scx::Uri new_uri = uri;
    new_uri.set_host(h->get_hostname());
    log("Host redirect '" + uri.get_string() + "' to '" + new_uri.get_string() + "'"); 
    
    response.set_status(http::Status::Found);
    response.set_header("Content-Type","text/html");
    response.set_header("Location",new_uri.get_string());
    endpoint->write("<html><head></head><body><h1>Host redirect</h1></body></html>");
    return false;
  }
  
  request.set_host(h);
  return h->connect_request(endpoint,request,response);
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
      "alias" == name ||
      "redirect" == name) {
    return new scx::ArgObjectFunction(new scx::ArgObject(this),name);
  }

  // Properties
  
  if ("list" == name) {
    std::ostringstream oss;
    for (HostMap::const_iterator it = m_hosts.begin();
	 it != m_hosts.end();
	 ++it) {
      oss << it->first << "\n";
    }
    return new scx::ArgString(oss.str());
  }

  if ("lsmap" == name) {
    std::ostringstream oss;
    HostNameMap::const_iterator it = m_aliases.begin();
    while (it != m_aliases.end()) {
      oss << "'" << it->first << "' -> " << it->second << "\n";
      it++;
    }
    return new scx::ArgString(oss.str());
  }

  // Sub-objects

  HostMap::const_iterator it = m_hosts.find(name);
  if (it != m_hosts.end()) {
    Host* h = it->second;
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
    const scx::ArgString* a_id =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_id) {
      return new scx::ArgError("add() No host ID specified");
    }
    std::string s_id = a_id->get_string();

    const scx::ArgString* a_hostname =
      dynamic_cast<const scx::ArgString*>(l->get(1));
    if (!a_hostname) {
      return new scx::ArgError("add() No hostname specified");
    }
    std::string s_hostname = a_hostname->get_string();
    
    const scx::ArgString* a_path =
      dynamic_cast<const scx::ArgString*>(l->get(2));
    if (!a_path) {
      return new scx::ArgError("add() No path specified");
    }

    HostMap::const_iterator it = m_hosts.find(s_id);
    if (it != m_hosts.end()) {
      return new scx::ArgError("add() Host with this ID already exists");
    }
        
    log("Adding {HOST:" + s_id + "} hostname '" + s_hostname + "' path '" + a_path->get_string() + "'");
    scx::FilePath path = m_module.get_conf_path() + a_path->get_string();
    Host* host = new Host(m_module,*this,s_id,s_hostname,path.path());
    host->init();
    m_hosts[s_id] = host;
    m_aliases[s_hostname] = s_id;
    return 0;
  }

  if ("remove" == name) {
    const scx::ArgString* a_host =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_host) {
      return new scx::ArgError("remove() No name specified");
    }
    std::string s_hostname = a_host->get_string();

    HostMap::iterator it = m_hosts.find(s_hostname);
    if (it == m_hosts.end()) {
      return new scx::ArgError("remove() Host not found");
    }
    
    log("Removing host '" + s_hostname + "'");
    delete it->second;
    m_hosts.erase(it);
    return 0;
  }
  
  if ("alias" == name) {
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
    m_aliases[s_pattern] = s_target;
    return 0;
  }

  if ("redirect" == name) {
    const scx::ArgString* a_pattern =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_pattern) {
      return new scx::ArgError("redirect() No pattern specified");
    }
    std::string s_pattern = a_pattern->get_string();
    
    const scx::ArgString* a_target =
      dynamic_cast<const scx::ArgString*>(l->get(1));
    if (!a_target) {
      return new scx::ArgError("redirect() No target specified");
    }
    std::string s_target = a_target->get_string();

    log("Redirecting host pattern '" + s_pattern + "' to ID '" + s_target + "'"); 
    m_redirects[s_pattern] = s_target;
    return 0;
  }
    
  return ArgObjectInterface::arg_function(name,args);
}

//=============================================================================
bool HostMapper::lookup(const HostNameMap& map, const std::string& pattern, std::string& result)
{
  int bailout = 100;
  std::string::size_type idot;
  std::string key = pattern;

  while (--bailout > 0) {
  
    HostNameMap::const_iterator it = map.find(key);
    if (it != map.end()) {
      result = it->second;
      return true;
    }

    if (key.size()<=0 || key=="*") {
      return false;
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
  log(std::string("Lookup failure (BAILOUT): '") + pattern + "'");
  return false;
}



};
