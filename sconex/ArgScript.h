/* SconeServer (http://www.sconemad.com)

Arg Script parser and engine

This is a stream object which parses and runs entine SconeScript programs
read from the data stream. Various common language constructs are supported
such as conditionals and loops, using a 'C'-like syntax. Statements are parsed
into a tree of ArgStatement-objects which is then evaluated, see ArgStatement
for more details.

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

#ifndef scxArgScript_h
#define scxArgScript_h

#include "sconex/sconex.h"
#include "sconex/StreamTokenizer.h"
#include "sconex/ArgProc.h"
namespace scx {

class ArgObject;
class ArgStatement;
class ArgStatementGroup;
  
//=============================================================================
class SCONEX_API ArgScript : public StreamTokenizer {

public:

  ArgScript(
    const Auth& auth,
    ArgObject* ctx
  );
  // Create an ArgScript parser to run in the specified context

  virtual ~ArgScript();
  // Destructor

  virtual Condition event(Stream::Event e);
  // Handle event

  ArgStatement* parse_token(const std::string& token);
  // Make a new ArgStatement based on the current token

  void set_error_des(Descriptor* error_des);
  
protected:

  virtual bool next_token(
    const Buffer& buffer,
    int& pre_skip,
    int& length,
    int& post_skip
  );
  // Find the next token in the buffer
  // Note that the parsing mode can depend on the current statement
  // e.g. following an 'if' token we look for a conditional expression in
  // brackets, whereas normally we look for either reserved words or
  // semicolon-terminated expressions.

private:

  ArgProc m_proc;
  ArgObject* m_ctx;

  ArgStatementGroup* m_root;
  // Root statement group
  
  std::stack<ArgStatement*> m_stack;
  // Stack of statements being parsed
  // The root group is always the bottom statement, the top is the current
  // statement being parsed.

  Descriptor* m_error_des;
  // Descriptor to write error output to
  
};

};
#endif
