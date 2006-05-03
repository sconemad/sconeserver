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
ArgString::ArgString(const char* str)
  : m_string(str)
{
}

//===========================================================================
ArgString::ArgString(const std::string& str)
  : m_string(str)
{
}

//===========================================================================
ArgString::ArgString(const ArgString& c)
  : m_string(c.m_string)
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
  if (opname=="+") {
    return new ArgString(m_string + right->get_string());
  }
  return 0;
}


//===========================================================================
ArgInt::ArgInt(int value)
  : m_value(value)
{
}

//===========================================================================
ArgInt::ArgInt(const ArgInt& c)
  : m_value(c.m_value)
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

      } else if ("!"==opname) { // Not
        return new ArgInt(!m_value);
      }
    } break;

    case Arg::Postfix: {
      if ("!"==opname) { // Factorial
        int a = 1;
        for (int i=abs((int)m_value); i>1; --i) a *= i;
        return new ArgInt(a * (m_value<0 ? -1 : 1));
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
          return new ArgInt(m_value / rvalue);

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

        } else if ("&"==opname) { // And
          return new ArgInt(m_value && rvalue);

        } else if ("|"==opname) { // Or
          return new ArgInt(m_value | rvalue);
          
        } else if ("xor"==opname) { // Xor
          return new ArgInt((m_value!=0) ^ (rvalue!=0));

        }
      }
    } break;

  }

  return 0;
}


//===========================================================================
ArgReal::ArgReal(double value)
  : m_value(value)
{
}

//===========================================================================
ArgReal::ArgReal(const ArgReal& c)
  : m_value(c.m_value)
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

        }
      }
    } break;

  }

  return 0;
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
  std::list<Arg*>::iterator iter = m_list.begin();
  while (iter != m_list.end()) {
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
Arg* ArgList::op(OpType type, const std::string& opname, Arg* right)
{
  return 0;
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
ArgFunction::ArgFunction(const std::string& name)
  : m_name(name)
{
}

//===========================================================================
ArgFunction::ArgFunction(const ArgFunction& c)
  : m_name(c.m_name)
{
}

//===========================================================================
ArgFunction::~ArgFunction()
{
}

//===========================================================================
Arg* ArgFunction::new_copy() const
{
  return new ArgFunction(*this);
}

//===========================================================================
std::string ArgFunction::get_string() const
{
  std::ostringstream oss;
  oss << m_name << " function";
  return oss.str();
}

//===========================================================================
int ArgFunction::get_int() const
{
  return !m_name.empty();
}

//===========================================================================
Arg* ArgFunction::op(OpType optype, const std::string& opname, Arg* right)
{
  //  ArgList* args = (ArgList*)right;

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
