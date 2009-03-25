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
#include "sconex/Uri.h"
namespace http {

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
  m_mods["."] = "dirindex";
  m_mods["!"] = "errorpage";
  m_mods["*"] = "getfile";
}

//=========================================================================
DocRoot::~DocRoot()
{

}

//=========================================================================
bool DocRoot::connect_request(scx::Descriptor* endpoint, MessageStream& message)
{
  std::string modname;

  if (message.get_status().code() != http::Status::Ok) {
    modname = lookup_mod("!");
    
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
    
    scx::FilePath path = m_path + uripath;
    message.set_path(path);

    scx::FileStat stat(path);

    if (!stat.exists()) {
      message.set_status(http::Status::NotFound);
      return false;
    
    } else if (stat.is_dir()) {
      modname = lookup_mod(".");
    } else {
      modname = lookup_mod(uri.get_path());
    }
  }

  // Lookup module
  scx::ModuleRef ref = m_module.get_module(modname.c_str());
  if (!ref.valid()) {
    m_module.log("No module found to handle request",scx::Logger::Error);
    message.set_status(http::Status::ServiceUnavailable);
    return false;
  }

  // Connect module
  scx::ArgList args;
  return ref.module()->connect(endpoint,&args);
}

//=========================================================================
std::string DocRoot::lookup_mod(const std::string& name) const
{
  int bailout=100;
  std::string::size_type idot;
  std::string key=name;
  while (--bailout > 0) {

    std::map<std::string,std::string>::const_iterator it = m_mods.find(key);
    if (it != m_mods.end()) {
      return (*it).second;
    }
    
    if (key.size()<=0 || key=="*") {
      return "";
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
  DEBUG_LOG("lookup_mod() Pattern match bailout");
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
      "set_param" == name) {
    return new scx::ArgObjectFunction(new scx::ArgObject(this),name);
  }

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

    log("Mapping '" + s_pattern + "' --> " + s_module);
    m_mods[s_pattern] = s_module;

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

};
