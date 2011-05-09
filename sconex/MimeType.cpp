/* SconeServer (http://www.sconemad.com)

MIME Type

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

#include <sconex/MimeType.h>
#include <sconex/ScriptTypes.h>
#include <sconex/utils.h>
namespace scx {

//=============================================================================
MimeType::MimeType()
{
  DEBUG_COUNT_CONSTRUCTOR(MimeType);
}

//=============================================================================
MimeType::MimeType(const std::string& str)
{
  DEBUG_COUNT_CONSTRUCTOR(MimeType);
  from_string(str);
}

//=============================================================================
MimeType::MimeType(
  const std::string& type,
  const std::string& subtype,
  const std::string& params
) : m_type(type),
    m_subtype(subtype)
{
  DEBUG_COUNT_CONSTRUCTOR(MimeType);
  strlow(m_type);
  strlow(m_subtype);
  params_from_string(params);
}

//=============================================================================
MimeType::MimeType(const ScriptRef* args)
{
  DEBUG_COUNT_CONSTRUCTOR(MimeType);

  const ScriptString* str = get_method_arg<ScriptString>(args,0,"value");
  if (str) {
    // Set from string
    from_string(str->get_string());
  }
}

//=============================================================================
MimeType::MimeType(const MimeType& c)
  : ScriptObject(c),
    m_type(c.m_type),
    m_subtype(c.m_subtype),
    m_params(c.m_params)
{
  DEBUG_COUNT_CONSTRUCTOR(MimeType);
}

//=============================================================================
MimeType::~MimeType()
{
  DEBUG_COUNT_DESTRUCTOR(MimeType);
}
 
//=============================================================================
ScriptObject* MimeType::new_copy() const
{
  return new MimeType(*this);
}

//=============================================================================
void MimeType::set_type(const std::string& type)
{
  m_type = type;
  strlow(m_type);
}

//=============================================================================
void MimeType::set_subtype(const std::string& subtype)
{
  m_subtype = subtype;
  strlow(m_subtype);
}

//=============================================================================
void MimeType::set_param(const std::string& name, const std::string& value)
{
  std::string ci_name(name);
  strlow(ci_name);
  m_params[ci_name] = value;
}

//=============================================================================
bool MimeType::erase_param(const std::string& name)
{
  ParamMap::iterator it = m_params.find(name);
  if (it == m_params.end()) {
    return false;
  }
  m_params.erase(it);
  return true;
}

//=============================================================================
const std::string& MimeType::get_type() const
{
  return m_type;
}

//=============================================================================
const std::string& MimeType::get_subtype() const
{
  return m_subtype;
}

//=============================================================================
std::string MimeType::get_param(const std::string& name) const
{
  ParamMap::const_iterator it = m_params.find(name);
  if (it == m_params.end()) {
    return "";
  }
  return it->second;  
}

//=============================================================================
std::string MimeType::get_string() const
{
  std::ostringstream oss;
  oss << m_type << "/" << m_subtype;

  for (ParamMap::const_iterator it = m_params.begin();
       it != m_params.end();
       it++) {
    oss << "; " << it->first << "=\"" << it->second << "\"";
  }

  return oss.str();
}

//=============================================================================
int MimeType::get_int() const
{
  return (!m_type.empty() || !m_subtype.empty());
}

//=============================================================================
ScriptRef* MimeType::script_op(const ScriptAuth& auth,
			       const ScriptRef& ref,
			       const ScriptOp& op,
			       const ScriptRef* right)
{
  if (right) { // binary ops
    const MimeType* rv = dynamic_cast<const MimeType*>(right->object());
    if (rv) { // MimeType x MimeType ops
      switch (op.type()) {
      case ScriptOp::Equality:
	return ScriptInt::new_ref(*this == *rv);
      case ScriptOp::Inequality:
	return ScriptInt::new_ref(*this != *rv);
      case ScriptOp::Assign:
	if (!ref.is_const()) {
	  *this = *rv;
	}
	return ref.ref_copy();
      default: break;
      }
    }
    
    if (ScriptOp::Lookup == op.type()) { 
      std::string name = right->object()->get_string();
      if (name == "type") return ScriptString::new_ref(m_type);
      if (name == "subtype") return ScriptString::new_ref(m_subtype);
      if (name == "params") {
	ScriptMap* map = new ScriptMap();
	ScriptRef* map_ref = new ScriptRef(map);
	for (ParamMap::const_iterator it = m_params.begin();
	     it != m_params.end();
	     it++) {
	  map->give(it->first, ScriptString::new_ref(it->second));
	}
	return map_ref;
      }
    }
  }
  
  return ScriptObject::script_op(auth,ref,op,right);
}

//=============================================================================
MimeType& MimeType::operator=(const MimeType& v)
{
  m_type = v.m_type;
  m_subtype = v.m_subtype;
  m_params = v.m_params;
  return *this;
}

//=============================================================================
bool MimeType::operator==(const MimeType& v) const
{
  if (m_type != v.m_type) return false;
  if (m_subtype != v.m_subtype) return false;
  
  ParamMap::const_iterator it;
  for (it = m_params.begin();
       it != m_params.end();
       it++) {
    if (v.get_param(it->first) != it->second) return false;
  }
  
  for (it = v.m_params.begin();
       it != v.m_params.end();
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
    m_type = std::string(str,start,end-start);
    strlow(m_type);
    start = end + 1;
  }

  // Find subtype
  end = str.find(";",start);
  if (end == std::string::npos) {
    m_subtype = std::string(str,start);
    strlow(m_subtype);
    start = end;
  } else {
    m_subtype = std::string(str,start,end-start);
    strlow(m_subtype);
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
