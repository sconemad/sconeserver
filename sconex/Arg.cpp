/* SconeServer (http://www.sconemad.com)

Argument classes

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

#include "sconex/Arg.h"
#include "sconex/ArgProc.h"
#include "sconex/ArgStatement.h"
namespace scx {

//===========================================================================
Arg::Arg()
  : m_refs(new int(1)),
    m_const(false)
{
  DEBUG_COUNT_CONSTRUCTOR(Arg);
}

//===========================================================================
Arg::Arg(const Arg& c)
  : m_method(c.m_method),
    m_refs(new int(1)),
    m_const(false)
{
  DEBUG_COUNT_CONSTRUCTOR(Arg);
}

//===========================================================================
Arg::Arg(RefType ref, Arg& c)
  : m_method(c.m_method),
    m_refs(c.m_refs),
    m_const(c.m_const || ref == ConstRef)
{
  DEBUG_COUNT_CONSTRUCTOR(Arg);
  ++(*m_refs);
}

//===========================================================================
Arg::~Arg()
{
  if (last_ref()) {
    delete m_refs;
  } else {
    --(*m_refs);
  }
  DEBUG_COUNT_DESTRUCTOR(Arg);
}

//===========================================================================
Arg* Arg::ref_copy(RefType ref)
{
  // Used if the arg doesn't support ref copies
  return new_copy();
}

//===========================================================================
Arg* Arg::new_method(const std::string& method)
{
  // Create a ref copy of this and set the method name
  Arg* a = ref_copy(Ref);
  a->m_method = method;
  return a;
}

//===========================================================================
Arg* Arg::op(const Auth& auth, OpType optype, const std::string& opname, Arg* right)
{
  if (is_method_call(optype,opname)) {
    return new ArgError("Unsupported");
  }
  
  int value = get_int();
  int rvalue = right ? right->get_int() : 0;
  switch (optype) {

    case Arg::Prefix:
      if ("!"==opname) { // Not
        return new ArgInt(!value);
      }
      break;

    case Arg::Binary:
      if (">"==opname) { // Greater than
        return new ArgInt(value > rvalue);
        
      } else if ("<"==opname) { // Less than
        return new ArgInt(value < rvalue);
        
      } else if (">="==opname) { // Greater than or equal to
        return new ArgInt(value >= rvalue);
        
      } else if ("<="==opname) { // Less than or equal to
        return new ArgInt(value <= rvalue);
        
      } else if ("=="==opname) { // Equal to
        return new ArgInt(value == rvalue);
        
      } else if ("!="==opname) { // Not equal to
        return new ArgInt(value != rvalue);
        
      } else if ("&"==opname) { // And
	if (value && right) return right->new_copy();
	return new_copy();
        
      } else if ("|"==opname) { // Or
	if (value) return new_copy();
	if (right) return right->new_copy();
	return 0;

      } else if ("xor"==opname) { // Xor
        return new ArgInt((value!=0) ^ (rvalue!=0));
      }
      break;
      
    default:
      break;
  }

  return new ArgError("Unsupported");
}

//===========================================================================
bool Arg::last_ref() const
{
  return (*m_refs == 1);
}

//===========================================================================
bool Arg::is_const() const
{
  return m_const;
}

//===========================================================================
bool Arg::is_method_call(OpType optype, const std::string& opname)
{
  return (!m_method.empty() && optype == Binary && opname == "(");
}


//===========================================================================
ArgString::ArgString(const char* str)
  : m_string(new std::string(str))
{
  DEBUG_COUNT_CONSTRUCTOR(ArgString);
}

//===========================================================================
ArgString::ArgString(const std::string& str)
  : m_string(new std::string(str))
{
  DEBUG_COUNT_CONSTRUCTOR(ArgString);
}

//===========================================================================
ArgString::ArgString(const ArgString& c)
  : Arg(c),
    m_string(new std::string(*c.m_string))
{
  DEBUG_COUNT_CONSTRUCTOR(ArgString);
}

//===========================================================================
ArgString::ArgString(RefType ref, ArgString& c)
  : Arg(ref,c),
    m_string(c.m_string)
{
  DEBUG_COUNT_CONSTRUCTOR(ArgString);
}

//===========================================================================
ArgString::~ArgString()
{
  if (last_ref()) {
    delete m_string;
  }
  DEBUG_COUNT_DESTRUCTOR(ArgString);
}

//===========================================================================
Arg* ArgString::new_copy() const
{
  return new ArgString(*this);
}

//===========================================================================
Arg* ArgString::ref_copy(RefType ref)
{
  return new ArgString(ref,*this);
}

//===========================================================================
std::string ArgString::get_string() const
{
  return *m_string;
}

//===========================================================================
int ArgString::get_int() const
{
  return 1;
}

//===========================================================================
Arg* ArgString::op(const Auth& auth, OpType optype, const std::string& opname, Arg* right)
{
  if (is_method_call(optype,opname)) {

    if ("clear" == m_method) {
      if (is_const()) return new ArgError("Not permitted");
      *m_string = "";
      return 0;
    }
    
  } else if (optype == Arg::Binary) {

    if ("+"==opname) { // Concatenate
      return new ArgString(*m_string + right->get_string());
      
    } else if ("=="==opname) { // Equal to
      return new ArgInt(*m_string == right->get_string());
      
    } else if ("!="==opname) { // Not equal to
      return new ArgInt(*m_string != right->get_string());

    } else if ("="==opname) { // Assignment
      if (!is_const()) {
	*m_string = right->get_string();
      }
      return ref_copy(Ref);

    } else if (opname == ".") {
      std::string name = right->get_string();
      if ("length" == name) return new ArgInt(m_string->size());
      if ("empty" == name) return new ArgInt(m_string->empty());

      if ("clear" == name) return new_method(name);
    }
  }

  return Arg::op(auth,optype,opname,right);
}


//===========================================================================
ArgInt::ArgInt(int value)
  : m_value(new int(value))
{
  DEBUG_COUNT_CONSTRUCTOR(ArgInt);
}

//===========================================================================
ArgInt::ArgInt(const ArgInt& c)
  : Arg(c),
    m_value(new int(*c.m_value))
{
  DEBUG_COUNT_CONSTRUCTOR(ArgInt);
}

//===========================================================================
ArgInt::ArgInt(RefType ref, ArgInt& c)
  : Arg(ref,c),
    m_value(c.m_value)
{
  DEBUG_COUNT_CONSTRUCTOR(ArgInt);
}

//===========================================================================
ArgInt::~ArgInt()
{
  if (last_ref()) {
    delete m_value;
  }
  DEBUG_COUNT_DESTRUCTOR(ArgInt);
}

//===========================================================================
Arg* ArgInt::new_copy() const
{
  return new ArgInt(*this);
}

//===========================================================================
Arg* ArgInt::ref_copy(RefType ref)
{
  return new ArgInt(ref,*this);
}

//===========================================================================
std::string ArgInt::get_string() const
{
  std::ostringstream oss;
  oss << *m_value;
  return oss.str();
}

//===========================================================================
int ArgInt::get_int() const
{
  return *m_value;
}

//===========================================================================
Arg* ArgInt::op(const Auth& auth, OpType optype, const std::string& opname, Arg* right)
{
  int& value = *m_value;
  switch (optype) {

    case Arg::Prefix: {
      if ("+"==opname) { // Positive (like, duh)
        return new ArgInt(+value);

      } else if ("-"==opname) { // Negative
        return new ArgInt(-value);

      } else if ("++"==opname) { // Pre-increment
	if (!is_const()) {
	  ++value;
	}
        return ref_copy(ConstRef);

      } else if ("--"==opname) { // Pre-decrement
	if (!is_const()) {
	  --value;
	}
        return ref_copy(ConstRef);
      }
    } break;

    case Arg::Postfix: {
      if ("!"==opname) { // Factorial (wtf do we need this?)
        int a = 1;
        for (int i=abs((int)value); i>1; --i) a *= i;
        return new ArgInt(a * (value<0 ? -1 : 1));

      } else if ("++"==opname) { // Post-increment
	int pre_value = value;
        if (!is_const()) {
	  ++value;
        }
        return new ArgInt(pre_value);

      } else if ("--"==opname) { // Post-decrement
	int pre_value = value;
        if (!is_const()) {
	  --value;
        }
        return new ArgInt(pre_value);
      }
    } break;

    case Arg::Binary: {
      ArgInt* rnum = dynamic_cast<ArgInt*> (right);
      if (rnum) {
        int rvalue = rnum->get_int();
        
        if ("+"==opname) { // Plus
          return new ArgInt(value + rvalue);

        } else if ("-"==opname) { // Minus
          return new ArgInt(value - rvalue);

        } else if ("*"==opname) { // Multiply
          return new ArgInt(value * rvalue);

        } else if ("/"==opname) { // Divide
          if (rvalue != 0) {
            return new ArgInt(value / rvalue);
          } else {
            return new ArgError("Divide by zero");
          }

        } else if ("%"==opname) { // Modulus
          if (rvalue != 0) {
            return new ArgInt(value % rvalue);
          } else {
            return new ArgError("Divide by zero");
          }

        } else if ("="==opname) { // Assignment
	  if (!is_const()) {
	    value = right->get_int();
	  }
          return ref_copy(Ref);
        }
      }
    } break;

  }

  return Arg::op(auth,optype,opname,right);
}


//===========================================================================
ArgReal::ArgReal(double value)
  : m_value(new double(value))
{
  DEBUG_COUNT_CONSTRUCTOR(ArgReal);
}

//===========================================================================
ArgReal::ArgReal(const ArgReal& c)
  : Arg(c),
    m_value(new double(*c.m_value))
{
  DEBUG_COUNT_CONSTRUCTOR(ArgReal);
}

//===========================================================================
ArgReal::ArgReal(RefType ref, ArgReal& c)
  : Arg(ref,c),
    m_value(c.m_value)
{
  DEBUG_COUNT_CONSTRUCTOR(ArgReal);
}

//===========================================================================
ArgReal::~ArgReal()
{
  if (last_ref()) {
    delete m_value;
  }
  DEBUG_COUNT_DESTRUCTOR(ArgReal);
}

//===========================================================================
Arg* ArgReal::new_copy() const
{
  return new ArgReal(*this);
}

//===========================================================================
Arg* ArgReal::ref_copy(RefType ref)
{
  return new ArgReal(ref,*this);
}

//===========================================================================
std::string ArgReal::get_string() const
{
  std::ostringstream oss;
  oss << *m_value;
  return oss.str();
}

//===========================================================================
int ArgReal::get_int() const
{
  return (int)(*m_value);
}

//===========================================================================
Arg* ArgReal::op(const Auth& auth, OpType optype, const std::string& opname, Arg* right)
{
  double& value = *m_value;
  switch (optype) {

    case Arg::Prefix: {
      if ("+"==opname) { // Positive (is there much point)
        return new ArgReal(+value);

      } else if ("-"==opname) { // Negative
        return new ArgReal(-value);

      } else if ("!"==opname) { // Not
        return new ArgReal(!value);
      }
    } break;

    case Arg::Postfix: {
      if ("!"==opname) { // Factorial
        int a = 1;
        for (int i=abs((int)value); i>1; --i) a *= i;
        return new ArgReal(a * (value<0 ? -1 : 1));
      }
    } break;

    case Arg::Binary: {
      ArgReal* rnum = dynamic_cast<ArgReal*> (right);
      if (rnum) {
        double rvalue = rnum->get_real();
        
        if ("+"==opname) { // Plus
          return new ArgReal(value + rvalue);

        } else if ("-"==opname) { // Minus
          return new ArgReal(value - rvalue);

        } else if ("*"==opname) { // Multiply
          return new ArgReal(value * rvalue);

        } else if ("/"==opname) { // Divide
          return new ArgReal(value / rvalue);

        } else if (">"==opname) { // Greater than
          return new ArgInt(value > rvalue);

        } else if ("<"==opname) { // Less than
          return new ArgInt(value < rvalue);

        } else if (">="==opname) { // Greater than or equal to
          return new ArgInt(value >= rvalue);

        } else if ("<="==opname) { // Less than or equal to
          return new ArgInt(value <= rvalue);

        } else if ("=="==opname) { // Equal to
          return new ArgInt(value == rvalue);

        } else if ("!="==opname) { // Not equal to
          return new ArgInt(value != rvalue);

        } else if ("="==opname) { // Assignment
	  if (!is_const()) {
	    value = rvalue;
	  }
          return ref_copy(Ref);
        }
      }
    } break;

  }

  return Arg::op(auth,optype,opname,right);
}

//===========================================================================
double ArgReal::get_real() const
{
  return *m_value;
}


//===========================================================================
ArgList::ArgList()
  : m_list(new ArgListData())
{
  DEBUG_COUNT_CONSTRUCTOR(ArgList);
}

//===========================================================================
ArgList::ArgList(const ArgList& c)
  : Arg(c),
    m_list(new ArgListData())
{
  DEBUG_COUNT_CONSTRUCTOR(ArgList);
  for (ArgListData::const_iterator it = c.m_list->begin();
       it != c.m_list->end();
       ++it) {
    m_list->push_back( (*it)->new_copy() );
  }
}

//===========================================================================
ArgList::ArgList(RefType ref, ArgList& c)
  : Arg(ref,c),
    m_list(c.m_list)
{
  DEBUG_COUNT_CONSTRUCTOR(ArgList);
}

//===========================================================================
ArgList::~ArgList()
{
  if (last_ref()) {
    for (ArgListData::iterator it = m_list->begin();
	 it != m_list->end();
	 ++it) {
      delete *it;
    }
    delete m_list;
  }
  DEBUG_COUNT_DESTRUCTOR(ArgList);
}

//===========================================================================
Arg* ArgList::new_copy() const
{
  return new ArgList(*this);
}

//===========================================================================
Arg* ArgList::ref_copy(RefType ref)
{
  return new ArgList(ref,*this);
}

//===========================================================================
std::string ArgList::get_string() const
{
  std::ostringstream oss;
  oss << "[";
  for (ArgListData::const_iterator it = m_list->begin();
       it != m_list->end();
       ++it) {
    const Arg* arg = *it;
    oss << (it==m_list->begin() ? "" : ",") 
	<< arg->get_string();
  }
  oss << "]";
  return oss.str();
}

//===========================================================================
int ArgList::get_int() const
{
  return m_list->size();
}

//===========================================================================
Arg* ArgList::op(const Auth& auth, OpType optype, const std::string& opname, Arg* right)
{
  if (is_method_call(optype,opname)) {

    ArgList* l = dynamic_cast<ArgList*>(right);
    
    if ("push" == m_method) {
      if (is_const()) return new ArgError("Not permitted");

      Arg* a_value = l->take(0);
      if (!a_value) return new ArgError("ArgList::push() No value specified");
      give(a_value);
      return 0;
    }

    if ("splice" == m_method) {
      if (is_const()) return new ArgError("Not permitted");

      Arg* a_offs = l->get(0);
      if (!a_offs) return new ArgError("ArgList::splice() No offset specified");
      ArgInt* offs = dynamic_cast<ArgInt*>(a_offs);
      if (!offs) return new ArgError("ArgList::splice() Offset not an integer");
      
      Arg* entry = take(offs->get_int());
      delete entry;
      return 0;
    }
    
  } else if (optype == Arg::Binary) {
    if (opname == "[") {
      ArgInt* aidx = dynamic_cast<ArgInt*> (right);
      if (aidx) {
	int idx = aidx->get_int();
	if (idx < 0 || idx >= (int)m_list->size()) {
	  return new ArgError("Array index out of bounds");
	}
	Arg* a = get(idx);
	if (a) {
	  return a->ref_copy(is_const() ? ConstRef : Ref);
	}
	return new ArgError("Not defined");
      }
    } else if (opname == ".") {
      std::string name = right->get_string();
      if ("size" == name) return new ArgInt(size());

      if ("push" == name ||
          "splice" == name) {
        return new_method(name);
      }
    }
  }

  return Arg::op(auth,optype,opname,right);
}

//===========================================================================
int ArgList::size() const
{
  return m_list->size();
}

//===========================================================================
const Arg* ArgList::get(int i) const
{
  int ic=0;
  for (ArgListData::const_iterator it = m_list->begin();
       it != m_list->end();
       ++it) {
    if (ic++==i) return *it;
  }
  return 0;
}

//===========================================================================
Arg* ArgList::get(int i)
{
  int ic=0;
  for (ArgListData::iterator it = m_list->begin();
       it != m_list->end();
       ++it) {
    if (ic++==i) return *it;
  }
  return 0;
}

//===========================================================================
void ArgList::give(Arg* arg, int i)
{
  if (-1==i) {
    m_list->push_back(arg);
  } else {
    int ic=0;
    ArgListData::iterator it;
    for (it = m_list->begin();
	 it != m_list->end();
	 ++it) {
      if (ic++==i) break;
    }
    m_list->insert(it,arg);
  }
}

//===========================================================================
Arg* ArgList::take(int i)
{
  int ic=0;
  for (ArgListData::iterator it = m_list->begin();
       it != m_list->end();
       ++it) {
    if (ic++==i) {
      Arg* arg = *it;
      m_list->erase(it);
      return arg;
    }
  }
  return 0;
}


//===========================================================================
ArgMap::ArgMap()
  : m_map(new ArgMapData())
{
  DEBUG_COUNT_CONSTRUCTOR(ArgMap);
}

//===========================================================================
ArgMap::ArgMap(const ArgMap& c)
  : Arg(c),
    m_map(new ArgMapData())
{
  DEBUG_COUNT_CONSTRUCTOR(ArgMap);
  for (ArgMapData::const_iterator it = c.m_map->begin();
       it != c.m_map->end();
       ++it) {
    const std::string key = it->first;
    const Arg* arg = it->second;
    m_map->insert( std::pair<std::string,Arg*>(key,arg->new_copy()) );
  }
}

//===========================================================================
ArgMap::ArgMap(RefType ref, ArgMap& c)
  : Arg(ref,c),
    m_map(c.m_map)
{
  DEBUG_COUNT_CONSTRUCTOR(ArgMap);
}

//===========================================================================
ArgMap::~ArgMap()
{
  if (last_ref()) {
    for (ArgMapData::iterator it = m_map->begin();
	 it != m_map->end();
	 ++it) {
      delete it->second;
    }
    delete m_map;
  }
  DEBUG_COUNT_DESTRUCTOR(ArgMap);
}

//===========================================================================
Arg* ArgMap::new_copy() const
{
  return new ArgMap(*this);
}

//===========================================================================
Arg* ArgMap::ref_copy(RefType ref)
{
  return new ArgMap(ref,*this);
}

//===========================================================================
std::string ArgMap::get_string() const
{
  std::ostringstream oss;
  oss << "{";
  for (ArgMapData::const_iterator it = m_map->begin();
       it != m_map->end();
       ++it) {
    const std::string key = it->first;
    const Arg* arg = it->second;
    oss << (it==m_map->begin() ? "" : ",") 
        << key << ":"
	<< (arg ? arg->get_string() : "NULL");
  }
  oss << "}";
  return oss.str();
}

//===========================================================================
int ArgMap::get_int() const
{
  return m_map->size();
}

//===========================================================================
Arg* ArgMap::op(const Auth& auth, OpType optype, const std::string& opname, Arg* right)
{
  if (optype == Arg::Binary) {
    if (opname == "[") {
      ArgString* akey = dynamic_cast<ArgString*> (right);
      if (akey) {
	const std::string key = akey->get_string();
	Arg* a = lookup(key);
	if (a) {
	  return a->ref_copy(is_const() ? ConstRef : Ref);
	}
	return new ArgError("Not defined");
      }
    } else if (opname == ".") {
      std::string name = right->get_string();
      if (name == "size") return new ArgInt(size());
      if (name == "keys") {
	ArgList* list = new ArgList();
	for (ArgMapData::const_iterator it = m_map->begin();
	     it != m_map->end();
	     ++it) {
	  list->give( new ArgString(it->first) );
	}
	return list;
      }
      
      Arg* a = lookup(name);
      if (a) {
	return a->ref_copy(is_const() ? ConstRef : Ref);
      }
    }
  }

  return Arg::op(auth,optype,opname,right);
}

//===========================================================================
int ArgMap::size() const
{
  return m_map->size();
}

//===========================================================================
void ArgMap::keys(std::vector<std::string>& keyvec) const
{
  keyvec.clear();
  keyvec.reserve(size());
  for (ArgMapData::const_iterator it = m_map->begin();
       it != m_map->end();
       ++it) {
    keyvec.push_back(it->first);
  }
}

//===========================================================================
const Arg* ArgMap::lookup(const std::string& key) const
{
  ArgMapData::const_iterator it = m_map->find(key);
  if (it != m_map->end()) {
    return it->second;
  }
  return 0;
}

//===========================================================================
Arg* ArgMap::lookup(const std::string& key)
{
  ArgMapData::iterator it = m_map->find(key);
  if (it != m_map->end()) {
    return it->second;
  }
  return 0;
}

//===========================================================================
void ArgMap::give(const std::string& key, Arg* arg)
{
  delete lookup(key);
  (*m_map)[key] = arg;
}

//===========================================================================
Arg* ArgMap::take(const std::string& key)
{
  ArgMapData::iterator it = m_map->find(key);
  if (it != m_map->end()) {
    Arg* arg = it->second;
    m_map->erase(it);
    return arg;
  }
  return 0;
}


//===========================================================================
ArgSub::ArgSub(const std::string& name, ArgStatement* body, ArgProc& proc)
  : m_name(name),
    m_body(body),
    m_proc(new ArgProc(proc))
{
  DEBUG_COUNT_CONSTRUCTOR(ArgSub);
}

//===========================================================================
ArgSub::ArgSub(const ArgSub& c)
  : Arg(c),
    m_name(c.m_name),
    m_body(0),
    m_proc(new ArgProc(*c.m_proc))
{
  DEBUG_COUNT_CONSTRUCTOR(ArgSub);
  if (c.m_body) {
    m_body = c.m_body->new_copy();
  }
}

//===========================================================================
ArgSub::~ArgSub()
{
  if (last_ref()) {
    delete m_body;
    delete m_proc;
  }
  DEBUG_COUNT_DESTRUCTOR(ArgSub);
}

//===========================================================================
Arg* ArgSub::new_copy() const
{
  return new ArgSub(*this);
}

//===========================================================================
std::string ArgSub::get_string() const
{
  std::ostringstream oss;
  oss << m_name << " sub";
  return oss.str();
}

//===========================================================================
int ArgSub::get_int() const
{
  return !m_name.empty();
}

//===========================================================================
Arg* ArgSub::op(const Auth& auth, OpType optype, const std::string& opname, Arg* right)
{
  // Only allow binary ( operator - subroutine call
  if (Arg::Binary == optype && "(" == opname) {
    return call(auth,right);
  }
  
  return Arg::op(auth,optype,opname,right);
}

//=============================================================================
Arg* ArgSub::call(const Auth& auth, Arg* args)
{
  if (m_body) {
    // Setup the argument vector
    ArgList vargs;
    vargs.give(new ArgString("ARGV"));
    vargs.give(args->new_copy());
    m_body->arg_function(auth,"var",&vargs);
    
    return m_body->execute(*m_proc);
  }
  return 0;
}


//===========================================================================
ArgError::ArgError(const char* str)
  : m_string(str)
{
  DEBUG_COUNT_CONSTRUCTOR(ArgError);
}

//===========================================================================
ArgError::ArgError(const std::string& str)
  : m_string(str)
{
  DEBUG_COUNT_CONSTRUCTOR(ArgError);
}

//===========================================================================
ArgError::ArgError(const ArgError& c)
  : Arg(c),
    m_string(c.m_string)
{
  DEBUG_COUNT_CONSTRUCTOR(ArgError);
}

//===========================================================================
ArgError::~ArgError()
{
  DEBUG_COUNT_DESTRUCTOR(ArgError);
}

//===========================================================================
Arg* ArgError::new_copy() const
{
  return new ArgError(*this);
}

//===========================================================================
std::string ArgError::get_string() const
{
  std::ostringstream oss;
  oss << "ERROR: " << m_string;
  return oss.str();
}

//===========================================================================
int ArgError::get_int() const
{
  return 0;
}

//===========================================================================
Arg* ArgError::op(const Auth& auth, OpType optype, const std::string& opname, Arg* right)
{
  return Arg::op(auth,optype,opname,right);
}


};
