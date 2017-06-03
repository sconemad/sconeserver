/* SconeServer (http://www.sconemad.com)

HTTP Host mapper

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


#include <http/HostMapper.h>
#include <http/Host.h>
#include <http/HTTPModule.h>
#include <http/Request.h>
#include <http/Response.h>
#include <http/MessageStream.h>
#include <sconex/ScriptTypes.h>
#include <sconex/Descriptor.h>
#include <sconex/Kernel.h>
#include <sconex/Log.h>
namespace http {

#define LOG(msg) scx::Log("http.hosts").submit(msg);
  
//=========================================================================
HostMapper::HostMapper(HTTPModule& module)
  : m_module(module)
{
  m_parent = &module;
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
scx::Condition HostMapper::connect_request(MessageStream* message,
                                           Request& request,
                                           Response& response)
{
  const scx::Uri& uri = request.get_uri();
  const std::string& hostname = uri.get_host();
  std::string mapped_host;
  bool redirect = false;
  
  if (lookup(m_redirects,hostname,mapped_host)) {
    // Redirect
    redirect = true;

  } else if (lookup(m_aliases,hostname,mapped_host)) {
    // Alias map

  } else {
    // This is bad, user should have setup a default host
    LOG("Unknown host '" + hostname + "'");
    message->send_simple_response(http::Status::NotFound);
    return scx::Close;
  }

  HostMap::const_iterator it = m_hosts.find(mapped_host);
  if (it == m_hosts.end()) {
    LOG(std::string("Lookup failure: '") + hostname + 
        "' maps to unknown host '" + mapped_host + "'");
    message->send_simple_response(http::Status::NotFound);
    return scx::Close;
  }
  Host* host = it->second->object();

  if (redirect) {
    // Redirect to host's uri
    scx::Uri new_uri = uri;
    new_uri.set_host(host->get_hostname());
    LOG("Host redirect '" + uri.get_string() + 
        "' to '" + new_uri.get_string() + "'"); 
    response.set_header("Location",new_uri.get_string());
    message->send_simple_response(http::Status::Found);
    return scx::Close;
  }
  
  request.set_host(host);
  return host->connect_request(message,request,response);
}

//=============================================================================
Host* HostMapper::lookup_host(const std::string& name)
{
  HostMap::iterator it = m_hosts.find(name);
  if (it != m_hosts.end()) {
    return it->second->object();
  }
  return 0;
}

//=============================================================================
scx::ScriptRef* HostMapper::script_op(const scx::ScriptAuth& auth,
				      const scx::ScriptRef& ref,
				      const scx::ScriptOp& op,
				      const scx::ScriptRef* right)
{
  if (op.type() == scx::ScriptOp::Lookup) {
    const std::string name = right->object()->get_string();
  
    // Methods
    if ("add" == name ||
	"remove" == name ||
	"alias" == name ||
	"redirect" == name) {
      return new scx::ScriptMethodRef(ref,name);
    }

    // Properties
    
    if ("list" == name) {
      scx::ScriptList* list = new scx::ScriptList();
      scx::ScriptRef* list_ref = new scx::ScriptRef(list);
      for (HostMap::const_iterator it = m_hosts.begin();
	   it != m_hosts.end();
	   ++it) {
	Host::Ref* hostref = it->second;
	list->give(hostref->ref_copy(ref.reftype()));
      }
      return list_ref;
    }
    
    if ("aliases" == name) {
      scx::ScriptMap* map = new scx::ScriptMap();
      scx::ScriptRef* map_ref = new scx::ScriptRef(map);
      for (HostNameMap::const_iterator it = m_aliases.begin();
	   it != m_aliases.end();
	   ++it) {
	map->give(it->first,scx::ScriptString::new_ref(it->second));
      }
      return map_ref;
    }
    
    if ("redirects" == name) {
      scx::ScriptMap* map = new scx::ScriptMap();
      scx::ScriptRef* map_ref = new scx::ScriptRef(map);
      for (HostNameMap::const_iterator it = m_redirects.begin();
	   it != m_redirects.end();
	   ++it) {
	map->give(it->first,scx::ScriptString::new_ref(it->second));
      }
      return map_ref;
    }
    
    // Sub-objects
    HostMap::const_iterator it = m_hosts.find(name);
    if (it != m_hosts.end()) {
      Host::Ref* hostref = it->second;
      return hostref->ref_copy(ref.reftype());
    }      
  }

  return scx::ScriptObject::script_op(auth,ref,op,right);
}

//=============================================================================
scx::ScriptRef* HostMapper::script_method(const scx::ScriptAuth& auth,
					  const scx::ScriptRef& ref,
					  const std::string& name,
					  const scx::ScriptRef* args)
{
  if (!auth.admin()) return scx::ScriptError::new_ref("Not permitted");

  if ("add" == name) {
    const scx::ScriptString* a_id =
      scx::get_method_arg<scx::ScriptString>(args,0,"id");
    if (!a_id) {
      return scx::ScriptError::new_ref("add() No host ID specified");
    }
    std::string s_id = a_id->get_string();

    const scx::ScriptString* a_hostname =
      scx::get_method_arg<scx::ScriptString>(args,1,"hostname");
    if (!a_hostname) {
      return scx::ScriptError::new_ref("add() No hostname specified");
    }
    std::string s_hostname = a_hostname->get_string();
    
    const scx::ScriptString* a_path =
      scx::get_method_arg<scx::ScriptString>(args,2,"path");
    if (!a_path) {
      return scx::ScriptError::new_ref("add() No path specified");
    }
    
    HostMap::const_iterator it = m_hosts.find(s_id);
    if (it != m_hosts.end()) {
      return scx::ScriptError::new_ref("add() Host with this ID already exists");
    }
        
    LOG("Adding host '" + s_id + "' hostname '" + s_hostname +
        "' path '" + a_path->get_string() + "'");
    scx::FilePath path = scx::Kernel::get()->get_conf_path() + a_path->get_string();
    Host* host = new Host(m_module, *this, s_id, s_hostname, path.path(), "art");
    host->init();
    m_hosts[s_id] = new Host::Ref(host);
    m_aliases[s_hostname] = s_id;
    return new Host::Ref(host);
  }

  if ("remove" == name) {
    const scx::ScriptString* a_host =
      scx::get_method_arg<scx::ScriptString>(args,0,"id");
    if (!a_host) {
      return scx::ScriptError::new_ref("remove() No host id specified");
    }
    std::string s_hostname = a_host->get_string();

    HostMap::iterator it = m_hosts.find(s_hostname);
    if (it == m_hosts.end()) {
      return scx::ScriptError::new_ref("remove() Host not found");
    }
    
    LOG("Removing host '" + s_hostname + "'");
    delete it->second;
    m_hosts.erase(it);
    return 0;
  }
  
  if ("alias" == name) {
    const scx::ScriptString* a_pattern =
      scx::get_method_arg<scx::ScriptString>(args,0,"pattern");
    if (!a_pattern) {
      return scx::ScriptError::new_ref("map() No pattern specified");
    }
    std::string s_pattern = a_pattern->get_string();
    
    const scx::ScriptString* a_target =
      scx::get_method_arg<scx::ScriptString>(args,1,"target");
    if (!a_target) {
      return scx::ScriptError::new_ref("map() No target specified");
    }
    std::string s_target = a_target->get_string();

    LOG("Mapping host pattern '" + s_pattern + "' to ID '" + s_target + "'"); 
    m_aliases[s_pattern] = s_target;
    return 0;
  }

  if ("redirect" == name) {
    const scx::ScriptString* a_pattern =
      scx::get_method_arg<scx::ScriptString>(args,0,"pattern");
    if (!a_pattern) {
      return scx::ScriptError::new_ref("redirect() No pattern specified");
    }
    std::string s_pattern = a_pattern->get_string();
    
    const scx::ScriptString* a_target =
      scx::get_method_arg<scx::ScriptString>(args,1,"target");
    if (!a_target) {
      return scx::ScriptError::new_ref("redirect() No target specified");
    }
    std::string s_target = a_target->get_string();

    LOG("Redirecting host pattern '" + s_pattern +
        "' to ID '" + s_target + "'"); 
    m_redirects[s_pattern] = s_target;
    return 0;
  }
    
  return scx::ScriptObject::script_method(auth,ref,name,args);
}

//=============================================================================
bool HostMapper::lookup(const HostNameMap& map,
			const std::string& pattern,
			std::string& result)
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
  LOG(std::string("Lookup failure (BAILOUT): '") + pattern + "'");
  return false;
}

};
