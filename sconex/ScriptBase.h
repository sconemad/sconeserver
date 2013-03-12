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

#ifndef ScriptBase_h
#define ScriptBase_h

#include <sconex/sconex.h>
namespace scx {

class ScriptStatement;
class ScriptProc;
class ScriptRef;
class ScriptError;
class IOBase;

// Macro to help determine if a ScriptRef is 'bad' -
// meaning it is NULL or refers to a ScriptError object.
#define BAD_SCRIPTREF(a) \
(!a || (typeid(*a->object()) == typeid(scx::ScriptError)))

//=============================================================================
// ScriptAuth - Authorisation level for callers of operations and methods on
// SconeScript objects.
//
// An SconeScript object can use the ScriptAuth parameter passed into script_op
// and script_method to decide whether to allow a particular operation or 
// method.
//
// The authorisation levels are currently:
//
// Admin - The call is being made from a configuration script or console, so 
//   all operations required to configure the system should be permissable.
//
// Trusted - The call is being made from a script written and and only editable
//   by a registered user of the system.
//
// Untrusted - The call is being made from a script from an unknown source, or
//   could have been edited by an unknown user.
//
class SCONEX_API ScriptAuth {
public:

  enum AuthType { Admin, Trusted, Untrusted };
  
  ScriptAuth(AuthType type) 
    : m_type(type) {};

  ScriptAuth(const ScriptAuth& c) 
    : m_type(c.m_type) {};

  ~ScriptAuth() {};
  
  bool admin() const { return (m_type == Admin); };
  bool trusted() const { return (m_type == Trusted || m_type == Admin); };

private:

  AuthType m_type;

};

//=============================================================================
// ScriptOp - A script operation.
//
class SCONEX_API ScriptOp {
public:

  enum OpType {
    Unknown = 0,
    Resolve, Lookup,
    Sequential,
    Assign, AddAssign, SubtractAssign, MultiplyAssign, DivideAssign,
    Or, Xor, And, Not,
    Equality, Inequality,
    GreaterThan, LessThan, GreaterThanOrEqualTo, LessThanOrEqualTo,
    Add, Subtract, Multiply, Divide, Positive, Negative, 
    Modulus, Power, Factorial,
    PreIncrement, PreDecrement, PostIncrement, PostDecrement,
    Subscript, List, Map
  };

 ScriptOp(OpType type) : m_type(type) {};

  OpType type() const { return m_type; };

  //  bool is_binary() const;
  //  bool is_prefix() const;
  //  bool is_postfix() const;

private:

  OpType m_type;

};


//=============================================================================
// ScriptObject - Base class for all SconeScript objects.
//
class SCONEX_API ScriptObject {
public:

  ScriptObject();
  ScriptObject(const ScriptObject& c);
  virtual ~ScriptObject();

  // Return a deep copy of this object, or NULL if it does not allow copying
  virtual ScriptObject* new_copy() const;

  // Get a string representation of this object
  virtual std::string get_string() const;

  // Get an integer representation of this object (also used for boolean tests)
  virtual int get_int() const;

  // Perform an operation on this object, returning an object as a result
  //
  // auth: The authorisation of the caller.
  // ref: The reference through which the call was made.
  // op.type(): The nature of the operation, and can be:
  //   Prefix: 
  //     A unary prefix op, e.g. ++object.
  //   Postfix: 
  //     A unary postfix op, e.g. object++.
  //   Binary: 
  //     A binary op, e.g. object + 1. 
  //     In this case 'right' specifies the object on the right of the binary 
  //     operation.
  // op.name(): The name of the operation, e.g. '++' or '-'.
  //
  virtual ScriptRef* script_op(const ScriptAuth& auth,
			       const ScriptRef& ref,
			       const ScriptOp& op,
			       const ScriptRef* right=0);

  // Call a method on this object, returning an object as a result
  //
  // auth: The authorisation of the caller.
  // ref: The reference through which the call was made
  // name: The name of the method.
  // args: Method arguments.
  //
  virtual ScriptRef* script_method(const ScriptAuth& auth,
				   const ScriptRef& ref,
				   const std::string& name,
				   const ScriptRef* args);

  // Write the object in textual form to the specified output
  virtual void serialize(IOBase& output) const;

  // Get the number of ScriptRefs currently referring to this object
  int num_refs() const;

protected:

  // Parent object, if set, used by Resolve operation for scope resolution
  ScriptObject* m_parent;

private:

  friend class ScriptRef;
  friend class ScriptEngineExec;
  int add_ref();
  int remove_ref();

  int m_refs;

};


//=============================================================================
// ScriptRef - A reference-counted pointer to a ScriptObject.
//
class SCONEX_API ScriptRef {
public:

  enum RefType { Ref, ConstRef };

  explicit ScriptRef(ScriptObject* object, RefType ref=Ref);
  ScriptRef(const ScriptRef& c);
  ScriptRef(RefType ref, ScriptRef& c);
  virtual ~ScriptRef();

  // Make a new reference to the same object
  virtual ScriptRef* ref_copy(RefType ref=Ref) const;

  // Make a new reference to a new copy of the object
  virtual ScriptRef* new_copy(RefType ref=Ref) const;

  // Assignment
  virtual ScriptRef& operator=(const ScriptRef& c);

  // Is this a non-modifiable reference?
  bool is_const() const;

  // Get the ref type
  RefType reftype() const;

  // Access the object (these will never return NULL)
  ScriptObject* object();
  const ScriptObject* object() const;
  
  // Perform an operation on the object being referenced
  virtual ScriptRef* script_op(const ScriptAuth& auth, 
			       const ScriptOp& op, 
			       const ScriptRef* right=0);
protected:

  void add_ref();
  void remove_ref();

  ScriptObject* m_object;
  RefType m_reftype;

};


//=============================================================================
// ScriptMethodRef - A reference-counted pointer to a ScriptObject method.
//
class SCONEX_API ScriptMethodRef : public ScriptRef {
public:

  ScriptMethodRef(ScriptObject* object,
		  const std::string& method,
		  RefType ref=Ref);

  ScriptMethodRef(const ScriptRef& c, const std::string& method);
  ScriptMethodRef(const ScriptMethodRef& c);
  ScriptMethodRef(RefType ref, ScriptMethodRef& c);
  virtual ~ScriptMethodRef();

  // Get the method name
  const std::string& method() const;

  // Call the method
  ScriptRef* call(const ScriptAuth& auth, const ScriptRef* args);

  // Make a new reference to this object
  virtual ScriptRef* ref_copy(RefType ref=Ref) const;

  // Make a new reference to a new copy of the object
  virtual ScriptRef* new_copy(RefType ref=Ref) const;

  // Assignment
  virtual ScriptMethodRef& operator=(const ScriptMethodRef& c);

  // Perform an operation on the object being referenced
  virtual ScriptRef* script_op(const ScriptAuth& auth, 
			       const ScriptOp& op, 
			       const ScriptRef* right=0);
  
protected:

  std::string m_method;

};

//=============================================================================
// ScriptRefTo<T> - A reference-counted pointer that must point to a 
// particular ScriptObject-derived class.
//
template<class T> class ScriptRefTo : public ScriptRef {
public:

  ScriptRefTo(T* object, RefType ref=Ref) : ScriptRef(object,ref) {};
  ScriptRefTo(const ScriptRefTo<T>& c) : ScriptRef(c) {};
  ScriptRefTo(RefType ref, ScriptRefTo<T>& c) : ScriptRef(ref,c) {};

  ScriptRefTo<T>* ref_copy(RefType ref=Ref) const { 
    return new ScriptRefTo<T>(ref,*(const_cast< ScriptRefTo<T>* >(this))); 
  };

  ScriptRefTo<T>* new_copy(RefType ref=Ref) const { 
    return new ScriptRefTo<T>((T*)ScriptRef::object()->new_copy(),ref); 
  };

  virtual ScriptRefTo<T>& operator=(const ScriptRefTo<T>& c) { 
    if (this != &c && m_object != c.m_object) {
      remove_ref();
      m_object = c.m_object;
      add_ref();
    }
    return *this;
  };

  // Is the ref valid (i.e. is it a ref to an object of type T)
  bool valid() const { 
    return (0 != dynamic_cast<const T*>(ScriptRef::object())); 
  };

  // Access the object (these return NULL if !valid())
  T* object() { 
    if (valid()) return (T*)ScriptRef::object();
    return 0;
  };
  const T* object() const { 
    if (valid()) return (const T*)ScriptRef::object();
    return 0;
  };

};

  
};
#endif
