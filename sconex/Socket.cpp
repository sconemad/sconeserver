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

#include "sconex/Socket.h"
#include "sconex/Time.h"
namespace scx {

//=============================================================================
Socket::Socket()
  : m_socket(-1),
    m_addr_local(0)
{
  DEBUG_COUNT_CONSTRUCTOR(Socket);
}


//=============================================================================
Socket::~Socket()
{
  // Make sure the descriptor is closed
  if (state() != Closed) {
    close();
  }

  delete m_addr_local;
  DEBUG_COUNT_DESTRUCTOR(Socket);
}

//=============================================================================
void Socket::close()
//
// Disconnect the socket
//
{
  if (m_socket >= 0) {
    ::close(m_socket);
    m_socket = -1;
    m_state = Descriptor::Closed;
  }
}

//=============================================================================
std::string Socket::describe() const
{
  return Descriptor::describe() + " (" +
    (m_addr_local ? m_addr_local->get_string() : "") +
    ")";
}

//=============================================================================
int Socket::fd()
{
  return (int)m_socket;
}

//=============================================================================
void Socket::shutdown(ShutdownMethod m)
{
  DEBUG_ASSERT(0 == ::shutdown(m_socket,m),"Socket shutdown failed");
}

//=============================================================================
const SocketAddress* Socket::get_local_addr() const
{
  return m_addr_local;
}

//=============================================================================
int Socket::event_create()
{
  DEBUG_ASSERT(m_socket>=0,"event_create() invalid socket");

  // Set default socket options
  set_blocking(false);
  return 0;
}

//=============================================================================
int Socket::event_connecting()
{
  int so_err = 0;
  socklen_t so_len = sizeof(so_err);

  int err = ::getsockopt(m_socket,SOL_SOCKET,SO_ERROR,&so_err,&so_len);

  if (err != 0) {
    // Couldn't tell
    DESCRIPTOR_DEBUG_LOG("Unable to determine connection state, error " << err);
    return 1;
    
  } else if (so_err != 0) {
    // Connection failed
    DESCRIPTOR_DEBUG_LOG("Connection failed with socket error " << so_err);
    return 1;
  }    

  // Succesful connection
  m_state = Connected;

  // Determine local address if required
  if (m_addr_local) {
    socklen_t sa_size = m_addr_local->get_sockaddr_size();
    char* sa_local_buffer = new char[sa_size];
    memset(sa_local_buffer,0,sa_size);
    struct sockaddr* sa_local = (struct sockaddr*)sa_local_buffer;
    
    if (::getsockname(m_socket,sa_local,&sa_size) != 0) {
      DESCRIPTOR_DEBUG_LOG("event_connecting() Could not determine local address");
      delete [] sa_local_buffer;
      return 0;
    }
    
    m_addr_local->set_sockaddr(sa_local);
    delete [] sa_local_buffer;
  }

  return 0;
}

//=============================================================================
int Socket::create_socket()
{
  DEBUG_ASSERT(m_socket==-1,"create_socket() socket already in use");
  DEBUG_ASSERT(m_addr_local!=0,"create_socket() local address not set");

  // Create a new socket
  m_socket = m_addr_local->socket_create();

  if (m_socket == -1) {
    DESCRIPTOR_DEBUG_LOG("create_socket() could not create socket");
    return 1;
  }

  event_create();
  return 0;
}

//=============================================================================
int Socket::bind(
  const SocketAddress* addr
)
{
  if (addr==0 || !addr->valid_for_bind()) {
    DESCRIPTOR_DEBUG_LOG("bind() Invalid address");
    return 1;
  }
  
  int ier = addr->socket_bind(m_socket);
  
  if (ier < 0) {
    DESCRIPTOR_DEBUG_LOG("bind() could not bind to address, e=" << ier);
    return 1;
  }
  
  return 0;
}

};
