/* SconeServer (http://www.sconemad.com)

http Request

Copyright (c) 2000-2005 Andrew Wedgbury <wedge@sconemad.com>

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


#include "http/Request.h"
#include "http/Session.h"

#include "sconex/ModuleRef.h"
#include "sconex/StreamSocket.h"
#include "sconex/File.h"
#include "sconex/Module.h"
namespace http {

//===========================================================================
Request::Request(const std::string& profile, const std::string& id)
  : m_host(0),
    m_profile(profile),
    m_id(id),
    m_docroot(0),
    m_session(0)
{
  
}

//===========================================================================
Request::~Request()
{

}

//===========================================================================
const std::string& Request::get_method() const
{
  return m_method;
}

//===========================================================================
const scx::Uri& Request::get_uri() const
{
  return m_uri;
}

//===========================================================================
const scx::VersionTag& Request::get_version() const
{
  return m_version;
}

//===========================================================================
bool Request::parse_request(const std::string& str, bool secure)
{
  std::string::size_type start = 0;
  std::string::size_type end = 0;
  end = str.find_first_of(" ");
  if (end == std::string::npos) {
    return false;
  }
  m_method = std::string(str,start,end-start);
  start = end + 1;

  start = str.find_first_not_of(" ",start);
  end = str.find_first_of(" ",start);
  if (end == std::string::npos) {
    return false;
  }
  m_uri = scx::Uri(std::string(str,start,end-start));
  m_uri.set_scheme(secure ? "https" : "http");
  start = end + 1;

  start = str.find_first_not_of(" ",start);
  end = str.find_first_of("/",start);
  if (end == std::string::npos) {
    return false;
  }
  std::string proto = std::string(str,start,end-start);
  if (proto != "HTTP") {
    return false;
  }
  start = end + 1;

  std::string sver;
  int ver_major=1;
  int ver_minor=0;
  end = str.find_first_of(".",start);
  if (end == std::string::npos) {
    return false;
  }
  sver = std::string(str,start,end-start);
  ver_major = atoi(sver.c_str());
  start = end + 1;
  
  end = str.find_first_of(" ",start);
  sver = std::string(str,start,end-start);
  ver_minor = atoi(sver.c_str());
  start = end + 1;
  
  m_version = scx::VersionTag(ver_major,ver_minor);

  return true;
}

//===========================================================================
std::string Request::get_header(const std::string& name) const
{
  return m_headers.get(name);
}

//===========================================================================
scx::MimeHeader Request::get_header_parsed(const std::string& name) const
{
  return m_headers.get_parsed(name);
}

//===========================================================================
bool Request::parse_header(const std::string& str)
{
  std::string name = m_headers.parse_line(str);
  if (name.empty()) {
    return false;
  }

  // Some special cases
  if (name == "Host" && m_uri.get_host().empty()) {
    std::string value = m_headers.get(name);
    // Split into host:port if required
    std::string::size_type colon = value.find(":");
    if (colon != std::string::npos) {
      std::string port(value,colon+1);
      m_uri.set_port(atoi(port.c_str()));
      m_uri.set_host(std::string(value,0,colon));
    } else {
      m_uri.set_host(value);
    }
  }

  return true;
}

//=============================================================================
void Request::set_host(Host* host)
{
  m_host = host;
}

//=============================================================================
const Host* Request::get_host() const
{
  return m_host;
}

//=============================================================================
const std::string& Request::get_profile() const
{
  return m_profile;
}

//=============================================================================
const std::string& Request::get_id() const
{
  return m_id;
}

//=============================================================================
void Request::set_docroot(DocRoot* docroot)
{
  m_docroot = docroot;
}

//=============================================================================
const DocRoot* Request::get_docroot() const
{
  return m_docroot;
}

//=============================================================================
void Request::set_session(Session* session)
{
  m_session = session;
}

//=============================================================================
const Session* Request::get_session() const
{
  return m_session;
}

//=============================================================================
Session* Request::get_session()
{
  return m_session;
}

//=============================================================================
void Request::set_path(const scx::FilePath& path)
{
  m_path = path;
}

//=============================================================================
const scx::FilePath& Request::get_path() const
{
  return m_path;
}

//=============================================================================
void Request::set_auth_user(const std::string& user)
{
  m_auth_user = user;
}

//=============================================================================
const std::string& Request::get_auth_user() const
{
  return m_auth_user;
}

//=============================================================================
void Request::set_path_info(const std::string& pathinfo)
{
  m_pathinfo = pathinfo;
}

//=============================================================================
const std::string& Request::get_path_info() const
{
  return m_pathinfo;
}

//=============================================================================
void Request::set_param(const std::string& name, scx::Arg* value)
{
  m_params.give(name,value);
}

//=============================================================================
void Request::set_param(const std::string& name, const std::string& value)
{
  const std::string int_pattern = "int_";
  std::string::size_type ip = name.find(int_pattern);
  if (ip == 0) {
    int i_value = atoi(value.c_str());
    set_param(name,new scx::ArgInt(i_value));
              
  } else {
    set_param(name,new scx::ArgString(value));
  }
}

//=============================================================================
std::string Request::get_param(const std::string& name) const
{
  const scx::Arg* a = m_params.lookup(name);
  if (a == 0) {
    return "";
  }
  return a->get_string();
}

//=============================================================================
bool Request::is_param(const std::string& name) const
{
  return (m_params.lookup(name) != 0);
}

//=========================================================================
scx::Arg* Request::arg_lookup(const std::string& name)
{
  // Methods
  if ("test" == name) {
    return new_method(name);
  }
  
  if (name == "auth") return new scx::ArgInt(m_auth_user != "");
  if (name == "user") return new scx::ArgString(m_auth_user);
  if (name == "method") return new scx::ArgString(m_method);
  if (name == "uri") return m_uri.new_copy();
  if (name == "version") return m_version.new_copy();
  if (name == "profile") return new scx::ArgString(m_profile);
  if (name == "id") return new scx::ArgString(m_id);
  if (name == "params") return m_params.new_copy();
  if (name == "session" && m_session) return new scx::ArgObject(m_session);

  return SCXBASE ArgObjectInterface::arg_lookup(name);
}

//=========================================================================
scx::Arg* Request::arg_method(const scx::Auth& auth,const std::string& name,scx::Arg* args)
{
  //  scx::ArgList* l = dynamic_cast<scx::ArgList*>(args);

  if (name == "test") {
    return 0;
  }

  return SCXBASE ArgObjectInterface::arg_method(auth,name,args);
}

};
