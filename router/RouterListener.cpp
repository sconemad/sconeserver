/* SconeServer (http://www.sconemad.com)

Router connection listener

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

#include "RouterListener.h"
#include "RouterModule.h"

#include "sconex/Kernel.h" 
#include "sconex/StreamSocket.h"
#include "sconex/ListenerSocket.h"

//=============================================================================
RouterListener::RouterListener(
  const std::string& route,
  scx::Module& module
) : Stream("router"),
    m_route(route),
    m_module(module)
  
{
  enable_event(Stream::Readable,true);
}

//=============================================================================
RouterListener::~RouterListener()
{

}

//=============================================================================
scx::Condition RouterListener::event(scx::Stream::Event e)
{ 
  if (e == scx::Stream::Readable) {
    scx::StreamSocket* s = new scx::StreamSocket();
    
    // Accept the incoming connection
    
    if ( ((scx::ListenerSocket&)endpoint()).accept(s) != 0 ) {
      // Client aborted before we got a chance to accept, oh well.
      delete s;
      return scx::Ok;
    }
    
    // Construct argument list with route name
    scx::ArgList* args = new scx::ArgList();
    args->give(new scx::ArgString(m_route));
    
    // Pass to the router to connect any streams
    if (m_module.connect(s,args)) {
      
      // Socket routed succesfully, give to kernel
      scx::Kernel::get()->connect(s,args);
      
    } else {
      // Failed to route connection, terminate it
      delete s;
    }
    
    delete args;
  }
  
  return scx::Ok;
}

//=============================================================================
std::string RouterListener::stream_status() const
{
  return m_route;
}
