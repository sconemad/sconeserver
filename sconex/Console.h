/* SconeServer (http://www.sconemad.com)

Console descriptor(s)

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

#ifndef scxConsole_h
#define scxConsole_h

#include <sconex/sconex.h>
#include <sconex/Descriptor.h>
namespace scx {

//=============================================================================
class SCONEX_API Console : public Descriptor {

public:

  Console();
  virtual ~Console();

  virtual void close();

  virtual std::string describe() const;
  // Describe the socket
  
  virtual int fd();

protected:

  virtual Condition endpoint_read(void* buffer,int n,int& na);
  virtual Condition endpoint_write(const void* buffer,int n,int& na);

private:

};


};
#endif
