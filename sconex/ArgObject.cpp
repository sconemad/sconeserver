/* SconeServer (http://www.sconemad.com)

Argument object

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

#include "sconex/ArgObject.h"
#include "sconex/ArgProc.h"

namespace scx {

//=============================================================================
ArgObjectInterface::ArgObjectInterface()
{

}
  
//=============================================================================
ArgObjectInterface::~ArgObjectInterface()
{

}

//=============================================================================
void ArgObjectInterface::log(
  const std::string& message,
  Logger::Level level
)
{
  ArgObject ctx(this);
  ArgProc proc(&ctx);
  std::ostringstream oss;
  oss << "log(\"{" << name() << "} " << message << "\"," << (int)level << ")";
  delete proc.evaluate(oss.str());
}

//=============================================================================
Arg* ArgObjectInterface::arg_lookup(const std::string& name)
{
  return new ArgError("Unknown name '" + name + "'");
}

//=============================================================================
Arg* ArgObjectInterface::arg_resolve(const std::string& name)
{
  return arg_lookup(name);
}

//=============================================================================
Arg* ArgObjectInterface::arg_function(const std::string& name, Arg* args)
{
  return new ArgError("Unknown function '" + name + "'");  
}

//=============================================================================
ArgObject::ArgObject(
  ArgObjectInterface* obj
)
  : m_obj(obj)
{

}

//=============================================================================
ArgObject::ArgObject(const ArgObject& c)
  : m_obj(c.m_obj)
{
}

//=============================================================================
ArgObject::~ArgObject()
{

}

//=============================================================================
Arg* ArgObject::new_copy() const
{
  return new ArgObject(*this);
}

//=============================================================================
std::string ArgObject::get_string() const
{
  std::ostringstream oss;
  oss << "OBJECT: "
      << (m_obj ? m_obj->name() : "NULL");

  return oss.str();
}

//===========================================================================
int ArgObject::get_int() const
{
  return (m_obj != 0);
}

//=============================================================================
Arg* ArgObject::op(OpType optype, const std::string& opname, Arg* right)
{
  if (m_obj) {
    if (Arg::Binary == optype) {
      // Lookup
      if ("." == opname) {
        return m_obj->arg_lookup(right->get_string());
      }
      // Resolve
      if (":" == opname) {
        return m_obj->arg_resolve(right->get_string());        
      }
    }
  }
  
  return 0;
}

//=============================================================================
void ArgObject::log(const std::string& message,Logger::Level level)
{
  if (m_obj) {
    m_obj->log(message,level);
  }
}

//=============================================================================
ArgObjectInterface* ArgObject::get_object()
{
  return m_obj;
}

//=============================================================================
ArgObjectFunction::ArgObjectFunction(
  ArgObject* obj,
  const std::string& name
)
  : m_obj(obj),
    m_name(name)
{

}

//=============================================================================
ArgObjectFunction::ArgObjectFunction(const ArgObjectFunction& c)
  : m_obj(c.m_obj),
    m_name(c.m_name)
{
}

//=============================================================================
ArgObjectFunction::~ArgObjectFunction()
{
  delete m_obj;
}

//=============================================================================
Arg* ArgObjectFunction::new_copy() const
{
  return new ArgObjectFunction(*this);
}

//=============================================================================
std::string ArgObjectFunction::get_string() const
{
  std::ostringstream oss;
  ArgObjectInterface* obj = m_obj->get_object();
  oss << "METHOD: "
      << (obj ? obj->name() : "NULL")
      << "::" << m_name;

  return oss.str();
}

//===========================================================================
int ArgObjectFunction::get_int() const
{
  return (m_obj != 0) && m_obj->get_int() && !m_name.empty();
}

//=============================================================================
Arg* ArgObjectFunction::op(
  OpType optype,
  const std::string& opname,
  Arg* right
)
{
  ArgObjectInterface* obj = m_obj->get_object();
  if (obj) {
    // Only allow binary ( operator - object method call
    if (Arg::Binary == optype &&
        "(" == opname) {
      return obj->arg_function(m_name,right);
    }
  }
  
  return 0;
}


};
