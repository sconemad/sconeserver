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

#include <sconex/DatagramSocket.h>
#include <sconex/DatagramMultiplexer.h>
#include <sconex/Stream.h>
#include <sconex/Kernel.h>
namespace scx {

// Uncomment to enable debug logging
//#define DATAGRAMSOCKET_DEBUG_LOG(m) DESCRIPTOR_DEBUG_LOG(m)

#ifndef DATAGRAMSOCKET_DEBUG_LOG
#  define DATAGRAMSOCKET_DEBUG_LOG(m)
#endif


//=============================================================================
DatagramSocket::DatagramSocket()
{
  DEBUG_COUNT_CONSTRUCTOR(DatagramSocket);
}

//=============================================================================
DatagramSocket::~DatagramSocket()
{
  DEBUG_COUNT_DESTRUCTOR(DatagramSocket);
}

//=============================================================================
std::string DatagramSocket::describe() const
{
  return Socket::describe();
}

//=============================================================================
int DatagramSocket::listen(
  const SocketAddress* addr_local
)
{
  m_addr_local = dynamic_cast<SocketAddress*>(addr_local->new_copy());
  DEBUG_ASSERT(m_addr_local!=0,"ctor() Invalid SocketAddress");

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
Condition DatagramSocket::connect(
  const SocketAddress* addr_remote
)
{
  m_addr_local = dynamic_cast<SocketAddress*>(addr_remote->new_copy());
  DEBUG_ASSERT(m_addr_local!=0,"ctor() Invalid SocketAddress");

  if (m_state == Connected) {
    return scx::Error;
  }

  if (create_socket()) {
    return scx::Error;
  }

  const struct sockaddr* sa = addr_remote->get_sockaddr();
  socklen_t sa_size = addr_remote->get_sockaddr_size();

  if ( ::connect(m_socket,sa,sa_size) < 0) {
    DATAGRAMSOCKET_DEBUG_LOG("Connect failed");
    return scx::Error;
  }

  return (event_connecting() == 0) ? scx::Ok : scx::Error;
}

//=============================================================================
Condition DatagramSocket::endpoint_read(
  void* buffer,
  int n,
  int& na
)
{
  if (state()!=Descriptor::Connected &&
      state()!=Descriptor::Closing) {
    DATAGRAMSOCKET_DEBUG_LOG("read() attempted on closed socket");
    return scx::Error;
  }

  na = recv(m_socket,(char*)buffer,n,0);

  if (na >= 0) {
    // Some or all requested bytes were read
    return scx::Ok;

  } else if (error() == Descriptor::Wait) {
    // No data available right now, but not an error as such
    na = 0;
    return scx::Wait;
  }

  // Fatal error occured,
  na = 0;
  DATAGRAMSOCKET_DEBUG_LOG("read() error: " << error() << ", errno:" << errno);
  m_state = Socket::Closed;
  return scx::Error;
}

//=============================================================================
Condition DatagramSocket::endpoint_write(
  const void* buffer,
  int n,
  int& na
)
{
  if (state()!=Descriptor::Connected &&
      state()!=Descriptor::Closing) {
    DATAGRAMSOCKET_DEBUG_LOG("write() attempted on closed socket");
    return scx::Error;
  }

  na = send(m_socket,(char*)buffer,n,0);

  if (na >= 0 || n==na) {
    // Sent some or all of the data ok
    return scx::Ok;

  } else if (error() == Descriptor::Wait) {
    // Cannot send right now
    na=0;
    return scx::Wait;
  }

  // Fatal error occured
  na = 0;
  DATAGRAMSOCKET_DEBUG_LOG("write() error: " << error() << ", errno:" << errno);
  m_state = Socket::Closed;
  return scx::Error;
}

//=============================================================================
Condition DatagramSocket::endpoint_readfrom(void* buffer,int n,int& na,
					    SocketAddress*& sa)
{
  if (!m_addr_local) return scx::Error;
  sa = dynamic_cast<SocketAddress*>(m_addr_local->new_copy());

  socklen_t sa_size = sa->get_sockaddr_size();
  char* sa_local_buffer = new char[sa_size];
  memset(sa_local_buffer,0,sa_size);
  struct sockaddr* sa_local = (struct sockaddr*)sa_local_buffer;

  na = recvfrom(m_socket,(char*)buffer,n,0,sa_local,&sa_size);

  sa->set_sockaddr(sa_local);
  delete [] sa_local_buffer;

  if (na >= 0) {
    // Some or all requested bytes were read
    return scx::Ok;

  } else if (error() == Descriptor::Wait) {
    // No data available right now, but not an error as such
    na = 0;
    return scx::Wait;
  }

  // Fatal error occured,
  na = 0;
  m_state = Socket::Closed;
  return scx::Error;
}

//=============================================================================
Condition DatagramSocket::endpoint_writeto(const void* buffer,int n,int& na,
					   const SocketAddress& sa)
{
  na = sendto(m_socket,(char*)buffer,n,0,
	      sa.get_sockaddr(),
	      sa.get_sockaddr_size());

  if (na >= 0) {
    // Sent some or all of the data ok
    return scx::Ok;

  } else if (error() == Descriptor::Wait) {
    // Cannot send right now
    na=0;
    return scx::Wait;
  }

  // Fatal error occured
  na = 0;
  m_state = Socket::Closed;
  return scx::Error;
}

//=============================================================================
SocketAddress* DatagramSocket::peek_address()
{
  if (!m_addr_local) return 0;
  SocketAddress* sa = dynamic_cast<SocketAddress*>(m_addr_local->new_copy());

  socklen_t sa_size = sa->get_sockaddr_size();
  char* sa_local_buffer = new char[sa_size];
  memset(sa_local_buffer,0,sa_size);
  struct sockaddr* sa_local = (struct sockaddr*)sa_local_buffer;

  char buffer[1];
  int na=0;
  na = recvfrom(m_socket,(char*)buffer,0,MSG_PEEK,sa_local,&sa_size);

  sa->set_sockaddr(sa_local);
  delete [] sa_local_buffer;

  return sa;
}


};
