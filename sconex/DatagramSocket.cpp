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

#include "sconex/DatagramSocket.h"
namespace scx {

//=============================================================================
DatagramSocket::DatagramSocket(
  const SocketAddress* sockaddr
)
{
  DEBUG_COUNT_CONSTRUCTOR(DatagramSocket);
  m_addr_local = dynamic_cast<SocketAddress*>(sockaddr->new_copy());
  DEBUG_ASSERT(m_addr_local!=0,"ctor() Invalid SocketAddress");
}

//=============================================================================
DatagramSocket::~DatagramSocket()
{
  DEBUG_COUNT_DESTRUCTOR(DatagramSocket);
}

//=============================================================================
int DatagramSocket::listen()
{
  if (create_socket()) {
    return 1;
  }
  
  if (bind(m_addr_local)) {
    return 1;
  }

  m_state = Connected;
  return 0;
}

//=============================================================================
Condition DatagramSocket::endpoint_read(void* buffer,int n,int& na)
{
  DESCRIPTOR_DEBUG_LOG("endpoint_read() Not valid for this type of socket!");
  return scx::Error;
}

//=============================================================================
Condition DatagramSocket::endpoint_write(const void* buffer,int n,int& na)
{
  DESCRIPTOR_DEBUG_LOG("endpoint_write() Not valid for this type of socket!");
  return scx::Error;
}

};
