/* SconeServer (http://www.sconemad.com)

Configuration stream

Reads lines from a stream, evaluating them as SconeScript expressions.

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

#ifndef scxConfigStream_h
#define scxConfigStream_h

#include "Module.h"
#include "Descriptor.h"
#include "LineBuffer.h"
#include "ArgProc.h"

namespace scx {

class ArgMod;

//=============================================================================
class SCONEX_API ConfigStream : public LineBuffer {
public:

  ConfigStream(
    ModuleRef root_ref,
    bool shutdown_on_exit = false
  );
  
  virtual ~ConfigStream();

  virtual Condition event(Stream::Event e);

private:

  ArgProc m_proc;
  ArgModule* m_argmod;
  bool m_shutdown_on_exit;
  
};

};
#endif

