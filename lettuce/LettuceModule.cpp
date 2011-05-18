/* SconeServer (http://www.sconemad.com)

Lettuce module

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

#include "LettuceModule.h"
#include "LettuceCommandStream.h"
#include "LettuceMediaStream.h"

#include <sconex/ModuleInterface.h>
#include <sconex/ScriptTypes.h>

SCONESERVER_MODULE(LettuceModule);

//=========================================================================
LettuceModule::LettuceModule(
) : scx::Module("lettuce",scx::version())
{
  scx::Stream::register_stream("lettuce-command",this);
  scx::Stream::register_stream("lettuce-media",this);
}

//=========================================================================
LettuceModule::~LettuceModule()
{
  scx::Stream::unregister_stream("lettuce-command",this);
  scx::Stream::unregister_stream("lettuce-media",this);
}

//=========================================================================
std::string LettuceModule::info() const
{
  return "Services for the Lettuce automation platform\n"
         "See http://sconemad.com/lettuce for more details";
}

//=========================================================================
void LettuceModule::provide(const std::string& type,
			    const scx::ScriptRef* args,
			    scx::Stream*& object)
{
  if (type == "lettuce-command") {
    object = new LettuceCommandStream(this);

  } else if (type == "lettuce-media") {
    object = new LettuceMediaStream(this);

  }
}
