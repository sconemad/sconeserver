/* SconeServer (http://www.sconemad.com)

http Document root

Copyright (c) 2000-2006 Andrew Wedgbury <wedge@sconemad.com>

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


#include "http/DocRoot.h"
#include "http/Host.h"
#include "http/MessageStream.h"
#include "http/Request.h"
#include "http/AuthRealm.h"
#include "sconex/Uri.h"
#include "sconex/Base64.h"
#include "sconex/utils.h"
namespace http {


//=========================================================================
ModuleMap::ModuleMap(
  HTTPModule& module,
  const std::string& name,
  scx::ArgList* args
) : m_module(module),
    m_name(name),
    m_args(args)
{

}
  
//=========================================================================
ModuleMap::~ModuleMap()
{
  delete m_args;
}

//=========================================================================
const std::string& ModuleMap::get_name() const
{
  return m_name;
}
  
//=========================================================================
bool ModuleMap::connect_module(scx::Descriptor* d)
{
  scx::ModuleRef ref = m_module.get_module(m_name.c_str());
  if (!ref.valid()) {
    return false;
  }

  // Connect module
  return ref.module()->connect(d,m_args);
}


  
//=========================================================================
DocRoot::DocRoot(
  HTTPModule& module,
  Host& host,
  const std::string profile,
  const scx::FilePath& path
) : m_module(module),
    m_host(host),
    m_profile(profile),
    m_path(path)
{
  // A bit of a hack really, but set up default module mappings so it will
  // do something sensible if there is no host config file present.
  m_extn_mods["."] = new ModuleMap(m_module,"dirindex",0);
  m_extn_mods["!"] = new ModuleMap(m_module,"errorpage",0);
  m_extn_mods["*"] = new ModuleMap(m_module,"getfile",0);
}

//=========================================================================
DocRoot::~DocRoot()
{

}

//=========================================================================
bool DocRoot::connect_request(scx::Descriptor* endpoint, MessageStream& message)
{
  ModuleMap* modmap = 0;

  if (message.get_status().code() != http::Status::Ok) {
    modmap = lookup_extn_mod("!");
    
  } else {
    const scx::Uri& uri = message.get_request().get_uri();
    const std::string& uripath = uri.get_path();
    if (uripath.length() > 1 && uripath[0] == '/') {
      m_module.log("Request uri starts with /",scx::Logger::Error);
      message.set_status(http::Status::Forbidden);
      return false;

    } else if (uripath.find("..") != std::string::npos) {
      m_module.log("Request uri contains ..",scx::Logger::Error);
      message.set_status(http::Status::Forbidden);
      return false;
    }

    // Check http authorization
    if (!check_auth(message)) {
      message.set_status(http::Status::Unauthorized);
      return false;
    }

    scx::FilePath path = m_path + uripath;
    message.set_path(path);

    modmap = lookup_path_mod(uripath);
    if (modmap) {
      // Path mapped module
      m_module.log("Using path mapping",scx::Logger::Info);
    } else {
      // Normal file mapping
      scx::FileStat stat(path);
      if (!stat.exists()) {
        message.set_status(http::Status::NotFound);
        return false;
      } else if (stat.is_dir()) {
        modmap = lookup_extn_mod(".");
      } else {
        modmap = lookup_extn_mod(uripath);
      }
    }
  }

  // Check we have a module map
  if (modmap == 0) {
    m_module.log("No module map found to handle request",scx::Logger::Error);
    message.set_status(http::Status::ServiceUnavailable);
    return false;
  }

  // Connect the module
  if (!modmap->connect_module(endpoint)) {
    m_module.log("Failed to connect module to handle request",scx::Logger::Error);
    message.set_status(http::Status::ServiceUnavailable);
    return false;
  }

  // Success
  return true;
}

//=========================================================================
ModuleMap* DocRoot::lookup_extn_mod(const std::string& name) const
{
  int bailout=100;
  std::string::size_type idot;
  std::string key=name;
  while (--bailout > 0) {

    std::map<std::string,ModuleMap*>::const_iterator it = m_extn_mods.find(key);
    if (it != m_extn_mods.end()) {
      return (*it).second;
    }
    
    if (key.size()<=0 || key=="*") {
      return 0;
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
  DEBUG_LOG("lookup_extn_mod() Pattern match bailout");
  return 0; // Bailed out
}

//=========================================================================
ModuleMap* DocRoot::lookup_path_mod(const std::string& name) const
{
  int bailout=100;
  std::string::size_type idot;
  std::string key=name;
  while (--bailout > 0) {

    std::map<std::string,ModuleMap*>::const_iterator it = m_path_mods.find(key);
    if (it != m_path_mods.end()) {
      return (*it).second;
    }
    
    if (key.size()<=0 || key=="/") {
      return 0;
    }

    idot = key.find_last_of("/");
    
    if (idot==key.npos) {
      key="/";
    } else {
      key = key.substr(0,idot);
    }
    
  }
  DEBUG_LOG("lookup_path_mod() Pattern match bailout");
  return 0; // Bailed out
}

//=========================================================================
std::string DocRoot::lookup_realm_map(const std::string& name) const
{
  int bailout=100;
  std::string::size_type idot;
  std::string key=name;
  while (--bailout > 0) {

    std::map<std::string,std::string>::const_iterator it = m_realm_maps.find(key);
    if (it != m_realm_maps.end()) {
      return (*it).second;
    }
    
    if (key.size()<=0 || key=="/") {
      return "";
    }

    idot = key.find_last_of("/");
    
    if (idot==key.npos) {
      key="/";
    } else {
      key = key.substr(0,idot);
    }
    
  }
  DEBUG_LOG("lookup_realm_map() Pattern match bailout");
  return ""; // Bailed out
}

//=========================================================================
const std::string DocRoot::get_profile() const
{
  return m_profile;
}

//=============================================================================
const scx::Arg* DocRoot::get_param(const std::string& name) const
{
  std::map<std::string,scx::Arg*>::const_iterator it =
    m_params.find(name);
  if (it != m_params.end()) {
    return (*it).second;
  }
  return 0;
}

//=============================================================================
void DocRoot::set_param(const std::string& name, scx::Arg* value)
{
  const scx::Arg* existing_value = get_param(name);
  delete existing_value;
  m_params[name] = value;
}

//=========================================================================
std::string DocRoot::name() const
{
  std::ostringstream oss;
  oss << "DOCROOT:" << get_profile();
  return oss.str();
}

//=============================================================================
scx::Arg* DocRoot::arg_resolve(const std::string& name)
{
  scx::Arg* a = SCXBASE ArgObjectInterface::arg_resolve(name);
  if (a==0 || (dynamic_cast<scx::ArgError*>(a)!=0)) {
    delete a;
    return m_host.arg_resolve(name);
  }
  return a;
}

//=============================================================================
scx::Arg* DocRoot::arg_lookup(const std::string& name)
{
  // Methods
  if ("map" == name ||
      "map_path" == name ||
      "add_realm" == name ||
      "map_realm" == name ||
      "set_param" == name) {
    return new scx::ArgObjectFunction(new scx::ArgObject(this),name);
  }

  // Sub-objects
  
  return SCXBASE ArgObjectInterface::arg_lookup(name);
}

//=============================================================================
scx::Arg* DocRoot::arg_function(
  const std::string& name,
  scx::Arg* args
)
{
  scx::ArgList* l = dynamic_cast<scx::ArgList*>(args);

  if ("map" == name) {
    const scx::ArgString* a_pattern =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_pattern) {
      return new scx::ArgError("map() No pattern specified");
    }
    std::string s_pattern = a_pattern->get_string();

    const scx::ArgString* a_module =
      dynamic_cast<const scx::ArgString*>(l->get(1));
    if (!a_module) {
      return new scx::ArgError("map() No module specified");
    }
    std::string s_module = a_module->get_string();

    // Transfer the remaining arguments to a new list, to be stored with
    // the module map.
    std::string logargs;
    scx::ArgList* ml = new scx::ArgList();
    while (l->size() > 2) {
      scx::Arg* a = l->take(2);
      DEBUG_ASSERT(a!=0,"add() NULL argument in list");
      ml->give(a);
      if (!logargs.empty()) logargs += ",";
      logargs += a->get_string();
    }
    
    log("Mapping pattern '" + s_pattern + "' to module " + s_module + "(" + logargs + ")");
    
    ModuleMap* modmap = new ModuleMap(m_module,s_module,ml);
    m_extn_mods[s_pattern] = modmap;

    return 0;
  }

  if ("map_path" == name) {
    const scx::ArgString* a_pattern =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_pattern) {
      return new scx::ArgError("map_path() No pattern specified");
    }
    std::string s_pattern = a_pattern->get_string();

    const scx::ArgString* a_module =
      dynamic_cast<const scx::ArgString*>(l->get(1));
    if (!a_module) {
      return new scx::ArgError("map_path() No module specified");
    }
    std::string s_module = a_module->get_string();

    // Transfer the remaining arguments to a new list, to be stored with
    // the module map.
    std::string logargs;
    scx::ArgList* ml = new scx::ArgList();
    while (l->size() > 2) {
      scx::Arg* a = l->take(2);
      DEBUG_ASSERT(a!=0,"add() NULL argument in list");
      ml->give(a);
      if (!logargs.empty()) logargs += ",";
      logargs += a->get_string();
    }
    
    log("Mapping path '" + s_pattern + "' to module " + s_module + "(" + logargs + ")");

    ModuleMap* modmap = new ModuleMap(m_module,s_module,ml);
    m_path_mods[s_pattern] = modmap;

    return 0;
  }

  if ("map_realm" == name) {
    const scx::ArgString* a_pattern =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_pattern) {
      return new scx::ArgError("map_realm() No pattern specified");
    }
    std::string s_pattern = a_pattern->get_string();

    const scx::ArgString* a_realm =
      dynamic_cast<const scx::ArgString*>(l->get(1));
    if (!a_realm) {
      return new scx::ArgError("map_realm() No realm specified");
    }
    std::string s_realm = a_realm->get_string();

    log("Mapping path '" + s_pattern + "' to realm " + s_realm);
    m_realm_maps[s_pattern] = s_realm;

    return 0;
  }

  if ("set_param" == name) {
    const scx::ArgString* a_name =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_name) {
      return new scx::ArgError("set_param() No parameter name specified");
    }
    std::string s_name = a_name->get_string();

    scx::Arg* value = l->take(1);
    set_param(s_name,value);

    return 0;
  }

  return 0;
}

//=============================================================================
bool DocRoot::check_auth(MessageStream& message)
{
  // Check auth
  const scx::Uri& uri = message.get_request().get_uri();
  const std::string& uripath = uri.get_path();
  
  std::string realm_name = lookup_realm_map(uripath);
  AuthRealm* realm = m_host.lookup_realm(realm_name);
  if (realm) {
    std::string auth = message.get_request().get_header("AUTHORIZATION");
    std::string user;
    std::string pass;
    if (!auth.empty()) {
      std::string::size_type is = auth.find(" ");
      std::string method;
      std::string enc;
      if (is != std::string::npos) {
        method = auth.substr(0,is);
        scx::strup(method);
        enc = auth.substr(is+1);
      }
      
      if (method == "BASIC") {
        std::istringstream auth64in(enc);
        std::ostringstream auth64out;
        scx::Base64::decode(auth64in,auth64out);
        auth = auth64out.str();
        std::string::size_type ic = auth.find(":");
        if (ic != std::string::npos) {
          user = auth.substr(0,ic);
          pass = auth.substr(ic+1);
        }
      }
    }
    if (realm->authorised(user,pass)) {
      m_module.log("Auth passed for realm '" + realm_name + "' (" + user + ":" + pass + ")",scx::Logger::Info);
      message.set_auth_user(user);
    } else {
      m_module.log("Auth failed for realm '" + realm_name + "' (" + user + ":" + pass + ")",scx::Logger::Info);
      message.set_header("WWW-AUTHENTICATE","Basic realm=\"" + realm_name + "\"");
      return false;
    }
  }
  
  return true;
}


};
