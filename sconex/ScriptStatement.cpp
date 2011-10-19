/* SconeServer (http://www.sconemad.com)

Script script statements

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

#include <sconex/ScriptStatement.h>
#include <sconex/ScriptEngine.h>
#include <sconex/ScriptExpr.h>
namespace scx {


//=============================================================================
ScriptTracer::ScriptTracer(const ScriptAuth& auth,
			   const std::string& file,
			   int line_offset)
  : m_expr(new ScriptExpr(auth)),
    m_file(file),
    m_line_offset(line_offset)
{

}

//=============================================================================
ScriptTracer::ScriptTracer(const ScriptTracer& c)
  : m_expr(new ScriptExpr(*c.m_expr)),
    m_file(c.m_file),
    m_line_offset(c.m_line_offset)
{
  
}

//=============================================================================
ScriptTracer::~ScriptTracer()
{
  delete m_expr;
}

//=============================================================================
ScriptTracer* ScriptTracer::new_copy()
{
  return new ScriptTracer(*this);
}

//=============================================================================
ScriptRef* ScriptTracer::evaluate(const std::string& expr,
				  ScriptStatement* ctx)
{
  scx::ScriptRef ctxr(ctx);
  m_expr->set_ctx(&ctxr);
  ScriptRef* ret = m_expr->evaluate(expr);

  if (ret && BAD_SCRIPTREF(ret)) {
    std::ostringstream oss;
    oss << m_file << ":" << (m_line_offset + ctx->get_line()) << ": " 
	<< ret->object()->get_string() << " (" << expr << ")";
    m_errors.push_back(oss.str());
  }

  return ret;
}

//=============================================================================
ScriptExpr& ScriptTracer::get_expr() 
{ 
  return *m_expr;
}

//=============================================================================
const std::string& ScriptTracer::get_file() const
{
  return m_file;
}

//=============================================================================
int ScriptTracer::get_line_offset() const
{
  return m_line_offset;
}

//=============================================================================
ScriptTracer::ErrorList& ScriptTracer::errors()
{
  return m_errors;
}

//=============================================================================
ScriptStatement::ScriptStatement(int line)
  : m_line(line)
{
  DEBUG_COUNT_CONSTRUCTOR(ScriptStatement);
}

//=============================================================================
ScriptStatement::ScriptStatement(const ScriptStatement& c)
  : ScriptObject(c),
    m_line(c.m_line)
{
  DEBUG_COUNT_CONSTRUCTOR(ScriptStatement)
}
  
//=============================================================================
ScriptStatement::~ScriptStatement()
{
  DEBUG_COUNT_DESTRUCTOR(ScriptStatement);
}

//=============================================================================
ScriptStatement::ParseMode ScriptStatement::parse_mode() const
{
  return ScriptStatement::SemicolonTerminated;
}

//=============================================================================
ScriptRef* ScriptStatement::execute(ScriptTracer& tracer)
{
  ScriptStatement::FlowMode flow = ScriptStatement::Normal;
  return run(tracer,flow);
}

//=============================================================================
std::string ScriptStatement::get_string() const
{
  return "Statement";
}

//=============================================================================
ScriptRef* ScriptStatement::script_method(
  const ScriptAuth& auth,
  const ScriptRef& ref,
  const std::string& name,
  const ScriptRef* args)
{
  if (m_parent) {
    // Cascade to parent
    return m_parent->script_method(auth,ref,name,args);
  }

  return 0;
}

//=============================================================================
void ScriptStatement::set_parent(ScriptObject* parent)
{
  m_parent = parent;
}

//=============================================================================
int ScriptStatement::get_line() const
{
  return m_line;
}

//=============================================================================
ScriptStatementExpr::ScriptStatementExpr(int line, const std::string& expr)
  : ScriptStatement(line),
    m_expr(expr)
{

}

//=============================================================================
ScriptStatementExpr::ScriptStatementExpr(const ScriptStatementExpr& c)
  : ScriptStatement(c),
    m_expr(c.m_expr)
{

}

//=============================================================================
ScriptStatementExpr::~ScriptStatementExpr()
{

}

//=============================================================================
ScriptObject* ScriptStatementExpr::new_copy() const
{
  return new ScriptStatementExpr(*this);
}

//=============================================================================
ScriptStatement::ParseResult ScriptStatementExpr::parse(
  ScriptEngine& script,
  const std::string& token
)
{
  return ScriptStatement::Pop;
}

//=============================================================================
ScriptRef* ScriptStatementExpr::run(ScriptTracer& tracer, FlowMode& flow)
{
  return tracer.evaluate(m_expr,this);
}


//=============================================================================
ScriptStatementGroup::ScriptStatementGroup(int line, ScriptMap* env)
  : ScriptStatement(line),
    m_env(env),
    m_own_env(false)
{
  if (m_env == 0) {
    // If no environment was given, create our own
    m_env = new ScriptMap();
    m_own_env = true;
  }
}

//=============================================================================
ScriptStatementGroup::ScriptStatementGroup(const ScriptStatementGroup& c)
  : ScriptStatement(c),
    m_env(c.m_env),
    m_own_env(c.m_own_env)
{
  if (m_own_env) {
    // If original has its own environment, make our own copy
    m_env = new ScriptMap(*c.m_env);
  }

  for (StatementList::const_iterator it = c.m_statements.begin();
       it != c.m_statements.end();
       ++it) {
    ScriptStatement::Ref* ac = (ScriptStatement::Ref*)((*it)->new_copy());
    ac->object()->set_parent(this);
    m_statements.push_back(ac);
  }
}

//=============================================================================
ScriptStatementGroup::~ScriptStatementGroup()
{
  clear();

  if (m_own_env) {
    delete m_env;
  }
}

//=============================================================================
ScriptObject* ScriptStatementGroup::new_copy() const
{
  return new ScriptStatementGroup(*this);
}

//=============================================================================
ScriptStatement::ParseResult ScriptStatementGroup::parse(
  ScriptEngine& script,
  const std::string& token
)
{
  if (token == "}") {
    return ScriptStatement::End;
  }

  ScriptStatement::Ref* sub = script.parse_token(token);
  if (sub==0) {
    return ScriptStatement::Error;
  }

  sub->object()->set_parent(this);
  m_statements.push_back(sub);
  return ScriptStatement::Continue;
}

//=============================================================================
ScriptRef* ScriptStatementGroup::run(ScriptTracer& tracer, FlowMode& flow)
{
  ScriptRef* ret=0;
  for (StatementList::iterator it = m_statements.begin();
       it != m_statements.end();
       ++it) {
    delete ret;
    ret = (*it)->object()->run(tracer,flow);
    if (ret && (typeid(*ret->object()) == typeid(scx::ScriptError))) {
      return ret;
    } else if (flow != Normal) {
      return ret;
    }
  }
  
  return ret;
}

//=============================================================================
ScriptRef* ScriptStatementGroup::script_op(const ScriptAuth& auth,
					   const ScriptRef& ref,
					   const ScriptOp& op,
					   const ScriptRef* right)
{
  if (op.type() == ScriptOp::Lookup) {
    std::string name = right->object()->get_string();
    if ("var" == name) {
      return new ScriptMethodRef(ref,name);
    }
    ScriptRef* var = m_env->lookup(name);
    if (var) {
      return var->ref_copy(ref.reftype());
    }
  }

  return ScriptObject::script_op(auth,ref,op,right);
}

//=============================================================================
ScriptRef* ScriptStatementGroup::script_method(const ScriptAuth& auth,
					       const ScriptRef& ref,
					       const std::string& name,
					       const ScriptRef* args)
{
  if ("var" == name) {
    const ScriptString* a_name = get_method_arg<ScriptString>(args,0,"name");
    if (a_name) {
      std::string var_name = a_name->get_string();
      const ScriptRef* a_val = 
	get_method_arg_ref(args,1,"value");
      if (a_val) {
	m_env->give(var_name, a_val->ref_copy());
      } else {
	// If no initialiser was given, default to integer 0
	m_env->give(var_name, ScriptInt::new_ref(0));
      }
    }
    return 0;
  }
  
  return ScriptStatement::script_method(auth,ref,name,args);
}

//=============================================================================
void ScriptStatementGroup::clear()
{
  StatementList::iterator it = m_statements.begin();
  while (it != m_statements.end()) {
    delete (*it);
    ++it;
  }
  m_statements.clear();
}


//=============================================================================
ScriptStatementConditional::ScriptStatementConditional(int line, const std::string& condition)
  : ScriptStatement(line),
    m_seq(0),
    m_condition(condition),
    m_true_statement(0),
    m_false_statement(0)
{

}

//=============================================================================
ScriptStatementConditional::ScriptStatementConditional(const ScriptStatementConditional& c)
  : ScriptStatement(c),
    m_seq(c.m_seq),
    m_condition(c.m_condition),
    m_true_statement(0),
    m_false_statement(0)
    
{
  if (c.m_true_statement) {
    m_true_statement = (ScriptStatement::Ref*)c.m_true_statement->new_copy();
    m_true_statement->object()->set_parent(this);
  }

  if (c.m_false_statement) {
    m_false_statement = (ScriptStatement::Ref*)c.m_false_statement->new_copy();
    m_false_statement->object()->set_parent(this);
  }
}

//=============================================================================
ScriptStatementConditional::~ScriptStatementConditional()
{
  delete m_true_statement;
  delete m_false_statement;
}

//=============================================================================
ScriptObject* ScriptStatementConditional::new_copy() const
{
  return new ScriptStatementConditional(*this);
}

//=============================================================================
ScriptStatement::ParseResult ScriptStatementConditional::parse(
  ScriptEngine& script,
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
      if (m_true_statement) m_true_statement->object()->set_parent(this);
    } break;

    case 3: {
      if (token != "else") {
        return ScriptStatement::Pop;
      }
    } break;

    case 4: {
      delete m_false_statement;
      m_false_statement = script.parse_token(token);
      if (m_false_statement) m_false_statement->object()->set_parent(this);
    } break;

    default: {
      return ScriptStatement::Pop;
    }
  }

  return ScriptStatement::Continue;
}

//=============================================================================
ScriptStatement::ParseMode ScriptStatementConditional::parse_mode() const
{
  return (m_seq==0 ?
          ScriptStatement::Bracketed :
          ScriptStatement::SemicolonTerminated);
}

//=============================================================================
ScriptRef* ScriptStatementConditional::run(ScriptTracer& tracer, FlowMode& flow)
{
  // Evaluate the condition
  ScriptRef* result = tracer.evaluate(m_condition,this);
  bool cond = (result && 0 != result->object()->get_int());
  delete result;

  if (cond) {
    if (m_true_statement) return m_true_statement->object()->run(tracer,flow);
  } else {
    if (m_false_statement) return m_false_statement->object()->run(tracer,flow);
  }
  
  return 0;
}


//=============================================================================
ScriptStatementWhile::ScriptStatementWhile(int line, const std::string& condition)
  : ScriptStatement(line),
    m_seq(0),
    m_condition(condition),
    m_body(0)
{

}

//=============================================================================
ScriptStatementWhile::ScriptStatementWhile(const ScriptStatementWhile& c)
  : ScriptStatement(c),
    m_seq(c.m_seq),
    m_condition(c.m_condition),
    m_body(0)
{
  if (c.m_body) {
    m_body = (ScriptStatement::Ref*)c.m_body->new_copy();
    m_body->object()->set_parent(this);
  }
}

//=============================================================================
ScriptStatementWhile::~ScriptStatementWhile()
{
  delete m_body;
}

//=============================================================================
ScriptObject* ScriptStatementWhile::new_copy() const
{
  return new ScriptStatementWhile(*this);
}

//=============================================================================
ScriptStatement::ParseResult ScriptStatementWhile::parse(
  ScriptEngine& script,
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
      if (m_body) m_body->object()->set_parent(this);
    } break;

    default: {
      return ScriptStatement::Pop;
    }
  }

  return ScriptStatement::Continue;
}

//=============================================================================
ScriptStatement::ParseMode ScriptStatementWhile::parse_mode() const
{
  return (m_seq==0 ?
          ScriptStatement::Bracketed :
          ScriptStatement::SemicolonTerminated);
}

//=============================================================================
ScriptRef* ScriptStatementWhile::run(ScriptTracer& tracer, FlowMode& flow)
{
  ScriptRef* ret=0;
  
  while (true) {
    // Evaluate the condition
    ScriptRef* result = tracer.evaluate(m_condition,this);
    bool cond = (result && 0 != result->object()->get_int());
    delete result;

    if (!cond) {
      // Condition is false, break out of the loop
      break;
    }

    if (m_body) {
      delete ret;
      ret = m_body->object()->run(tracer,flow);
      if (ret && (typeid(*ret->object()) == typeid(scx::ScriptError))) {
	return ret;
      } else if (flow == Return) {
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
ScriptStatementFor::ScriptStatementFor(int line)
  : ScriptStatement(line),
    m_seq(0),
    m_body(0)
{

}

//=============================================================================
ScriptStatementFor::ScriptStatementFor(const ScriptStatementFor& c)
  : ScriptStatement(c),
    m_seq(c.m_seq),
    m_initialiser(c.m_initialiser),
    m_condition(c.m_condition),
    m_increment(c.m_increment),
    m_body(0)
{
  if (c.m_body) {
    m_body = c.m_body->new_copy();
    m_body->object()->set_parent(this);
  }
}

//=============================================================================
ScriptStatementFor::~ScriptStatementFor()
{
  delete m_body;
}

//=============================================================================
ScriptObject* ScriptStatementFor::new_copy() const
{
  return new ScriptStatementFor(*this);
}

//=============================================================================
ScriptStatement::ParseResult ScriptStatementFor::parse(
  ScriptEngine& script,
  const std::string& token
)
{
  switch (++m_seq) {
    case 1: {
      std::string::size_type isc1 = token.find(";",0);
      if (isc1 == std::string::npos) return ScriptStatement::Error;
      m_initialiser = token.substr(0,isc1);
      std::string::size_type isc2 = token.find(";",isc1+1);
      if (isc2 == std::string::npos) return ScriptStatement::Error;
      m_condition = token.substr(isc1+1,isc2-isc1-1);
      m_increment = token.substr(isc2+1);
    } break;

    case 2: {
      delete m_body;
      m_body = script.parse_token(token);
      if (m_body) m_body->object()->set_parent(this);
    } break;

    default: {
      return ScriptStatement::Pop;
    }
  }

  return ScriptStatement::Continue;
}

//=============================================================================
ScriptStatement::ParseMode ScriptStatementFor::parse_mode() const
{
  return (m_seq==0 ?
          ScriptStatement::Bracketed :
          ScriptStatement::SemicolonTerminated);
}

//=============================================================================
ScriptRef* ScriptStatementFor::run(ScriptTracer& tracer, FlowMode& flow)
{
  ScriptRef* ret=0;
  
  // Evaluate the initialiser
  ScriptRef* result = tracer.evaluate(m_initialiser,this);
  delete result;

  while (true) {
    // Evaluate the condition
    result = tracer.evaluate(m_condition,this);
    bool cond = (result && 0 != result->object()->get_int());
    delete result;

    if (!cond) {
      // Condition is false, break out of the loop
      break;
    }

    if (m_body) {
      delete ret;
      ret = m_body->object()->run(tracer,flow);
      if (ret && (typeid(*ret->object()) == typeid(scx::ScriptError))) {
	return ret;
      } else if (flow == Return) {
	return ret;
      } else if (flow == Last) {
	flow = Normal;
	return ret;
      } else if (flow == Next) {
	flow = Normal;
      }
    }

    // Evaluate the increment
    result = tracer.evaluate(m_increment,this);
    delete result;
  }

  return ret;
}


//=============================================================================
ScriptStatementFlow::ScriptStatementFlow(int line, FlowMode flow)
  : ScriptStatement(line),
    m_seq(0),
    m_flow(flow)
{

}

//=============================================================================
ScriptStatementFlow::ScriptStatementFlow(const ScriptStatementFlow& c)
  : ScriptStatement(c),
    m_seq(c.m_seq),
    m_flow(c.m_flow),
    m_ret_expr(c.m_ret_expr)
{

}

//=============================================================================
ScriptStatementFlow::~ScriptStatementFlow()
{

}

//=============================================================================
  ScriptObject* ScriptStatementFlow::new_copy() const
{
  return new ScriptStatementFlow(*this);
}

//=============================================================================
ScriptStatement::ParseResult ScriptStatementFlow::parse(
  ScriptEngine& script,
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
      return ScriptStatement::Pop;
    }
  }

  return ScriptStatement::Continue;
}

//=============================================================================
ScriptStatement::ParseMode ScriptStatementFlow::parse_mode() const
{
  return ScriptStatement::SemicolonTerminated;
}

//=============================================================================
ScriptRef* ScriptStatementFlow::run(ScriptTracer& tracer, FlowMode& flow)
{
  ScriptRef* ret=0;
  if (!m_ret_expr.empty()) {
    ret = tracer.evaluate(m_ret_expr,this);
  }

  flow = m_flow;
  return ret;
}


//=============================================================================
ScriptStatementDecl::ScriptStatementDecl(int line,
					 DefType deftype, 
					 const std::string& name)
  : ScriptStatement(line),
    m_seq(0),
    m_deftype(deftype),
    m_name(name)
{

}

//=============================================================================
ScriptStatementDecl::ScriptStatementDecl(const ScriptStatementDecl& c)
  : ScriptStatement(c),
    m_seq(c.m_seq),
    m_deftype(c.m_deftype),
    m_name(c.m_name),
    m_initialiser(c.m_initialiser)
{

}

//=============================================================================
ScriptStatementDecl::~ScriptStatementDecl()
{

}

//=============================================================================
ScriptObject* ScriptStatementDecl::new_copy() const
{
  return new ScriptStatementDecl(*this);
}

//=============================================================================
ScriptStatement::ParseResult ScriptStatementDecl::parse(
  ScriptEngine& script,
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
      return ScriptStatement::Pop;
    }
  }

  return ScriptStatement::Continue;
}

//=============================================================================
ScriptStatement::ParseMode ScriptStatementDecl::parse_mode() const
{
  return (m_seq==0 ?
          ScriptStatement::Name :
          ScriptStatement::SemicolonTerminated);
}

//=============================================================================
ScriptRef* ScriptStatementDecl::run(ScriptTracer& tracer, FlowMode& flow)
{
  ScriptRef* initialiser=0;
  if (!m_initialiser.empty()) {
    initialiser = tracer.evaluate(m_initialiser,this);
  }

  ScriptList::Ref args(new ScriptList());
  args.object()->give(ScriptString::new_ref(m_name));
  if (initialiser) {
    switch (m_deftype) {
    case Var:
      args.object()->give(initialiser->new_copy(ScriptRef::Ref));
      break;
    case Const:
      args.object()->give(initialiser->new_copy(ScriptRef::ConstRef));
      break;
    case Ref:
      args.object()->give(initialiser->ref_copy(ScriptRef::Ref));
      break;
    case ConstRef:
      args.object()->give(initialiser->ref_copy(ScriptRef::ConstRef));
      break;
    }
    delete initialiser;
  }

  return ScriptMethodRef(this,"var").call(tracer.get_expr().get_auth(),&args);
}


//=============================================================================
ScriptStatementSub::ScriptStatementSub(int line, const std::string& name)
  : ScriptStatement(line),
    m_seq(0),
    m_name(name),
    m_body(0)
{ 

}

//=============================================================================
ScriptStatementSub::ScriptStatementSub(const ScriptStatementSub& c)
  : ScriptStatement(c),
    m_seq(c.m_seq),
    m_name(c.m_name),
    m_body(0)
{
  if (c.m_body) {
    m_body = c.m_body->new_copy();
    m_body->object()->set_parent(this);
  }
}

//=============================================================================
ScriptStatementSub::~ScriptStatementSub()
{
  delete m_body;
}

//=============================================================================
ScriptObject* ScriptStatementSub::new_copy() const
{
  return new ScriptStatementSub(*this);
}

//=============================================================================
ScriptStatement::ParseResult ScriptStatementSub::parse(
  ScriptEngine& script,
  const std::string& token
)
{
  switch (++m_seq) {
    case 1: {
      m_name = token;
    } break;

    case 2: {
      std::string::size_type start = 0;
      std::string::size_type end;
      // Find function arguments as comma separated subtokens
      do {
        end = token.find(",",start);
        std::string sub;
        if (end == std::string::npos) {
          sub = token.substr(start);
        } else {
          sub = token.substr(start,end-start);
        }
        // Remove any spaces surrounding argument
        std::string::size_type sub_start = sub.find_first_not_of(" ");
        std::string::size_type sub_end = sub.find_last_not_of(" ");
        if (sub_start != std::string::npos &&
            sub_end != std::string::npos) {
          sub = sub.substr(sub_start,sub_end-sub_start+1);
          m_arg_names.push_back(sub);
        }
        start = end + 1;
      } while (end != std::string::npos);
    } break;

    case 3: {
      delete m_body;
      m_body = script.parse_token(token);
      if (m_body) m_body->object()->set_parent(m_parent);
    } break;

    default: {
      return ScriptStatement::Pop;
    }
  }

  return ScriptStatement::Continue;
}

//=============================================================================
ScriptStatement::ParseMode ScriptStatementSub::parse_mode() const
{
  switch (m_seq) {
    case 0: return ScriptStatement::Name;
    case 1: return ScriptStatement::Bracketed;
  }
  return ScriptStatement::SemicolonTerminated;
}

//=============================================================================
ScriptRef* ScriptStatementSub::run(ScriptTracer& tracer, FlowMode& flow)
{
  ScriptList::Ref args(new ScriptList());
  args.object()->give(ScriptString::new_ref(m_name));
  args.object()->give(new ScriptRef(new ScriptSub(m_name,
						  m_arg_names,
						  m_body,
						  tracer)));
  m_body = 0;

  return ScriptMethodRef(this,"var").call(tracer.get_expr().get_auth(),&args);
}


};
