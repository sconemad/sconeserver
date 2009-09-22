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
#include "sconex/VersionTag.h"
#include "sconex/Uri.h"
#include "sconex/MimeType.h"

namespace scx {

Mutex* ArgObjectInterface::m_ref_mutex = 0;

//=============================================================================
ArgObjectInterface::ArgObjectInterface()
  : m_refs(0)
{
  if (!m_ref_mutex) {
    m_ref_mutex = new Mutex();
  }
}
  
//=============================================================================
ArgObjectInterface::~ArgObjectInterface()
{
  MutexLocker locker(*m_ref_mutex);
  DEBUG_ASSERT(m_refs==0,"Destroying object which has refs");
}

//=============================================================================
void ArgObjectInterface::log(
  const std::string& message,
  Logger::Level level
)
{
  Arg* a_logger = arg_resolve("log");
  if (a_logger) {

    ArgObjectFunction* logger = dynamic_cast<ArgObjectFunction*>(a_logger);
    if (logger) {
      ArgList args;
      args.give( new ArgString(message) );
      args.give( new ArgInt((int)level) );
      
      Arg* ret = logger->call(&args);
      delete ret;
    }
    delete a_logger;
  }
}

//=============================================================================
Arg* ArgObjectInterface::arg_lookup(const std::string& name)
{
  if ("Version" == name ||
      "Date" == name ||
      "Time" == name ||
      "Uri" == name ||
      "MimeType" == name) {
    return new ArgObjectFunction(new ArgObject(this),name);
  }

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
  if ("Version" == name) {
    return new VersionTag(args);
  }
  if ("Date" == name) {
    return new Date(args);
  }
  if ("Time" == name) {
    return new Time(args);
  }
  if ("Uri" == name) {
    return new Uri(args);
  }
  if ("MimeType" == name) {
    return new MimeType(args);
  }

  return new ArgError("Unknown function '" + name + "'");  
}

//=============================================================================
int ArgObjectInterface::get_num_refs() const
{
  return m_refs;
}

//=============================================================================
void ArgObjectInterface::add_ref()
{
  MutexLocker locker(*m_ref_mutex);
  ++m_refs;
}

//=============================================================================
void ArgObjectInterface::remove_ref()
{
  MutexLocker locker(*m_ref_mutex);
  DEBUG_ASSERT(m_refs>0,"Reference count going negative!");
  --m_refs;
}

//=============================================================================
ArgObject::ArgObject(
  ArgObjectInterface* obj
)
  : m_obj(obj)
{
  DEBUG_ASSERT(m_obj,"Constructing NULL ArgObject");
  m_obj->add_ref();
}

//=============================================================================
ArgObject::ArgObject(const ArgObject& c)
  : m_obj(c.m_obj)
{
  DEBUG_ASSERT(m_obj,"Copy constructing NULL ArgObject");
  m_obj->add_ref();
}

//=============================================================================
ArgObject::~ArgObject()
{
  m_obj->remove_ref();
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
  
  return Arg::op(optype,opname,right);
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
  // Only allow binary ( operator - object method call
  if (Arg::Binary == optype && "(" == opname) {
    return call(right);
  }
  
  return Arg::op(optype,opname,right);
}

//=============================================================================
Arg* ArgObjectFunction::call(Arg* args)
{
  ArgObjectInterface* obj = m_obj->get_object();
  if (obj) {
    return obj->arg_function(m_name,args);
  }
  return 0;
}

};
