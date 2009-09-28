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
 
//=============================================================================
ArgScript::ArgScript(
  const Auth& auth, 
  ArgObject* ctx
)
  : StreamTokenizer("ArgScript",4096),
    m_proc(auth,ctx),
    m_ctx(ctx),
    m_error_des(0)
{
  DEBUG_COUNT_CONSTRUCTOR(ArgScript);

  // Make an enclosing group and push onto parse stack
  m_root = new ArgStatementGroup();
  m_root->set_parent(m_ctx->get_object());
  m_stack.push(m_root);
  
  enable_event(Stream::Readable,true);
}

//=============================================================================
ArgScript::~ArgScript()
{
  delete m_ctx;
  delete m_root;

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
        Arg* ret = m_root->execute(m_proc);
        if (ret && m_error_des) {
	  m_error_des->write(ret->get_string() + "\n");
        }
        delete ret;
        m_root->clear();
      }
    }

    return c;
  }

  return Ok;
}

//=============================================================================
ArgStatement* ArgScript::parse_token(const std::string& token)
{
  ArgStatement* s=0;
  
  if (token == "{") {
    ArgScript_DEBUG_LOG("parse_token: New statement group");
    s = new ArgStatementGroup();
    
  } else if (token == "if") {
    ArgScript_DEBUG_LOG("parse_token: New conditional");
    s = new ArgStatementConditional();

  } else if (token == "while") {
    ArgScript_DEBUG_LOG("parse_token: New while loop");
    s = new ArgStatementWhile();

  } else if (token == "for") {
    ArgScript_DEBUG_LOG("parse_token: New for loop");
    s = new ArgStatementFor();

  } else if (token == "return") {
    ArgScript_DEBUG_LOG("parse_token: New flow return");
    s = new ArgStatementFlow(ArgStatement::Return);

  } else if (token == "break") {
    ArgScript_DEBUG_LOG("parse_token: New flow last");
    s = new ArgStatementFlow(ArgStatement::Last);

  } else if (token == "continue") {
    ArgScript_DEBUG_LOG("parse_token: New flow next");
    s = new ArgStatementFlow(ArgStatement::Next);

  } else if (token == "var") {
    ArgScript_DEBUG_LOG("parse_token: New variable declaration");
    s = new ArgStatementVar();

  } else if (token == "sub") {
    ArgScript_DEBUG_LOG("parse_token: New subroutine declaration");
    s = new ArgStatementSub();
    
  } else {
    ArgScript_DEBUG_LOG("parse_token: New expression");
    s = new ArgStatementExpr(token);
    // (don't need to stack plain expressions)
    return s;
  }

  m_stack.push(s);
  return s;
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
        if (str == "if" ||
            str == "else" ||
            str == "while" ||
            str == "for" ||
            str == "return" ||
            str == "break" ||
            str == "continue" ||
            str == "var" ||
            str == "sub" ||
            str == "{" ||
            str == "}") {
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
void ArgScript::set_error_des(Descriptor* error_des)
{
  m_error_des = error_des;
}


};
