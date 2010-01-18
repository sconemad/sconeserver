/* SconeServer (http://www.sconemad.com)

Regular Expression

Copyright (c) 2000-2010 Andrew Wedgbury <wedge@sconemad.com>

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

#include "sconex/RegExp.h"
namespace scx {

//=============================================================================
RegExp::RegExp(
  const std::string& pattern
) : m_pattern(0),
    m_pcre(0)
{
  DEBUG_COUNT_CONSTRUCTOR(RegExp);
  from_string(*m_pattern);
}

//=============================================================================
RegExp::RegExp(Arg* args)
  : m_pattern(0),
    m_pcre(0)
{
  DEBUG_COUNT_CONSTRUCTOR(RegExp);
  ArgList* l = dynamic_cast<ArgList*>(args);

  const ArgString* str = dynamic_cast<const ArgString*>(l->get(0));
  if (str) {
    // Set from string
    from_string(str->get_string());
    return;
  }
}

//=============================================================================
RegExp::RegExp(const RegExp& c)
  : Arg(c),
    m_pattern(new std::string(*c.m_pattern)),
    m_pcre(c.m_pcre)
{
  if (m_pcre) {
    ::pcre_refcount(m_pcre,+1);
  }
  DEBUG_COUNT_CONSTRUCTOR(RegExp);
}

//=============================================================================
RegExp::RegExp(RefType ref, RegExp& c)
  : Arg(ref,c),
    m_pattern(c.m_pattern),
    m_pcre(c.m_pcre)
{
  DEBUG_COUNT_CONSTRUCTOR(RegExp);
}

//=============================================================================
RegExp::~RegExp()
{
  if (last_ref()) {
    delete m_pattern;
    if (m_pcre && 0 == ::pcre_refcount(m_pcre,-1)) {
      ::pcre_free(m_pcre);
    }
  }
  DEBUG_COUNT_DESTRUCTOR(RegExp);
}

//=============================================================================
Arg* RegExp::new_copy() const
{
  return new RegExp(*this);
}

//=============================================================================
Arg* RegExp::ref_copy(RefType ref)
{
  return new RegExp(ref,*this);
}

//=============================================================================
std::string RegExp::get_string() const
{
  return *m_pattern;
}

//=============================================================================
int RegExp::get_int() const
{
  return (m_pcre != 0);
}

//=============================================================================
Arg* RegExp::op(const Auth& auth, OpType optype, const std::string& opname, Arg* right)
{
  if (is_method_call(optype,opname)) {

    ArgList* l = dynamic_cast<ArgList*>(right);
    
    if ("match" == m_method) {
      Arg* a_string = l->get(0);
      if (!a_string) return new ArgError("No string specified");
      if (!m_pcre) return new ArgError("Invalid RegExp pattern");

      int ovector[30];
      std::string subject = a_string->get_string();
      int ret = ::pcre_exec(
        m_pcre,
        0,
        subject.c_str(),
        subject.length(),
        0,
        0,
        ovector,
        30
      );

      ArgList* list = new ArgList();
      if (ret > 0) {
	for (int i=0; i<ret; ++i) {
	  int o = (i*2);
	  std::string str(subject,ovector[o],ovector[o+1]-ovector[o]);
	  list->give(new ArgString(str));
	}
      }
      return list;
    }

  } else if (optype == Arg::Binary) {

    RegExp* rv = dynamic_cast<RegExp*>(right);
    if (rv) {
      if ("=="==opname) { // Equality
        return new ArgInt(*this == *rv);

      } else if ("!="==opname) { // Inequality
        return new ArgInt(*this != *rv);
        
      }
    }
    
    if ("." == opname) { // Scope resolution
      std::string name = right->get_string();
      if (name == "pattern") return new ArgString(*m_pattern);

      if (name == "match") return new_method(name);
    }
  }
  
  return Arg::op(auth,optype,opname,right);
}

//=============================================================================
bool RegExp::operator==(const RegExp& v) const
{
  return (*m_pattern == *v.m_pattern);
}

//=============================================================================
bool RegExp::operator!=(const RegExp& v) const
{
  return (*m_pattern != *v.m_pattern);
}

//=============================================================================
void RegExp::from_string(const std::string& str)
{
  const char* error_str = 0;
  int error_offs = 0;
  m_pcre = ::pcre_compile(
    str.c_str(),
    0,
    &error_str,
    &error_offs,
    0
  );
  if (m_pcre) {
    m_pattern = new std::string(str);
  }
}

};
