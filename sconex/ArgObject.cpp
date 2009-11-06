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
#include "sconex/Kernel.h"
#include "sconex/utils.h"

namespace scx {

Mutex* ArgObjectInterface::m_ref_mutex = 0;

//=============================================================================
ArgObjectInterface::ArgObjectInterface()
  : m_refs(0)
{
  DEBUG_COUNT_CONSTRUCTOR(ArgObjectInterface);
  if (!m_ref_mutex) {
    m_ref_mutex = new Mutex();
  }
}

//=============================================================================
ArgObjectInterface::ArgObjectInterface(const ArgObjectInterface& c)
  : m_refs(0)
{
  DEBUG_COUNT_CONSTRUCTOR(ArgObjectInterface);
  if (!m_ref_mutex) {
    m_ref_mutex = new Mutex();
  }
}
  
//=============================================================================
ArgObjectInterface::~ArgObjectInterface()
{
  MutexLocker locker(*m_ref_mutex);
  DEBUG_ASSERT(m_refs==0,"Destroying object which has refs");
  DEBUG_COUNT_DESTRUCTOR(ArgObjectInterface);
}

//=============================================================================
std::string  ArgObjectInterface::name() const
{
  return "";
}

//=============================================================================
void ArgObjectInterface::log(
  const std::string& message,
  Logger::Level level
)
{
  Arg* a_logger = arg_resolve("log");
  if (a_logger) {

    ArgObject* logger = dynamic_cast<ArgObject*>(a_logger);
    if (logger) {
      ArgList args;
      args.give( new ArgString(message) );
      args.give( new ArgInt((int)level) );
      
      Auth auth(Auth::Untrusted);
      Arg* ret = logger->op(auth,Arg::Binary,"(",&args);
      delete ret;
    }
    delete a_logger;
  }
}

//=============================================================================
Arg* ArgObjectInterface::arg_lookup(const std::string& aname)
{
  if ("_name" == aname) {
    return new scx::ArgString(name());
  }

  return new ArgError("Unknown name '" + aname + "'");
}

//=============================================================================
Arg* ArgObjectInterface::arg_resolve(const std::string& name)
{
  Arg* a = arg_lookup(name);
  if (BAD_ARG(a)) {
    Kernel* k = Kernel::get();
    if (k != this) {
      delete a;
      a = k->arg_resolve(name);
    }
  }
  return a;
}

//=============================================================================
Arg* ArgObjectInterface::arg_method(const Auth& auth, const std::string& name, Arg* args)
{
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
Arg* ArgObjectInterface::new_method(const std::string& method)
{
  return new ArgObject(this,method);
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
  ArgObjectInterface* obj,
  const std::string& method
) : m_obj(obj)
{
  DEBUG_COUNT_CONSTRUCTOR(ArgObject);
  DEBUG_ASSERT(m_obj,"Constructing NULL ArgObject");
  if (m_obj) m_obj->add_ref();
  m_method = method;
}

//=============================================================================
ArgObject::ArgObject(const ArgObject& c)
  : Arg(c),
    m_obj(c.m_obj)
{
  DEBUG_COUNT_CONSTRUCTOR(ArgObject);
  DEBUG_ASSERT(m_obj,"Copy constructing NULL ArgObject");
  if (m_obj) m_obj->add_ref();
}

//=============================================================================
ArgObject::~ArgObject()
{
  if (m_obj) m_obj->remove_ref();
  DEBUG_COUNT_DESTRUCTOR(ArgObject);
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

  if (m_obj) {
    oss << "(" << type_name(typeid(*m_obj)) << ")" << m_obj->name();

  } else {
    oss << "(NULL)";
  }

  if (!m_method.empty()) {
    oss << "::" << m_method;
  }
  
  return oss.str();
}

//===========================================================================
int ArgObject::get_int() const
{
  return (m_obj != 0);
}

//=============================================================================
Arg* ArgObject::op(const Auth& auth, OpType optype, const std::string& opname, Arg* right)
{
  if (m_obj) {
    if (is_method_call(optype,opname)) {
      return m_obj->arg_method(auth,m_method,right);
    
    } else if (Arg::Binary == optype) {
      // Lookup
      if ("." == opname || "[" == opname) {
        return m_obj->arg_lookup(right->get_string());
      }
      // Resolve
      if (":" == opname) {
        return m_obj->arg_resolve(right->get_string());        
      }
    }
  }

  // Assignment
  if (Arg::Binary == optype && "=" == opname) {
    ArgObject* rv = dynamic_cast<ArgObject*>(right);
    if (!is_const()) {
      if (rv) {
        *this = *rv;
      }
    }
    return ref_copy(Ref);
  }
   
  return Arg::op(auth,optype,opname,right);
}

//=============================================================================
ArgObject& ArgObject::operator=(const ArgObject& v)
{
  if (m_obj) m_obj->remove_ref();
  m_obj = v.m_obj;
  if (m_obj) m_obj->add_ref();
  return *this;
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

};
