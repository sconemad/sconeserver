/* SconeServer (http://www.sconemad.com)

Connection forwarding module

Copyright (c) 2000-2007 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef forwardModule_h
#define forwardModule_h

#include "sconex/Module.h"
#include "sconex/Stream.h"

//=============================================================================
// ForwardModule - A module to forward connections on to another host
//
class ForwardModule : public scx::Module,
                      public scx::Provider<scx::Stream> {
public:

  ForwardModule();
  virtual ~ForwardModule();

  virtual std::string info() const;
  
  // Provider<scx::Stream> method
  virtual void provide(const std::string& type,
		       const scx::ScriptRef* args,
		       scx::Stream*& object);
protected:

private:

};

#endif

