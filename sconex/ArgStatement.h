/* SconeServer (http://www.sconemad.com)

Arg script statements

Objects of these classes are created dynamically by the ArgScript parser and
used to evaluate and run SconeScript programs.

* ArgStatement - Generic statement.

* ArgStatementExpr - An expression, evaluated by ArgProc.

* ArgStatementGroup - A group of expressions which are run in sequence.

* ArgStatementConditional - A statement which is only run if a specified
  condition evaluates to true.

* ArgStatementWhile - A statement which is run repeatedly while a specified
  conditon evaluates to true.

* ArgStatementFor - A 'for' loop with initialiser, condition and test
  expressions. 

* ArgStatementFlow - Program flow control statements, i.e. return, break 
  and continue.

* ArgStatementDecl - Declaration statement, for declaring variables,
  references and constants.

* ArgStatementSub - Subroutine definition

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

#ifndef scxArgStatement_h
#define scxArgStatement_h

#include "sconex/sconex.h"
#include "sconex/ArgObject.h"
#include "sconex/Arg.h"
namespace scx {

class ArgProc;
class ArgScript;
class ArgStatementSub;
  
//=============================================================================
class SCONEX_API ArgStatement : public ArgObjectInterface {

public:

  ArgStatement();
  ArgStatement(const ArgStatement& c);
  virtual ~ArgStatement();
  virtual ArgStatement* new_copy() const =0;

  enum ParseResult { Continue, End, Pop, Error };
  virtual ParseResult parse(ArgScript& script, const std::string& token) =0;

  enum ParseMode { SemicolonTerminated, Bracketed, Name };
  virtual ParseMode parse_mode() const;

  Arg* execute(ArgProc& proc);

  enum FlowMode { Normal, Return, Last, Next };
  virtual Arg* run(ArgProc& proc, FlowMode& flow) =0;

  // ArgObjectInterface methods
  virtual std::string name() const;
  virtual Arg* arg_lookup(const std::string& name);
  virtual Arg* arg_resolve(const std::string& name);
  virtual Arg* arg_method(const Auth& auth, const std::string& name, Arg* args);

  void set_parent(ArgObjectInterface* parent);

protected:
  
  ArgObjectInterface* m_parent;
  
};

  
//=============================================================================
class SCONEX_API ArgStatementExpr : public ArgStatement {

public:
  
  ArgStatementExpr(const std::string& expr);
  ArgStatementExpr(const ArgStatementExpr& c);
  virtual ~ArgStatementExpr();
  virtual ArgStatement* new_copy() const;

  virtual ParseResult parse(ArgScript& script, const std::string& token);
  virtual Arg* run(ArgProc& proc, FlowMode& flow);

protected:

  std::string m_expr;
  // Expression to evaluate as this statement
  
};

  
//=============================================================================
class SCONEX_API ArgStatementGroup : public ArgStatement {

public:
  
  ArgStatementGroup(ArgMap* env=0);
  ArgStatementGroup(const ArgStatementGroup& c);
  virtual ~ArgStatementGroup();
  virtual ArgStatement* new_copy() const;

  virtual ParseResult parse(ArgScript& script, const std::string& token);
  virtual Arg* run(ArgProc& proc, FlowMode& flow);

  virtual Arg* arg_lookup(const std::string& name);
  virtual Arg* arg_method(const Auth& auth, const std::string& name, Arg* args);

  void clear();
  
protected:

  std::list<ArgStatement*> m_statements;
  // List of statements in this group

  ArgMap* m_env;
  // Local environment defined within this group's scope

  bool m_own_env;
  // Do we own the environment, or was one supplied
  
};

  
//=============================================================================
class SCONEX_API ArgStatementConditional : public ArgStatement {

public:
  
  ArgStatementConditional(const std::string& condition = "");
  ArgStatementConditional(const ArgStatementConditional& c);
  virtual ~ArgStatementConditional();
  virtual ArgStatement* new_copy() const;

  virtual ParseResult parse(ArgScript& script, const std::string& token);
  virtual ParseMode parse_mode() const;
  virtual Arg* run(ArgProc& proc, FlowMode& flow);

protected:

  int m_seq;
  // Sequence counter used for parsing
  
  std::string m_condition;
  // Condition expression
  
  ArgStatement* m_true_statement;
  // Statement to run if condition is true
  
  ArgStatement* m_false_statement;
  // Statement to run if condition is false (the 'else' clause)
  
};

  
//=============================================================================
class SCONEX_API ArgStatementWhile : public ArgStatement {

public:
  
  ArgStatementWhile(const std::string& condition = "");
  ArgStatementWhile(const ArgStatementWhile& c);
  virtual ~ArgStatementWhile();
  virtual ArgStatement* new_copy() const;

  virtual ParseResult parse(ArgScript& script, const std::string& token);
  virtual ParseMode parse_mode() const;
  virtual Arg* run(ArgProc& proc, FlowMode& flow);

protected:

  int m_seq;
  // Sequence counter used for parsing
  
  std::string m_condition;
  // Condition expression to determine whether to keep looping
  
  ArgStatement* m_body;
  // Statement to loop over
  
};


//=============================================================================
class SCONEX_API ArgStatementFor : public ArgStatement {

public:
  
  ArgStatementFor();
  ArgStatementFor(const ArgStatementFor& c);
  virtual ~ArgStatementFor();
  virtual ArgStatement* new_copy() const;

  virtual ParseResult parse(ArgScript& script, const std::string& token);
  virtual ParseMode parse_mode() const;
  virtual Arg* run(ArgProc& proc, FlowMode& flow);

protected:

  int m_seq;
  // Sequence counter used for parsing

  std::string m_initialiser;
  // Initialiser expression
  
  std::string m_condition;
  // Condition expression to determine whether to keep looping

  std::string m_increment;
  // Increment expression
  
  ArgStatement* m_body;
  // Statement to loop over
  
};

  
//=============================================================================
class SCONEX_API ArgStatementFlow : public ArgStatement {

public:
  
  ArgStatementFlow(FlowMode flow);
  ArgStatementFlow(const ArgStatementFlow& c);
  virtual ~ArgStatementFlow();
  virtual ArgStatement* new_copy() const;

  virtual ParseResult parse(ArgScript& script, const std::string& token);
  virtual ParseMode parse_mode() const;
  virtual Arg* run(ArgProc& proc, FlowMode& flow);

protected:

  int m_seq;
  // Sequence counter used for parsing

  FlowMode m_flow;
  // Flow mode to change to
  
  std::string m_ret_expr;
  // Expression for return value
};


//=============================================================================
class SCONEX_API ArgStatementDecl : public ArgStatement {

public:

  enum DefType { Var, Ref, Const };
  
  ArgStatementDecl(DefType deftype, const std::string& name = "");
  ArgStatementDecl(const ArgStatementDecl& c);
  virtual ~ArgStatementDecl();
  virtual ArgStatement* new_copy() const;

  virtual ParseResult parse(ArgScript& script, const std::string& token);
  virtual ParseMode parse_mode() const;
  virtual Arg* run(ArgProc& proc, FlowMode& flow);

protected:

  int m_seq;
  // Sequence counter used for parsing

  DefType m_deftype;
  // Definition type

  std::string m_name;
  // Variable name
  
  std::string m_initialiser;
  // Initialiser expression
};

  
//=============================================================================
class SCONEX_API ArgStatementSub : public ArgStatement {

public:
  
  ArgStatementSub(const std::string& name = "");
  ArgStatementSub(const ArgStatementSub& c);
  virtual ~ArgStatementSub();
  virtual ArgStatement* new_copy() const;

  virtual ParseResult parse(ArgScript& script, const std::string& token);
  virtual ParseMode parse_mode() const;
  virtual Arg* run(ArgProc& proc, FlowMode& flow);

protected:

  int m_seq;
  // Sequence counter used for parsing

  std::string m_name;
  // Variable name
  
  ArgStatement* m_body;
  // Subroutine body
};
  
  
};
#endif
