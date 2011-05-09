/* SconeServer (http://www.sconemad.com)

Server datagram multiplexer

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

#include "ServerDatagramMultiplexer.h"
#include "ServerModule.h"

#include <sconex/Kernel.h> 
#include <sconex/DatagramChannel.h>
#include <sconex/ScriptTypes.h>

//=============================================================================
ServerDatagramMultiplexer::ServerDatagramMultiplexer(ServerModule& module,
						     const std::string& chain
) : scx::DatagramMultiplexer(),
    m_module(module),
    m_chain(chain)
  
{

}

//=============================================================================
ServerDatagramMultiplexer::~ServerDatagramMultiplexer()
{

}

//=============================================================================
bool ServerDatagramMultiplexer::channel_open(scx::DatagramChannel* channel)
{
  scx::ScriptList::Ref args(new scx::ScriptList());
  args.object()->give(scx::ScriptString::new_ref(m_chain));

  // Pass to the server to connect any streams
  return m_module.connect(channel,&args);
}

//=============================================================================
void ServerDatagramMultiplexer::channel_close(scx::DatagramChannel* channel)
{

}





