/* SconeServer (http://www.sconemad.com)

Regular Expression

Copyright (c) 2000-2010 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef scxRegExp_h
#define scxRegExp_h

#include <sconex/sconex.h>
#include <sconex/ScriptBase.h>
namespace scx {

//===========================================================================
class SCONEX_API RegExp : public ScriptObject {

public:

  RegExp(const std::string& pattern);  
  RegExp(const ScriptRef* args);
  RegExp(const RegExp& c);
  virtual ~RegExp();
  
  ScriptObject* new_copy() const;

  virtual std::string get_string() const;
  virtual int get_int() const;
  
  virtual ScriptRef* script_op(const ScriptAuth& auth,
			       const ScriptRef& ref,
			       const ScriptOp& op,
			       const ScriptRef* right=0);

  virtual ScriptRef* script_method(const ScriptAuth& auth,
				   const ScriptRef& ref,
				   const std::string& name,
				   const ScriptRef* args);
  
  bool operator==(const RegExp& v) const;
  bool operator!=(const RegExp& v) const;

protected:

  void from_string(const std::string& str);
  
  std::string m_pattern;
  pcre* m_pcre;
};

};

#endif
