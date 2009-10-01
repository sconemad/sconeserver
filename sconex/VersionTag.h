/* SconeServer (http://www.sconemad.com)

Version tag "major.minor.sub" form

Copyright (c) 2000-2004 Andrew Wedgbury <wedge@sconemad.com>

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

#include "sconex/sconex.h"
#include "sconex/Arg.h"
namespace scx {

//===========================================================================
class SCONEX_API VersionTag : public Arg {

public:

  VersionTag(
    int major = -1,
    int minor = -1,
    int sub = -1,
    const std::string& extra = ""
  );  
  VersionTag(const std::string& str);
  VersionTag(Arg* args);

  VersionTag(const VersionTag& c);
  VersionTag(RefType ref, VersionTag& c);
  virtual ~VersionTag();
  
  Arg* new_copy() const;
  Arg* ref_copy(RefType ref);

  int get_major() const;
  int get_minor() const;
  int get_sub() const;
  const std::string& get_extra() const;

  virtual std::string get_string() const;
  virtual int get_int() const;
  
  virtual Arg* op(const Auth& auth, OpType optype, const std::string& opname, Arg* right);
  
  VersionTag& operator=(const VersionTag& v);

  bool operator==(const VersionTag& v) const;
  bool operator!=(const VersionTag& v) const;
  bool operator>(const VersionTag& v) const;
  bool operator<(const VersionTag& v) const;
  bool operator>=(const VersionTag& v) const;
  bool operator<=(const VersionTag& v) const;

protected:
  
  void from_string(const std::string& str);
  
  int* m_major;
  int* m_minor;
  int* m_sub;
  std::string* m_extra;
};

};

#endif
