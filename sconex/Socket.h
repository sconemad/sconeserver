/* SconeServer (http://www.sconemad.com)

SconeX base TCP/IP network socket

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

#ifndef scxSocket_h
#define scxSocket_h

#include "sconex/sconex.h"
#include "sconex/Descriptor.h"
#include "sconex/SocketAddress.h"
namespace scx {

//=============================================================================
class SCONEX_API Socket : public Descriptor {

public:

  Socket();
  virtual ~Socket();

  virtual void close();
  // Close the socket

  virtual std::string describe() const;
  // Describe the socket

  virtual int fd();

  enum ShutdownMethod { ShutdownRead, ShutdownWrite, ShutdownBoth };
  void shutdown(ShutdownMethod m);
  
  const SocketAddress* get_local_addr() const;
  // Get the local address

protected:

  virtual int event_create();
  // This fires when a socket gets created

  virtual int event_connecting();
  // This fires when a socket is connecting
  
  int create_socket();
  // Create a socket

  int bind(const SocketAddress* addr);
  // Bind to address

  SOCKET m_socket;

  SocketAddress* m_addr_local;

private:

};

};
#endif
