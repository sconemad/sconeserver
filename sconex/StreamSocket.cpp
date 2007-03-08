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

#include "sconex/StreamSocket.h"
namespace scx {

// Uncomment to enable debug logging
//#define STREAMSOCKET_DEBUG_LOG(m) DESCRIPTOR_DEBUG_LOG(m)

#ifndef STREAMSOCKET_DEBUG_LOG
#  define STREAMSOCKET_DEBUG_LOG(m)
#endif
 
//=============================================================================
StreamSocket::StreamSocket()
  : m_addr_remote(0)
{
  DEBUG_COUNT_CONSTRUCTOR(StreamSocket);
}

//=============================================================================
StreamSocket::~StreamSocket()
{
  delete m_addr_remote;
  DEBUG_COUNT_DESTRUCTOR(StreamSocket);
}

//=============================================================================
std::string StreamSocket::describe() const
{
  return Socket::describe() + " --> (" +
    (m_addr_remote!=0 ? m_addr_remote->get_string() : "") +
    ")";
}

//=============================================================================
void StreamSocket::pair(
  StreamSocket*& a,
  StreamSocket*& b,
  const std::string& name_a,
  const std::string& name_b
)
{
  DEBUG_ASSERT(a==0,"pair() socket a already exists");
  DEBUG_ASSERT(b==0,"pair() socket b already exists");
  
  int d[2];
  if (socketpair(PF_UNIX,SOCK_STREAM,0,d) < 0) {
    DEBUG_LOG("pair() socketpair failed");
    return;
  }
  
  a = new StreamSocket();
  a->m_state = Socket::Connected;
  a->m_socket = d[0];
  a->m_addr_local = new AnonSocketAddress(name_a);
  a->m_addr_remote = new AnonSocketAddress(name_b);
  a->set_blocking(false);
    
  b = new StreamSocket();
  b->m_state = Socket::Connected;
  b->m_socket = d[1];
  b->set_blocking(true);
}

//=============================================================================
Condition StreamSocket::connect(
  const SocketAddress* addr
)
{
  DEBUG_ASSERT(state()==Descriptor::Closed,"connect() called on open socket");

  if (addr==0 || !addr->valid_for_connect()) {
    if (addr) {
      STREAMSOCKET_DEBUG_LOG("connect() address (" <<
                             addr->get_string() << ") invalid for connect");
    } else {
      STREAMSOCKET_DEBUG_LOG("connect() null address!");
    }
    return scx::Error;
  }

  // Copy address into our local and remote addresses, local address will be
  // determined on connection, but we want to ensure we have an address object
  // of the correct class.
  if (addr != m_addr_local) {
    delete m_addr_local;
    m_addr_local = dynamic_cast<SocketAddress*> (addr->new_copy());
    DEBUG_ASSERT(m_addr_local!=0,"connect() SocketAddress cast failed");
  }
  if (addr != m_addr_remote) {
    delete m_addr_remote;
    m_addr_remote = dynamic_cast<SocketAddress*> (addr->new_copy());
    DEBUG_ASSERT(m_addr_remote!=0,"connect() SocketAddress cast failed");
  }
  
  if (create_socket()) {
    STREAMSOCKET_DEBUG_LOG("connect() failed to create socket");
    return scx::Error;
  }

  const struct sockaddr* sa = addr->get_sockaddr();
  socklen_t sa_size = addr->get_sockaddr_size();

  // Call connect
  if ( ::connect(m_socket,sa,sa_size) < 0) {
    switch (error()) {
    
      case Descriptor::Wait:
        // Non-blocking connect
        STREAMSOCKET_DEBUG_LOG("connect() waiting for connection");
        m_state=Descriptor::Connecting;
        return scx::Ok;

      default:
        STREAMSOCKET_DEBUG_LOG("connect() could not connect, error=" << (int)errno);
        close();
        return scx::Error;
    } 
  }

  // Connection occured straight away, probably a blocking connect
  STREAMSOCKET_DEBUG_LOG("connect() connected straight away");

  return (event_connecting() == 0) ? scx::Ok : scx::Error;
}

//=============================================================================
const SocketAddress* StreamSocket::get_remote_addr() const
{
  return m_addr_remote;
}



//=============================================================================
Condition StreamSocket::endpoint_read(
  void* buffer,
  int n,
  int& na
)
{
  if (state()!=Descriptor::Connected &&
      state()!=Descriptor::Closing) {
    STREAMSOCKET_DEBUG_LOG("read() attempted on closed socket");
    return scx::Error;
  }

  na = recv(m_socket,(char*)buffer,n,0);

  if (na > 0) {
    // Some or all requested bytes were read
    return scx::Ok;

  } else if (na == 0) {
    // Client closed the connection gracefully
    m_state = Socket::Closing;
    STREAMSOCKET_DEBUG_LOG("READ closing");
    return scx::End;

  } else if (error() == Descriptor::Wait) {
    // No data available right now, but not an error as such
    na = 0;
    return scx::Wait;
  }

  // Fatal error occured,
  na = 0;
  STREAMSOCKET_DEBUG_LOG("read() error: " << error());
  STREAMSOCKET_DEBUG_LOG("error code: " << errno);
  m_state = Socket::Closed;
  return scx::Error;
}

//=============================================================================
Condition StreamSocket::endpoint_write(
  const void* buffer,
  int n,
  int& na
)
{
  if (state()!=Descriptor::Connected &&
      state()!=Descriptor::Closing) {
    STREAMSOCKET_DEBUG_LOG("write() attempted on closed socket");
    return scx::Error;
  }

  na = send(m_socket,(char*)buffer,n,0);

  if (na > 0) {
    // Sent some or all of the data ok
    return scx::Ok;

  } else if (error() == Descriptor::Wait) {
    // Cannot send right now
    na=0;
    return scx::Wait;
  }

  // Fatal error occured
  na = 0;
  STREAMSOCKET_DEBUG_LOG("write() error: " << error());
  m_state = Socket::Closed;
  return scx::Error;
}

//=============================================================================
int StreamSocket::accept(
  SOCKET sock,
  SocketAddress* addr_remote
)	
{
  if (sock<=0) {
    // Invalid socket
    m_socket = -1;
    m_state = Descriptor::Closed;
    delete addr_remote;
    STREAMSOCKET_DEBUG_LOG("set_socket() passed a null socket");
    return 1;
  }

  m_socket = sock;

  delete m_addr_remote;
  m_addr_remote = addr_remote;

  event_create();

  // Determine local address
  socklen_t sa_size = m_addr_remote->get_sockaddr_size();
  char* sa_local_buffer = new char[sa_size];
  memset(sa_local_buffer,0,sa_size);
  struct sockaddr* sa_local = (struct sockaddr*)sa_local_buffer;
  
  if (::getsockname(m_socket,sa_local,&sa_size) != 0) {
    STREAMSOCKET_DEBUG_LOG("accept() Could not determine local address");
    delete [] sa_local_buffer;
    return 1;
  }

  delete m_addr_local;
  m_addr_local = dynamic_cast<SocketAddress*> (m_addr_remote->new_copy());
  DEBUG_ASSERT(m_addr_local!=0,"accept() SocketAddress cast failed");
  m_addr_local->set_sockaddr(sa_local);
  delete [] sa_local_buffer;
  
  // Set flags
  m_state = Descriptor::Connected;
  
  return 0;
}

};
