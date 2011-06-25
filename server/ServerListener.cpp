/* SconeServer (http://www.sconemad.com)

Server connection listener

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

#include "ServerListener.h"
#include "ServerModule.h"

#include <sconex/Kernel.h> 
#include <sconex/StreamSocket.h>
#include <sconex/ListenerSocket.h>
#include <sconex/ScriptTypes.h>

//=============================================================================
ServerListener::ServerListener(ServerModule* module,
			       const std::string& chain
) : Stream("server"),
    m_module(module),
    m_chain(chain)
  
{
  enable_event(Stream::Readable,true);
}

//=============================================================================
ServerListener::~ServerListener()
{

}

//=============================================================================
scx::Condition ServerListener::event(scx::Stream::Event e)
{ 
  if (e == scx::Stream::Readable) {

    scx::ListenerSocket* stream_listener = 
      dynamic_cast<scx::ListenerSocket*>(&endpoint());
    if (stream_listener) {
      scx::StreamSocket* s = new scx::StreamSocket();
    
      // Accept the incoming connection
      if (stream_listener->accept(s) != 0) {
	// Client aborted before we got a chance to accept, oh well.
	delete s;
	return scx::Ok;
      }
    
      // Construct argument list with chain name
      scx::ScriptList::Ref args(new scx::ScriptList());
      args.object()->give(scx::ScriptString::new_ref(m_chain));
      
      // Pass to the server to connect any streams
      if (m_module.object()->connect(s,&args)) {
	
	// Socket connected succesfully, give to kernel
	scx::Kernel::get()->connect(s);
	
      } else {
	// Failed to chain connection, terminate it
	delete s;
      }
    
      return scx::Ok;
    }

    // Unknown socket type, shouldn't be used with this listener
    return scx::Error;
  }
  
  return scx::Ok;
}

//=============================================================================
std::string ServerListener::stream_status() const
{
  return m_chain;
}
