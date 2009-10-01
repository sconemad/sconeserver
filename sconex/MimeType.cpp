/* SconeServer (http://www.sconemad.com)

MIME Type

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

#include "sconex/MimeType.h"
#include "sconex/utils.h"
namespace scx {

//=============================================================================
MimeType::MimeType()
  : m_type(new std::string()),
    m_subtype(new std::string()),
    m_params(new ParamMap())
{

}

//=============================================================================
MimeType::MimeType(const std::string& str)
  : m_type(new std::string()),
    m_subtype(new std::string()),
    m_params(new ParamMap())
{
  from_string(str);
}

//=============================================================================
MimeType::MimeType(
  const std::string& type,
  const std::string& subtype,
  const std::string& params
) : m_type(new std::string(type)),
    m_subtype(new std::string(subtype)),
    m_params(new ParamMap())
{
  strlow(*m_type);
  strlow(*m_subtype);
  params_from_string(params);
}

//=============================================================================
MimeType::MimeType(Arg* args)
  : m_type(new std::string()),
    m_subtype(new std::string()),
    m_params(new ParamMap())
{
  ArgList* l = dynamic_cast<ArgList*>(args);

  const ArgString* str = dynamic_cast<const ArgString*>(l->get(0));
  if (str) {
    // Set from string
    from_string(str->get_string());
  }
}

//=============================================================================
MimeType::MimeType(const MimeType& c)
  : Arg(c),
    m_type(new std::string(*c.m_type)),
    m_subtype(new std::string(*c.m_subtype)),
    m_params(new ParamMap(*c.m_params))
{

}

//=============================================================================
MimeType::MimeType(RefType ref, MimeType& c)
  : Arg(ref,c),
    m_type(c.m_type),
    m_subtype(c.m_subtype),
    m_params(c.m_params)
{

}

//=============================================================================
MimeType::~MimeType()
{
  if (*m_refs == 1) {
    delete m_type;
    delete m_subtype;
    delete m_params;
  }
}
 
//=============================================================================
Arg* MimeType::new_copy() const
{
  return new MimeType(*this);
}

//=============================================================================
Arg* MimeType::ref_copy(RefType ref)
{
  return new MimeType(ref,*this);
}

//=============================================================================
void MimeType::set_type(const std::string& type)
{
  *m_type = type;
  strlow(*m_type);
}

//=============================================================================
void MimeType::set_subtype(const std::string& subtype)
{
  *m_subtype = subtype;
  strlow(*m_subtype);
}

//=============================================================================
void MimeType::set_param(const std::string& name, const std::string& value)
{
  std::string ci_name(name);
  strlow(ci_name);
  (*m_params)[ci_name] = value;
}

//=============================================================================
bool MimeType::erase_param(const std::string& name)
{
  ParamMap::iterator it = m_params->find(name);
  if (it == m_params->end()) {
    return false;
  }
  m_params->erase(it);
  return true;
}

//=============================================================================
const std::string& MimeType::get_type() const
{
  return *m_type;
}

//=============================================================================
const std::string& MimeType::get_subtype() const
{
  return *m_subtype;
}

//=============================================================================
std::string MimeType::get_param(const std::string& name) const
{
  ParamMap::const_iterator it = m_params->find(name);
  if (it == m_params->end()) {
    return "";
  }
  return it->second;  
}

//=============================================================================
std::string MimeType::get_string() const
{
  std::ostringstream oss;
  oss << *m_type << "/" << *m_subtype;

  for (ParamMap::const_iterator it = m_params->begin();
       it != m_params->end();
       it++) {
    oss << "; " << it->first << "=\"" << it->second << "\"";
  }

  return oss.str();
}

//=============================================================================
int MimeType::get_int() const
{
  return (!m_type->empty() || !m_subtype->empty());
}

//=============================================================================
Arg* MimeType::op(const Auth& auth, OpType optype, const std::string& opname, Arg* right)
{
  switch (optype) {
    case Arg::Binary: {
      MimeType* rv = dynamic_cast<MimeType*>(right);
      if (rv) {
        if ("=="==opname) { // Equality
          return new ArgInt(*this == *rv);

        } else if ("!="==opname) { // Inequality
          return new ArgInt(*this != *rv);
	}
      }

      if ("." == opname) { // Scope resolution
	std::string name = right->get_string();
	if (name == "type") return new scx::ArgString(*m_type);
	if (name == "subtype") return new scx::ArgString(*m_subtype);
	if (name == "params") {
	  ArgMap* map = new ArgMap();
	  for (ParamMap::const_iterator it = m_params->begin();
	       it != m_params->end();
	       it++) {
	    map->give(it->first, new ArgString(it->second));
	  }
	  return map;
        }
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
MimeType& MimeType::operator=(const MimeType& v)
{
  *m_type = *v.m_type;
  *m_subtype = *v.m_subtype;
  *m_params = *v.m_params;
  return *this;
}

//=============================================================================
bool MimeType::operator==(const MimeType& v) const
{
  if (*m_type != *v.m_type) return false;
  if (*m_subtype != *v.m_subtype) return false;
  
  ParamMap::const_iterator it;
  for (it = m_params->begin();
       it != m_params->end();
       it++) {
    if (v.get_param(it->first) != it->second) return false;
  }
  
  for (it = v.m_params->begin();
       it != v.m_params->end();
       it++) {
    if (get_param(it->first) != it->second) return false;
  }
  
  return true;
}

//=============================================================================
bool MimeType::operator!=(const MimeType& v) const
{
  return !(*this == v);
}

//=============================================================================
void MimeType::from_string(const std::string& str)
{
  std::string::size_type start = 0;
  std::string::size_type end = 0;

  // Find type
  end = str.find("/",start);
  if (end != std::string::npos) {
    *m_type = std::string(str,start,end-start);
    strlow(*m_type);
    start = end + 1;
  }

  // Find subtype
  end = str.find(";",start);
  if (end == std::string::npos) {
    *m_subtype = std::string(str,start);
    strlow(*m_subtype);
    start = end;
  } else {
    *m_subtype = std::string(str,start,end-start);
    strlow(*m_subtype);
    start = end + 1;
    params_from_string(std::string(str,start));
  }
}

//=============================================================================
void MimeType::params_from_string(const std::string& str)
{
  std::string::size_type start = 0;
  std::string::size_type end = 0;

  while (start != std::string::npos) {
    std::string name;
    end = str.find("=",start);
    if (end != std::string::npos) {
      name = std::string(str,start,end-start);
      start = end + 1;
    } else {
      name = std::string(str,start);
      start = std::string::npos;
    }
    strlow(name);

    //TODO: Cope with values in quotes

    std::string value;
    end = str.find(";",start);
    if (end != std::string::npos) {
      value = std::string(str,start,end-start);
      start = end + 1;
    } else {
      value = std::string(str,start);
      start = std::string::npos;
    }

    set_param(name,value);
  }
}

};
