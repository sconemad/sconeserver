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


#include <http/Request.h>
#include <http/Session.h>

#include <sconex/StreamSocket.h>
#include <sconex/File.h>
#include <sconex/Module.h>
namespace http {

//===========================================================================
Request::Request(const std::string& profile, const std::string& id)
  : m_host(0),
    m_profile(profile),
    m_id(id),
    m_docroot(0),
    m_session(0),
    m_params(new scx::ScriptMap())
{
  
}

//===========================================================================
Request::~Request()
{
  delete m_session;
}

//===========================================================================
void Request::set_method(const std::string& method)
{
  m_method = method;
}

//===========================================================================
const std::string& Request::get_method() const
{
  return m_method;
}

//===========================================================================
void Request::set_uri(const scx::Uri& uri)
{
  m_uri = uri;
}

//===========================================================================
const scx::Uri& Request::get_uri() const
{
  return m_uri;
}

//===========================================================================
void Request::set_version(const scx::VersionTag& ver)
{
  m_version = ver;
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
void Request::set_header(const std::string& name, const std::string& value)
{
  m_headers.set(name,value);
}

//===========================================================================
bool Request::remove_header(const std::string& name)
{
  return m_headers.erase(name);
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
void Request::give_session(Session::Ref* session)
{
  DEBUG_ASSERT(m_session == 0,"Request should not already have a session");
  m_session = session;
}

//=============================================================================
const Session* Request::get_session() const
{
  if (m_session) return m_session->object();
  return 0;
}

//=============================================================================
Session* Request::get_session()
{
  if (m_session) return m_session->object();
  return 0;
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
void Request::set_param(const std::string& name, scx::ScriptRef* value)
{
  m_params.object()->give(name,value);
}

//=============================================================================
void Request::set_param(const std::string& name, const std::string& value)
{
  const std::string int_pattern = "int_";
  std::string::size_type ip = name.find(int_pattern);
  if (ip == 0) {
    int i_value = atoi(value.c_str());
    set_param(name,scx::ScriptInt::new_ref(i_value));
              
  } else {
    set_param(name,scx::ScriptString::new_ref(value));
  }
}

//=============================================================================
std::string Request::get_param(const std::string& name) const
{
  const scx::ScriptRef* par = m_params.object()->lookup(name);
  if (par == 0) {
    return "";
  }
  return par->object()->get_string();
}

//=============================================================================
bool Request::is_param(const std::string& name) const
{
  return (m_params.object()->lookup(name) != 0);
}

//=========================================================================
std::string Request::build_header_string()
{
  std::string str = m_method + " " + m_uri.get_string() + 
                    " HTTP/" + m_version.get_string() + CRLF +
                    m_headers.get_all() + CRLF;
  return str;
}

//=========================================================================
std::string Request::get_string() const
{
  return "Request";
}

//=========================================================================
scx::ScriptRef* Request::script_op(const scx::ScriptAuth& auth,
				   const scx::ScriptRef& ref,
				   const scx::ScriptOp& op,
				   const scx::ScriptRef* right)
{
  if (op.type() == scx::ScriptOp::Lookup) {
    const std::string name = right->object()->get_string();
    
    // Methods
    if ("get_header" == name) {
      return new scx::ScriptMethodRef(ref,name);
    }
  
    if (name == "auth") return scx::ScriptInt::new_ref(m_auth_user != "");
    if (name == "user") return scx::ScriptString::new_ref(m_auth_user);
    if (name == "method") return scx::ScriptString::new_ref(m_method);
    if (name == "uri") return new scx::ScriptRef(m_uri.new_copy());
    if (name == "version") return new scx::ScriptRef(m_version.new_copy());
    if (name == "profile") return scx::ScriptString::new_ref(m_profile);
    if (name == "id") return scx::ScriptString::new_ref(m_id);
    if (name == "params") 
      return m_params.ref_copy(ref.reftype());
    if (name == "session" && m_session) 
      return m_session->ref_copy(ref.reftype());
  }

  return scx::ScriptObject::script_op(auth,ref,op,right);
}

//=========================================================================
scx::ScriptRef* Request::script_method(const scx::ScriptAuth& auth,
				       const scx::ScriptRef& ref,
				       const std::string& name,
				       const scx::ScriptRef* args)
{
  if (name == "get_header") {
    const scx::ScriptString* a_header = 
      scx::get_method_arg<scx::ScriptString>(args,0,"name");
    if (!a_header) 
      return scx::ScriptError::new_ref("get_header() No name specified");

    std::string value = m_headers.get(a_header->get_string());
    if (value.empty()) return 0;

    return scx::ScriptString::new_ref(value);
  }

  return scx::ScriptObject::script_method(auth,ref,name,args);
}

};
