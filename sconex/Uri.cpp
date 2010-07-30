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
  : m_scheme(new std::string()),
    m_host(new std::string()),
    m_port(new short(0)),
    m_path(new std::string()),
    m_query(new std::string())
{
  DEBUG_COUNT_CONSTRUCTOR(Uri);
}

//=============================================================================
Uri::Uri(const std::string& str)
  : m_scheme(new std::string()),
    m_host(new std::string()),
    m_port(new short(0)),
    m_path(new std::string()),
    m_query(new std::string())
{
  DEBUG_COUNT_CONSTRUCTOR(Uri);
  from_string(str);
}

//=============================================================================
Uri::Uri(
  const std::string& scheme,
  const std::string& host,
  short port,
  const std::string& path,
  const std::string& query
) : m_scheme(new std::string(scheme)),
    m_host(new std::string(host)),
    m_port(new short(port)),
    m_path(new std::string(path)),
    m_query(new std::string(query))
{
  DEBUG_COUNT_CONSTRUCTOR(Uri);
  scx::strlow(*m_scheme);
  scx::strlow(*m_host);
}

//=============================================================================
Uri::Uri(Arg* args)
  : m_scheme(new std::string()),
    m_host(new std::string()),
    m_port(new short(0)),
    m_path(new std::string()),
    m_query(new std::string())
{
  DEBUG_COUNT_CONSTRUCTOR(Uri);
  ArgList* l = dynamic_cast<ArgList*>(args);

  const ArgString* str = dynamic_cast<const ArgString*>(l->get(0));
  if (str) {
    // Set from string
    from_string(str->get_string());
  }
}

//=============================================================================
Uri::Uri(const Uri& c)
  : Arg(c),
    m_scheme(new std::string(*c.m_scheme)),
    m_host(new std::string(*c.m_host)),
    m_port(new short(*c.m_port)),
    m_path(new std::string(*c.m_path)),
    m_query(new std::string(*c.m_query))
{
  DEBUG_COUNT_CONSTRUCTOR(Uri);
}

//=============================================================================
Uri::Uri(RefType ref, Uri& c)
  : Arg(ref,c),
    m_scheme(c.m_scheme),
    m_host(c.m_host),
    m_port(c.m_port),
    m_path(c.m_path),
    m_query(c.m_query)
{
  DEBUG_COUNT_CONSTRUCTOR(Uri);
}

//=============================================================================
Uri::~Uri()
{
  if (last_ref()) {
    delete m_scheme;
    delete m_host;
    delete m_port;
    delete m_path;
    delete m_query;
  }
  DEBUG_COUNT_DESTRUCTOR(Uri);
}
 
//=============================================================================
Arg* Uri::new_copy() const
{
  return new Uri(*this);
}

//=============================================================================
Arg* Uri::ref_copy(RefType ref)
{
  return new Uri(ref,*this);
}

//=============================================================================
void Uri::set_scheme(const std::string& scheme)
{
  *m_scheme = scheme;
  scx::strlow(*m_scheme);
}

//=============================================================================
void Uri::set_host(const std::string& host)
{
  *m_host = host;
  scx::strlow(*m_host);
}

//=============================================================================
void Uri::set_port(short port)
{
  *m_port = port;
}

//=============================================================================
void Uri::set_path(const std::string& path)
{
  *m_path = path;
}

//=============================================================================
void Uri::set_query(const std::string& query)
{
  *m_query = query;
}

//=============================================================================
const std::string& Uri::get_scheme() const
{
  return *m_scheme;
}

//=============================================================================
const std::string& Uri::get_host() const
{
  return *m_host;
}

//=============================================================================
short Uri::get_port() const
{
  return (*m_port > 0) ? *m_port : default_port(*m_scheme);
}

//=============================================================================
const std::string& Uri::get_path() const
{
  return *m_path;
}

//=============================================================================
const std::string& Uri::get_query() const
{
  return *m_query;
}

//=============================================================================
std::string Uri::get_base() const
{
  std::ostringstream oss;
  if (!m_scheme->empty()) {
    oss << *m_scheme << "://";
  }
  if (!m_host->empty()) {
    oss << *m_host;
  }
  if (*m_port > 0) {
    oss << ":" << *m_port;
  }
  return oss.str();
} 

//=============================================================================
std::string Uri::get_string() const
{
  std::ostringstream oss;
  oss << get_base();

  if (!m_path->empty()) {
    oss << "/" << *m_path;
  }
  if (!m_query->empty()) {
    if (m_path->empty()) {
      oss << "/";
    }
    oss << "?" << *m_query;
  }
  return oss.str();
}

//=============================================================================
int Uri::get_int() const
{
  return (!m_host->empty() || !m_path->empty());
}

//=============================================================================
Arg* Uri::op(const Auth& auth, OpType optype, const std::string& opname, Arg* right)
{
  switch (optype) {
    case Arg::Binary: {
      if ("==" == opname) { // Equality
	Uri* rv = dynamic_cast<Uri*>(right);
	if (rv) return new ArgInt(*this == *rv);
	
      } else if ("!=" == opname) { // Inequality
	Uri* rv = dynamic_cast<Uri*>(right);
	if (rv) return new ArgInt(*this != *rv);

      } else if ("="==opname) { // Assignment
	Uri* rv = dynamic_cast<Uri*>(right);
        if (!is_const()) {
          if (rv) {
            *this = *rv;
          }
        }
        return ref_copy(Ref);
        
      } else if ("." == opname) { // Scope resolution
	std::string name = right->get_string();
	if (name == "scheme") return new scx::ArgString(get_scheme());
	if (name == "host") return new scx::ArgString(get_host());
	if (name == "port") return new scx::ArgInt(get_port());
	if (name == "path") return new scx::ArgString(get_path());
	if (name == "query") return new scx::ArgString(get_query());
	if (name == "base") return new scx::ArgString(get_base());
      }
    } break;
    case Arg::Prefix: 
    case Arg::Postfix:
      // Don't do anything for these
      break;
  }
  
  return Arg::op(auth,optype,opname,right);
}

//=============================================================================
Uri& Uri::operator=(const Uri& v)
{
  *m_scheme = *v.m_scheme;
  *m_host = *v.m_host;
  *m_port = *v.m_port;
  *m_path = *v.m_path;
  *m_query = *v.m_query;
  return *this;
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
std::string Uri::encode(const std::string& str)
{
  std::string ret;
  for (int i=0; i<str.size(); ++i) {
    char c = str[i];
    switch (c) {
      //TODO: This needs improving somewhat!
      case ' ':
        ret += "%20";
        break;
      
      default:
        ret += c;
        break;
    }
  }
  
  return ret;
}

//=============================================================================
std::string Uri::decode(const std::string& str)
{
  std::string ret;
  std::string::size_type start = 0;
  std::string::size_type end;

  while (true) {
    end = str.find_first_of("%+",start);
    if (end == std::string::npos) {
      ret += str.substr(start);
      break;

    } else {
      ret += str.substr(start,end-start);
    }

    if (str[end] == '+') {
      ret += " ";
      start = end + 1;

    } else if (str[end] == '%') {
      if (end+2 >= str.length()) {
	break;
      }
      start = end + 3;
      char c = 0;
      
      char a = str[end+1];
      if (a >= '0' && a <= '9') c += (a - '0');
      else if (a >= 'A' && a <= 'F') c += (10 + a - 'A');
      else if (a >= 'a' && a <= 'f') c += (10 + a - 'a');
      else continue;
      c = (c << 4);
      
      a = str[end+2];
      if (a >= '0' && a <= '9') c += (a - '0');
      else if (a >= 'A' && a <= 'F') c += (10 + a - 'A');
      else if (a >= 'a' && a <= 'f') c += (10 + a - 'a');
      else continue;
      
      char cs[2] = {c,'\0'};
      ret += cs;
    }
  }
  return ret;
}

//=============================================================================
void Uri::from_string(const std::string& str)
{
  std::string::size_type start = 0;
  std::string::size_type end = 0;

  // Find scheme
  end = str.find("://",start);
  if (end != std::string::npos) {
    *m_scheme = std::string(str,start,end-start);
    scx::strlow(*m_scheme);
    start = end + 3;
  }

  // Find address
  end = str.find("/",start);
  if (end != std::string::npos) {
    *m_host = std::string(str,start,end-start);
    start = end + 1;
  } else {
    *m_host = std::string(str,start);
    start = end;
  }
  scx::strlow(*m_host);
  
  // Split address into host:port
  std::string::size_type colon = m_host->find(":");
  if (colon != std::string::npos) {
    std::string port(*m_host,colon+1);
    *m_port = atoi(port.c_str());
    *m_host = std::string(*m_host,0,colon);
  }

  // Anything left
  if (end != std::string::npos) {

    // Find path
    end = str.find("?",start);
    if (end == std::string::npos) {
      *m_path = std::string(str,start);
    } else {

      *m_path = std::string(str,start,end-start);
      // Remainder must be query
      *m_query = std::string(str,end+1);
    }

  }
}

};
