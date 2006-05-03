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
namespace scx {

//=============================================================================
VersionTag::VersionTag(int major,int minor, int sub) 
  : m_major(major),
    m_minor(minor),
    m_sub(sub)
{
  
}

//=============================================================================
VersionTag::VersionTag(const std::string& str) 
  : m_major(0),
    m_minor(0),
    m_sub(0)
{
  from_string(str);
}

//=============================================================================
VersionTag::VersionTag(Arg* args)
  : m_major(0),
    m_minor(0),
    m_sub(0)
{
  ArgList* l = dynamic_cast<ArgList*>(args);

  const ArgString* str = dynamic_cast<const ArgString*>(l->get(0));
  if (str) {
    // Set from string
    from_string(str->get_string());
    return;
  }

  // Set from numerics
  const ArgInt* v1 = dynamic_cast<const ArgInt*>(l->get(0));
  m_major = v1 ? v1->get_int() : 0;

  const ArgInt* v2 = dynamic_cast<const ArgInt*>(l->get(1));
  m_minor = v2 ? v2->get_int() : 0;

  const ArgInt* v3 = dynamic_cast<const ArgInt*>(l->get(2));
  m_sub = v3 ? v3->get_int() : 0;
}

//=============================================================================
VersionTag::VersionTag(const VersionTag& c)
  : m_major(c.m_major),
    m_minor(c.m_minor),
    m_sub(c.m_sub)
{

}

//=============================================================================
VersionTag::~VersionTag()
{

}

//=============================================================================
Arg* VersionTag::new_copy() const
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
  return oss.str();
}

//=============================================================================
int VersionTag::get_int() const
{
  return (m_major > 0) || (m_minor > 0) || (m_sub > 0);
}

//=============================================================================
Arg* VersionTag::op(OpType optype, const std::string& opname, Arg* right)
{
  switch (optype) {
    case Arg::Binary: {
      VersionTag* rv = dynamic_cast<VersionTag*>(right);
      if (rv) {
        if ("=="==opname) { // Equality
          return new ArgInt(*this == *rv);

        } else if ("!="==opname) { // Inequality
          return new ArgInt(*this != *rv);

        } else if (">"==opname) { // Greater than
          return new ArgInt(*this > *rv);

        } else if ("<"==opname) { // Less than
          return new ArgInt(*this < *rv);

        } else if (">="==opname) { // Greater than or equal to
          return new ArgInt(*this >= *rv);

        } else if ("<="==opname) { // Less than or equal to
          return new ArgInt(*this <= *rv);
        }
      }
    } break;
    case Arg::Prefix: 
    case Arg::Postfix:
      // Don't do anything for these
      break;
  }
  
  return 0;
}

//=============================================================================
bool VersionTag::operator==(const VersionTag& v) const
{
  return m_major==v.m_major && m_minor==v.m_minor && m_sub==v.m_sub;
}

//=============================================================================
bool VersionTag::operator!=(const VersionTag& v) const
{
  return m_major!=v.m_major || m_minor!=v.m_minor || m_sub!=v.m_sub;
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
    int value = atoi(tok.c_str());
    switch (++i) {
      case 1: m_major = value; break;
      case 2: m_minor = value; break;
      case 3: m_sub   = value; break;
    }
  }
}

};
