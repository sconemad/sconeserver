/* SconeServer (http://www.sconemad.com)

SconeX Datagram socket

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

#ifndef scxDatagramSocket_h
#define scxDatagramSocket_h

#include <sconex/sconex.h>
#include <sconex/Socket.h>
namespace scx {

//=============================================================================
class SCONEX_API DatagramSocket : public Socket {

public:

  DatagramSocket();
  // Construct a datagram socket

  virtual ~DatagramSocket();

  virtual std::string describe() const;

  int listen(const SocketAddress* addr_local);
  // Start listening

  Condition connect(const SocketAddress* addr_remote);
  // Connect this socket 

protected:

  virtual Condition endpoint_read(void* buffer,int n,int& na);
  virtual Condition endpoint_write(const void* buffer,int n,int& na);

  virtual Condition endpoint_readfrom(void* buffer,int n,int& na,
				      SocketAddress*& sa);
  virtual Condition endpoint_writeto(const void* buffer,int n,int& na,
				     const SocketAddress& sa);

  SocketAddress* peek_address();

  friend class DatagramMultiplexer;
  friend class DatagramChannel;

private:

};

};
#endif
