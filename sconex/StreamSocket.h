/* SconeServer (http://www.sconemad.com)

SconeX TCP network stream socket

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

#ifndef scxStreamSocket_h
#define scxStreamSocket_h

#include "sconex/sconex.h"
#include "sconex/Socket.h"
#include "sconex/Stream.h"
namespace scx {

class ListenerSocket;

//=============================================================================
class SCONEX_API StreamSocket : public Socket {

public:

  StreamSocket();
  virtual ~StreamSocket();

  virtual std::string describe() const;
  
  static void pair(
    StreamSocket*& a,
    StreamSocket*& b,
    const std::string& name_a = "",
    const std::string& name_b = ""
  );
  // Create a pair of linked UNIX domain sockets (using socketpair)
  
  Condition connect(const SocketAddress* addr);
  // Connect to address

  const SocketAddress* get_remote_addr() const;
  // Get the remote address

protected:

  virtual Condition endpoint_read(void* buffer,int n,int& na);
  virtual Condition endpoint_write(const void* buffer,int n,int& na);
  // Socket I/O

  friend class ListenerSocket;
  
  virtual int accept(
    SOCKET sock,
    SocketAddress* addr_remote
  );

  SocketAddress* m_addr_remote;

private:

};

};
#endif
