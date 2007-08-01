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
namespace scx {

// Uncomment to enable debug info
//#define ArgProc_DEBUG_LOG(m) DEBUG_LOG(m)

#ifndef ArgProc_DEBUG_LOG
#  define ArgProc_DEBUG_LOG(m)
#endif
  
std::map<std::string,int>* ArgProc::s_binary_ops;
std::map<std::string,int>* ArgProc::s_prefix_ops;
std::map<std::string,int>* ArgProc::s_postfix_ops;

//===========================================================================
ArgProc::ArgProc(Arg* ctx)
  : m_ctx(ctx)
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
  
  // Procuate the expression
  Arg* result = expression(0,true);

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
Arg* ArgProc::expression(int p, bool f)
{
  Arg* left = primary(f);

  while (true) {
    if (ArgProc::Null == m_type) {
      ArgProc_DEBUG_LOG("expr: (null)");
      break;

    } else if (ArgProc::Operator == m_type) {
      ArgProc_DEBUG_LOG("expr: (op) " << m_name);
      std::string op = m_name;
      f = !("."==op || ":"==op);
      int p2 = prec(Arg::Binary,op);
      if (p2 >= 0) {
        // Binary operation
        if (p2 < p) break;

        if ("["==op) {
          // Subscript operator
          Arg* right = expression(0,true);
          if (0==right) {
            m_type = ArgProc::Null;
            return right;
          }
          Arg* new_left = left->op(Arg::Binary,op,right);
          delete left;
          delete right;
          left = new_left;

          // Check its followed by a closing ']'
          if (m_type!=ArgProc::Operator || "]"==m_name) {
            m_type = ArgProc::Null;
            delete left;
            return 0;
          }
          next();

        } else if ("("==op) {
          // Function call operator
          int s = m_stack.size();
          m_stack.push(expression(0,true));
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

          ArgProc_DEBUG_LOG("call: " << left->get_string());
          Arg* new_left = left->op(Arg::Binary,op,args);
          delete left;
          delete args;
          left = new_left;

        } else {

          Arg* right = expression(p2+1,f);

          if (0==right) {
            m_type=ArgProc::Null;
            delete left;
            return 0;
          }

          ArgProc_DEBUG_LOG("binary op: " << op);
          
          if (","==op) {
            m_stack.push(left);
            left=right;
          } else {
            Arg* new_left = left->op(Arg::Binary,op,right);
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
        Arg* new_left = left->op(Arg::Postfix,op);
        delete left;
        left = new_left;
        next();

      } else {
        break;
      }

    } else {
      ArgProc_DEBUG_LOG("expr: (unknown)");
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
Arg* ArgProc::primary(bool f)
{
  next();

  switch (m_type) {
    case ArgProc::Operator: {
      ArgProc_DEBUG_LOG("primary: (op) " << m_name);
      std::string op = m_name;
      int p2 = prec(Arg::Prefix,op);

      // Unary prefix operation
      if (p2 >= 0) {
        Arg* right = expression(p2,f);
        if (right) {
          ArgProc_DEBUG_LOG("prefix op: " << op);
          Arg* result = right->op(Arg::Prefix,op);
          delete right;
          return result;
        }
        return 0;
      }

      // Sub-expression
      if ("("==op) {
        int s = m_stack.size();
        m_stack.push(expression(0,true));
        s = m_stack.size() - s;
        Arg* result;

        // List
        if (s>1) {
          ArgList* l = new ArgList();
          for (int i=0; i<s; ++i) {
            l->give(m_stack.top(),0);
            m_stack.pop();
          }
          result = l;
        } else {
          result = m_stack.top();
          m_stack.pop();
        }

        if (s==1 && result==0 && m_name==")") {
          // Empty array, that's ok just return it
          result = new ArgString("empty array");

        } else if (m_type == ArgProc::Null) {
          // Error in subexpression
          delete result;
          result = 0;
          //          if (result != 0) {
          //            result = result[result->size()-1];
          //          }
          return result;

        } else {
          // Check its followed by a closing ')'
          if (m_type!=ArgProc::Operator || m_name!=")") {
            m_type = ArgProc::Null;
            delete result;
            return 0;
          }
        }
        next();
        return result;
      }


    } break;

    case ArgProc::Value: {
      ArgProc_DEBUG_LOG("primary: (value) " << m_value->get_string());
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
          Arg* b = m_ctx->op(Arg::Binary,":",a);
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
    int num_co=0;  // Counts colon delimiters
    char pc=c;     // Previous c

    while (++i < len) {
      c = m_expr[i];
      ends = !isdigit(c);
      switch (c) {
        case '.':
          ends=false;
          ++num_dd;
          break;
        case ':':
          ends=false;
          ++num_co;
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

    // Name
    m_type = ArgProc::Name;
    ArgProc_DEBUG_LOG("next: (name) " << m_name);
    return;
  }

  // Parenthesis
  if ('('==c || ')'==c || '['==c || ']'==c) {
    m_type = ArgProc::Operator;
    m_name = m_expr.substr(m_pos,1);
    ++m_pos;
    ArgProc_DEBUG_LOG("next: (op) " << m_name);
    return;
  }

  // Quote delimited string
  if ('"'==c || '\''==c) {
    char delim = c;
    while (++i < len) {
      c = m_expr[i];
      if (c==delim) {
        m_name = m_expr.substr(m_pos+1,i-m_pos-1);
        m_pos = i+1;
        m_type = ArgProc::Value;
        m_value = new ArgString(m_name);
        ArgProc_DEBUG_LOG("next: (string) " << m_name);
        return;
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
      std::map<std::string,int>::const_iterator iter = s_prefix_ops->find(op);
      if (iter!=s_prefix_ops->end()) return (*iter).second;
    } break;
    case Arg::Postfix: {
      std::map<std::string,int>::const_iterator iter = s_postfix_ops->find(op);
      if (iter!=s_postfix_ops->end()) return (*iter).second;
    } break;
    case Arg::Binary: {
      std::map<std::string,int>::const_iterator iter = s_binary_ops->find(op);
      if (iter!=s_binary_ops->end()) return (*iter).second;
    } break;
  }
  return -1;
}

//===========================================================================
void ArgProc::init()
{
  if (s_binary_ops) return;

  s_binary_ops = new std::map<std::string,int>;
  s_prefix_ops = new std::map<std::string,int>;
  s_postfix_ops = new std::map<std::string,int>;
  
  int p=0;
  (*s_binary_ops)[","]  = ++p; // Sequential evaluation

  (*s_binary_ops)["="]  = ++p; // Assignment

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

  (*s_binary_ops)[":"]  = ++p; // Scope resoltion
  (*s_binary_ops)["."]  = ++p; // Member access
}

};
