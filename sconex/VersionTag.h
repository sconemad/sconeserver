/* SconeServer (http://www.sconemad.com)

Version tag "major.minor.sub" form

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

#ifndef scxVersionTag_h
#define scxVersionTag_h

#include <sconex/sconex.h>
#include <sconex/ScriptBase.h>
namespace scx {

//===========================================================================
class SCONEX_API VersionTag : public ScriptObject {

public:

  VersionTag(
    int major = -1,
    int minor = -1,
    int sub = -1,
    const std::string& extra = ""
  );  
  VersionTag(const std::string& str);
  VersionTag(const ScriptRef* args);

  VersionTag(const VersionTag& c);
  virtual ~VersionTag();
  
  virtual ScriptObject* new_copy() const;
  
  int get_major() const;
  int get_minor() const;
  int get_sub() const;
  const std::string& get_extra() const;

  virtual std::string get_string() const;
  virtual int get_int() const;
  
  virtual ScriptRef* script_op(const ScriptAuth& auth,
			       const ScriptRef& ref,
			       const ScriptOp& op,
			       const ScriptRef* right=0);
  
  VersionTag& operator=(const VersionTag& v);

  bool operator==(const VersionTag& v) const;
  bool operator!=(const VersionTag& v) const;
  bool operator>(const VersionTag& v) const;
  bool operator<(const VersionTag& v) const;
  bool operator>=(const VersionTag& v) const;
  bool operator<=(const VersionTag& v) const;

protected:
  
  void from_string(const std::string& str);
  
  int m_major;
  int m_minor;
  int m_sub;
  std::string m_extra;
};

};

#endif
