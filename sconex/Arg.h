/* SconeServer (http://www.sconemad.com)

The basic object types for SconeScript:

* Arg - Abstract base for SconeScript classes

* ArgString - A text string.
* ArgInt    - An integer.
* ArgReal   - A real number.
* ArgList   - A list of 0...n Arg objects.
* ArgMap    - A map (associative array) of Arg objects.
* ArgSub    - A subroutine.
* ArgError  - An object representing an error condition.

These are created dynamically during expression parsing in ArgProc.

Copyright (c) 2000-2009 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef Arg_h
#define Arg_h

#include "sconex/sconex.h"
namespace scx {

class ArgStatement;
class ArgProc;

#define BAD_ARG(a) (!a || (typeid(*a) == typeid(scx::ArgError)))

//=============================================================================
class SCONEX_API Auth {

public:

  enum AuthType { Admin, Trusted, Untrusted };
  
  Auth(AuthType type) : m_type(type) {};
  Auth(const Auth& c) : m_type(c.m_type) {};
  ~Auth() {};
  
  bool admin() const { return (m_type == Admin); };
  bool trusted() const { return (m_type == Trusted || m_type == Admin); };

private:

  AuthType m_type;

};

//=============================================================================
class SCONEX_API Arg {

public:

  enum RefType { Ref, ConstRef };

  Arg();
  Arg(const Arg& c);
  Arg(RefType ref, Arg& c);
  virtual ~Arg();

  virtual Arg* new_copy() const =0;
  // Make a new Arg which is a deep copy of this Arg

  virtual Arg* ref_copy(RefType ref);
  // Make a new Arg which is a (modifiable or const) reference to this Arg's data

  virtual Arg* new_method(const std::string& method);
  // Make a new Arg which represents the specified method call on this Arg

  virtual std::string get_string() const =0;
  virtual int get_int() const =0;
  // Get representations of this Arg in various standard forms

  enum OpType { Prefix, Postfix, Binary };
  virtual Arg* op(const Auth& auth, OpType optype, const std::string& opname, Arg* right=0);
  // Perform an operation on an Arg object

  bool last_ref() const;
  // Is this the last Arg refering to the underlying data

  bool is_const() const;
  // Is this Arg non-modifiable

protected:

  bool is_method_call(OpType optype, const std::string& opname);
  std::string m_method;
  
private:

  friend class ArgStatementDecl;
  
  int* m_refs;
  bool m_const;

};

//=============================================================================
class SCONEX_API ArgString : public Arg {

public:

  ArgString(const char* str);
  ArgString(const std::string& str);

  ArgString(const ArgString& c);
  ArgString(RefType ref, ArgString& c);
  virtual ~ArgString();

  virtual Arg* new_copy() const;
  virtual Arg* ref_copy(RefType ref);

  virtual std::string get_string() const;
  virtual int get_int() const;

  virtual Arg* op(const Auth& auth, OpType optype, const std::string& opname, Arg* right);

protected:

  std::string* m_string;
  
};


//=============================================================================
class SCONEX_API ArgInt : public Arg {

public:

  ArgInt(int value);

  ArgInt(const ArgInt& c);
  ArgInt(RefType ref, ArgInt& c);
  virtual ~ArgInt();

  virtual Arg* new_copy() const;
  virtual Arg* ref_copy(RefType ref);

  virtual std::string get_string() const;
  virtual int get_int() const;

  virtual Arg* op(const Auth& auth, OpType optype, const std::string& opname, Arg* right);

protected:

  int* m_value;

};


//=============================================================================
class SCONEX_API ArgReal : public Arg {

public:

  ArgReal(double value);

  ArgReal(const ArgReal& c);
  ArgReal(RefType ref, ArgReal& c);
  virtual ~ArgReal();

  virtual Arg* new_copy() const;
  virtual Arg* ref_copy(RefType ref);

  virtual std::string get_string() const;
  virtual int get_int() const;

  virtual Arg* op(const Auth& auth, OpType optype, const std::string& opname, Arg* right);

  double get_real() const;

protected:

  double* m_value;

};


//=============================================================================
class SCONEX_API ArgList : public Arg {

public:

  ArgList();

  ArgList(const ArgList& c);
  ArgList(RefType ref, ArgList& c);
  virtual ~ArgList();

  virtual Arg* new_copy() const;
  virtual Arg* ref_copy(RefType ref);

  virtual std::string get_string() const;
  virtual int get_int() const;

  virtual Arg* op(const Auth& auth, OpType optype, const std::string& opname, Arg* right);

  int size() const;

  const Arg* get(int i) const;
  Arg* get(int i);

  void give(Arg* arg, int pos=-1);
  Arg* take(int pos);

protected:

  typedef std::list<Arg*> ArgListData;
  //  typedef std::vector<Arg*> ArgListData;
  ArgListData* m_list;

};


//=============================================================================
class SCONEX_API ArgMap : public Arg {

public:

  ArgMap();

  ArgMap(const ArgMap& c);
  ArgMap(RefType ref, ArgMap& c);
  virtual ~ArgMap();

  virtual Arg* new_copy() const;
  virtual Arg* ref_copy(RefType ref);

  virtual std::string get_string() const;
  virtual int get_int() const;

  virtual Arg* op(const Auth& auth, OpType optype, const std::string& opname, Arg* right);

  int size() const;
  void keys(std::vector<std::string>& keyvec) const;

  const Arg* lookup(const std::string& key) const;
  Arg* lookup(const std::string& key);

  void give(const std::string& key, Arg* arg);
  Arg* take(const std::string& key);

protected:

  typedef std::map<std::string,Arg*> ArgMapData;
  ArgMapData* m_map;

};


//=============================================================================
class SCONEX_API ArgSub : public Arg {

public:

  ArgSub(const std::string& name, ArgStatement* body, ArgProc& proc);

  ArgSub(const ArgSub& c);
  virtual ~ArgSub();

  virtual Arg* new_copy() const;

  virtual std::string get_string() const;
  virtual int get_int() const;

  virtual Arg* op(const Auth& auth, OpType optype, const std::string& opname, Arg* right);

  Arg* call(const Auth& auth, Arg* args);
  
protected:

  std::string m_name;
  ArgStatement* m_body;
  ArgProc* m_proc;

};


//=============================================================================
class SCONEX_API ArgError : public Arg {

public:

  ArgError(const char* str);
  ArgError(const std::string& str);

  ArgError(const ArgError& c);
  virtual ~ArgError();

  virtual Arg* new_copy() const;

  virtual std::string get_string() const;
  virtual int get_int() const;

  virtual Arg* op(const Auth& auth, OpType optype, const std::string& opname, Arg* right);

protected:

  std::string m_string;

};
 
  
};
#endif
