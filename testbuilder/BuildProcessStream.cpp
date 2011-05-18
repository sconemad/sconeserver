/* SconeServer (http://www.sconemad.com)

Build process observer stream

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

#include "BuildProcessStream.h"
#include "BuildStep.h"
#include <sconex/Process.h>

//=========================================================================
BuildProcessStream::BuildProcessStream(TestBuilderModule* module,
				       BuildStep* step)
  : scx::Stream("testbuilder:process"),
    m_module(module),
    m_step(step)
{

}

//=========================================================================
BuildProcessStream::~BuildProcessStream()
{

}

//=========================================================================
void BuildProcessStream::cancel()
{
  scx::Process* proc = dynamic_cast<scx::Process*>(&endpoint());
  proc->kill();
  m_step = 0;
}

//=========================================================================
scx::Condition BuildProcessStream::event(scx::Stream::Event e)
{
  if (e == scx::Stream::Closing) {
    scx::Process* proc = dynamic_cast<scx::Process*>(&endpoint());
    int code = 0;
    if (proc->get_exitcode(code)) {
      if (m_step) m_step->process_exited(code);
      return scx::Ok;
    } else {
      return scx::Wait;
    }
  }
  
  return scx::Ok;
}
