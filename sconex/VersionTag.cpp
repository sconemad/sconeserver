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
VersionTag::VersionTag(
  int major,
  int minor,
  int sub,
  const std::string& extra
) : m_major(new int(major)),
    m_minor(new int(minor)),
    m_sub(new int(sub)),
    m_extra(new std::string(extra))
{
  
}

//=============================================================================
VersionTag::VersionTag(const std::string& str) 
  : m_major(new int(-1)),
    m_minor(new int(-1)),
    m_sub(new int(-1)),
    m_extra(new std::string())
{
  from_string(str);
}

//=============================================================================
VersionTag::VersionTag(Arg* args)
  : m_major(new int(-1)),
    m_minor(new int(-1)),
    m_sub(new int(-1)),
    m_extra(new std::string())
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
  *m_major = v1 ? v1->get_int() : 0;

  const ArgInt* v2 = dynamic_cast<const ArgInt*>(l->get(1));
  *m_minor = v2 ? v2->get_int() : 0;

  const ArgInt* v3 = dynamic_cast<const ArgInt*>(l->get(2));
  *m_sub = v3 ? v3->get_int() : 0;

  const ArgString* v4 = dynamic_cast<const ArgString*>(l->get(3));
  *m_extra = v4 ? v4->get_string() : "";
}

//=============================================================================
VersionTag::VersionTag(const VersionTag& c)
  : Arg(c),
    m_major(new int(*c.m_major)),
    m_minor(new int(*c.m_minor)),
    m_sub(new int(*c.m_sub)),
    m_extra(new std::string(*c.m_extra))
{

}

//=============================================================================
VersionTag::VersionTag(RefType ref, VersionTag& c)
  : Arg(ref,c),
    m_major(c.m_major),
    m_minor(c.m_minor),
    m_sub(c.m_sub),
    m_extra(c.m_extra)
{

}

//=============================================================================
VersionTag::~VersionTag()
{
  if (last_ref()) {
    delete m_major;
    delete m_minor;
    delete m_sub;
    delete m_extra;
  }
}

//=============================================================================
Arg* VersionTag::new_copy() const
{
  return new VersionTag(*this);
}

//=============================================================================
Arg* VersionTag::ref_copy(RefType ref)
{
  return new VersionTag(ref,*this);
}

//=============================================================================
int VersionTag::get_major() const
{
  return *m_major; 
}
 
//=============================================================================
int VersionTag::get_minor() const
{
  return *m_minor;
}

//=============================================================================
int VersionTag::get_sub() const
{
  return *m_sub;
}

//=============================================================================
const std::string& VersionTag::get_extra() const
{
  return *m_extra;
}

//=============================================================================
std::string VersionTag::get_string() const
{
  std::ostringstream oss;
  if (*m_major>=0) {
    oss << *m_major;
    if (*m_minor>=0) {
      oss << "." << *m_minor;
      if (m_sub>=0) {
        oss << "." << *m_sub;
      }
    }
  }
  if (!m_extra->empty()) {
    oss << *m_extra;
  }
  return oss.str();
}

//=============================================================================
int VersionTag::get_int() const
{
  return (*m_major > 0) || (*m_minor > 0) || (*m_sub > 0);
}

//=============================================================================
Arg* VersionTag::op(const Auth& auth, OpType optype, const std::string& opname, Arg* right)
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

      if ("." == opname) { // Scope resolution
	std::string name = right->get_string();
	if (name == "major") return new scx::ArgInt(*m_major);
	if (name == "minor") return new scx::ArgInt(*m_minor);
	if (name == "sub") return new scx::ArgInt(*m_sub);
	if (name == "extra") return new scx::ArgString(*m_extra);
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
VersionTag& VersionTag::operator=(const VersionTag& v)
{
  *m_major = *v.m_major;
  *m_minor = *v.m_minor;
  *m_sub = *v.m_sub;
  *m_extra = *v.m_extra;
  return *this;
}

//=============================================================================
bool VersionTag::operator==(const VersionTag& v) const
{
  return (*m_major==*v.m_major &&
          *m_minor==*v.m_minor &&
          *m_sub==*v.m_sub &&
          *m_extra==*v.m_extra);
}

//=============================================================================
bool VersionTag::operator!=(const VersionTag& v) const
{
  return (*m_major!=*v.m_major ||
          *m_minor!=*v.m_minor ||
          *m_sub!=*v.m_sub ||
          *m_extra!=*v.m_extra);
}

//=============================================================================
bool VersionTag::operator>(const VersionTag& v) const
{
  if (*m_major != *v.m_major) {
    return *m_major > *v.m_major;
  }
  if (*m_minor != *v.m_minor) {
    return *m_minor > *v.m_minor;
  }
  return *m_sub > *v.m_sub;
}

//=============================================================================
bool VersionTag::operator<(const VersionTag& v) const
{
  if (*m_major != *v.m_major) {
    return *m_major < *v.m_major;
  }
  if (*m_minor != *v.m_minor) {
    return *m_minor < *v.m_minor;
  }
  return *m_sub < *v.m_sub;
}

//=============================================================================
bool VersionTag::operator>=(const VersionTag& v) const
{
  if (*m_major != *v.m_major) {
    return *m_major > *v.m_major;
  }
  if (*m_minor != *v.m_minor) {
    return *m_minor > *v.m_minor;
  }
  return *m_sub >= *v.m_sub;
}

//=============================================================================
bool VersionTag::operator<=(const VersionTag& v) const
{
  if (*m_major != *v.m_major) {
    return *m_major < *v.m_major;
  }
  if (*m_minor != *v.m_minor) {
    return *m_minor < *v.m_minor;
  }
  return *m_sub <= *v.m_sub;
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
      case 1: *m_major = value; break;
      case 2: *m_minor = value; break;
      case 3: *m_sub   = value; break;
    }
    int extralen = tok.length() - (cend-cstart);
    if (extralen > 0) {
      *m_extra = std::string(cend,extralen);
      break;
    }
  }
}

};
