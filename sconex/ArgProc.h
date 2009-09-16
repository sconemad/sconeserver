/* SconeServer (http://www.sconemad.com)

Argument processor

This is the SconeScript expression evaluator. It parses and evaluates
single line SconeScript expressions given as a string, dynamically creating
the required Arg objects and returning the calculated result.

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

#ifndef ArgProc_h
#define ArgProc_h

#include "sconex/sconex.h"
#include "sconex/Arg.h"
namespace scx {

//=============================================================================
class SCONEX_API ArgProc {

public:

  ArgProc(Arg* ctx=0);
  ArgProc(const ArgProc& c);
  ~ArgProc();

  Arg* evaluate(const std::string& expr);

  void set_ctx(Arg* ctx);

protected:

  Arg* expression(int p, bool f, bool exec=true);
  Arg* primary(bool f, bool exec=true);
  void next();
  int prec(Arg::OpType optype, const std::string& op) const;

  int m_pos;
  std::string m_expr;
  // Expression being evaluated

  enum TokenType { Null, Operator, Name, Value };
  TokenType m_type;
  std::string m_name;
  Arg* m_value;
  // Current token data

  Arg* m_ctx;
  // Current context

  std::stack<Arg*> m_stack;
  // Stack

  static void init();

  static std::map<std::string,int>* s_binary_ops;
  static std::map<std::string,int>* s_prefix_ops;
  static std::map<std::string,int>* s_postfix_ops;
};

};
#endif
