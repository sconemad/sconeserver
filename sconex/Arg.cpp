/* SconeServer (http://www.sconemad.com)

Argument classes

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

#include "sconex/Arg.h"
#include "sconex/ArgProc.h"
#include "sconex/ArgStatement.h"
namespace scx {

//===========================================================================
Arg::Arg()
{
  DEBUG_COUNT_CONSTRUCTOR(Arg);
}

//===========================================================================
Arg::Arg(const Arg& c)
{
  DEBUG_COUNT_CONSTRUCTOR(Arg);
}

//===========================================================================
Arg::~Arg()
{
  DEBUG_COUNT_DESTRUCTOR(Arg);
}

//===========================================================================
Arg* Arg::var_copy()
{
  // Used if the arg doesn't support variable copies
  return new_copy();
}

//===========================================================================
Arg* Arg::op(OpType optype, const std::string& opname, Arg* right)
{

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
        return new ArgInt(value && rvalue);
        
      } else if ("|"==opname) { // Or
        return new ArgInt(value | rvalue);

      } else if ("xor"==opname) { // Xor
        return new ArgInt((value!=0) ^ (rvalue!=0));
      }
      break;
      
    default:
      break;
  }

  return 0;
}

//===========================================================================
ArgString::ArgString(const char* str)
  : m_string(str),
    m_orig(0)
{
}

//===========================================================================
ArgString::ArgString(const std::string& str)
  : m_string(str),
    m_orig(0)
{
}

//===========================================================================
ArgString::ArgString(const ArgString& c)
  : m_string(c.m_string),
    m_orig(c.m_orig)
{
}

//===========================================================================
ArgString::~ArgString()
{
}

//===========================================================================
Arg* ArgString::new_copy() const
{
  return new ArgString(*this);
}

//===========================================================================
Arg* ArgString::var_copy()
{
  ArgString* c = new ArgString(*this);
  c->m_orig = (ArgString*)this;
  return c;
}

//===========================================================================
std::string ArgString::get_string() const
{
  return  m_string;
}

//===========================================================================
int ArgString::get_int() const
{
  return !m_string.empty();
}

//===========================================================================
Arg* ArgString::op(OpType optype, const std::string& opname, Arg* right)
{
  if (optype == Arg::Binary) {

    if ("+"==opname) { // Concatenate
      return new ArgString(m_string + right->get_string());
      
    } else if ("=="==opname) { // Equal to
      return new ArgInt(m_string == right->get_string());
      
    } else if ("!="==opname) { // Not equal to
      return new ArgInt(m_string != right->get_string());

    } else if ("="==opname) { // Assignment
      m_string = right->get_string();
      if (m_orig) {
        m_orig->m_string = m_string;
      }
      return new_copy();
    }
  }

  return Arg::op(optype,opname,right);
}


//===========================================================================
ArgInt::ArgInt(int value)
  : m_value(value),
    m_orig(0)
{
}

//===========================================================================
ArgInt::ArgInt(const ArgInt& c)
  : m_value(c.m_value),
    m_orig(c.m_orig)
{
}

//===========================================================================
ArgInt::~ArgInt()
{
}

//===========================================================================
Arg* ArgInt::new_copy() const
{
  return new ArgInt(*this);
}

//===========================================================================
Arg* ArgInt::var_copy()
{
  ArgInt* c = new ArgInt(*this);
  c->m_orig = (ArgInt*)this;
  return c;
}

//===========================================================================
std::string ArgInt::get_string() const
{
  std::ostringstream oss;
  oss << m_value;
  return oss.str();
}

//===========================================================================
int ArgInt::get_int() const
{
  return m_value;
}

//===========================================================================
Arg* ArgInt::op(OpType optype, const std::string& opname, Arg* right)
{

  switch (optype) {

    case Arg::Prefix: {
      if ("+"==opname) { // Positive (is there much point)
        return new ArgInt(+m_value);

      } else if ("-"==opname) { // Negative
        return new ArgInt(-m_value);

      } else if ("++"==opname) { // Pre-increment
        ++m_value;
        if (m_orig) {
          m_orig->m_value = m_value;
        }
        return new_copy();        

      } else if ("--"==opname) { // Pre-decrement
        --m_value;
        if (m_orig) {
          m_orig->m_value = m_value;
        }
        return new_copy();        
      }
    } break;

    case Arg::Postfix: {
      if ("!"==opname) { // Factorial
        int a = 1;
        for (int i=abs((int)m_value); i>1; --i) a *= i;
        return new ArgInt(a * (m_value<0 ? -1 : 1));

      } else if ("++"==opname) { // Post-increment
        if (m_orig) {
          m_orig->m_value = m_value + 1;
        }
        return new_copy();        

      } else if ("--"==opname) { // Post-decrement
        if (m_orig) {
          m_orig->m_value = m_value - 1;
        }
        return new_copy();        
      }
    } break;

    case Arg::Binary: {
      ArgInt* rnum = dynamic_cast<ArgInt*> (right);
      if (rnum) {
        int rvalue = rnum->get_int();
        
        if ("+"==opname) { // Plus
          return new ArgInt(m_value + rvalue);

        } else if ("-"==opname) { // Minus
          return new ArgInt(m_value - rvalue);

        } else if ("*"==opname) { // Multiply
          return new ArgInt(m_value * rvalue);

        } else if ("/"==opname) { // Divide
          if (rvalue != 0) {
            return new ArgInt(m_value / rvalue);
          } else {
            return new ArgError("Divide by zero");
          }

        } else if ("="==opname) { // Assignment
          m_value = right->get_int();
          if (m_orig) {
            m_orig->m_value = m_value;
          }
          return new_copy();
        }
      }
    } break;

  }

  return Arg::op(optype,opname,right);
}


//===========================================================================
ArgReal::ArgReal(double value)
  : m_value(value),
    m_orig(0)
{
}

//===========================================================================
ArgReal::ArgReal(const ArgReal& c)
  : m_value(c.m_value),
    m_orig(c.m_orig)
{
}

//===========================================================================
ArgReal::~ArgReal()
{
}

//===========================================================================
Arg* ArgReal::new_copy() const
{
  return new ArgReal(*this);
}

//===========================================================================
Arg* ArgReal::var_copy()
{
  ArgReal* c = new ArgReal(*this);
  c->m_orig = (ArgReal*)this;
  return c;
}

//===========================================================================
std::string ArgReal::get_string() const
{
  std::ostringstream oss;
  oss << m_value;
  return oss.str();
}

//===========================================================================
int ArgReal::get_int() const
{
  return (int)m_value;
}

//===========================================================================
Arg* ArgReal::op(OpType optype, const std::string& opname, Arg* right)
{

  switch (optype) {

    case Arg::Prefix: {
      if ("+"==opname) { // Positive (is there much point)
        return new ArgReal(+m_value);

      } else if ("-"==opname) { // Negative
        return new ArgReal(-m_value);

      } else if ("!"==opname) { // Not
        return new ArgReal(!m_value);
      }
    } break;

    case Arg::Postfix: {
      if ("!"==opname) { // Factorial
        int a = 1;
        for (int i=abs((int)m_value); i>1; --i) a *= i;
        return new ArgReal(a * (m_value<0 ? -1 : 1));
      }
    } break;

    case Arg::Binary: {
      ArgReal* rnum = dynamic_cast<ArgReal*> (right);
      if (rnum) {
        double rvalue = rnum->get_real();
        
        if ("+"==opname) { // Plus
          return new ArgReal(m_value + rvalue);

        } else if ("-"==opname) { // Minus
          return new ArgReal(m_value - rvalue);

        } else if ("*"==opname) { // Multiply
          return new ArgReal(m_value * rvalue);

        } else if ("/"==opname) { // Divide
          return new ArgReal(m_value / rvalue);

        } else if (">"==opname) { // Greater than
          return new ArgInt(m_value > rvalue);

        } else if ("<"==opname) { // Less than
          return new ArgInt(m_value < rvalue);

        } else if (">="==opname) { // Greater than or equal to
          return new ArgInt(m_value >= rvalue);

        } else if ("<="==opname) { // Less than or equal to
          return new ArgInt(m_value <= rvalue);

        } else if ("=="==opname) { // Equal to
          return new ArgInt(m_value == rvalue);

        } else if ("!="==opname) { // Not equal to
          return new ArgInt(m_value != rvalue);

        } else if ("="==opname) { // Assignment
          m_value = rnum->get_real();
          if (m_orig) {
            m_orig->m_value = m_value;
          }
          return new_copy();
        }
      }
    } break;

  }

  return Arg::op(optype,opname,right);
}

//===========================================================================
double ArgReal::get_real() const
{
  return m_value;
}


//===========================================================================
ArgList::ArgList()
{
}

//===========================================================================
ArgList::ArgList(const ArgList& c)
{
  std::list<Arg*>::const_iterator iter = c.m_list.begin();
  while (iter != c.m_list.end()) {
    m_list.push_back( (*iter)->new_copy() );
    ++iter;
  }
}

//===========================================================================
ArgList::~ArgList()
{
  std::list<Arg*>::iterator iter = m_list.begin();
  while (iter != m_list.end()) {
    delete *iter;
    ++iter;
  }
}

//===========================================================================
Arg* ArgList::new_copy() const
{
  return new ArgList(*this);
}

//===========================================================================
std::string ArgList::get_string() const
{
  std::ostringstream oss;
  oss << "(";
  std::list<Arg*>::const_iterator iter = m_list.begin();
  int i=0;
  while (iter != m_list.end()) {
    const Arg* arg = *iter;
    oss << (++i>1 ? "," : "") << arg->get_string();
    ++iter;
  }
  oss << ")";
  return oss.str();
}

//===========================================================================
int ArgList::get_int() const
{
  return m_list.size();
}

//===========================================================================
Arg* ArgList::op(OpType optype, const std::string& opname, Arg* right)
{
  if (optype == Arg::Binary && opname == "[") {
    ArgInt* aidx = dynamic_cast<ArgInt*> (right);
    if (aidx) {
      int idx = aidx->get_int();
      if (idx < 0 || idx >= m_list.size()) {
        return new ArgError("Array index out of bounds");
      }
      const Arg* a = get(idx);
      if (a) {
        return a->new_copy();
      }
    }  
  }

  return Arg::op(optype,opname,right);
}

//===========================================================================
int ArgList::size() const
{
  return m_list.size();
}

//===========================================================================
const Arg* ArgList::get(int i) const
{
  std::list<Arg*>::const_iterator iter = m_list.begin();
  int ic=0;
  while (iter != m_list.end()) {
    if (ic++==i) return *iter;
    ++iter;
  }
  return 0;
}

//===========================================================================
void ArgList::give(Arg* arg, int i)
{
  if (-1==i) {
    m_list.push_back(arg);
  } else {
    std::list<Arg*>::iterator iter = m_list.begin();
    int ic=0;
    while (iter != m_list.end()) {
      if (ic++==i) break;
      ++iter;
    }
    m_list.insert(iter,arg);
  }
}

//===========================================================================
Arg* ArgList::take(int i)
{
  std::list<Arg*>::iterator iter = m_list.begin();
  int ic=0;
  while (iter != m_list.end()) {
    if (ic++==i) {
      Arg* arg = *iter;
      m_list.erase(iter);
      return arg;
    }
    ++iter;
  }
  return 0;
}


//===========================================================================
ArgSub::ArgSub(const std::string& name, ArgStatement* body, ArgProc& proc)
  : m_name(name),
    m_body(body),
    m_proc(new ArgProc(proc))
{
}

//===========================================================================
ArgSub::ArgSub(const ArgSub& c)
  : m_name(c.m_name),
    m_body(0),
    m_proc(new ArgProc(*c.m_proc))
{
  if (c.m_body) {
    m_body = c.m_body->new_copy();
  }
}

//===========================================================================
ArgSub::~ArgSub()
{
  delete m_body;
  delete m_proc;
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
Arg* ArgSub::op(OpType optype, const std::string& opname, Arg* right)
{
  // Only allow binary ( operator - subroutine call
  if (Arg::Binary == optype && "(" == opname) {
    return call(right);
  }
  
  return Arg::op(optype,opname,right);
}

//=============================================================================
Arg* ArgSub::call(Arg* args)
{
  if (m_body) {
    // Setup the argument vector
    ArgList vargs;
    vargs.give(new ArgString("ARGV"));
    vargs.give(args->new_copy());
    m_body->arg_function("var",&vargs);
    
    return m_body->run(*m_proc);
  }
  return 0;
}


//===========================================================================
ArgError::ArgError(const char* str)
  : m_string(str)
{
}

//===========================================================================
ArgError::ArgError(const std::string& str)
  : m_string(str)
{
}

//===========================================================================
ArgError::ArgError(const ArgError& c)
  : m_string(c.m_string)
{
}

//===========================================================================
ArgError::~ArgError()
{
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
Arg* ArgError::op(OpType optype, const std::string& opname, Arg* right)
{
  return new ArgError(m_string);
}


};
