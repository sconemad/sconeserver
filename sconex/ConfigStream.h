/* SconeServer (http://www.sconemad.com)

Configuration stream

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

#ifndef scxConfigStream_h
#define scxConfigStream_h

#include "Module.h"
#include "Descriptor.h"
#include "LineBuffer.h"
#include "ScriptExpr.h"

namespace scx {

//=============================================================================
// Reads lines from a stream, evaluating them as SconeScript expressions.
//
class SCONEX_API ConfigStream : public LineBuffer {
public:

  ConfigStream(ScriptRef* ctx, bool shutdown_on_exit = false);
  
  virtual ~ConfigStream();

  virtual Condition event(Stream::Event e);

  bool write_object(const ScriptRef* object, int indent=0);

private:

  ScriptExpr m_proc;
  ScriptRef* m_ctx;
  bool m_shutdown_on_exit;

  enum OutputMode {
    None = 0,
    Formatted,
    Serialized
  };

  OutputMode m_output_mode;

};

};
#endif

