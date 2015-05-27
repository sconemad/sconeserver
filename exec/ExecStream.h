/* SconeServer (http://www.sconemad.com)

External program execution Stream

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

#ifndef execStream_h
#define execStream_h

#include "ExecModule.h"
#include <sconex/Stream.h>
#include <sconex/Module.h>
#include <sconex/ScriptTypes.h>

namespace scx { class Process; }

//=========================================================================
class ExecStream : public scx::Stream {
public:

  ExecStream(ExecModule* module,
	     scx::ScriptList* args);

  ~ExecStream();
  
protected:

  ExecStream(const ExecStream& c);
  ExecStream& operator=(const ExecStream& v);
  // Prohibit copy

  virtual scx::Condition event(scx::Stream::Event e);

  virtual std::string stream_status() const;
  
  bool spawn_process();
  
private:

  scx::ScriptRefTo<ExecModule> m_module;

  scx::ScriptList* m_args;
  scx::Process* m_process;

  bool m_cgi_mode;
  int m_launched;
};

#endif
