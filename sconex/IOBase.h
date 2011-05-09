/* SconeServer (http://www.sconemad.com)

IO Base class

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

#ifndef scxIOBase_h
#define scxIOBase_h

#include <sconex/sconex.h>
namespace scx {

enum Condition { Ok, Wait, End, Close, Error };

//=============================================================================
class SCONEX_API IOBase {

public:

  virtual Condition read(void* buffer,int n,int& na) =0;
  
  virtual Condition write(const void* buffer,int n,int& na) =0;
  virtual int write(const char* string) =0;
  virtual int write(const std::string& string) =0;

};

};
#endif
