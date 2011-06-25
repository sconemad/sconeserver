/* SconeServer (http://www.sconemad.com)

SconeScript basic types

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

#ifndef ScriptTypes_h
#define ScriptTypes_h

#include <sconex/sconex.h>
#include <sconex/ScriptBase.h>
namespace scx {

class ScriptExpr;
class ScriptTracer;

//=============================================================================
// ScriptString - A SconeScript text string
//
// SconeScript ops:
//   string + object // concatenation
//   string == != > < object // relational
//   string = object // assign
//   string += object // append
// 
// SconeScript methods:
//   clear() // clear the string
//   split(pattern) // split the string on the occurance of pattern
//   uc() // convert the string to uppercase
//   lc() // convert the string to lowercase
//
class SCONEX_API ScriptString : public ScriptObject {
public:

  // Convenience method to create a new ScriptRef to a new ScriptString
  static ScriptRef* new_ref(const std::string& str);

  ScriptString(const char* str);
  ScriptString(const std::string& str);
  ScriptString(const ScriptString& c);
  virtual ~ScriptString();

  static ScriptObject* create(const ScriptRef* args);
  virtual ScriptObject* new_copy() const;

  virtual std::string get_string() const;
  virtual int get_int() const;

  virtual ScriptRef* script_op(const ScriptAuth& auth,
			       const ScriptRef& ref,
			       const ScriptOp& op,
			       const ScriptRef* right);
  
  virtual ScriptRef* script_method(const ScriptAuth& auth,
				   const ScriptRef& ref,
				   const std::string& name,
				   const ScriptRef* args);

  virtual void serialize(IOBase& output) const;
  
  typedef ScriptRefTo<ScriptString> Ref;

protected:

  std::string m_string;
  
};


//=============================================================================
// ScriptNum - A SconeScript numeric type
//
class SCONEX_API ScriptNum : public ScriptObject {
public:

  virtual double get_real() const;

  typedef ScriptRefTo<ScriptNum> Ref;

};


//=============================================================================
// ScriptInt - A SconeScript integer
//
// SconeScript ops:
//   int + - * / int // add subtract, multiply, divide
//   int ^ % int // power, modulus
//   int = int // assign
//   int += -= *= /= int // op assign
//   +int -int // positive, negative
//   ++int --int // preincrement, predecrement
//   int! // factorial
//   int++ int-- // postincrement, postdecrement
//
class SCONEX_API ScriptInt : public ScriptNum {
public:

  // Convenience method to create a new ScriptRef to a new ScriptInt
  static ScriptRef* new_ref(int value);

  ScriptInt(long value);
  ScriptInt(const ScriptInt& c);
  virtual ~ScriptInt();

  static ScriptObject* from_string(const std::string& str, int base=10);
  static ScriptObject* create(const ScriptRef* args);
  virtual ScriptObject* new_copy() const;

  virtual std::string get_string() const;
  virtual int get_int() const;

  virtual ScriptRef* script_op(const ScriptAuth& auth,
			       const ScriptRef& ref,
			       const ScriptOp& op,
			       const ScriptRef* right);
  
  virtual void serialize(IOBase& output) const;

  typedef ScriptRefTo<ScriptNum> Ref;

protected:

  long m_value;

};


//=============================================================================
// ScriptReal - A SconeScript real number
//
// SconeScript ops:
//   int + - * / int // add subtract, multiply, divide
//   int == != > < int // relational
//   int ^ % int // power, modulus
//   int = int // assign
//   int += -= *= /= int // op assign
//   +int -int // positive, negative
//   ++int --int // preincrement, predecrement
//   int! // factorial
//   int++ int-- // postincrement, postdecrement
//
class SCONEX_API ScriptReal : public ScriptNum {
public:

  // Convenience method to create a new ScriptRef to a new ScriptReal
  static ScriptRef* new_ref(double value);

  ScriptReal(double value);
  ScriptReal(const ScriptReal& c);
  virtual ~ScriptReal();

  static ScriptObject* create(const ScriptRef* args);
  virtual ScriptObject* new_copy() const;

  virtual std::string get_string() const;
  virtual int get_int() const;

  virtual ScriptRef* script_op(const ScriptAuth& auth,
			       const ScriptRef& ref,
			       const ScriptOp& op,
			       const ScriptRef* right);

  virtual void serialize(IOBase& output) const;

  // Come on now, get real
  double get_real() const;

  typedef ScriptRefTo<ScriptReal> Ref;

protected:

  double m_value;

};


//=============================================================================
// ScriptList - A SconeScript list
//
// SconeScript ops:
//   list[n] // subscript
// 
// SconeScript properties:
//   size // number of elements in list
//
// SconeScript methods:
//   push(object) // append an object to the end of the list
//   splice(i) // remove object at offset i
//   reverse() // reverse the order of objects in the list
//   sort(predicate) // sort using predicate, which should be an expression 
//     in terms of a & b (default "a < b")
//   join(glue) // return a string containing elements printed with "glue" in
//     between each element
//   clear() // remove all elements in the list
//
class SCONEX_API ScriptList : public ScriptObject {
public:

  ScriptList();
  ScriptList(const ScriptList& c);
  virtual ~ScriptList();

  virtual ScriptObject* new_copy() const;

  virtual std::string get_string() const;
  virtual int get_int() const;

  virtual ScriptRef* script_op(const ScriptAuth& auth,
			       const ScriptRef& ref,
			       const ScriptOp& op,
			       const ScriptRef* right);
  
  virtual ScriptRef* script_method(const ScriptAuth& auth,
				   const ScriptRef& ref,
				   const std::string& name,
				   const ScriptRef* args);

  virtual void serialize(IOBase& output) const;

  int size() const;

  const ScriptRef* get(int i) const;
  ScriptRef* get(int i);

  void give(ScriptRef* item, int pos=-1);
  ScriptRef* take(int pos);

  void clear();

  typedef ScriptRefTo<ScriptList> Ref;

protected:

  typedef std::list<ScriptRef*> ScriptListData;
  ScriptListData m_list;

};


//=============================================================================
// ScriptMap - A SconeScript map or associative array
//
// SconeScript ops:
//   map[key] // lookup element corresponding to key
// 
// SconeScript properties:
//   size // number of elements in the map
//   keys // retrieve a list of keys corresponding to element in the map
//
// SconeScript methods:
//   set(key,object) // set an element in the map corresponding to the key
//   remove(key) // remove element from the map with this key
//   clear() // remove all elements from the map
//
class SCONEX_API ScriptMap : public ScriptObject {
public:

  ScriptMap();
  ScriptMap(const ScriptMap& c);
  virtual ~ScriptMap();

  virtual ScriptObject* new_copy() const;

  virtual std::string get_string() const;
  virtual int get_int() const;

  virtual ScriptRef* script_op(const ScriptAuth& auth,
			       const ScriptRef& ref,
			       const ScriptOp& op,
			       const ScriptRef* right);

  virtual ScriptRef* script_method(const ScriptAuth& auth,
				   const ScriptRef& ref,
				   const std::string& name,
				   const ScriptRef* args);

  virtual void serialize(IOBase& output) const;

  int size() const;
  void keys(std::vector<std::string>& keyvec) const;

  const ScriptRef* lookup(const std::string& key) const;
  ScriptRef* lookup(const std::string& key);

  void give(const std::string& key, ScriptRef* value);
  ScriptRef* take(const std::string& key);

  void clear();

  typedef ScriptRefTo<ScriptMap> Ref;

protected:

  typedef std::map<std::string,ScriptRef*> ScriptMapData;
  ScriptMapData m_map;

};


//=============================================================================
// ScriptSub - A SconeScript subroutine
//
// SconeScript ops:
//   sub(args...) // call the subroutine, passing in args
//
class SCONEX_API ScriptSub : public ScriptObject {
public:

  typedef std::vector<std::string> ScriptSubArgNames;

  ScriptSub(const std::string& name,
	    const ScriptSubArgNames& args,
	    ScriptRefTo<ScriptStatement>* body, 
	    ScriptTracer& tracer);

  ScriptSub(const ScriptSub& c);
  virtual ~ScriptSub();

  virtual ScriptObject* new_copy() const;

  virtual std::string get_string() const;
  virtual int get_int() const;

  virtual ScriptRef* script_op(const ScriptAuth& auth,
			       const ScriptRef& ref,
			       const ScriptOp& op,
			       const ScriptRef* right);

  ScriptRef* call(const ScriptAuth& auth, const ScriptRef* args);
  
  typedef ScriptRefTo<ScriptSub> Ref;

protected:

  std::string m_name;
  ScriptSubArgNames m_arg_names;
  ScriptRefTo<ScriptStatement>* m_body;
  ScriptTracer* m_tracer;

};


//=============================================================================
// SciptError - A SconeScript error
//
class SCONEX_API ScriptError : public ScriptObject {
public:

  static ScriptRef* new_ref(const std::string& error);

  ScriptError(const char* str);
  ScriptError(const std::string& str);
  ScriptError(const ScriptError& c);
  virtual ~ScriptError();

  static ScriptObject* create(const ScriptRef* args);
  virtual ScriptObject* new_copy() const;

  virtual std::string get_string() const;
  virtual int get_int() const;

  virtual void serialize(IOBase& output) const;

  typedef ScriptRefTo<ScriptError> Ref;

protected:

  std::string m_string;

};


//=============================================================================
// get_method_arg_ref - A function to extract an individual argument ref from 
// an argument ref passed to script_method() or similar. 
// Arguments can be looked up by index or name.
//
// e.g.
// const ScriptRef* name = get_method_arg_ref(args,0,"name");
//
inline const ScriptRef* get_method_arg_ref(const ScriptRef* args,
					   int index,
					   const std::string& name="")
{
  const ScriptList* args_list = 
    dynamic_cast<const ScriptList*>(args->object());
  if (args_list) {
    const ScriptRef* arg = args_list->get(index);
    if (!arg) return 0;
    return arg;
  }
  const ScriptMap* args_map = 
    dynamic_cast<const ScriptMap*>(args->object());
  if (args_map) {
    const ScriptRef* arg = args_map->lookup(name);
    if (!arg) return 0;
    return arg;
  }
  return 0;
}


//=============================================================================
// get_method_arg<T> - A template function to extract an individual argument 
// from an argument ref passed to script_method() or similar, and return an 
// object of the requested type.
// Arguments can be looked up by index or name.
//
// NULL is returned if the specified argument does not exist, or is not of the 
// specified type (use ScriptObject if the argument can be of any type).
//
// e.g.
// ScriptString* name = get_method_arg<ScriptString>(args,0,"name");
//
template<class T> const T* get_method_arg(const ScriptRef* args,
					  int index,
					  const std::string& name="")
{
  const ScriptRef* arg = get_method_arg_ref(args,index,name);
  if (!arg) return 0;
  return dynamic_cast<const T*>(arg->object());
}

 
};

#endif
