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

#include "sconex/ModuleRef.h"
#include "sconex/StreamSocket.h"
#include "sconex/File.h"
#include "sconex/Module.h"
namespace http {

//===========================================================================
Request::Request(const std::string& profile)
  : m_host(0),
    m_profile(profile),
    m_docroot(0)
{
  
}

//===========================================================================
Request::Request(const Request& c)
  : m_method(c.m_method),
    m_uri(c.m_uri),
    m_version(c.m_version),
    m_headers(c.m_headers),
    m_profile(c.m_profile)
{

}
  
//===========================================================================
Request::~Request()
{

}

//===========================================================================
scx::Arg* Request::new_copy() const
{
  return new Request(*this);
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
std::string Request::get_string() const
{
  return std::string("REQUEST(") + m_uri.get_string() + ")";
}

//=============================================================================
int Request::get_int() const
{
  return !m_uri.get_string().empty();
}

//=============================================================================
scx::Arg* Request::op(
  scx::Arg::OpType optype,
  const std::string& opname,
  scx::Arg* right
)
{
  return 0;
}

};
