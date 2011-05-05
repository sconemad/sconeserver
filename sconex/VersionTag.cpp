/* SconeServer (http://www.sconemad.com)

Version tag

Copyright (c) 2000-2004 Andrew Wedgbury <wedge@sconemad.com>

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

#include "sconex/VersionTag.h"
#include "sconex/ScriptTypes.h"
namespace scx {

//=============================================================================
VersionTag::VersionTag(
  int major,
  int minor,
  int sub,
  const std::string& extra
) : m_major(major),
    m_minor(minor),
    m_sub(sub),
    m_extra(extra)
{
  DEBUG_COUNT_CONSTRUCTOR(VersionTag);
}

//=============================================================================
VersionTag::VersionTag(const std::string& str) 
  : m_major(-1),
    m_minor(-1),
    m_sub(-1)
{
  DEBUG_COUNT_CONSTRUCTOR(VersionTag);
  from_string(str);
}

//=============================================================================
VersionTag::VersionTag(const ScriptRef* args)
  : m_major(-1),
    m_minor(-1),
    m_sub(-1)
{
  DEBUG_COUNT_CONSTRUCTOR(VersionTag);

  const ScriptString* str = 
    get_method_arg<ScriptString>(args,0,"version");
  if (str) {
    // Set from string
    from_string(str->get_string());
    return;
  }

  // Set from numerics
  const ScriptInt* v1 = get_method_arg<ScriptInt>(args,0,"major");
  m_major = v1 ? v1->get_int() : 0;

  const ScriptInt* v2 = get_method_arg<ScriptInt>(args,1,"minor");
  m_minor = v2 ? v2->get_int() : 0;

  const ScriptInt* v3 = get_method_arg<ScriptInt>(args,2,"sub");
  m_sub = v3 ? v3->get_int() : 0;

  const ScriptString* v4 = get_method_arg<ScriptString>(args,3,"extra");
  m_extra = v4 ? v4->get_string() : "";
}

//=============================================================================
VersionTag::VersionTag(const VersionTag& c)
  : ScriptObject(c),
    m_major(c.m_major),
    m_minor(c.m_minor),
    m_sub(c.m_sub),
    m_extra(c.m_extra)
{
  DEBUG_COUNT_CONSTRUCTOR(VersionTag);
}

//=============================================================================
VersionTag::~VersionTag()
{
  DEBUG_COUNT_DESTRUCTOR(VersionTag);
}

//=============================================================================
ScriptObject* VersionTag::new_copy() const
{
  return new VersionTag(*this);
}

//=============================================================================
int VersionTag::get_major() const
{
  return m_major; 
}
 
//=============================================================================
int VersionTag::get_minor() const
{
  return m_minor;
}

//=============================================================================
int VersionTag::get_sub() const
{
  return m_sub;
}

//=============================================================================
const std::string& VersionTag::get_extra() const
{
  return m_extra;
}

//=============================================================================
std::string VersionTag::get_string() const
{
  std::ostringstream oss;
  if (m_major>=0) {
    oss << m_major;
    if (m_minor>=0) {
      oss << "." << m_minor;
      if (m_sub>=0) {
        oss << "." << m_sub;
      }
    }
  }
  if (!m_extra.empty()) {
    oss << m_extra;
  }
  return oss.str();
}

//=============================================================================
int VersionTag::get_int() const
{
  return (m_major > 0) || (m_minor > 0) || (m_sub > 0);
}

//=============================================================================
ScriptRef* VersionTag::script_op(const ScriptAuth& auth,
				 const ScriptRef& ref,
				 const ScriptOp& op,
				 const ScriptRef* right)
{
  if (right) { // binary ops
    const VersionTag* rv = 
      right ? dynamic_cast<const VersionTag*>(right->object()) : 0;
    if (rv) { // VersionTag x VersionTag ops
      switch (op.type()) {
      case ScriptOp::Equality:
	return ScriptInt::new_ref(*this == *rv);
	
      case ScriptOp::Inequality:
	return ScriptInt::new_ref(*this != *rv);
	
      case ScriptOp::GreaterThan:
	return ScriptInt::new_ref(*this > *rv);
	
      case ScriptOp::LessThan:
	return ScriptInt::new_ref(*this < *rv);
	
      case ScriptOp::GreaterThanOrEqualTo:
	return ScriptInt::new_ref(*this >= *rv);
	
      case ScriptOp::LessThanOrEqualTo:
	return ScriptInt::new_ref(*this <= *rv);
        
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
      if (name == "major") return ScriptInt::new_ref(m_major);
      if (name == "minor") return ScriptInt::new_ref(m_minor);
      if (name == "sub") return ScriptInt::new_ref(m_sub);
      if (name == "extra") return ScriptString::new_ref(m_extra);
    }
  }
  
  return ScriptObject::script_op(auth,ref,op,right);
}

//=============================================================================
VersionTag& VersionTag::operator=(const VersionTag& v)
{
  if (this != &v) {
    m_major = v.m_major;
    m_minor = v.m_minor;
    m_sub = v.m_sub;
    m_extra = v.m_extra;
  }
  return *this;
}

//=============================================================================
bool VersionTag::operator==(const VersionTag& v) const
{
  return (m_major==v.m_major &&
          m_minor==v.m_minor &&
          m_sub==v.m_sub &&
          m_extra==v.m_extra);
}

//=============================================================================
bool VersionTag::operator!=(const VersionTag& v) const
{
  return (m_major!=v.m_major ||
          m_minor!=v.m_minor ||
          m_sub!=v.m_sub ||
          m_extra!=v.m_extra);
}

//=============================================================================
bool VersionTag::operator>(const VersionTag& v) const
{
  if (m_major != v.m_major) {
    return m_major > v.m_major;
  }
  if (m_minor != v.m_minor) {
    return m_minor > v.m_minor;
  }
  return m_sub > v.m_sub;
}

//=============================================================================
bool VersionTag::operator<(const VersionTag& v) const
{
  if (m_major != v.m_major) {
    return m_major < v.m_major;
  }
  if (m_minor != v.m_minor) {
    return m_minor < v.m_minor;
  }
  return m_sub < v.m_sub;
}

//=============================================================================
bool VersionTag::operator>=(const VersionTag& v) const
{
  if (m_major != v.m_major) {
    return m_major > v.m_major;
  }
  if (m_minor != v.m_minor) {
    return m_minor > v.m_minor;
  }
  return m_sub >= v.m_sub;
}

//=============================================================================
bool VersionTag::operator<=(const VersionTag& v) const
{
  if (m_major != v.m_major) {
    return m_major < v.m_major;
  }
  if (m_minor != v.m_minor) {
    return m_minor < v.m_minor;
  }
  return m_sub <= v.m_sub;
}

//=============================================================================
void VersionTag::from_string(const std::string& str)
{
  std::string::size_type start=0;
  int i=0;
  std::string tok;
  while(start!=std::string::npos) {
    std::string::size_type end = str.find(".",start);
    tok = std::string(str,start,end-start);
    start = end + (end==std::string::npos ? 0 : 1);
    char* cend = 0;
    const char* cstart = tok.c_str();
    int value = strtol(cstart,&cend,0);
    switch (++i) {
      case 1: m_major = value; break;
      case 2: m_minor = value; break;
      case 3: m_sub   = value; break;
    }
    int extralen = tok.length() - (cend-cstart);
    if (extralen > 0) {
      m_extra = std::string(cend,extralen);
      break;
    }
  }
}

};
