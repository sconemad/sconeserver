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

#ifndef ServerDatagramMultiplexer_h
#define ServerDatagramMultiplexer_h

#include "sconex/DatagramMultiplexer.h"

class ServerModule;

//=============================================================================
// ServerListener - This detects incoming connections on a listening 
// socket, accepts connections and uses the given connection chain to setup
// streams for the descriptor, before finally passing it to kernel to manage.
//
class ServerDatagramMultiplexer : public scx::DatagramMultiplexer {
public:

  ServerDatagramMultiplexer(ServerModule& module, 
			    const std::string& chain);

  virtual ~ServerDatagramMultiplexer();

  // DatagramMultiplexer methods
  virtual bool channel_open(scx::DatagramChannel* channel);
  virtual void channel_close(scx::DatagramChannel* channel);
  
private:

  ServerModule& m_module;
  std::string m_chain;

};

#endif
