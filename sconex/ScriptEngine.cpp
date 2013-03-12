/* SconeServer (http://www.sconemad.com)

Script engine

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

#include <sconex/ScriptEngine.h>
#include <sconex/ScriptStatement.h>
#include <sconex/Module.h>
#include <sconex/Log.h>
namespace scx {

// Uncomment to enable debug info
//#define ScriptEngine_DEBUG_LOG(m) DEBUG_LOG(m)

#ifndef ScriptEngine_DEBUG_LOG
#  define ScriptEngine_DEBUG_LOG(m)
#endif

ScriptEngine::TokenMap* ScriptEngine::s_tokens = 0;

enum ScriptEngineToken {
  ScriptEngineToken_IF,
  ScriptEngineToken_ELSE,
  ScriptEngineToken_WHILE,
  ScriptEngineToken_FOR,
  ScriptEngineToken_RETURN,
  ScriptEngineToken_BREAK,
  ScriptEngineToken_CONTINUE,
  ScriptEngineToken_VAR,
  ScriptEngineToken_CONST,
  ScriptEngineToken_REF,
  ScriptEngineToken_CONST_REF,
  ScriptEngineToken_SUB,
  ScriptEngineToken_OPEN_BRACE,
  ScriptEngineToken_CLOSE_BRACE
};
 
//=============================================================================
ScriptEngine::ScriptEngine(
  ScriptStatement::Ref* root
) : StreamTokenizer("ScriptEngine",4096),
    m_root(root),
    m_error_type(None)
{
  DEBUG_COUNT_CONSTRUCTOR(ScriptEngine);
  DEBUG_ASSERT(root,"No root group specified");
  init();

  // Push root onto parse stack
  m_stack.push(m_root);

  // Enable line number tracking
  m_line = 1;
  
  enable_event(Stream::Readable,true);
}

//=============================================================================
ScriptEngine::~ScriptEngine()
{
  DEBUG_COUNT_DESTRUCTOR(ScriptEngine);
}

//=============================================================================
Condition ScriptEngine::event(Stream::Event e)
{
  if (e == Stream::Readable) {
    return parse();
  }

  return Ok;
}

//=============================================================================
Condition ScriptEngine::parse()
{
  std::string buffer;
  Condition c = Ok;
  m_error_type = None;

  try {

    while (c == Ok) {
      c = tokenize(buffer);
      
      if (c==Error) {
	ScriptEngine_DEBUG_LOG("parse: Tokenization error");
	m_error_type = Tokenization;
	if (!event_error()) {
	  throw "Tokenisation error";
	}
      }

      if (c==Ok) {
        ScriptEngine_DEBUG_LOG("parse: Token '" << buffer << "'");
        if (buffer.size() > 0 && buffer[0] == '#') {
          continue;
        }
        
        while (true) {
          ScriptStatement::Ref* cur = m_stack.top();
          ScriptStatement::ParseResult result = 
	    cur->object()->parse(*this,buffer);
          if (result == ScriptStatement::Pop ||
              result == ScriptStatement::End) {
            if (m_stack.size() > 1) {
              m_stack.pop();
            } else {
	      ScriptEngine_DEBUG_LOG("parse: Parse stack underflow");
	      m_error_type = Underflow;
              if (!event_error()) {
                throw "Parse stack underflow";
              }
            }
          }
          if (result == ScriptStatement::Error) {
	    ScriptEngine_DEBUG_LOG("parse: syntax error");
	      m_error_type = Syntax;
            if (!event_error()) {
              throw "Syntax error";
            }
          }
          if (result != ScriptStatement::Pop) {
            break;
          }
        }
      }
      
      if (m_stack.top() == m_root || c==End) {
        ScriptEngine_DEBUG_LOG("parse: Run");
        if (!event_runnable()) {
          // Abort request
          break;
        }
      }
    }
  } catch (...) {
    c = Error;
  }
  
  return c;
}

//=============================================================================
ScriptStatement::Ref* ScriptEngine::parse_token(const std::string& token)
{
  TokenMap::const_iterator it = s_tokens->find(token);

  if (it == s_tokens->end()) {
    // Not a reserved token, so treat as an expression
    ScriptEngine_DEBUG_LOG("parse_token: New expression");
    return new ScriptStatement::Ref(new ScriptStatementExpr(m_line, token));
    // (don't need to stack plain expressions)
  }
  
  ScriptStatement* s=0;
  switch (it->second) {

    case ScriptEngineToken_OPEN_BRACE:
      ScriptEngine_DEBUG_LOG("parse_token: New statement group");
      s = new ScriptStatementGroup(m_line);
      break;

    case ScriptEngineToken_IF:
      ScriptEngine_DEBUG_LOG("parse_token: New conditional");
      s = new ScriptStatementConditional(m_line);
      break;

    case ScriptEngineToken_WHILE:
      ScriptEngine_DEBUG_LOG("parse_token: New while loop");
      s = new ScriptStatementWhile(m_line);
      break;

    case ScriptEngineToken_FOR:
      ScriptEngine_DEBUG_LOG("parse_token: New for loop");
      s = new ScriptStatementFor(m_line);
      break;

    case ScriptEngineToken_RETURN:
      ScriptEngine_DEBUG_LOG("parse_token: New flow return");
      s = new ScriptStatementFlow(m_line, ScriptStatement::Return);
      break;

    case ScriptEngineToken_BREAK:
      ScriptEngine_DEBUG_LOG("parse_token: New flow last");
      s = new ScriptStatementFlow(m_line, ScriptStatement::Last);
      break;

    case ScriptEngineToken_CONTINUE:
      ScriptEngine_DEBUG_LOG("parse_token: New flow next");
      s = new ScriptStatementFlow(m_line, ScriptStatement::Next);
      break;

    case ScriptEngineToken_VAR:
      ScriptEngine_DEBUG_LOG("parse_token: New variable declaration");
      s = new ScriptStatementDecl(m_line, ScriptStatementDecl::Var);
      break;

    case ScriptEngineToken_CONST:
      ScriptEngine_DEBUG_LOG("parse_token: New constant declaration");
      s = new ScriptStatementDecl(m_line, ScriptStatementDecl::Const);
      break;

    case ScriptEngineToken_REF:
      ScriptEngine_DEBUG_LOG("parse_token: New reference declaration");
      s = new ScriptStatementDecl(m_line, ScriptStatementDecl::Ref);
      break;

    case ScriptEngineToken_CONST_REF:
      ScriptEngine_DEBUG_LOG("parse_token: New constant reference declaration");
      s = new ScriptStatementDecl(m_line, ScriptStatementDecl::ConstRef);
      break;

    case ScriptEngineToken_SUB:
      ScriptEngine_DEBUG_LOG("parse_token: New subroutine declaration");
      s = new ScriptStatementSub(m_line);
      break;
 
    default:
      DEBUG_LOG("Unknown token type:" << it->second);
      return 0;
  }

  ScriptStatement::Ref* sr = new ScriptStatement::Ref(s);
  m_stack.push(sr);
  return sr;
}

//=============================================================================
ScriptEngine::ErrorType ScriptEngine::get_error_type() const
{
  return m_error_type;
}

//=============================================================================
int ScriptEngine::get_error_line() const
{
  return m_line;
}

//=============================================================================
bool ScriptEngine::event_runnable()
{
  return true; // Keep parsing
}

//=============================================================================
bool ScriptEngine::event_error()
{
  return false; // Abort parsing
}

//=============================================================================
bool ScriptEngine::next_token(
  const Buffer& buffer,
  int& pre_skip,
  int& length,
  int& post_skip
)
{
  const char* cur = (char*)buffer.head();
  const char* end = cur + buffer.used();

  for ( ; cur<end; ++cur) {
    if (!isspace(*cur)) {
      break;
    }
    ++pre_skip;
  }

  const char* start = cur;
  char in_quote = 0; // Holds the current quote char (" or ') when in quote
  bool escape = false; // Was the last char an escape (\)
  bool in_comment = false;
  bool first = true;

  ScriptStatement::ParseMode pmode = m_stack.top()->object()->parse_mode();
  int in_bracket = 0;
  
  for ( ; cur<end; ++cur) {
    switch (pmode) {

      case ScriptStatement::SemicolonTerminated: {
        // SEMICOLON TERMINATED parse mode

        if ((*cur == ';') || (*cur == '(') || isspace(*cur)) {
          std::string str(start,length);
          if (s_tokens->count(str)) {
            return true;
          }
        }
        
        if (in_quote) {
          // Ignore everything until (unescaped) end quote
          if ((*cur) == in_quote && !escape) {
            in_quote = 0;
          }
	  // Check for escape sequences
	  if ((*cur) == '\\') {
	    escape = true;
	  } else {
	    escape = false;
	  }
          
        } else if (in_comment) {
          // Ignore everything until end of line
          while ((*cur) == '\r' || (*cur) == '\n') {
            in_comment = false;
            ++post_skip;
            ++cur;
            if (cur == end) throw "Oh shit!";
          }
          if (!in_comment) {
            return true;
          }
          
        } else {
          switch (*cur) {
            case ';':
              ++post_skip;
              return true;
            case '"':
            case '\'':
              in_quote = (*cur);
              break;
            case '#':
              in_comment = true;
              break;
          }      
        }
        
      } break;

      case ScriptStatement::Bracketed: {
        // BRACKETED parse mode

        if (in_quote) {
          // Ignore everything until (unescaped) end quote
          if ((*cur) == in_quote && !escape) {
            in_quote = 0;
          }
	  // Check for escape sequences
	  if ((*cur) == '\\') {
	    escape = true;
	  } else {
	    escape = false;
	  }
          
        } else {
          switch (*cur) {
            case '"':
            case '\'':
              in_quote = (*cur);
              break;
            case '(':
              if (in_bracket==0) {
                ++pre_skip;
                --length;
              }
              ++in_bracket;
              break;
            case ')':
              --in_bracket;
              if (in_bracket==0) {
                ++post_skip;
                return true;
              }
              break;
          }      
        }
      } break;
      
      case ScriptStatement::Name: {
        // NAME parse mode
        if (first) {
          if (!isalpha(*cur)) {
            // Name must start with alpha
            return false;
          }
          break;
        }
        // Subsequent characters can be alpha, digit or underscore
        if (!isalpha(*cur) && !isdigit(*cur) && (*cur != '_')) {
          return true;
        }
      } break;
      
    }
    
    ++length;
    first = false;
  }
  
  return false;
}

//=============================================================================
void ScriptEngine::init()
{
  if (s_tokens) return;

  s_tokens = new TokenMap();

  (*s_tokens)["if"] = ScriptEngineToken_IF;
  (*s_tokens)["else"] = ScriptEngineToken_ELSE;
  (*s_tokens)["while"] = ScriptEngineToken_WHILE;
  (*s_tokens)["for"] = ScriptEngineToken_FOR;
  (*s_tokens)["return"] = ScriptEngineToken_RETURN;
  (*s_tokens)["break"] = ScriptEngineToken_BREAK;
  (*s_tokens)["continue"] = ScriptEngineToken_CONTINUE;
  (*s_tokens)["var"] = ScriptEngineToken_VAR;
  (*s_tokens)["const"] = ScriptEngineToken_CONST;
  (*s_tokens)["ref"] = ScriptEngineToken_REF;
  (*s_tokens)["constref"] = ScriptEngineToken_CONST_REF;
  (*s_tokens)["sub"] = ScriptEngineToken_SUB;
  (*s_tokens)["{"] = ScriptEngineToken_OPEN_BRACE;
  (*s_tokens)["}"] = ScriptEngineToken_CLOSE_BRACE;
}

// ScriptEngineExec:

//=============================================================================
ScriptEngineExec::ScriptEngineExec(const ScriptAuth& auth, 
				   ScriptRef* ctx,
				   const std::string& file)
  : ScriptEngine(new ScriptStatement::Ref(new ScriptStatementGroup(0))),
    m_tracer(auth,file),
    m_ctx(ctx),
    m_error_des(0)
{
  m_root->object()->set_parent(m_ctx->object());
}

//=============================================================================
ScriptEngineExec::~ScriptEngineExec()
{
  delete m_ctx;
  delete m_root;
}

//=============================================================================
void ScriptEngineExec::set_error_des(Descriptor* error_des)
{
  m_error_des = error_des;
}

//=============================================================================
bool ScriptEngineExec::event_runnable()
{
  ScriptRef* ret = m_root->object()->execute(m_tracer);
  delete ret;

  // Output any errors
  for (ScriptTracer::ErrorList::iterator it = m_tracer.errors().begin();
       it != m_tracer.errors().end(); ++it) {
    ScriptTracer::ErrorEntry& err = *it;
    Log log("ScriptEngine");
    log.attach("file", err.file);
    log.attach("line", ScriptInt::new_ref(err.line));
    log.submit(err.error);

    if (m_error_des) {
      std::ostringstream oss;
      oss << err.file << ":" << err.line << ": " << err.error << "\n";
      m_error_des->write(oss.str());
    }    
  }
  m_tracer.errors().clear();

  ScriptStatementGroup* g_root = 
    dynamic_cast<ScriptStatementGroup*>(m_root->object());
  DEBUG_ASSERT(g_root,"Root statement should be type ScriptStatementGroup");
  g_root->clear();

  return true;
}

//=============================================================================
bool ScriptEngineExec::event_error()
{
  std::ostringstream oss;
  oss << m_tracer.get_file() << ":" 
      << get_error_line() << ": Script ";
  switch (get_error_type()) {
  case Tokenization: oss << "tokenization"; break;
  case Syntax: oss << "syntax"; break;
  case Underflow: oss << "underflow"; break;
  default: oss << "unknown"; break;
  }
  oss << " error";
  Log("ScriptEngine").submit(oss.str());
  if (m_error_des) {
    oss << "\n";
    m_error_des->write(oss.str());
  }
  return false;
}

};
