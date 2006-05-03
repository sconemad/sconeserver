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
ArgStatementExpr::~ArgStatementExpr()
{
  DEBUG_COUNT_DESTRUCTOR(ArgStatementExpr);
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
bool ArgStatementExpr::run(ArgProc& proc)
{
  ArgObject ctx(this);
  proc.set_ctx(&ctx);
  Arg* result = proc.evaluate(m_expr);
  delete result;
  return true;
}


//=============================================================================
ArgStatementGroup::ArgStatementGroup()
{
  DEBUG_COUNT_CONSTRUCTOR(ArgStatementGroup);
}

//=============================================================================
ArgStatementGroup::~ArgStatementGroup()
{
  clear();
  DEBUG_COUNT_DESTRUCTOR(ArgStatementGroup);
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
bool ArgStatementGroup::run(ArgProc& proc)
{
  std::list<ArgStatement*>::iterator it = m_statements.begin();
  while (it != m_statements.end()) {
    if (!(*it)->run(proc)) {
      return false;
    }
    ++it;
  }
  return true;
}

//=============================================================================
Arg* ArgStatementGroup::arg_lookup(const std::string& name)
{
  if ("set" == name ||
      "var" == name) {
    return new ArgObjectFunction(new ArgObject(this),name);
  }

  std::map<std::string,Arg*>::const_iterator it = m_vars.find(name);
  if (it != m_vars.end()) {
    Arg* var = (*it).second;
    return var->new_copy();
  }

  return ArgObjectInterface::arg_lookup(name);
}

//=============================================================================
Arg* ArgStatementGroup::arg_function(const std::string& name, Arg* args)
{
  ArgList* l = dynamic_cast<ArgList*> (args);
  
  if ("set" == name) {
    const ArgString* a_var = dynamic_cast<const ArgString*> (l->get(0));
    if (a_var) {
      std::map<std::string,Arg*>::iterator it =
        m_vars.find(a_var->get_string());
      if (it != m_vars.end()) {
        (*it).second = l->take(1);
        return 0;
      } else if (m_parent) {
        // Cascade to parent
        return m_parent->arg_function(name,args);
      }
    }
    return 0;
  }

  if ("var" == name) {
    const ArgString* a_var = dynamic_cast<const ArgString*> (l->get(0));
    if (a_var) {
      m_vars[a_var->get_string()] = l->take(1);
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
ArgStatementConditional::~ArgStatementConditional()
{
  delete m_true_statement;
  delete m_false_statement;
  DEBUG_COUNT_DESTRUCTOR(ArgStatementConditional);
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
bool ArgStatementConditional::run(ArgProc& proc)
{
  // Evaluate the condition
  ArgObject ctx(this);
  proc.set_ctx(&ctx);
  Arg* result = proc.evaluate(m_condition);
  bool cond = (0 != result->get_int());
  delete result;

  // "if"...
  if (cond) {
    // Condition is true
    if (m_true_statement) return m_true_statement->run(proc);
    return true;
  }

  // "else"...
  if (m_false_statement) return m_false_statement->run(proc);
  return true;
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
ArgStatementWhile::~ArgStatementWhile()
{
  delete m_body;
  DEBUG_COUNT_DESTRUCTOR(ArgStatementWhile);
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
bool ArgStatementWhile::run(ArgProc& proc)
{
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

    if (m_body && !m_body->run(proc)) {
      return false;
    }
  }

  return true;
}


//=============================================================================
ArgStatementFor::ArgStatementFor()
  : m_seq(0),
    m_body(0)
{
  DEBUG_COUNT_CONSTRUCTOR(ArgStatementFor);
}

//=============================================================================
ArgStatementFor::~ArgStatementFor()
{
  delete m_body;
  DEBUG_COUNT_DESTRUCTOR(ArgStatementFor);
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
bool ArgStatementFor::run(ArgProc& proc)
{
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

    if (m_body && !m_body->run(proc)) {
      return false;
    }

    // Evaluate the increment
    proc.set_ctx(&ctx);
    result = proc.evaluate(m_increment);
    delete result;
  }

  return true;
}


};
