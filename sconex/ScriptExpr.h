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

#ifndef ScriptExpr_h
#define ScriptExpr_h

#include <sconex/sconex.h>
#include <sconex/ScriptBase.h>
#include <sconex/ScriptTypes.h>
#include <sconex/Provider.h>
namespace scx {

//=============================================================================
// ScriptExpr - this is the SconeScript expression evaluator. 
// It parses and evaluates single line SconeScript expressions given as a 
// string, dynamically creating the required Script objects and returning 
// the calculated result.
//
class SCONEX_API ScriptExpr {
public:

  ScriptExpr(const ScriptAuth& auth, ScriptRef* ctx=0);
  ScriptExpr(const ScriptExpr& c);
  ~ScriptExpr();

  // Evaluate a SconeScript expression string
  ScriptRef* evaluate(const std::string& expr);

  // Set evaluation context (can also be specified in constructor)
  // include_standard specifies whether the standard context should be used
  // in addition, which contains constructors for standard object types and
  // base language features.
  void set_ctx(ScriptRef* ctx, bool include_standard=true);

  // Set/get execution authority (can also be specified in constructor)
  void set_auth(const ScriptAuth& auth);
  const ScriptAuth& get_auth();

  // Set type to use for integer literals (default "Int")
  void set_int_type(const std::string type);
  const std::string& get_int_type() const;

  // Set type to use for real literals (default "Real")
  void set_real_type(const std::string type);
  const std::string& get_real_type() const;

protected:

  // Evaluate expression or sub-expression
  ScriptRef* expression(int p, bool f, bool exec=true);

  // Evaluate primary
  ScriptRef* primary(bool f, bool exec=true);

  // Lookup operator type from name
  enum OpMode { Prefix, Postfix, Binary };
  ScriptOp::OpType op_type(OpMode opmode, const std::string& opname) const;

  // Lookup precedence for operator
  int op_prec(ScriptOp::OpType optype) const;

  // Resolve a name in execution context(s)
  ScriptRef* resolve_name(const std::string& name);

  // Parse the next token from the expression
  void next();


  // Current position with expression
  int m_pos;

  // Expression being evaluated
  std::string m_expr;

  // Current token data
  enum TokenType { Null, Operator, Name, Value };
  TokenType m_type;
  std::string m_name;
  ScriptRef* m_value;

  // Current authority
  ScriptAuth m_auth;

  // Type for integer literals
  std::string m_int_type;

  // Type for real literals
  std::string m_real_type;

  // Execution contexts
  typedef std::list<ScriptRef*> ContextList;
  ContextList m_contexts;

  // Stack
  typedef std::stack<ScriptRef*> ScriptStack;
  ScriptStack m_stack;

  // Static initialiser
  static void init();

  // Operator name -> type mapping
  typedef std::map<std::string,ScriptOp::OpType> OperatorMap;
  static OperatorMap* s_binary_ops;
  static OperatorMap* s_prefix_ops;
  static OperatorMap* s_postfix_ops;

  // Operator type -> precedence mapping
  typedef std::map<ScriptOp::OpType,int> PrecedenceMap;
  static PrecedenceMap* s_op_precs;
};

};
#endif
