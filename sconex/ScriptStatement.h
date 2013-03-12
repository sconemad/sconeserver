/* SconeServer (http://www.sconemad.com)

Script script statements

Objects of these classes are created dynamically by the ScriptScript parser and
used to evaluate and run SconeScript programs.

* ScriptStatement - Generic statement.

* ScriptStatementExpr - An expression, evaluated by ScriptProc.

* ScriptStatementGroup - A group of expressions which are run in sequence.

* ScriptStatementConditional - A statement which is only run if a specified
  condition evaluates to true.

* ScriptStatementWhile - A statement which is run repeatedly while a specified
  conditon evaluates to true.

* ScriptStatementFor - A 'for' loop with initialiser, condition and test
  expressions. 

* ScriptStatementFlow - Program flow control statements, i.e. return, break 
  and continue.

* ScriptStatementDecl - Declaration statement, for declaring variables,
  references and constants.

* ScriptStatementSub - Subroutine definition

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

#ifndef scxScriptStatement_h
#define scxScriptStatement_h

#include <sconex/sconex.h>
#include <sconex/ScriptBase.h>
namespace scx {

class ScriptExpr;
class ScriptEngine;
class ScriptStatement;
class ScriptStatementSub;
class ScriptMap;

//=============================================================================
class SCONEX_API ScriptTracer {
public:

  ScriptTracer(const ScriptAuth& auth, 
	       const std::string& file="",
	       int line_offset=0);
  ScriptTracer(const ScriptTracer& c);
  virtual ~ScriptTracer();

  virtual ScriptTracer* new_copy();

  ScriptRef* evaluate(const std::string& expr,
		      ScriptStatement* ctx = 0);

  ScriptExpr& get_expr();
  const std::string& get_file() const;
  int get_line_offset() const;

  struct ErrorEntry {
    std::string file;
    int line;
    std::string error;
  };
  
  typedef std::vector<ErrorEntry> ErrorList;
  ErrorList& errors();

protected:

  ScriptExpr* m_expr;
  std::string m_file;
  int m_line_offset;

  ErrorList m_errors;

};
  
//=============================================================================
class SCONEX_API ScriptStatement : public ScriptObject {
public:

  ScriptStatement(int line);
  ScriptStatement(const ScriptStatement& c);
  virtual ~ScriptStatement();

  enum ParseResult { Continue, End, Pop, Error };
  virtual ParseResult parse(ScriptEngine& script, const std::string& token) =0;

  enum ParseMode { SemicolonTerminated, Bracketed, Name };
  virtual ParseMode parse_mode() const;

  ScriptRef* execute(ScriptTracer& tracer);

  enum FlowMode { Normal, Return, Last, Next };
  virtual ScriptRef* run(ScriptTracer& tracer, FlowMode& flow) =0;

  // ScriptObject methods
  virtual std::string get_string() const;

  virtual ScriptRef* script_method(const ScriptAuth& auth,
				   const ScriptRef& ref,
				   const std::string& name,
				   const ScriptRef* args);

  void set_parent(ScriptObject* parent);

  typedef ScriptRefTo<ScriptStatement> Ref;

  int get_line() const;

private:

  int m_line;

};

  
//=============================================================================
class SCONEX_API ScriptStatementExpr : public ScriptStatement {
public:
  
  ScriptStatementExpr(int line, const std::string& expr);
  ScriptStatementExpr(const ScriptStatementExpr& c);
  virtual ~ScriptStatementExpr();
  virtual ScriptObject* new_copy() const;

  virtual ParseResult parse(ScriptEngine& script, const std::string& token);
  virtual ScriptRef* run(ScriptTracer& tracer, FlowMode& flow);

protected:

  std::string m_expr;
  // Expression to evaluate as this statement
  
};

  
//=============================================================================
class SCONEX_API ScriptStatementGroup : public ScriptStatement {
public:
  
  ScriptStatementGroup(int line, ScriptMap* env=0);
  ScriptStatementGroup(const ScriptStatementGroup& c);
  virtual ~ScriptStatementGroup();
  virtual ScriptObject* new_copy() const;

  virtual ParseResult parse(ScriptEngine& script, const std::string& token);
  virtual ScriptRef* run(ScriptTracer& tracer, FlowMode& flow);

  virtual ScriptRef* script_op(const ScriptAuth& auth,
			       const ScriptRef& ref,
			       const ScriptOp& op,
			       const ScriptRef* right=0);

  virtual ScriptRef* script_method(const ScriptAuth& auth,
				   const ScriptRef& ref,
				   const std::string& name,
				   const ScriptRef* args);

  void set_env(ScriptMap* env);
  void clear();

protected:

  // List of statements in this group
  typedef std::list<ScriptStatement::Ref*> StatementList;
  StatementList m_statements;

  // Local environment defined within this group's scope
  ScriptMap* m_env;

  // Do we own the environment, or was one supplied
  bool m_own_env;
  
};

typedef ScriptRefTo<ScriptStatementGroup> StatementGroupRef;
  
//=============================================================================
class SCONEX_API ScriptStatementConditional : public ScriptStatement {
public:
  
  ScriptStatementConditional(int line, const std::string& condition = "");
  ScriptStatementConditional(const ScriptStatementConditional& c);
  virtual ~ScriptStatementConditional();
  virtual ScriptObject* new_copy() const;

  virtual ParseResult parse(ScriptEngine& script, const std::string& token);
  virtual ParseMode parse_mode() const;
  virtual ScriptRef* run(ScriptTracer& tracer, FlowMode& flow);

protected:

  // Sequence counter used for parsing
  int m_seq;
  
  // Condition expression
  std::string m_condition;
  
  // Statement to run if condition is true
  ScriptStatement::Ref* m_true_statement;
  
  // Statement to run if condition is false (the 'else' clause)
  ScriptStatement::Ref* m_false_statement;
  
};

  
//=============================================================================
class SCONEX_API ScriptStatementWhile : public ScriptStatement {
public:
  
  ScriptStatementWhile(int line, const std::string& condition = "");
  ScriptStatementWhile(const ScriptStatementWhile& c);
  virtual ~ScriptStatementWhile();
  virtual ScriptObject* new_copy() const;

  virtual ParseResult parse(ScriptEngine& script, const std::string& token);
  virtual ParseMode parse_mode() const;
  virtual ScriptRef* run(ScriptTracer& tracer, FlowMode& flow);

protected:

  // Sequence counter used for parsing
  int m_seq;
  
  // Condition expression to determine whether to keep looping
  std::string m_condition;
  
  // Statement to loop over
  ScriptStatement::Ref* m_body;
  
};


//=============================================================================
class SCONEX_API ScriptStatementFor : public ScriptStatement {
public:
  
  ScriptStatementFor(int line);
  ScriptStatementFor(const ScriptStatementFor& c);
  virtual ~ScriptStatementFor();
  virtual ScriptObject* new_copy() const;

  virtual ParseResult parse(ScriptEngine& script, const std::string& token);
  virtual ParseMode parse_mode() const;
  virtual ScriptRef* run(ScriptTracer& tracer, FlowMode& flow);

protected:

  // Sequence counter used for parsing
  int m_seq;

  // Initialiser expression
  std::string m_initialiser;
  
  // Condition expression to determine whether to keep looping
  std::string m_condition;

  // Increment expression
  std::string m_increment;
  
  // Statement to loop over
  ScriptStatement::Ref* m_body;
  
};

  
//=============================================================================
class SCONEX_API ScriptStatementFlow : public ScriptStatement {
public:
  
  ScriptStatementFlow(int line, FlowMode flow);
  ScriptStatementFlow(const ScriptStatementFlow& c);
  virtual ~ScriptStatementFlow();
  virtual ScriptObject* new_copy() const;

  virtual ParseResult parse(ScriptEngine& script, const std::string& token);
  virtual ParseMode parse_mode() const;
  virtual ScriptRef* run(ScriptTracer& tracer, FlowMode& flow);

protected:

  // Sequence counter used for parsing
  int m_seq;

  // Flow mode to change to
  FlowMode m_flow;
  
  // Expression for return value
  std::string m_ret_expr;

};


//=============================================================================
class SCONEX_API ScriptStatementDecl : public ScriptStatement {
public:

  enum DefType { Var, Const, Ref, ConstRef };
  
  ScriptStatementDecl(int line, DefType deftype, const std::string& name = "");
  ScriptStatementDecl(const ScriptStatementDecl& c);
  virtual ~ScriptStatementDecl();
  virtual ScriptObject* new_copy() const;

  virtual ParseResult parse(ScriptEngine& script, const std::string& token);
  virtual ParseMode parse_mode() const;
  virtual ScriptRef* run(ScriptTracer& tracer, FlowMode& flow);

protected:

  // Sequence counter used for parsing
  int m_seq;

  // Definition type
  DefType m_deftype;

  // Variable name
  std::string m_name;
  
  // Initialiser expression
  std::string m_initialiser;

};

  
//=============================================================================
class SCONEX_API ScriptStatementSub : public ScriptStatement {
public:
  
  ScriptStatementSub(int line, const std::string& name = "");
  ScriptStatementSub(const ScriptStatementSub& c);
  virtual ~ScriptStatementSub();
  virtual ScriptObject* new_copy() const;

  virtual ParseResult parse(ScriptEngine& script, const std::string& token);
  virtual ParseMode parse_mode() const;
  virtual ScriptRef* run(ScriptTracer& tracer, FlowMode& flow);

protected:

  // Sequence counter used for parsing
  int m_seq;

  // Variable name
  std::string m_name;

  // Subroutine argument names
  std::vector<std::string> m_arg_names;
  
  // Subroutine body
  ScriptStatement::Ref* m_body;

};
  
  
};
#endif
