/* SconeServer (http://www.sconemad.com)

SconeX Listener socket

Copyright (c) 2000-2004 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef scxListenerSocket_h
#define scxListenerSocket_h

#include <sconex/sconex.h>
#include <sconex/Socket.h>
namespace scx {

class StreamSocket;

//=============================================================================
class SCONEX_API ListenerSocket : public Socket {

public:

  ListenerSocket(
    const SocketAddress* sockaddr,
    int backlog = 5
  );
  // Construct a listener socket for this address

  virtual ~ListenerSocket();

  int listen();
  // Start listening

  int accept(StreamSocket* s);
  // Accept a connection
  // s must be a valid new stream socket
  // Returns 0 to indicate success

protected:

  virtual Condition endpoint_read(void* buffer,int n,int& na);
  virtual Condition endpoint_write(const void* buffer,int n,int& na);
  // Not valid for listener sockets, so just assert

  int m_backlog;

private:

};

};
#endif
