/* SconeServer (http://www.sconemad.com)

Arg script statements

Copyright (c) 2000-2005 Andrew Wedgbury <wedge@sconemad.com>

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

#include "sconex/ArgStatement.h"
#include "sconex/ArgScript.h"
#include "sconex/ArgProc.h"
namespace scx {

//=============================================================================
ArgStatement::ArgStatement()
  : m_parent(0)
{
  DEBUG_COUNT_CONSTRUCTOR(ArgStatement);
}

//=============================================================================
ArgStatement::ArgStatement(const ArgStatement& c)
  : m_parent(c.m_parent)
{
  DEBUG_COUNT_CONSTRUCTOR(ArgStatement)
}
  
//=============================================================================
ArgStatement::~ArgStatement()
{
  DEBUG_COUNT_DESTRUCTOR(ArgStatement);
}

//=============================================================================
ArgStatement::ParseMode ArgStatement::parse_mode() const
{
  return ArgStatement::SemicolonTerminated;
}

//=============================================================================
Arg* ArgStatement::execute(ArgProc& proc)
{
  ArgStatement::FlowMode flow = ArgStatement::Normal;
  return run(proc,flow);
}

//=============================================================================
std::string ArgStatement::name() const
{
  return "STATEMENT";
}

//=============================================================================
Arg* ArgStatement::arg_lookup(const std::string& name)
{
  return ArgObjectInterface::arg_lookup(name);
}

//=============================================================================
Arg* ArgStatement::arg_resolve(const std::string& name)
{
  Arg* a = ArgObjectInterface::arg_resolve(name);
  if ( m_parent && (a==0 || (dynamic_cast<ArgError*>(a))!=0) ) {
    delete a;
    return m_parent->arg_resolve(name);
  }
  return a;
}

//=============================================================================
Arg* ArgStatement::arg_function(const std::string& name, Arg* args)
{
  // Cascade to parent
  return m_parent->arg_function(name,args);
}

//=============================================================================
void ArgStatement::set_parent(ArgObjectInterface* parent)
{
  m_parent = parent;
}

//=============================================================================
ArgStatementExpr::ArgStatementExpr(const std::string& expr)
  : m_expr(expr)
{
  DEBUG_COUNT_CONSTRUCTOR(ArgStatementExpr);
}

//=============================================================================
ArgStatementExpr::ArgStatementExpr(const ArgStatementExpr& c)
  : ArgStatement(c),
    m_expr(c.m_expr)
{
  DEBUG_COUNT_CONSTRUCTOR(ArgStatementExpr);
}

//=============================================================================
ArgStatementExpr::~ArgStatementExpr()
{
  DEBUG_COUNT_DESTRUCTOR(ArgStatementExpr);
}

//=============================================================================
ArgStatement* ArgStatementExpr::new_copy() const
{
  return new ArgStatementExpr(*this);
}

//=============================================================================
ArgStatement::ParseResult ArgStatementExpr::parse(
  ArgScript& script,
  const std::string& token
)
{
  return ArgStatement::Pop;
}

//=============================================================================
Arg* ArgStatementExpr::run(ArgProc& proc, FlowMode& flow)
{
  ArgObject ctx(this);
  proc.set_ctx(&ctx);
  return proc.evaluate(m_expr);
}


//=============================================================================
ArgStatementGroup::ArgStatementGroup()
{
  DEBUG_COUNT_CONSTRUCTOR(ArgStatementGroup);
}

//=============================================================================
ArgStatementGroup::ArgStatementGroup(const ArgStatementGroup& c)
  : ArgStatement(c),
    m_vars(c.m_vars)
{
  DEBUG_COUNT_CONSTRUCTOR(ArgStatementGroup);

  for (std::list<ArgStatement*>::const_iterator it = c.m_statements.begin();
       it != c.m_statements.end();
       ++it) {
    ArgStatement* ac = (*it)->new_copy();
    ac->set_parent(this);
    m_statements.push_back(ac);
  }
}

//=============================================================================
ArgStatementGroup::~ArgStatementGroup()
{
  clear();

  DEBUG_COUNT_DESTRUCTOR(ArgStatementGroup);
}

//=============================================================================
ArgStatement* ArgStatementGroup::new_copy() const
{
  return new ArgStatementGroup(*this);
}

//=============================================================================
ArgStatement::ParseResult ArgStatementGroup::parse(
  ArgScript& script,
  const std::string& token
)
{
  if (token == "}") {
    return ArgStatement::End;
  }

  ArgStatement* sub = script.parse_token(token);
  if (sub==0) {
    return ArgStatement::Error;
  }

  sub->set_parent(this);
  m_statements.push_back(sub);
  return ArgStatement::Continue;
}

//=============================================================================
Arg* ArgStatementGroup::run(ArgProc& proc, FlowMode& flow)
{
  Arg* ret=0;
  
  for (std::list<ArgStatement*>::iterator it = m_statements.begin();
       it != m_statements.end();
       ++it) {
    delete ret;
    ret = (*it)->run(proc,flow);
    if (flow != Normal) {
      return ret;
    }
  }
  
  return ret;
}

//=============================================================================
Arg* ArgStatementGroup::arg_lookup(const std::string& name)
{
  if ("var" == name) {
    return new ArgObjectFunction(new ArgObject(this),name);
  }

  Arg* var = m_vars.lookup(name);
  if (var) {
    return var->var_copy();
  }

  return ArgObjectInterface::arg_lookup(name);
}

//=============================================================================
Arg* ArgStatementGroup::arg_function(const std::string& name, Arg* args)
{
  ArgList* l = dynamic_cast<ArgList*> (args);
  
  if ("var" == name) {
    const ArgString* a_var = dynamic_cast<const ArgString*> (l->get(0));
    if (a_var) {
      std::string var_name = a_var->get_string();
      Arg* a_val = l->take(1);
      // If no initialiser was given, default to integer 0
      if (a_val == 0) {
        a_val = new ArgInt(0);
      }
      m_vars.give(var_name, a_val);
    }
    return 0;
  }
  
  return ArgObjectInterface::arg_function(name,args);
}

//=============================================================================
void ArgStatementGroup::clear()
{
  std::list<ArgStatement*>::iterator it = m_statements.begin();
  while (it != m_statements.end()) {
    delete (*it);
    ++it;
  }
  m_statements.clear();
}


//=============================================================================
ArgStatementConditional::ArgStatementConditional(const std::string& condition)
  : m_seq(0),
    m_condition(condition),
    m_true_statement(0),
    m_false_statement(0)
{
  DEBUG_COUNT_CONSTRUCTOR(ArgStatementConditional);
}

//=============================================================================
ArgStatementConditional::ArgStatementConditional(const ArgStatementConditional& c)
  : ArgStatement(c),
    m_seq(c.m_seq),
    m_condition(c.m_condition),
    m_true_statement(0),
    m_false_statement(0)
    
{
  DEBUG_COUNT_CONSTRUCTOR(ArgStatementConditional);
  
  if (c.m_true_statement) {
    m_true_statement = c.m_true_statement->new_copy();
    m_true_statement->set_parent(this);
  }

  if (c.m_false_statement) {
    m_false_statement = c.m_false_statement->new_copy();
    m_false_statement->set_parent(this);
  }
}

//=============================================================================
ArgStatementConditional::~ArgStatementConditional()
{
  delete m_true_statement;
  delete m_false_statement;
  DEBUG_COUNT_DESTRUCTOR(ArgStatementConditional);
}

//=============================================================================
ArgStatement* ArgStatementConditional::new_copy() const
{
  return new ArgStatementConditional(*this);
}

//=============================================================================
ArgStatement::ParseResult ArgStatementConditional::parse(
  ArgScript& script,
  const std::string& token
)
{
  switch (++m_seq) {
    case 1: {
      m_condition = token;
    } break;

    case 2: {
      delete m_true_statement;
      if (token == "else") {
        // Empty true statement
        ++m_seq;
        m_true_statement = 0;
        break;
      }
      m_true_statement = script.parse_token(token);
      m_true_statement->set_parent(this);
    } break;

    case 3: {
      if (token != "else") {
        return ArgStatement::Pop;
      }
    } break;

    case 4: {
      delete m_false_statement;
      m_false_statement = script.parse_token(token);
      m_false_statement->set_parent(this);
    } break;

    default: {
      return ArgStatement::Pop;
    }
  }

  return ArgStatement::Continue;
}

//=============================================================================
ArgStatement::ParseMode ArgStatementConditional::parse_mode() const
{
  return (m_seq==0 ?
          ArgStatement::Bracketed :
          ArgStatement::SemicolonTerminated);
}

//=============================================================================
Arg* ArgStatementConditional::run(ArgProc& proc, FlowMode& flow)
{
  // Evaluate the condition
  ArgObject ctx(this);
  proc.set_ctx(&ctx);
  Arg* result = proc.evaluate(m_condition);
  bool cond = (result && 0 != result->get_int());
  delete result;

  if (cond) {
    if (m_true_statement) return m_true_statement->run(proc,flow);
  } else {
    if (m_false_statement) return m_false_statement->run(proc,flow);
  }
  
  return 0;
}


//=============================================================================
ArgStatementWhile::ArgStatementWhile(const std::string& condition)
  : m_seq(0),
    m_condition(condition),
    m_body(0)
{
  DEBUG_COUNT_CONSTRUCTOR(ArgStatementWhile);
}

//=============================================================================
ArgStatementWhile::ArgStatementWhile(const ArgStatementWhile& c)
  : ArgStatement(c),
    m_seq(c.m_seq),
    m_condition(c.m_condition),
    m_body(0)
{
  DEBUG_COUNT_CONSTRUCTOR(ArgStatementWhile);

  if (c.m_body) {
    m_body = c.m_body->new_copy();
    m_body->set_parent(this);
  }
}

//=============================================================================
ArgStatementWhile::~ArgStatementWhile()
{
  delete m_body;
  DEBUG_COUNT_DESTRUCTOR(ArgStatementWhile);
}

//=============================================================================
ArgStatement* ArgStatementWhile::new_copy() const
{
  return new ArgStatementWhile(*this);
}

//=============================================================================
ArgStatement::ParseResult ArgStatementWhile::parse(
  ArgScript& script,
  const std::string& token
)
{
  switch (++m_seq) {
    case 1: {
      m_condition = token;
    } break;

    case 2: {
      delete m_body;
      m_body = script.parse_token(token);
      m_body->set_parent(this);
    } break;

    default: {
      return ArgStatement::Pop;
    }
  }

  return ArgStatement::Continue;
}

//=============================================================================
ArgStatement::ParseMode ArgStatementWhile::parse_mode() const
{
  return (m_seq==0 ?
          ArgStatement::Bracketed :
          ArgStatement::SemicolonTerminated);
}

//=============================================================================
Arg* ArgStatementWhile::run(ArgProc& proc, FlowMode& flow)
{
  Arg* ret=0;
  ArgObject ctx(this);
  
  while (true) {
    // Evaluate the condition
    proc.set_ctx(&ctx);
    Arg* result = proc.evaluate(m_condition);
    bool cond = (0 != result->get_int());
    delete result;

    if (!cond) {
      // Condition is false, break out of the loop
      break;
    }

    if (m_body) {
      delete ret;
      ret = m_body->run(proc,flow);
      if (flow == Return) {
	return ret;
      } else if (flow == Last) {
	flow = Normal;
	return ret;
      } else if (flow == Next) {
	flow = Normal;
      }
    }
  }

  return ret;
}


//=============================================================================
ArgStatementFor::ArgStatementFor()
  : m_seq(0),
    m_body(0)
{
  DEBUG_COUNT_CONSTRUCTOR(ArgStatementFor);
}

//=============================================================================
ArgStatementFor::ArgStatementFor(const ArgStatementFor& c)
  : ArgStatement(c),
    m_seq(c.m_seq),
    m_initialiser(c.m_initialiser),
    m_condition(c.m_condition),
    m_increment(c.m_increment),
    m_body(0)
{
  DEBUG_COUNT_CONSTRUCTOR(ArgStatementFor);

  if (c.m_body) {
    m_body = c.m_body->new_copy();
    m_body->set_parent(this);
  }
}

//=============================================================================
ArgStatementFor::~ArgStatementFor()
{
  delete m_body;
  DEBUG_COUNT_DESTRUCTOR(ArgStatementFor);
}

//=============================================================================
ArgStatement* ArgStatementFor::new_copy() const
{
  return new ArgStatementFor(*this);
}

//=============================================================================
ArgStatement::ParseResult ArgStatementFor::parse(
  ArgScript& script,
  const std::string& token
)
{
  switch (++m_seq) {
    case 1: {
      int isc1 = token.find(";",0);
      m_initialiser = token.substr(0,isc1);
      int isc2 = token.find(";",isc1+1);
      m_condition = token.substr(isc1+1,isc2-isc1-1);
      m_increment = token.substr(isc2+1);
    } break;

    case 2: {
      delete m_body;
      m_body = script.parse_token(token);
      m_body->set_parent(this);
    } break;

    default: {
      return ArgStatement::Pop;
    }
  }

  return ArgStatement::Continue;
}

//=============================================================================
ArgStatement::ParseMode ArgStatementFor::parse_mode() const
{
  return (m_seq==0 ?
          ArgStatement::Bracketed :
          ArgStatement::SemicolonTerminated);
}

//=============================================================================
Arg* ArgStatementFor::run(ArgProc& proc, FlowMode& flow)
{
  Arg* ret=0;
  ArgObject ctx(this);
  proc.set_ctx(&ctx);
  
  // Evaluate the initialiser
  Arg* result = proc.evaluate(m_initialiser);
  delete result;

  while (true) {
    // Evaluate the condition
    proc.set_ctx(&ctx);
    result = proc.evaluate(m_condition);
    bool cond = (0 != result->get_int());
    delete result;

    if (!cond) {
      // Condition is false, break out of the loop
      break;
    }

    if (m_body) {
      delete ret;
      ret = m_body->run(proc,flow);
      if (flow == Return) {
	return ret;
      } else if (flow == Last) {
	flow = Normal;
	return ret;
      } else if (flow == Next) {
	flow = Normal;
      }
    }

    // Evaluate the increment
    proc.set_ctx(&ctx);
    result = proc.evaluate(m_increment);
    delete result;
  }

  return ret;
}


//=============================================================================
ArgStatementFlow::ArgStatementFlow(FlowMode flow)
  : m_seq(0),
    m_flow(flow)
{
  DEBUG_COUNT_CONSTRUCTOR(ArgStatementFlow);
}

//=============================================================================
ArgStatementFlow::ArgStatementFlow(const ArgStatementFlow& c)
  : ArgStatement(c),
    m_seq(c.m_seq),
    m_flow(c.m_flow),
    m_ret_expr(c.m_ret_expr)
{
  DEBUG_COUNT_CONSTRUCTOR(ArgStatementFlow);
}

//=============================================================================
ArgStatementFlow::~ArgStatementFlow()
{
  DEBUG_COUNT_DESTRUCTOR(ArgStatementFlow);
}

//=============================================================================
ArgStatement* ArgStatementFlow::new_copy() const
{
  return new ArgStatementFlow(*this);
}

//=============================================================================
ArgStatement::ParseResult ArgStatementFlow::parse(
  ArgScript& script,
  const std::string& token
)
{
  switch (++m_seq) {
    case 1: {
      if (!token.empty()) {
        m_ret_expr = token;
      }
    } break;

    default: {
      return ArgStatement::Pop;
    }
  }

  return ArgStatement::Continue;
}

//=============================================================================
ArgStatement::ParseMode ArgStatementFlow::parse_mode() const
{
  return ArgStatement::SemicolonTerminated;
}

//=============================================================================
Arg* ArgStatementFlow::run(ArgProc& proc, FlowMode& flow)
{
  Arg* ret=0;
  if (!m_ret_expr.empty()) {
    ArgObject ctx(this);
    proc.set_ctx(&ctx);
    ret = proc.evaluate(m_ret_expr);
  }

  flow = m_flow;
  return ret;
}


//=============================================================================
ArgStatementVar::ArgStatementVar(const std::string& name)
  : m_seq(0),
    m_name(name)
{
  DEBUG_COUNT_CONSTRUCTOR(ArgStatementVar);
}

//=============================================================================
ArgStatementVar::ArgStatementVar(const ArgStatementVar& c)
  : ArgStatement(c),
    m_seq(c.m_seq),
    m_name(c.m_name),
    m_initialiser(c.m_initialiser)
{
  DEBUG_COUNT_CONSTRUCTOR(ArgStatementVar);
}

//=============================================================================
ArgStatementVar::~ArgStatementVar()
{
  DEBUG_COUNT_DESTRUCTOR(ArgStatementVar);
}

//=============================================================================
ArgStatement* ArgStatementVar::new_copy() const
{
  return new ArgStatementVar(*this);
}

//=============================================================================
ArgStatement::ParseResult ArgStatementVar::parse(
  ArgScript& script,
  const std::string& token
)
{
  switch (++m_seq) {
    case 1: {
      m_name = token;
    } break;

    case 2: {
      if (!token.empty()) {
        m_initialiser = token.substr(1);
      }
    } break;

    default: {
      return ArgStatement::Pop;
    }
  }

  return ArgStatement::Continue;
}

//=============================================================================
ArgStatement::ParseMode ArgStatementVar::parse_mode() const
{
  return (m_seq==0 ?
          ArgStatement::Name :
          ArgStatement::SemicolonTerminated);
}

//=============================================================================
Arg* ArgStatementVar::run(ArgProc& proc, FlowMode& flow)
{
  Arg* initialiser=0;
  if (!m_initialiser.empty()) {
    ArgObject ctx(this);
    proc.set_ctx(&ctx);
    initialiser = proc.evaluate(m_initialiser);
  }

  ArgList args;
  args.give(new ArgString(m_name));
  if (initialiser) args.give(initialiser);
  
  return m_parent->arg_function("var",&args);
}


//=============================================================================
ArgStatementSub::ArgStatementSub(const std::string& name)
  : m_seq(0),
    m_name(name),
    m_body(0)
{ 
  DEBUG_COUNT_CONSTRUCTOR(ArgStatementSub);
}

//=============================================================================
ArgStatementSub::ArgStatementSub(const ArgStatementSub& c)
  : ArgStatement(c),
    m_seq(c.m_seq),
    m_name(c.m_name),
    m_body(0)
{
  DEBUG_COUNT_CONSTRUCTOR(ArgStatementSub);

  if (c.m_body) {
    m_body = c.m_body->new_copy();
    m_body->set_parent(this);
  }
}

//=============================================================================
ArgStatementSub::~ArgStatementSub()
{
  delete m_body;
  DEBUG_COUNT_DESTRUCTOR(ArgStatementSub);
}

//=============================================================================
ArgStatement* ArgStatementSub::new_copy() const
{
  return new ArgStatementSub(*this);
}

//=============================================================================
ArgStatement::ParseResult ArgStatementSub::parse(
  ArgScript& script,
  const std::string& token
)
{
  switch (++m_seq) {
    case 1: {
      m_name = token;
    } break;

    case 2: {
      delete m_body;
      m_body = script.parse_token(token);
      m_body->set_parent(m_parent);
    } break;

    default: {
      return ArgStatement::Pop;
    }
  }

  return ArgStatement::Continue;
}

//=============================================================================
ArgStatement::ParseMode ArgStatementSub::parse_mode() const
{
  return (m_seq==0 ?
          ArgStatement::Name :
          ArgStatement::SemicolonTerminated);
}

//=============================================================================
Arg* ArgStatementSub::run(ArgProc& proc, FlowMode& flow)
{
  ArgList args;
  args.give(new ArgString(m_name));
  args.give(new ArgSub(m_name,m_body,proc));
  m_body = 0;

  return m_parent->arg_function("var",&args);
}


};
