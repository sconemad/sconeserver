/* SconeServer (http://www.sconemad.com)

Argument processor

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

#include "sconex/ArgProc.h"
#include "sconex/utils.h"
namespace scx {

// Uncomment to enable debug info
//#define ArgProc_DEBUG_LOG(m) DEBUG_LOG(m)

#ifndef ArgProc_DEBUG_LOG
#  define ArgProc_DEBUG_LOG(m)
#endif
  
ArgProc::PrecedenceMap* ArgProc::s_binary_ops = 0;
ArgProc::PrecedenceMap* ArgProc::s_prefix_ops = 0;
ArgProc::PrecedenceMap* ArgProc::s_postfix_ops = 0;

//===========================================================================
ArgProc::ArgProc(const Auth& auth, Arg* ctx)
  : m_auth(auth),
    m_ctx(ctx)
{
  init();
}

//===========================================================================
ArgProc::ArgProc(const ArgProc& c)
  : m_auth(c.m_auth),
    m_ctx(c.m_ctx)
{
  init();
}

//===========================================================================
ArgProc::~ArgProc()
{
}

//===========================================================================
Arg* ArgProc::evaluate(const std::string& expr)
{
  m_expr = expr;
  m_pos = 0;
  m_type = ArgProc::Null;
  m_name = "";
  m_value = 0;

  ArgProc_DEBUG_LOG("evaluate: " << expr);
  
  Arg* result = 0;

  try {

    // Evaluate the expression
    result = expression(0,true);

  } catch (...) {
    delete result;
    while (!m_stack.empty()) {
      delete m_stack.top();
      m_stack.pop();
    }
    DEBUG_LOG("EXCEPTION in ArgProc");
    throw;
  }

  ArgProc_DEBUG_LOG("result: " << (result ? result->get_string() : "NULL"));
  
  // Clean up the stack, if anything left
  while (!m_stack.empty()) {
    ArgProc_DEBUG_LOG("left on stack: " << m_stack.top()->get_string());
    delete m_stack.top();
    m_stack.pop();
  }

  return result;
}

//===========================================================================
void ArgProc::set_ctx(Arg* ctx)
{
  m_ctx = ctx;
}

//===========================================================================
void ArgProc::set_auth(const Auth& auth)
{
  m_auth = auth;
}
  
//===========================================================================
const Auth& ArgProc::get_auth()
{
  return m_auth;
}
  
//===========================================================================
Arg* ArgProc::expression(int p, bool f, bool exec)
{
  Arg* left = primary(f,exec);

  while (true) {
    if (ArgProc::Null == m_type) {
      ArgProc_DEBUG_LOG("expr: (null) " << (exec?"":"(nx) "));
      break;

    } else if (ArgProc::Operator == m_type) {
      ArgProc_DEBUG_LOG("expr: (op) " << (exec?"":"(nx) ") << m_name);
      std::string op = m_name;
      f = !("."==op);
      int p2 = prec(Arg::Binary,op);
      if (p2 >= 0) {
        // Binary operation
        if (p2 < p) break;

        if ("["==op) {
          // Subscript operator
          Arg* right = expression(0,true,exec);
          if (0==right) {
            m_type = ArgProc::Null;
            return right;
          }
          Arg* new_left = 0;
	  try {
	    if (exec) new_left = left->op(m_auth,Arg::Binary,op,right);
	  } catch (...) {
	    delete left;
	    delete right;
	    delete new_left;
	    throw;
	  }
          delete left;
          delete right;
          left = new_left;

          // Check its followed by a closing ']'
          if (m_type!=ArgProc::Operator || "]"!=m_name) {
            m_type = ArgProc::Null;
            delete left;
            return 0;
          }
          next();

        } else if ("("==op) {
          // Function call operator
          int s = m_stack.size();
          m_stack.push(expression(0,true,exec));
          s = m_stack.size() - s;

          // Make the argument array from the stack
          ArgList* args = new ArgList();
          for (int i=0; i<s; ++i) {
            args->give(m_stack.top(),0);
            m_stack.pop();
          }

          if (s==1 && args->get(0)==0 && ")"==m_name) {
            // Empty argument list, that's ok
            delete args;
            args = new ArgList();

          } else if (m_type == ArgProc::Null) {
            // Error in argument list
            // return args[args->size()-1];
            delete args;
            delete left;
            return 0;

          } else if (ArgProc::Operator!=m_type || ")"!=m_name) {
            m_type = ArgProc::Null;
            delete args;
            delete left;
            return 0;
          }
          next();

          // Call the function

	  const std::type_info& ti = typeid(*left);
          ArgProc_DEBUG_LOG("call: " << left->get_string() << " ("<< type_name(ti) << ")");
          Arg* new_left = 0;
	  try {
	    if (exec) new_left = left->op(m_auth,Arg::Binary,op,args);
	  } catch (...) {
	    delete left;
	    delete args;
	    delete new_left;
	    throw;
	  }
          delete left;
          delete args;
          left = new_left;

        } else {

          Arg* right = 0;

	  if ("|" == op && left->get_int()) {
	    // Short circuit | op - don't exec rhs if lhs is true
	    ArgProc_DEBUG_LOG("Shorting |");
	    right = expression(p2+1,f,false);
	    delete right;
	    return left;

	  } else if ("&" == op && !left->get_int()) {
	    // Short circuit & op - don't exec rhs if lhs is false
	    ArgProc_DEBUG_LOG("Shorting &");
	    right = expression(p2+1,f,false);
	    delete right;
	    return left;

	  } else {
	    right = expression(p2+1,f,exec);
	  }
	  
	  if (0==right) {
	    ArgProc_DEBUG_LOG("RHS Null op:" << op);
	    m_type=ArgProc::Null;
	    delete left;
	    return 0;
          }
	  
          ArgProc_DEBUG_LOG("binary op: " << (left ? left->get_string() : "NULL") << " "
			    << op << " " << (right ? right->get_string() : "NULL"));
          
	  if (","==op || ":"==op) {
            m_stack.push(left);
            left=right;
          } else {
	    // Normal binary op, handled by left Arg object
            Arg* new_left = 0;
	    try {
	      new_left = left->op(m_auth,Arg::Binary,op,right);
	    } catch (...) {
	      delete left;
	      delete right;
	      delete new_left;
	      throw;
	    }
            delete left;
            delete right;
            left = new_left;
          }
        }
        if (0==left) {
          m_type=ArgProc::Null;
          return 0;
        }
        
      } else if (prec(Arg::Postfix,op) >= 0) {
        // Unary postfix operation
        ArgProc_DEBUG_LOG("postfix op: " << op);
        Arg* new_left = 0;
	try {
	  if (exec) new_left = left->op(m_auth,Arg::Postfix,op);
	} catch (...) {
	  delete left;
	  delete new_left;
	  throw;
	}
        delete left;
        left = new_left;
        next();

      } else {
        break;
      }

    } else {
      ArgProc_DEBUG_LOG("expr: (unknown) " << (exec?"":"(nx) "));
      m_type=ArgProc::Null;
      // NOT SURE ABOUT THIS:
      //      delete left;
      //      return 0;
      return left;
    }

  }

  return left;
}

//===========================================================================
Arg* ArgProc::primary(bool f, bool exec)
{
  next();

  switch (m_type) {
    case ArgProc::Operator: {
      ArgProc_DEBUG_LOG("primary: (op) " << m_name);
      std::string op = m_name;
      int p2 = prec(Arg::Prefix,op);

      // Unary prefix operation
      if (p2 >= 0) {
        Arg* right = expression(p2,f,exec);
        if (right) {
          ArgProc_DEBUG_LOG("prefix op: " << op);
          Arg* result = 0;
	  try {
	    if (exec) result = right->op(m_auth,Arg::Prefix,op);
	  } catch (...) {
	    delete right;
	    delete result;
	    throw;
	  }
          delete right;
          return result;
        }
	// Special case: ! (NULL) should return true
	if (op == "!") return new ArgInt(1);
        return 0;
      }

      // Sub-expression
      if ("("==op) {
        Arg* result = expression(0,true,exec);

	// Check its followed by a closing ')'
	if (m_name != ")") {
	  m_type = ArgProc::Null;
	  delete result;
	  return 0;
	}
        next();
        return result;
      }

      // List initializer
      if ("["==op) {
        int s = m_stack.size();
        m_stack.push(expression(0,true,exec));
        s = m_stack.size() - s;
	
        // List
	ArgList* list = new ArgList();
	Arg* result = list;
	result = list;
	if (m_type != ArgProc::Null) {
	  for (int i=0; i<s; ++i) {
	    list->give(m_stack.top(),0);
	    m_stack.pop();
	  }
	}

	// Check its followed by a closing ']'
	if (m_name != "]") {
	  m_type = ArgProc::Null;
	  delete result;
	  return 0;
        }
        next();
        return result;
      }

      // Map initializer
      if ("{"==op) {
        int s = m_stack.size();
        m_stack.push(expression(0,true,exec));
        s = m_stack.size() - s;

        // Map
	ArgMap* map = new ArgMap();
	Arg* result = map;
	if (m_type != ArgProc::Null) {
	  if (s%2 != 0) {
	    delete result;
	    result = new ArgError("Map initialiser should contain an even number of values");
	  } else  {
	    for (int i=0; i<s; i+=2) {
	      Arg* aval = m_stack.top();
	      m_stack.pop();
	      Arg* aname = m_stack.top();
	      m_stack.pop();
	      map->give(aname->get_string(),aval);
	      delete aname;
	    }
	  }
	}

	// Check its followed by a closing ')'
	if (m_name != "}") {
	  m_type = ArgProc::Null;
	  delete result;
	  return 0;
	}
        next();
        return result;
      }

    } break;

    case ArgProc::Value: {
      ArgProc_DEBUG_LOG("primary: (value) " << (m_value ? m_value->get_string() : "NULL"));
      Arg* a = m_value;
      next();
      return a;
    }

    case ArgProc::Name: {
      ArgProc_DEBUG_LOG("primary: (name) " << m_name);
      Arg* a = 0;
      if (f) {
        // Lookup name using context ':' (resolve) op
        if (m_ctx) {
          ArgProc_DEBUG_LOG("resolving: " << m_name);
          a = new ArgString(m_name);
          Arg* b = 0;
	  try {
	    b = m_ctx->op(m_auth,Arg::Binary,":",a);
	  } catch (...) {
	    delete b;
	    delete a;
	    throw;
	  }
          delete a;
          a = b;
        }
      } else {
        a = new ArgString(m_name);
      }

      if (a) {
        next();
      } else {
        m_type=ArgProc::Null;
      }
      return a;
    }

    case ArgProc::Null:
      ArgProc_DEBUG_LOG("primary: (null)");
      break;
  }

  m_type=ArgProc::Null;
  return 0;
}

#undef ArgProc_DEBUG_LOG
#define ArgProc_DEBUG_LOG(m)

//===========================================================================
void ArgProc::next()
{
  int len = m_expr.length();
  m_type = ArgProc::Null;

  while (m_pos < len && isspace(m_expr[m_pos])) ++m_pos;
  if (m_pos >= len) {
    ArgProc_DEBUG_LOG("next: (end)");
    return;
  }

  int i = m_pos;
  char c = m_expr[i];

  // Operator
  int maxop=2;
  for (int ol=std::min(len - m_pos,maxop); ol>0; --ol) {
    m_name = m_expr.substr(m_pos,ol);
    if (s_binary_ops->count(m_name) ||
        s_prefix_ops->count(m_name) ||
        s_postfix_ops->count(m_name)) {
      m_type = ArgProc::Operator;
      m_pos += ol;
      ArgProc_DEBUG_LOG("next: (op) " << m_name);
      return;
    }
  }

  // Numeric
  if (isdigit(c)) {
    bool ends = true;
    int ex=0;      // Counts exponentiations
    int num_dd=0;  // Counts decimal delimiters
    char pc=c;     // Previous c

    while (++i < len) {
      c = m_expr[i];
      ends = !isdigit(c);
      switch (c) {
        case '.':
          ends=false;
          ++num_dd;
          break;
        case 'e': case 'E':
          ends=(++ex>1);
          break;
        case '+': case '-':
          ends=(pc!='e'&&pc!='E');
          break;
      }
      if (ends) break;
      pc=c;
    }

    m_name = m_expr.substr(m_pos,i-m_pos);
    m_pos = i;
    m_type = ArgProc::Value;

    if (num_dd==0) {
      int value=atoi(m_name.c_str());
      m_value = new ArgInt(value);
      ArgProc_DEBUG_LOG("next: (int) " << value);
      return;
      
    } else if (num_dd==1) {
      double value=atof(m_name.c_str());
      m_value = new ArgReal(value);
      ArgProc_DEBUG_LOG("next: (real) " << value);
      return;
      
    } else if (num_dd==3) {
      // IP Address
      m_value = new ArgString(m_name);
      ArgProc_DEBUG_LOG("next: (ipaddr) " << m_name);
      return;
    }
  }

  // Alpha
  if (isalpha(c) || '_'==c) {
    while (++i < len) {
      c = m_expr[i];
      if (!(isalnum(c) || '_'==c)) {
        break;
      }
    }

    m_name = m_expr.substr(m_pos,i-m_pos);
    m_pos = i;

    if (s_binary_ops->count(m_name) ||
        s_prefix_ops->count(m_name) ||
        s_postfix_ops->count(m_name)) {
      // Alpha operator
      m_type = ArgProc::Operator;
      ArgProc_DEBUG_LOG("next: (op) " << m_name);
      return;
    }

    // Special name
    if ("NULL" == m_name) {
      m_type = ArgProc::Value;
      m_value = new ArgError("NULL");
      ArgProc_DEBUG_LOG("next: (NULL) " << m_name);
      return;
    }
    
    // Name
    m_type = ArgProc::Name;
    ArgProc_DEBUG_LOG("next: (name) " << m_name);
    return;
  }

  // Parenthesis
  if ('('==c || ')'==c || '['==c || ']'==c || '{'==c || '}'==c) {
    m_type = ArgProc::Operator;
    m_name = m_expr.substr(m_pos,1);
    ++m_pos;
    ArgProc_DEBUG_LOG("next: (op) " << m_name);
    return;
  }

  // Quote delimited string
  if ('"'==c || '\''==c) {
    char delim = c;
    m_name = "";
    while (++i < len) {
      c = m_expr[i];

      if (c==delim) {
	// Delimiter reached
        m_pos = i+1;
        m_type = ArgProc::Value;
        m_value = new ArgString(m_name);
        ArgProc_DEBUG_LOG("next: (string) " << m_name);
        return;
      }

      if (c!='\\') {
	// Normal character
	m_name += c;

      } else {
	// Escape sequence
	if (++i >= len) {
	  break;
	}
	c = m_expr[i];
	switch (c) {
	  case 'n': m_name += '\n'; break;
	  case 'r': m_name += '\r'; break;
	  case 'b': m_name += '\b'; break;
	  case 't': m_name += '\t'; break;
	  case 'f': m_name += '\f'; break;
	  case 'a': m_name += '\a'; break;
	  case 'v': m_name += '\v'; break;
	  case '0': m_name += '\0'; break;
  	  case 'x': {
	    if (i+3 < len) {
	      char sx[3] = {m_expr[i+1],m_expr[i+2],0};
	      long x = strtol(sx,0,16);
	      i += 2;
	      m_name += (char)x;
	    }
	  } break;
  	  default: m_name += c; break;
	}
      }
    }
    // No closing delimiter!
    m_name = m_expr.substr(m_pos,i-m_pos);
    m_pos = i;
    ArgProc_DEBUG_LOG("next: (string) " << m_name);
    return;
  }

  // Don't know
  ArgProc_DEBUG_LOG("next: (unknown) " << m_name);
  return;
}

//===========================================================================
int ArgProc::prec(Arg::OpType optype, const std::string& op) const
{
  switch (optype) {
    case Arg::Prefix: {
      PrecedenceMap::const_iterator iter = s_prefix_ops->find(op);
      if (iter!=s_prefix_ops->end()) return iter->second;
    } break;
    case Arg::Postfix: {
      PrecedenceMap::const_iterator iter = s_postfix_ops->find(op);
      if (iter!=s_postfix_ops->end()) return iter->second;
    } break;
    case Arg::Binary: {
      PrecedenceMap::const_iterator iter = s_binary_ops->find(op);
      if (iter!=s_binary_ops->end()) return iter->second;
    } break;
  }
  return -1;
}

//===========================================================================
void ArgProc::init()
{
  if (s_binary_ops) return;

  s_binary_ops = new PrecedenceMap();
  s_prefix_ops = new PrecedenceMap();
  s_postfix_ops = new PrecedenceMap();
  
  int p=0;
  (*s_binary_ops)[","]  = ++p; // Sequential evaluation
  (*s_binary_ops)[":"]  = p;

  (*s_binary_ops)["="]  = ++p; // Assignment
  (*s_binary_ops)["+="]  = p;
  (*s_binary_ops)["-="]  = p;
  (*s_binary_ops)["*="]  = p;
  (*s_binary_ops)["/="]  = p;

  (*s_binary_ops)["|"]  = ++p; // Logical OR

  (*s_binary_ops)["xor"]= ++p; // Logical XOR

  (*s_binary_ops)["&"]  = ++p; // Logical AND

  (*s_binary_ops)["=="] = ++p; // Equality
  (*s_binary_ops)["!="] = p;

  (*s_binary_ops)[">"]  = ++p; // Relational
  (*s_binary_ops)["<"]  = p;
  (*s_binary_ops)[">="] = p;
  (*s_binary_ops)["<="] = p;

  (*s_binary_ops)["+"]  = ++p; // Additive
  (*s_binary_ops)["-"]  = p;

  (*s_binary_ops)["*"]  = ++p; // Multiplicative
  (*s_binary_ops)["/"]  = p;
  (*s_binary_ops)["%"]  = p;

  (*s_binary_ops)["^"]  = ++p; // Power

  (*s_postfix_ops)["!"] = ++p; // Unary postfix
  (*s_postfix_ops)["++"] = p; 
  (*s_postfix_ops)["--"] = p; 

  (*s_prefix_ops)["+"]  = ++p; // Unary prefix
  (*s_prefix_ops)["-"]  = p;
  (*s_prefix_ops)["!"]  = p;
  (*s_prefix_ops)["++"]  = p;
  (*s_prefix_ops)["--"]  = p;

  (*s_binary_ops)["["]  = ++p; // Subscript
  (*s_binary_ops)["("]  = ++p; // Function call
  (*s_binary_ops)["{"]  = ++p; // Map initializer

  (*s_binary_ops)["."]  = ++p; // Member access
}

};
