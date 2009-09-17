/* SconeServer (http://www.sconemad.com)

The basic object types for SconeScript:

* Arg - Abstract base for SconeScript classes

* ArgString - A text string.
* ArgInt - An integer.
* ArgReal - A real number.
* ArgList - A list of 0...n Arg objects.
* ArgFunction - A callable function.
* ArgError - An object representing an error condition.

These are created dynamically during expression parsing in ArgProc.

Copyright (c) 2000-2006 Andrew Wedgbury <wedge@sconemad.com>

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
  
//=============================================================================
class SCONEX_API Arg {

public:

  Arg();
  Arg(const Arg& c);
  virtual ~Arg();
  virtual Arg* new_copy() const =0;
  virtual Arg* var_copy();

  virtual std::string get_string() const =0;
  virtual int get_int() const =0;

  enum OpType { Prefix, Postfix, Binary };
  virtual Arg* op(OpType optype, const std::string& opname, Arg* right=0);

};

//=============================================================================
class SCONEX_API ArgString : public Arg {

public:

  ArgString(const char* str);
  ArgString(const std::string& str);
  ArgString(const ArgString& c);
  virtual ~ArgString();
  virtual Arg* new_copy() const;
  virtual Arg* var_copy();

  virtual std::string get_string() const;
  virtual int get_int() const;

  virtual Arg* op(OpType optype, const std::string& opname, Arg* right);

protected:

  std::string m_string;
  ArgString* m_orig;
  
};


//=============================================================================
class SCONEX_API ArgInt : public Arg {

public:

  ArgInt(int value);
  ArgInt(const ArgInt& c);
  virtual ~ArgInt();
  virtual Arg* new_copy() const;
  virtual Arg* var_copy();

  virtual std::string get_string() const;
  virtual int get_int() const;

  virtual Arg* op(OpType optype, const std::string& opname, Arg* right);

protected:

  int m_value;
  ArgInt* m_orig;

};


//=============================================================================
class SCONEX_API ArgReal : public Arg {

public:

  ArgReal(double value);
  ArgReal(const ArgReal& c);
  virtual ~ArgReal();
  virtual Arg* new_copy() const;
  virtual Arg* var_copy();

  virtual std::string get_string() const;
  virtual int get_int() const;

  virtual Arg* op(OpType optype, const std::string& opname, Arg* right);

  double get_real() const;

protected:

  double m_value;
  ArgReal* m_orig;

};


//=============================================================================
class SCONEX_API ArgList : public Arg {

public:

  ArgList();
  ArgList(const ArgList& c);
  virtual ~ArgList();
  virtual Arg* new_copy() const;
  virtual Arg* var_copy();

  virtual std::string get_string() const;
  virtual int get_int() const;

  virtual Arg* op(OpType optype, const std::string& opname, Arg* right);

  int size() const;

  const Arg* get(int i) const;
  Arg* get(int i);

  void give(Arg* arg, int pos=-1);
  Arg* take(int pos);

protected:

  typedef std::list<Arg*> ArgListData;
  ArgListData m_list;
  ArgList* m_orig;

};


//=============================================================================
class SCONEX_API ArgMap : public Arg {

public:

  ArgMap();
  ArgMap(const ArgMap& c);
  virtual ~ArgMap();
  virtual Arg* new_copy() const;
  virtual Arg* var_copy();

  virtual std::string get_string() const;
  virtual int get_int() const;

  virtual Arg* op(OpType optype, const std::string& opname, Arg* right);

  int size() const;
  void keys(std::vector<std::string>& keyvec) const;

  const Arg* lookup(const std::string& key) const;
  Arg* lookup(const std::string& key);

  void give(const std::string& key, Arg* arg);
  Arg* take(const std::string& key);

protected:

  typedef std::map<std::string,Arg*> ArgMapData;
  ArgMapData m_map;
  ArgMap* m_orig;

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

  virtual Arg* op(OpType optype, const std::string& opname, Arg* right);

  Arg* call(Arg* args);
  
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

  virtual Arg* op(OpType optype, const std::string& opname, Arg* right);

protected:

  std::string m_string;

};
 
  
};
#endif
