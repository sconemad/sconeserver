/* SconeServer (http://www.sconemad.com)

Arg Script parser

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

#include "sconex/ArgScript.h"
#include "sconex/ArgObject.h"
#include "sconex/ArgStatement.h"
#include "sconex/Module.h"
namespace scx {

// Uncomment to enable debug info
//#define ArgScript_DEBUG_LOG(m) DEBUG_LOG(m)

#ifndef ArgScript_DEBUG_LOG
#  define ArgScript_DEBUG_LOG(m)
#endif

ArgScript::TokenMap* ArgScript::s_tokens = 0;

enum ArgScriptToken {
  ArgScriptToken_IF,
  ArgScriptToken_ELSE,
  ArgScriptToken_WHILE,
  ArgScriptToken_FOR,
  ArgScriptToken_RETURN,
  ArgScriptToken_BREAK,
  ArgScriptToken_CONTINUE,
  ArgScriptToken_VAR,
  ArgScriptToken_CONST,
  ArgScriptToken_REF,
  ArgScriptToken_CONST_REF,
  ArgScriptToken_SUB,
  ArgScriptToken_OPEN_BRACE,
  ArgScriptToken_CLOSE_BRACE
};
 
//=============================================================================
ArgScript::ArgScript(
  ArgStatementGroup* root
) : StreamTokenizer("ArgScript",4096),
    m_root(root)
{
  DEBUG_COUNT_CONSTRUCTOR(ArgScript);
  DEBUG_ASSERT(root,"No root group specified");
  init();

  // Push root onto parse stack
  m_stack.push(m_root);
  
  enable_event(Stream::Readable,true);
}

//=============================================================================
ArgScript::~ArgScript()
{
  DEBUG_COUNT_DESTRUCTOR(ArgScript);
}

//=============================================================================
Condition ArgScript::event(Stream::Event e)
{
  if (e == Stream::Readable) {
    std::string buffer;
    Condition c = Ok;

    while (c == Ok) {
      c = tokenize(buffer);
      
      if (c==Ok) {
        ArgScript_DEBUG_LOG("event: Token '" << buffer << "'");
        if (buffer.size() > 0 && buffer[0] == '#') {
          continue;
        }
      
        while (true) {
          ArgStatement* cur = m_stack.top();
          ArgStatement::ParseResult result = cur->parse(*this,buffer);
          if (result == ArgStatement::Pop ||
              result == ArgStatement::End) {
            if (m_stack.size() > 1) {
              m_stack.pop();
            } else {
              ArgScript_DEBUG_LOG("event: Parse stack underflow");
            }
          }
          if (result != ArgStatement::Pop) {
            break;
          }
        }
      }

      if (m_stack.top() == m_root || c==End) {
        ArgScript_DEBUG_LOG("event: Run");
	if (!event_runnable()) {
	  // Abort request
	  break;
	}
      }
    }

    return c;
  }

  return Ok;
}

//=============================================================================
ArgStatement* ArgScript::parse_token(const std::string& token)
{
  TokenMap::const_iterator it = s_tokens->find(token);

  if (it == s_tokens->end()) {
    // Not a reserved token, so treat as an expression
    ArgScript_DEBUG_LOG("parse_token: New expression");
    return new ArgStatementExpr(token);
    // (don't need to stack plain expressions)
  }
  
  ArgStatement* s=0;
  switch (it->second) {

    case ArgScriptToken_OPEN_BRACE:
      ArgScript_DEBUG_LOG("parse_token: New statement group");
      s = new ArgStatementGroup();
      break;

    case ArgScriptToken_IF:
      ArgScript_DEBUG_LOG("parse_token: New conditional");
      s = new ArgStatementConditional();
      break;

    case ArgScriptToken_WHILE:
      ArgScript_DEBUG_LOG("parse_token: New while loop");
      s = new ArgStatementWhile();
      break;

    case ArgScriptToken_FOR:
      ArgScript_DEBUG_LOG("parse_token: New for loop");
      s = new ArgStatementFor();
      break;

    case ArgScriptToken_RETURN:
      ArgScript_DEBUG_LOG("parse_token: New flow return");
      s = new ArgStatementFlow(ArgStatement::Return);
      break;

    case ArgScriptToken_BREAK:
      ArgScript_DEBUG_LOG("parse_token: New flow last");
      s = new ArgStatementFlow(ArgStatement::Last);
      break;

    case ArgScriptToken_CONTINUE:
      ArgScript_DEBUG_LOG("parse_token: New flow next");
      s = new ArgStatementFlow(ArgStatement::Next);
      break;

    case ArgScriptToken_VAR:
      ArgScript_DEBUG_LOG("parse_token: New variable declaration");
      s = new ArgStatementDecl(ArgStatementDecl::Var);
      break;

    case ArgScriptToken_CONST:
      ArgScript_DEBUG_LOG("parse_token: New constant declaration");
      s = new ArgStatementDecl(ArgStatementDecl::Const);
      break;

    case ArgScriptToken_REF:
      ArgScript_DEBUG_LOG("parse_token: New reference declaration");
      s = new ArgStatementDecl(ArgStatementDecl::Ref);
      break;

    case ArgScriptToken_CONST_REF:
      ArgScript_DEBUG_LOG("parse_token: New constant reference declaration");
      s = new ArgStatementDecl(ArgStatementDecl::ConstRef);
      break;

    case ArgScriptToken_SUB:
      ArgScript_DEBUG_LOG("parse_token: New subroutine declaration");
      s = new ArgStatementSub();
      break;
 
    default:
      DEBUG_LOG("Unknown token type:" << it->second);
      return 0;
  }

  m_stack.push(s);
  return s;
}

//=============================================================================
bool ArgScript::event_runnable()
{
  return true; // Keep parsing
}

//=============================================================================
bool ArgScript::next_token(
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

  ArgStatement::ParseMode pmode = m_stack.top()->parse_mode();
  int in_bracket = 0;
  
  for ( ; cur<end; ++cur) {
    switch (pmode) {

      case ArgStatement::SemicolonTerminated: {
        // SEMICOLON TERMINATED parse mode

        std::string str(start,length);
        if (s_tokens->count(str)) {
          return true;
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

      case ArgStatement::Bracketed: {
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
      
      case ArgStatement::Name: {
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
void ArgScript::init()
{
  if (s_tokens) return;

  s_tokens = new TokenMap();

  (*s_tokens)["if"] = ArgScriptToken_IF;
  (*s_tokens)["else"] = ArgScriptToken_ELSE;
  (*s_tokens)["while"] = ArgScriptToken_WHILE;
  (*s_tokens)["for"] = ArgScriptToken_FOR;
  (*s_tokens)["return"] = ArgScriptToken_RETURN;
  (*s_tokens)["break"] = ArgScriptToken_BREAK;
  (*s_tokens)["continue"] = ArgScriptToken_CONTINUE;
  (*s_tokens)["var"] = ArgScriptToken_VAR;
  (*s_tokens)["const"] = ArgScriptToken_CONST;
  (*s_tokens)["ref"] = ArgScriptToken_REF;
  (*s_tokens)["constref"] = ArgScriptToken_CONST_REF;
  (*s_tokens)["sub"] = ArgScriptToken_SUB;
  (*s_tokens)["{"] = ArgScriptToken_OPEN_BRACE;
  (*s_tokens)["}"] = ArgScriptToken_CLOSE_BRACE;
}

// ArgScriptExec:

//=============================================================================
ArgScriptExec::ArgScriptExec(
  const Auth& auth, 
  ArgObject* ctx
) : ArgScript(new ArgStatementGroup()),
    m_proc(auth,ctx),
    m_ctx(ctx),
    m_error_des(0)
{
  m_root->set_parent(m_ctx->get_object());
}

//=============================================================================
ArgScriptExec::~ArgScriptExec()
{
  delete m_ctx;
  delete m_root;
}

//=============================================================================
void ArgScriptExec::set_error_des(Descriptor* error_des)
{
  m_error_des = error_des;
}

//=============================================================================
bool ArgScriptExec::event_runnable()
{
  Arg* ret = m_root->execute(m_proc);
  if (m_error_des && ret && BAD_ARG(ret)) {
    m_error_des->write(ret->get_string() + "\n");
  }
  delete ret;
  m_root->clear();
  return true;
}

};
