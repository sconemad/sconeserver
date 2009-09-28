/* SconeServer (http://www.sconemad.com)

Sconex Configuration stream

Copyright (c) 2000-2005 Andrew Wedgbury <wedge@sconemad.com>

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

#include "ConfigStream.h"
#include "sconex.h"
#include "ArgProc.h"

namespace scx {

//=========================================================================
ConfigStream::ConfigStream(
  ModuleRef root_ref,
  bool shutdown_on_exit
)
  : LineBuffer("config"),
    m_proc(Auth::Admin),
    m_shutdown_on_exit(shutdown_on_exit)
{
  m_argmod = new ArgModule(root_ref);
  m_proc.set_ctx(m_argmod);
  enable_event(Stream::Readable,true);
}

//=========================================================================
ConfigStream::~ConfigStream()
{
  delete m_argmod;
}

//=========================================================================
Condition ConfigStream::event(Stream::Event e)
{
  switch (e) {
    
    case Stream::Opening: { // OPENING

    } break;
   
    case Stream::Closing: { // CLOSING
      if (m_shutdown_on_exit) {
	Stream::write("Foreground console exiting - shutting down\n");
	m_proc.evaluate("shutdown()");
      }
    } return Close;
    
    case Stream::Readable: { // READABLE
      std::string buffer;
      Condition c;
      
      while ( (c = tokenize(buffer)) == Ok) {
	
	if (buffer == "exit") {
	  return Close;
	}
	
	Arg* result = m_proc.evaluate(buffer);
	if (result) {
	  Stream::write(result->get_string());
	  Stream::write("\n");
	  delete result;
	} else {
	  Stream::write("NULL\n");
	}
      }
    } break;
    
    default:
      break;
    
  }

  return Ok;
}

};
