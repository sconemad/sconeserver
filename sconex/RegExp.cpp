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

#include <sconex/RegExp.h>
#include <sconex/ScriptTypes.h>

#ifdef HAVE_LIBPCRE

namespace scx {

//=============================================================================
RegExp::RegExp(
  const std::string& pattern
) : m_pcre(0)
{
  DEBUG_COUNT_CONSTRUCTOR(RegExp);
  from_string(m_pattern);
}

//=============================================================================
RegExp::RegExp(const ScriptRef* args)
  : m_pcre(0)
{
  DEBUG_COUNT_CONSTRUCTOR(RegExp);

  const ScriptString* str = get_method_arg<ScriptString>(args,0,"pattern");
  if (str) {
    // Set from string
    from_string(str->get_string());
    return;
  }
}

//=============================================================================
RegExp::RegExp(const RegExp& c)
  : ScriptObject(c),
    m_pattern(c.m_pattern),
    m_pcre(c.m_pcre)
{
  if (m_pcre) {
    ::pcre_refcount(m_pcre,+1);
  }
  DEBUG_COUNT_CONSTRUCTOR(RegExp);
}

//=============================================================================
RegExp::~RegExp()
{
  if (m_pcre && 0 == ::pcre_refcount(m_pcre,-1)) {
    ::pcre_free(m_pcre);
  }
  DEBUG_COUNT_DESTRUCTOR(RegExp);
}

//=============================================================================
ScriptObject* RegExp::new_copy() const
{
  return new RegExp(*this);
}

//=============================================================================
std::string RegExp::get_string() const
{
  return m_pattern;
}

//=============================================================================
int RegExp::get_int() const
{
  return (m_pcre != 0);
}

//=============================================================================
ScriptRef* RegExp::script_op(const ScriptAuth& auth,
			     const ScriptRef& ref,
			     const ScriptOp& op,
			     const ScriptRef* right)
{
  if (right) { // binary ops
    const RegExp* rv = dynamic_cast<const RegExp*>(right->object());
    if (rv) { // RegExp x RegExp ops
      switch (op.type()) {
      case ScriptOp::Equality:
        return ScriptInt::new_ref(*this == *rv);
      case ScriptOp::Inequality:
        return ScriptInt::new_ref(*this != *rv);
      default: break;
      }
    }
    
    if (ScriptOp::Lookup == op.type()) {
      std::string name = right->object()->get_string();
      if (name == "pattern") return ScriptString::new_ref(m_pattern);

      if (name == "match") 
	return new ScriptMethodRef(ref,name);
    }
  }
  
  return ScriptObject::script_op(auth,ref,op,right);
}

//=============================================================================
ScriptRef* RegExp::script_method(const ScriptAuth& auth,
				 const ScriptRef& ref,
				 const std::string& name,
				 const ScriptRef* args)
{
  if ("match" == name) {
    if (!m_pcre) return ScriptError::new_ref("Invalid RegExp pattern");

    const ScriptString* a_string = 
      get_method_arg<ScriptString>(args,0,"string");
    if (!a_string) return ScriptError::new_ref("No string specified");
    
    int ovector[30];
    std::string subject = a_string->get_string();
    int ret = ::pcre_exec(m_pcre,
			  0,
			  subject.c_str(),
			  subject.length(),
			  0,
			  0,
			  ovector,
			  30);
    
    ScriptList* list = new ScriptList();
    ScriptRef* list_ref = new ScriptRef(list);
    if (ret > 0) {
      for (int i=0; i<ret; ++i) {
	int o = (i*2);
	std::string str(subject,ovector[o],ovector[o+1]-ovector[o]);
	list->give(ScriptString::new_ref(str));
      }
    }
    return list_ref;
  }

  return ScriptObject::script_method(auth,ref,name,args);
}

//=============================================================================
bool RegExp::operator==(const RegExp& v) const
{
  return (m_pattern == v.m_pattern);
}

//=============================================================================
bool RegExp::operator!=(const RegExp& v) const
{
  return (m_pattern != v.m_pattern);
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
    ::pcre_refcount(m_pcre,+1);
    m_pattern = str;
  }
}

};

#endif
