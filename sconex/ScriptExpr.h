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

class StandardContext;
class StandardTypeProvider;

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

  // Register/unregister object types
  // Modules can use these methods to register object types that they 
  // provide, to make them available in the standard context.
  static void register_type(const std::string& type,
			    Provider<ScriptObject>* factory);
  static void unregister_type(const std::string& type,
			      Provider<ScriptObject>* factory);

  // Method to programatically create an object of one of the registered
  // standard types.
  static ScriptObject* create_object(const std::string& type,
				     const ScriptRef* args);

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

  // Standard execution context  
  static ScriptRefTo<StandardContext>* s_standard_context;

};

//===========================================================================
// StandardContext - This is the standard context used by default when 
// evaluating expressions.
//
class StandardContext : public ScriptObject, 
                        public ProviderScheme<ScriptObject> {
public:

  StandardContext();
  ~StandardContext();

  virtual std::string get_string() const;

  virtual ScriptRef* script_op(const ScriptAuth& auth,
			       const ScriptRef& ref,
			       const ScriptOp& op,
			       const ScriptRef* right=0);

  virtual ScriptRef* script_method(const ScriptAuth& auth,
				   const ScriptRef& ref,
				   const std::string& name,
				   const ScriptRef* args);
private:

  StandardContext(const StandardContext& c);
  StandardContext& operator=(const StandardContext& v);
  // Prohibit copy

  StandardTypeProvider* m_type_provider;

};

//===========================================================================
// StandardTypeProvider - This registers the standard SconeScript types and
// provides constructors for them.
//
class StandardTypeProvider : public Provider<ScriptObject> {
public:

  StandardTypeProvider();
  virtual void provide(const std::string& type,
		       const ScriptRef* args,
		       ScriptObject*& object);
};

};
#endif
