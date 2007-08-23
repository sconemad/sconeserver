/* SconeServer (http://www.sconemad.com)

Uniform Resource Identifier

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

#include "sconex/Uri.h"
#include "sconex/utils.h"
namespace scx {

//=============================================================================
Uri::Uri()
  : m_port(0)
{

}

//=============================================================================
Uri::Uri(const std::string& str)
  : m_port(0)
{
  from_string(str);
}

//=============================================================================
Uri::Uri(
  const std::string& scheme,
  const std::string& host,
  short port,
  const std::string& path,
  const std::string& query
) : m_scheme(scheme),
    m_host(host),
    m_port(port),
    m_path(path),
    m_query(query)
{
  scx::strlow(m_scheme);
  scx::strlow(m_host);
}

//=============================================================================
Uri::Uri(Arg* args)
  : m_port(0)
{
  ArgList* l = dynamic_cast<ArgList*>(args);

  const ArgString* str = dynamic_cast<const ArgString*>(l->get(0));
  if (str) {
    // Set from string
    from_string(str->get_string());
  }
}

//=============================================================================
Uri::Uri(const Uri& c)
  : m_scheme(c.m_scheme),
    m_host(c.m_host),
    m_port(c.m_port),
    m_path(c.m_path),
    m_query(c.m_query)
{

}

//=============================================================================
Uri::~Uri()
{

}
 
//=============================================================================
Arg* Uri::new_copy() const
{
  return new Uri(*this);
}

//=============================================================================
void Uri::set_scheme(const std::string& scheme)
{
  m_scheme = scheme;
  scx::strlow(m_scheme);
}

//=============================================================================
void Uri::set_host(const std::string& host)
{
  m_host = host;
  scx::strlow(m_host);
}

//=============================================================================
void Uri::set_port(short port)
{
  m_port = port;
}

//=============================================================================
void Uri::set_path(const std::string& path)
{
  m_path = path;
}

//=============================================================================
void Uri::set_query(const std::string& query)
{
  m_query = query;
}

//=============================================================================
const std::string& Uri::get_scheme() const
{
  return m_scheme;
}

//=============================================================================
const std::string& Uri::get_host() const
{
  return m_host;
}

//=============================================================================
short Uri::get_port() const
{
  return (m_port > 0) ? m_port : default_port(m_scheme);
}

//=============================================================================
const std::string& Uri::get_path() const
{
  return m_path;
}

//=============================================================================
const std::string& Uri::get_query() const
{
  return m_query;
}

//=============================================================================
std::string Uri::get_string() const
{
  std::ostringstream oss;
  if (!m_scheme.empty()) {
    oss << m_scheme << "://";
  }
  if (!m_host.empty()) {
    oss << m_host;
  }
  if (m_port > 0) {
    oss << ":" << m_port;
  }
  if (!m_path.empty()) {
    oss << "/" << m_path;
  }
  if (!m_query.empty()) {
    oss << "?" << m_query;
  }
  return oss.str();
}

//=============================================================================
int Uri::get_int() const
{
  return (!m_host.empty() || !m_path.empty());
}

//=============================================================================
Arg* Uri::op(OpType optype, const std::string& opname, Arg* right)
{
  switch (optype) {
    case Arg::Binary: {
      Uri* rv = dynamic_cast<Uri*>(right);
      if (rv) {
        if ("=="==opname) { // Equality
          return new ArgInt(*this == *rv);

        } else if ("!="==opname) { // Inequality
          return new ArgInt(*this != *rv);
        }
      }
    } break;
    case Arg::Prefix: 
    case Arg::Postfix:
      // Don't do anything for these
      break;
  }
  
  return Arg::op(optype,opname,right);
}

//=============================================================================
bool Uri::operator==(const Uri& v) const
{
  return get_scheme() == v.get_scheme() &&
    get_host() == v.get_host() &&
    get_port() == v.get_port() &&
    get_path() == v.get_path() &&
    get_query() == v.get_query();
}

//=============================================================================
bool Uri::operator!=(const Uri& v) const
{
  return get_scheme() != v.get_scheme() ||
    get_host() != v.get_host() ||
    get_port() != v.get_port() ||
    get_path() != v.get_path() ||
    get_query() != v.get_query();
}

//=============================================================================
short Uri::default_port(const std::string& scheme)
{
  if (scheme == "http")   return 80;
  if (scheme == "https")  return 443;
  if (scheme == "ftp")    return 21;
  if (scheme == "tftp")   return 69;
  if (scheme == "ssh")    return 22;
  if (scheme == "telnet") return 23;
  if (scheme == "gopher") return 70;
  if (scheme == "smtp")   return 25;
  if (scheme == "nntp")   return 119;
  if (scheme == "pop3")   return 110;
  if (scheme == "imap")   return 143;
  // I can't think of any more

  return 0;
}

//=============================================================================
void Uri::from_string(const std::string& str)
{
  std::string::size_type start = 0;
  std::string::size_type end = 0;

  // Find scheme
  end = str.find("://",start);
  if (end != std::string::npos) {
    m_scheme = std::string(str,start,end-start);
    scx::strlow(m_scheme);
    start = end + 3;
  }

  // Find address
  end = str.find("/",start);
  if (end != std::string::npos) {
    m_host = std::string(str,start,end-start);
    start = end + 1;
  } else {
    m_host = std::string(str,start);
    start = end;
  }
  scx::strlow(m_host);
  
  // Split address into host:port
  std::string::size_type colon = m_host.find(":");
  if (colon != std::string::npos) {
    std::string port(m_host,colon+1);
    m_port = atoi(port.c_str());
    m_host = std::string(m_host,0,colon);
  }

  // Anything left
  if (end != std::string::npos) {

    // Find path
    end = str.find("?",start);
    if (end == std::string::npos) {
      m_path = std::string(str,start);
    } else {

      m_path = std::string(str,start,end-start);
      // Remainder must be query
      m_query = std::string(str,end+1);
    }

  }
}

};
