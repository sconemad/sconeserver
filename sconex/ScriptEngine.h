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

#ifndef scxScriptEngine_h
#define scxScriptEngine_h

#include <sconex/sconex.h>
#include <sconex/StreamTokenizer.h>
#include <sconex/ScriptExpr.h>
#include <sconex/ScriptStatement.h>
namespace scx {

//=============================================================================
// ScriptEngine - This is a stream object which parses SconeScript programs
// read from the data stream. Various common language constructs are supported
// such as conditionals and loops, using a syntax similar to C or JavaScript. 
// Statements are parsed into a tree of ScriptStatement-objects, which can 
// then be executed, see ScriptStatement for more details.
//
class SCONEX_API ScriptEngine : public StreamTokenizer {
public:

  // Create an ScriptEngine parser to parse statements into the specified root
  // statement (which should be a ScriptStatementGroup).
  ScriptEngine(ScriptStatement::Ref* root);

  // Destructor
  virtual ~ScriptEngine();

  // Handle event
  virtual Condition event(Stream::Event e);

  // Parse
  Condition parse();
  
  // Make a new ScriptStatement based on the current token
  ScriptStatement::Ref* parse_token(const std::string& token);

  // Access current error information
  enum ErrorType { None, Tokenization, Syntax, Underflow };
  ErrorType get_error_type() const;
  int get_error_line() const;

protected:

  ScriptEngine(const ScriptEngine& original);
  ScriptEngine& operator=(const ScriptEngine& rhs);
  // Prohibit copy

  // Called when there are statements to run
  // Return true to keep parsing, false to abort.
  // Default implementation returns true.
  virtual bool event_runnable();

  // Called when an error occurs
  // Return true to keep parsing, false to abort.
  // Default implementation returns false.
  virtual bool event_error();
  
  // Find the next token in the buffer
  // Note that the parsing mode can depend on the current statement
  // e.g. following an 'if' token we look for a conditional expression in
  // brackets, whereas normally we look for either reserved words or
  // semicolon-terminated expressions.
  virtual bool next_token(
    const Buffer& buffer,
    int& pre_skip,
    int& length,
    int& post_skip
  );

  // Root statement
  ScriptStatement::Ref* m_root;
  
  // Stack of statements being parsed
  // The root group is always the bottom statement, the top is the current
  // statement being parsed.
  std::stack<ScriptStatement::Ref*> m_stack;

  // Current error
  ErrorType m_error_type;

  // Initialise statics
  static void init();

  typedef std::map<std::string,int> TokenMap;
  static TokenMap* s_tokens;
  
};


//=============================================================================
// ScriptEngineExec - Script engine which automatically executes statements
// as they are parsed.
//
class SCONEX_API ScriptEngineExec : public ScriptEngine {

public:

  // Create a ScriptEngine parser to run in the specified context
  ScriptEngineExec(const ScriptAuth& auth,
		   ScriptRef* ctx,
		   const std::string& file);

  // Destructor
  virtual ~ScriptEngineExec();

  // Set descriptor for writing errors to
  void set_error_des(Descriptor* error_des);

protected:

  ScriptEngineExec(const ScriptEngineExec& original);
  ScriptEngineExec& operator=(const ScriptEngineExec& rhs);
  // Prohibit copy

  virtual bool event_runnable();
  virtual bool event_error();

  ScriptTracer m_tracer;
  ScriptRef* m_ctx;

  // Descriptor to write error output to
  Descriptor* m_error_des;

};


};
#endif
