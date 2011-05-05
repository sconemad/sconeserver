/* SconeServer (http://www.sconemad.com)

SconeScript expression evaluator

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

#include "sconex/ScriptExpr.h"
#include "sconex/VersionTag.h"
#include "sconex/Uri.h"
#include "sconex/Date.h"
#include "sconex/MimeType.h"
#include "sconex/RegExp.h"
#include "sconex/utils.h"
namespace scx {

// Uncomment to enable debug info
//#define ScriptExpr_DEBUG_LOG(m) DEBUG_LOG(m)

#ifndef ScriptExpr_DEBUG_LOG
#  define ScriptExpr_DEBUG_LOG(m)
#endif
  
ScriptExpr::OperatorMap* ScriptExpr::s_binary_ops = 0;
ScriptExpr::OperatorMap* ScriptExpr::s_prefix_ops = 0;
ScriptExpr::OperatorMap* ScriptExpr::s_postfix_ops = 0;
ScriptExpr::PrecedenceMap* ScriptExpr::s_op_precs = 0;
ScriptRefTo<StandardContext>* ScriptExpr::s_standard_context = 0;


//===========================================================================
ScriptExpr::ScriptExpr(const ScriptAuth& auth, ScriptRef* ctx)
  : m_auth(auth)
{
  init();
  m_contexts.push_back(s_standard_context);
  if (ctx) m_contexts.push_back(ctx);
}

//===========================================================================
ScriptExpr::ScriptExpr(const ScriptExpr& c)
  : m_auth(c.m_auth),
    m_contexts(c.m_contexts)
{
  init();
}

//===========================================================================
ScriptExpr::~ScriptExpr()
{
}

//===========================================================================
ScriptRef* ScriptExpr::evaluate(const std::string& expr)
{
  m_expr = expr;
  m_pos = 0;
  m_type = ScriptExpr::Null;
  m_name = "";
  m_value = 0;

  ScriptExpr_DEBUG_LOG("evaluate: " << expr);
  
  ScriptRef* result = 0;

  try {

    // Evaluate the expression
    result = expression(0,true);

  } catch (...) {
    delete result;
    while (!m_stack.empty()) {
      delete m_stack.top();
      m_stack.pop();
    }
    DEBUG_LOG("EXCEPTION in ScriptExpr");
    throw;
  }

  ScriptExpr_DEBUG_LOG("result: " 
		       << (result ? result->object()->get_string() : "NULL"));
  
  // Clean up the stack, if anything left
  while (!m_stack.empty()) {
    ScriptExpr_DEBUG_LOG("left on stack: " 
			 << m_stack.top()->object()->get_string());
    delete m_stack.top();
    m_stack.pop();
  }

  return result;
}

//===========================================================================
void ScriptExpr::set_ctx(ScriptRef* ctx, bool include_standard)
{
  m_contexts.clear();
  if (include_standard) m_contexts.push_back(s_standard_context);
  if (ctx) m_contexts.push_back(ctx);
}

//===========================================================================
void ScriptExpr::set_auth(const ScriptAuth& auth)
{
  m_auth = auth;
}
  
//===========================================================================
const ScriptAuth& ScriptExpr::get_auth()
{
  return m_auth;
}
  
//===========================================================================
void ScriptExpr::register_type(const std::string& type,
			       Provider<ScriptObject>* factory)
{
  init();
  s_standard_context->object()->register_provider(type,factory);
}

//===========================================================================
void ScriptExpr::unregister_type(const std::string& type,
				 Provider<ScriptObject>* factory)
{
  init();
  s_standard_context->object()->unregister_provider(type,factory);
}

//===========================================================================
ScriptRef* ScriptExpr::expression(int p, bool f, bool exec)
{
  ScriptRef* left = primary(f,exec);

  while (true) {
    if (ScriptExpr::Null == m_type) {
      ScriptExpr_DEBUG_LOG("expr: (null) " << (exec?"":"(nx) "));
      break;

    } else if (ScriptExpr::Operator == m_type) {
      ScriptExpr_DEBUG_LOG("expr: (op) " << (exec?"":"(nx) ") << m_name);
      ScriptOp::OpType op = op_type(Binary,m_name);

      f = (ScriptOp::Lookup != op);
      int p2 = op_prec(op);
      if (p2 >= 0) {
        // Binary operation
        if (p2 < p) break;

        if (ScriptOp::Subscript == op) {
          // Subscript operator
          ScriptRef* right = expression(0,true,exec);
          if (0==right) {
            m_type = ScriptExpr::Null;
            return right;
          }
          ScriptRef* new_left = 0;
	  try {
	    if (exec) 
	      new_left = left->script_op(m_auth,op,right);
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
          if (m_type!=ScriptExpr::Operator || "]"!=m_name) {
            m_type = ScriptExpr::Null;
            delete left;
            return 0;
          }
          next();

        } else if (ScriptOp::List == op) {
          // Function call operator
          int s = m_stack.size();
          m_stack.push(expression(0,true,exec));
          s = m_stack.size() - s;

          // Make the argument array from the stack
          ScriptList* args = new ScriptList();
	  ScriptRef* args_ref = new ScriptRef(args);
          for (int i=0; i<s; ++i) {
            args->give(m_stack.top(),0);
            m_stack.pop();
          }

          if (s==1 && args->get(0)==0 && ")"==m_name) {
            // Empty argument list, that's ok
            delete args_ref;
            args = new ScriptList();
	    args_ref = new ScriptRef(args);

          } else if (m_type == ScriptExpr::Null) {
            // Error in argument list
            // return args[args->size()-1];
            delete args_ref;
            delete left;
            return 0;

          } else if (ScriptExpr::Operator!=m_type || ")"!=m_name) {
            m_type = ScriptExpr::Null;
            delete args_ref;
            delete left;
            return 0;
          }
          next();

          // Call the function
          ScriptExpr_DEBUG_LOG("call: " 
			       << left->object()->get_string() 
			       << " (" << type_name(typeid(*left->object())) 
			       << ")");
          ScriptRef* new_left = 0;
	  try {
	    if (exec) 
	      new_left = left->script_op(m_auth,op,args_ref);
	  } catch (...) {
	    delete left;
	    delete args_ref;
	    delete new_left;
	    throw;
	  }
          delete left;
          delete args_ref;
          left = new_left;

        } else {

          ScriptRef* right = 0;

	  if (ScriptOp::Or == op && left->object()->get_int()) {
	    // Short circuit | op - don't exec rhs if lhs is true
	    ScriptExpr_DEBUG_LOG("Shorting |");
	    right = expression(p2+1,f,false);
	    delete right;
	    return left;

	  } else if (ScriptOp::And == op && !left->object()->get_int()) {
	    // Short circuit & op - don't exec rhs if lhs is false
	    ScriptExpr_DEBUG_LOG("Shorting &");
	    right = expression(p2+1,f,false);
	    delete right;
	    return left;

	  } else {
	    right = expression(p2+1,f,exec);
	  }
	  
	  if (0==right) {
	    ScriptExpr_DEBUG_LOG("RHS Null op:" << op);
	    m_type=ScriptExpr::Null;
	    delete left;
	    return 0;
          }
	  
          ScriptExpr_DEBUG_LOG("binary op: " 
			       << (left ? left->object()->get_string() : "NULL") 
			       << " " << op << " " 
			       << (right ? right->object()->get_string() : "NULL"));
          
	  if (ScriptOp::Sequential == op || ScriptOp::Resolve == op) {
            m_stack.push(left);
            left=right;
          } else {
	    // Normal binary op, handled by left Script object
            ScriptRef* new_left = 0;
	    try {
	      if (left) 
		new_left = left->script_op(m_auth,op,right);
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
          m_type=ScriptExpr::Null;
          return 0;
        }
        
      } else if ((op = op_type(Postfix,m_name)) && (op_prec(op) >= 0)) {
        // Unary postfix operation
        ScriptExpr_DEBUG_LOG("postfix op: " << op);
        ScriptRef* new_left = 0;
	try {
	  if (exec) 
	    new_left = left->script_op(m_auth,op);
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
      ScriptExpr_DEBUG_LOG("expr: (unknown) " << (exec?"":"(nx) "));
      m_type=ScriptExpr::Null;
      // NOT SURE ABOUT THIS:
      //      delete left;
      //      return 0;
      return left;
    }

  }

  return left;
}

//===========================================================================
ScriptRef* ScriptExpr::primary(bool f, bool exec)
{
  next();

  switch (m_type) {
    case ScriptExpr::Operator: {
      ScriptExpr_DEBUG_LOG("primary: (op) " << m_name);
      ScriptOp::OpType op = ScriptOp::Unknown;

      // Check for unary prefix operation
      op = op_type(Prefix,m_name);
      int p2 = op_prec(op);
      if (p2 >= 0) {
        ScriptRef* right = expression(p2,f,exec);
        if (right) {
          ScriptExpr_DEBUG_LOG("prefix op: " << op);
          ScriptRef* result = 0;
	  try {
	    if (exec) 
	      result = right->script_op(m_auth,op);
	  } catch (...) {
	    delete right;
	    delete result;
	    throw;
	  }
          delete right;
          return result;
        }
	// Special case: ! (NULL) should return true
	if (ScriptOp::Not == op) return ScriptInt::new_ref(1);
        return 0;
      }

      // Binary operation
      op = op_type(Binary,m_name);

      // Sub-expression
      if (ScriptOp::List == op) {
        ScriptRef* result = expression(0,true,exec);

	// Check its followed by a closing ')'
	if (m_name != ")") {
	  m_type = ScriptExpr::Null;
	  delete result;
	  return 0;
	}
        next();
        return result;
      }

      // List initializer
      if (ScriptOp::Subscript == op) {
        int s = m_stack.size();
        m_stack.push(expression(0,true,exec));
        s = m_stack.size() - s;
	
        // List
	ScriptList* list = new ScriptList();
	ScriptRef* result = new ScriptRef(list);
	if (m_type != ScriptExpr::Null) {
	  for (int i=0; i<s; ++i) {
	    list->give(m_stack.top(),0);
	    m_stack.pop();
	  }
	}

	// Check its followed by a closing ']'
	if (m_name != "]") {
	  m_type = ScriptExpr::Null;
	  delete result;
	  return 0;
        }
        next();
        return result;
      }

      // Map initializer
      if (ScriptOp::Map == op) {
        int s = m_stack.size();
        m_stack.push(expression(0,true,exec));
        s = m_stack.size() - s;

        // Map
	ScriptMap* map = new ScriptMap();
	ScriptRef* result = new ScriptRef(map);
	if (m_type != ScriptExpr::Null) {
	  if (s%2 != 0) {
	    delete result;
	    result = ScriptError::new_ref(
              "Map initialiser should contain an even number of values");
	  } else  {
	    for (int i=0; i<s; i+=2) {
	      ScriptRef* aval = m_stack.top();
	      m_stack.pop();
	      ScriptRef* aname = m_stack.top();
	      m_stack.pop();
	      map->give(aname->object()->get_string(),aval);
	      delete aname;
	    }
	  }
	}

	// Check its followed by a closing ')'
	if (m_name != "}") {
	  m_type = ScriptExpr::Null;
	  delete result;
	  return 0;
	}
        next();
        return result;
      }

    } break;

    case ScriptExpr::Value: {
      ScriptExpr_DEBUG_LOG("primary: (value) " 
        << (m_value ? m_value->object()->get_string() : "NULL"));
      ScriptRef* a = m_value;
      next();
      return a;
    }

    case ScriptExpr::Name: {
      ScriptExpr_DEBUG_LOG("primary: (name) " << m_name);
      ScriptRef* a = f ? resolve_name(m_name) : ScriptString::new_ref(m_name);
      if (a) {
        next();
      } else {
        m_type=ScriptExpr::Null;
      }
      return a;
    }

    case ScriptExpr::Null:
      ScriptExpr_DEBUG_LOG("primary: (null)");
      break;
  }

  m_type=ScriptExpr::Null;
  return 0;
}

//===========================================================================
ScriptOp::OpType ScriptExpr::op_type(ScriptExpr::OpMode opmode, 
				     const std::string& opname) const
{
  OperatorMap::const_iterator iter;

  switch (opmode) {
  case Prefix:
    iter = s_prefix_ops->find(opname);
    if (iter!=s_prefix_ops->end()) return iter->second;
    break;

  case Postfix:
    iter = s_postfix_ops->find(opname);
    if (iter!=s_postfix_ops->end()) return iter->second;
    break;

  case Binary:
    iter = s_binary_ops->find(opname);
    if (iter!=s_binary_ops->end()) return iter->second;
    break;
  }

  return ScriptOp::Unknown;
}

//===========================================================================
int ScriptExpr::op_prec(ScriptOp::OpType optype) const
{
  PrecedenceMap::const_iterator iter = s_op_precs->find(optype);
  if (iter!=s_op_precs->end()) return iter->second;
  return -1;
}

//===========================================================================
ScriptRef* ScriptExpr::resolve_name(const std::string& name)
{
  ScriptRef* name_ref = ScriptString::new_ref(name);
  ScriptRef* result = 0;

  for (ContextList::iterator it = m_contexts.begin();
       it != m_contexts.end();
       ++it) {
    delete result; result = 0;

    try {
      // Attempt to resolve name in context
      result = (*it)->script_op(m_auth, ScriptOp::Resolve, name_ref);
    } catch (...) {
      delete name_ref;
      delete result;
      throw;
    }

    if (!BAD_SCRIPTREF(result)) {
      break;
    }
  }

  delete name_ref;
  return result;
}

// Comment these if you want debug logging from next() (quite verbose)
#undef ScriptExpr_DEBUG_LOG
#define ScriptExpr_DEBUG_LOG(m)

//===========================================================================
void ScriptExpr::next()
{
  int len = m_expr.length();
  m_type = ScriptExpr::Null;

  while (m_pos < len && isspace(m_expr[m_pos])) ++m_pos;
  if (m_pos >= len) {
    ScriptExpr_DEBUG_LOG("next: (end)");
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
      m_type = ScriptExpr::Operator;
      m_pos += ol;
      ScriptExpr_DEBUG_LOG("next: (op) " << m_name);
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
    m_type = ScriptExpr::Value;

    if (num_dd==0) {
      int value=atoi(m_name.c_str());
      m_value = ScriptInt::new_ref(value);
      ScriptExpr_DEBUG_LOG("next: (int) " << value);
      return;
      
    } else if (num_dd==1) {
      double value=atof(m_name.c_str());
      m_value = ScriptReal::new_ref(value);
      ScriptExpr_DEBUG_LOG("next: (real) " << value);
      return;
      
    } else if (num_dd==3) {
      // IP Address
      m_value = ScriptString::new_ref(m_name);
      ScriptExpr_DEBUG_LOG("next: (ipaddr) " << m_name);
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
      m_type = ScriptExpr::Operator;
      ScriptExpr_DEBUG_LOG("next: (op) " << m_name);
      return;
    }

    // Special name
    if ("NULL" == m_name) {
      m_type = ScriptExpr::Value;
      m_value = ScriptError::new_ref("NULL");
      ScriptExpr_DEBUG_LOG("next: (NULL) " << m_name);
      return;
    }
    
    // Name
    m_type = ScriptExpr::Name;
    ScriptExpr_DEBUG_LOG("next: (name) " << m_name);
    return;
  }

  // Parenthesis
  if ('('==c || ')'==c || '['==c || ']'==c || '{'==c || '}'==c) {
    m_type = ScriptExpr::Operator;
    m_name = m_expr.substr(m_pos,1);
    ++m_pos;
    ScriptExpr_DEBUG_LOG("next: (op) " << m_name);
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
        m_type = ScriptExpr::Value;
        m_value = ScriptString::new_ref(m_name);
        ScriptExpr_DEBUG_LOG("next: (string) " << m_name);
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
    ScriptExpr_DEBUG_LOG("next: (string) " << m_name);
    return;
  }

  // Don't know
  ScriptExpr_DEBUG_LOG("next: (unknown) " << m_name);
  return;
}

//===========================================================================
void ScriptExpr::init()
{
  if (s_binary_ops) return;

  // Initialise operator name -> type mapping

  s_binary_ops = new OperatorMap();
  s_prefix_ops = new OperatorMap();
  s_postfix_ops = new OperatorMap();

  (*s_binary_ops)[","] = ScriptOp::Sequential;
  (*s_binary_ops)[":"] = ScriptOp::Resolve;
  (*s_binary_ops)["="] = ScriptOp::Assign;
  (*s_binary_ops)["+="] = ScriptOp::AddAssign;
  (*s_binary_ops)["-="] = ScriptOp::SubtractAssign;
  (*s_binary_ops)["*="] = ScriptOp::MultiplyAssign;
  (*s_binary_ops)["/="] = ScriptOp::DivideAssign;
  (*s_binary_ops)["|"]  = ScriptOp::Or;
  (*s_binary_ops)["xor"] = ScriptOp::Xor;
  (*s_binary_ops)["&"]  = ScriptOp::And;
  (*s_binary_ops)["=="] = ScriptOp::Equality;
  (*s_binary_ops)["!="] = ScriptOp::Inequality;
  (*s_binary_ops)[">"]  = ScriptOp::GreaterThan;
  (*s_binary_ops)["<"]  = ScriptOp::LessThan;
  (*s_binary_ops)[">="] = ScriptOp::GreaterThanOrEqualTo;
  (*s_binary_ops)["<="] = ScriptOp::LessThanOrEqualTo;
  (*s_binary_ops)["+"]  = ScriptOp::Add;
  (*s_binary_ops)["-"]  = ScriptOp::Subtract;
  (*s_binary_ops)["*"]  = ScriptOp::Multiply;
  (*s_binary_ops)["/"]  = ScriptOp::Divide;
  (*s_binary_ops)["%"]  = ScriptOp::Modulus;
  (*s_binary_ops)["^"]  = ScriptOp::Power;
  (*s_postfix_ops)["!"] = ScriptOp::Factorial;
  (*s_postfix_ops)["++"] = ScriptOp::PostIncrement;
  (*s_postfix_ops)["--"] = ScriptOp::PostDecrement;
  (*s_prefix_ops)["+"]  = ScriptOp::Positive;
  (*s_prefix_ops)["-"]  = ScriptOp::Negative;
  (*s_prefix_ops)["!"]  = ScriptOp::Not;
  (*s_prefix_ops)["++"]  = ScriptOp::PreIncrement;
  (*s_prefix_ops)["--"]  = ScriptOp::PreDecrement;
  (*s_binary_ops)["["]  = ScriptOp::Subscript;
  (*s_binary_ops)["("]  = ScriptOp::List;
  (*s_binary_ops)["{"]  = ScriptOp::Map;
  (*s_binary_ops)["."]  = ScriptOp::Lookup;

  // Initialise operator type -> precedence mapping

  s_op_precs = new PrecedenceMap();
  
  int p=0;
  (*s_op_precs)[ScriptOp::Sequential] = ++p; // Sequential evaluation
  (*s_op_precs)[ScriptOp::Resolve] = p;

  (*s_op_precs)[ScriptOp::Assign] = ++p; // Assignment
  (*s_op_precs)[ScriptOp::AddAssign] = p;
  (*s_op_precs)[ScriptOp::SubtractAssign] = p;
  (*s_op_precs)[ScriptOp::MultiplyAssign] = p;
  (*s_op_precs)[ScriptOp::DivideAssign] = p;

  (*s_op_precs)[ScriptOp::Or] = ++p; // Logical OR

  (*s_op_precs)[ScriptOp::Xor]= ++p; // Logical XOR

  (*s_op_precs)[ScriptOp::And] = ++p; // Logical AND

  (*s_op_precs)[ScriptOp::Equality] = ++p; // Equality
  (*s_op_precs)[ScriptOp::Inequality] = p;

  (*s_op_precs)[ScriptOp::GreaterThan] = ++p; // Relational
  (*s_op_precs)[ScriptOp::LessThan] = p;
  (*s_op_precs)[ScriptOp::GreaterThanOrEqualTo] = p;
  (*s_op_precs)[ScriptOp::LessThanOrEqualTo] = p;

  (*s_op_precs)[ScriptOp::Add] = ++p; // Additive
  (*s_op_precs)[ScriptOp::Subtract] = p;

  (*s_op_precs)[ScriptOp::Multiply] = ++p; // Multiplicative
  (*s_op_precs)[ScriptOp::Divide] = p;
  (*s_op_precs)[ScriptOp::Modulus] = p;

  (*s_op_precs)[ScriptOp::Power] = ++p; // Power

  (*s_op_precs)[ScriptOp::Factorial] = ++p; // Unary postfix
  (*s_op_precs)[ScriptOp::PostIncrement] = p; 
  (*s_op_precs)[ScriptOp::PostDecrement] = p; 

  (*s_op_precs)[ScriptOp::Positive] = ++p; // Unary prefix
  (*s_op_precs)[ScriptOp::Negative] = p;
  (*s_op_precs)[ScriptOp::Not] = p;
  (*s_op_precs)[ScriptOp::PreIncrement] = p;
  (*s_op_precs)[ScriptOp::PreDecrement] = p;

  (*s_op_precs)[ScriptOp::Subscript] = ++p; // Subscript
  (*s_op_precs)[ScriptOp::List] = ++p; // List initialiser or function call
  (*s_op_precs)[ScriptOp::Map] = ++p; // Map initializer

  (*s_op_precs)[ScriptOp::Lookup] = ++p; // Member access

  // Create standard context
  s_standard_context = 
    new ScriptRefTo<StandardContext>(new StandardContext());

  // This registers and handles creation of standard types
  new StandardTypeProvider();
}


//===========================================================================
StandardContext::StandardContext()
{

}

//===========================================================================
StandardContext::~StandardContext()
{

}

//===========================================================================
std::string StandardContext::get_string() const
{
  return "Standard";
}

//===========================================================================
ScriptRef* StandardContext::script_op(const ScriptAuth& auth,
				      const ScriptRef& ref,
				      const ScriptOp& op,
				      const ScriptRef* right)
{
  if (ScriptOp::Lookup == op.type() || 
      ScriptOp::Resolve == op.type()) {
    std::string name = right->object()->get_string();

    // Standard functions
    if ("defined" == name ||
	"ref" == name ||
	"constref" == name) {
      return new ScriptMethodRef(ref,name);
    }

    // Registered type constructors
    ProviderMap::iterator it = m_providers.find(name);
    if (it != m_providers.end()) {
      return new ScriptMethodRef(ref,name);
    }
  }
  return ScriptObject::script_op(auth,ref,op,right);
}

//===========================================================================
ScriptRef* StandardContext::script_method(const ScriptAuth& auth,
					  const ScriptRef& ref,
					  const std::string& name,
					  const ScriptRef* args)
{
  const ScriptList* argl = dynamic_cast<const ScriptList*>(args->object());
  const ScriptRef* ar = argl->get(0);

  if ("defined" == name) {
    // Is the argument defined (i.e. its not NULL or an error)
    return ScriptInt::new_ref(!BAD_SCRIPTREF(ar));
  }

  if ("ref" == name) {
    // Return a reference to the argument
    if (ar) return ar->ref_copy(ScriptRef::Ref);
    return 0;
  }

  if ("constref" == name) {
    // Return a const reference to the argument
    if (ar) return ar->ref_copy(ScriptRef::ConstRef);
    return 0;
  }

  // Registered type constructor, call provide to create the object
  return new ScriptRef(provide(name,args));
}

//===========================================================================
StandardTypeProvider::StandardTypeProvider() 
{
  ScriptExpr::register_type("String",this);
  ScriptExpr::register_type("Int",this);
  ScriptExpr::register_type("Real",this);
  ScriptExpr::register_type("Error",this);
  ScriptExpr::register_type("VersionTag",this);
  ScriptExpr::register_type("Date",this);
  ScriptExpr::register_type("Time",this);
  ScriptExpr::register_type("TimeZone",this);
  ScriptExpr::register_type("Uri",this);
  ScriptExpr::register_type("MimeType",this);
  ScriptExpr::register_type("RegExp",this);
};

//===========================================================================
void StandardTypeProvider::provide(const std::string& type,
				   const ScriptRef* args,
				   ScriptObject*& object)
{
  const ScriptObject* a = get_method_arg<ScriptObject>(args,0,"value");
  
  if ("String" == type) {
    object = new ScriptString(a ? a->get_string() : "");

  } else if ("Int" == type) {
    object = new ScriptInt(a ? a->get_int() : 0);

  } else if ("Real" == type) {
    object = new ScriptReal(a ? (double)a->get_int() : 0.0);

  } else if ("Error" == type) {
    object = new ScriptError(a ? a->get_string() : "unknown");

  } else if ("VersionTag" == type) {
    object = new VersionTag(args);

  } else if ("Date" == type) {
    object = new Date(args);

  } else if ("Time" == type) {
    object = new Time(args);

  } else if ("TimeZone" == type) {
    object = new TimeZone(args);

  } else if ("Uri" == type) {
    object = new Uri(args);

  } else if ("MimeType" == type) {
    object = new MimeType(args);

  } else if ("RegExp" == type) {
    object = new RegExp(args);
  }
};

};
