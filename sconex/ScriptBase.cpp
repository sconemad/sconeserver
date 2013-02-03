/* SconeServer (http://www.sconemad.com)

SconeScript base classes

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

#include <sconex/ScriptBase.h>
#include <sconex/ScriptTypes.h>
#include <sconex/ScriptExpr.h>
#include <sconex/ScriptStatement.h>
#include <sconex/IOBase.h>
#include <sconex/utils.h>
namespace scx {

// ### ScriptObject ###

//===========================================================================
ScriptObject::ScriptObject()
  : m_parent(0),
    m_refs(0)
{
  DEBUG_COUNT_CONSTRUCTOR(ScriptObject);
}

//===========================================================================
ScriptObject::ScriptObject(const ScriptObject& c)
  : m_parent(c.m_parent),
    m_refs(0)
{
  DEBUG_COUNT_CONSTRUCTOR(ScriptObject);
}

//===========================================================================
ScriptObject::~ScriptObject()
{
  DEBUG_COUNT_DESTRUCTOR(ScriptObject);
}

//===========================================================================
 ScriptObject* ScriptObject::new_copy() const
{
  DEBUG_LOG("Trying to copy object which does not support copying");
  return 0;
}

//===========================================================================
std::string ScriptObject::get_string() const
{
  return "";
}

//===========================================================================
int ScriptObject::get_int() const
{
  return 1;
}

//===========================================================================
ScriptRef* ScriptObject::script_op(const ScriptAuth& auth,
				   const ScriptRef& ref,
				   const ScriptOp& op,
				   const ScriptRef* right)
{
  int value = get_int();
  int rvalue = right ? right->object()->get_int() : 0;
  switch (op.type()) {
    
  case ScriptOp::Not:
    return ScriptInt::new_ref(!value);
    
  case ScriptOp::Resolve:
    {
      // Attempt a lookup on this object
      ScriptRef* a = script_op(auth,ref,ScriptOp::Lookup,right);
      if (m_parent && BAD_SCRIPTREF(a)) {
	delete a;
	// If this lookup failed, try resolving in parent
	ScriptRef parent_ref(m_parent,ref.reftype());
	a = m_parent->script_op(auth,parent_ref,op,right);
	//	a = m_parent->script_op(auth,ref,op,right);
      }
      return a;
    }

  case ScriptOp::Lookup: 
    {
      std::string name = right->object()->get_string();
      if (name == "_refs") 
	return scx::ScriptInt::new_ref(m_refs);
      return ScriptError::new_ref("Unknown name: "+name);
    }

  case ScriptOp::GreaterThan:
    return ScriptInt::new_ref(value > rvalue);
      
  case ScriptOp::LessThan: 
    return ScriptInt::new_ref(value < rvalue);
      
  case ScriptOp::GreaterThanOrEqualTo:
    return ScriptInt::new_ref(value >= rvalue);
      
  case ScriptOp::LessThanOrEqualTo:
    return ScriptInt::new_ref(value <= rvalue);
      
  case ScriptOp::Equality:
    return ScriptInt::new_ref(value == rvalue);
      
  case ScriptOp::Inequality:
    return ScriptInt::new_ref(value != rvalue);
      
  case ScriptOp::And:
    if (value && right) return right->ref_copy(ScriptRef::Ref);
    return new ScriptRef(this);
      
  case ScriptOp::Or:
    if (value) return new ScriptRef(this);
    if (right) return right->ref_copy(ScriptRef::Ref);
    return 0;
      
  case ScriptOp::Xor:
    return ScriptInt::new_ref((value!=0) ^ (rvalue!=0));
    
  default:
    break;
  }
  
  return ScriptError::new_ref("Unsupported operation");
}

//===========================================================================
ScriptRef* ScriptObject::script_method(const ScriptAuth& auth,
				       const ScriptRef& ref,
				       const std::string& name,
				       const ScriptRef* args)
{
  return ScriptError::new_ref("Unsupported method: " + name);
}

//===========================================================================
void ScriptObject::serialize(IOBase& output) const
{
  // This is the default for objects which cannot be serialized
  output.write("NULL");
}

//===========================================================================
int ScriptObject::num_refs() const
{
  return m_refs;
}

//=============================================================================
void ScriptObject::log(const std::string& message,
                       Logger::Level level,
                       const std::string& context)
{
  std::string c = get_log_context();
  if (!context.empty()) c += "." + context;
  if (m_parent) m_parent->log(message,level,c);
}

//===========================================================================
std::string ScriptObject::get_log_context() const
{
  return "";
}
  
//===========================================================================
int ScriptObject::add_ref()
{
  return ++m_refs;
}

//===========================================================================
int ScriptObject::remove_ref()
{
  return --m_refs;
}

// ### ScriptRef ###

//===========================================================================
ScriptRef::ScriptRef(ScriptObject* object, RefType ref)
  : m_object(object),
    m_reftype(ref)
{
  DEBUG_COUNT_CONSTRUCTOR(ScriptRef);
  if (!m_object) {
    // Ensure the ref always refers to an object
    m_object = new ScriptError("NULL");
  }
  add_ref();
}

//===========================================================================
ScriptRef::ScriptRef(const ScriptRef& c)
  : m_object(c.m_object),
    m_reftype(c.m_reftype)
{
  DEBUG_COUNT_CONSTRUCTOR(ScriptRef);
  add_ref();
}

//===========================================================================
ScriptRef::ScriptRef(RefType ref, ScriptRef& c)
  : m_object(c.m_object),
    m_reftype(c.is_const() ? ConstRef : ref)
{
  DEBUG_COUNT_CONSTRUCTOR(ScriptRef);
  add_ref();
}

//===========================================================================
ScriptRef::~ScriptRef()
{
  remove_ref();
  DEBUG_COUNT_DESTRUCTOR(ScriptRef);
}

//===========================================================================
ScriptRef* ScriptRef::ref_copy(RefType ref) const
{
  ScriptRef* uc = const_cast<ScriptRef*>(this);
  return new ScriptRef(ref,*uc);
}

//===========================================================================
ScriptRef* ScriptRef::new_copy(RefType ref) const
{
  return new ScriptRef(m_object->new_copy(),ref);
}

//===========================================================================
ScriptRef& ScriptRef::operator=(const ScriptRef& c)
{
  if (this != &c && m_object != c.m_object) {
    remove_ref();
    m_object = c.m_object;
    add_ref();
  }
  return *this;
}

//===========================================================================
bool ScriptRef::is_const() const
{
  return (m_reftype == ConstRef);
}

//===========================================================================
ScriptRef::RefType ScriptRef::reftype() const
{
  return m_reftype;
}

//===========================================================================
ScriptObject* ScriptRef::object()
{
  return m_object;
}

//===========================================================================
const ScriptObject* ScriptRef::object() const
{
  return m_object;
}

//===========================================================================
ScriptRef* ScriptRef::script_op(const ScriptAuth& auth, 
				const ScriptOp& op, 
				const ScriptRef* right)
{
  return m_object->script_op(auth,*this,op,right);
}

//===========================================================================
void ScriptRef::add_ref()
{
  if (m_object) m_object->add_ref();
}

//===========================================================================
void ScriptRef::remove_ref()
{
  if (m_object && 0 == m_object->remove_ref()) {
    delete m_object;
    m_object = 0;
  }
}

// ### ScriptMethodRef ###

//===========================================================================
ScriptMethodRef::ScriptMethodRef(ScriptObject* object, 
				 const std::string& method, 
				 RefType ref)
  : ScriptRef(object,ref),
    m_method(method)
{

}

//===========================================================================
ScriptMethodRef::ScriptMethodRef(const ScriptRef& c, 
				 const std::string& method)
  : ScriptRef(c),
    m_method(method)
{

}

//===========================================================================
ScriptMethodRef::ScriptMethodRef(const ScriptMethodRef& c)
  : ScriptRef(c),
    m_method(c.m_method)
{

}

//===========================================================================
ScriptMethodRef::ScriptMethodRef(RefType ref, ScriptMethodRef& c)
  : ScriptRef(ref,c),
    m_method(c.m_method)
{

}

//===========================================================================
ScriptMethodRef::~ScriptMethodRef()
{

}

//===========================================================================
const std::string& ScriptMethodRef::method() const
{
  return m_method;
}

//===========================================================================
ScriptRef* ScriptMethodRef::call(const ScriptAuth& auth, const ScriptRef* args)
{
  return m_object->script_method(auth,*this,m_method,args);
}

//===========================================================================
ScriptRef* ScriptMethodRef::ref_copy(RefType ref) const
{
  ScriptMethodRef* uc = const_cast<ScriptMethodRef*>(this);
  return new ScriptMethodRef(ref,*uc);
}

//===========================================================================
ScriptRef* ScriptMethodRef::new_copy(RefType ref) const
{
  return new ScriptMethodRef(m_object->new_copy(),m_method,ref);
}

//===========================================================================
ScriptMethodRef& ScriptMethodRef::operator=(const ScriptMethodRef& c)
{
  if (this != &c) {
    m_method = c.m_method;
  }
  ScriptRef::operator=(c);
  return *this;
}

//===========================================================================
ScriptRef* ScriptMethodRef::script_op(const ScriptAuth& auth, 
				      const ScriptOp& op, 
				      const ScriptRef* right)
{
  if (op.type() == ScriptOp::List) {
    return call(auth,right);
  }
  return ScriptError::new_ref("Unsupported operation on method");
}


};
