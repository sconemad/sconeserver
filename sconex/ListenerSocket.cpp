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

#include "sconex/ListenerSocket.h"
#include "sconex/StreamSocket.h"
namespace scx {

//=============================================================================
ListenerSocket::ListenerSocket(
  const SocketAddress* sockaddr,
  int backlog
) : m_backlog(backlog)
{
  DEBUG_COUNT_CONSTRUCTOR(ListenerSocket);
  m_addr_local = dynamic_cast<SocketAddress*> (sockaddr->new_copy());
  DEBUG_ASSERT(m_addr_local!=0,"ctor() SocketAddress cast failed");
}

//=============================================================================
ListenerSocket::~ListenerSocket()
{
  DEBUG_COUNT_DESTRUCTOR(ListenerSocket);
}

//=============================================================================
int ListenerSocket::listen()
{
  // Check socket is not already connected
  if (state() != Descriptor::Closed) {
    DESCRIPTOR_DEBUG_LOG("listen() socket in use - closing");
    close();
  }

  // Create a new socket
  if (create_socket() != 0) {
    return 1;
  }

  // Set reuse addr
  int optval=1;
  setsockopt(m_socket,SOL_SOCKET,SO_REUSEADDR,(const char*)&optval,sizeof(optval));

  // Bind socket to the interface and port
  if (bind(m_addr_local) != 0) {
    DESCRIPTOR_DEBUG_LOG("listen() could not bind listener socket");
    close();
    return 1;
  }

  // Start listening
  if ( ::listen(m_socket,m_backlog) < 0) {
    DESCRIPTOR_DEBUG_LOG("listen() socket listen failed");
    close();
    return 1;
  }

  m_state = Descriptor::Listening;

  return 0;
}

//=============================================================================
int ListenerSocket::accept(StreamSocket* s)
{
  DEBUG_ASSERT(m_socket!=0,"accept() invalid socket");

  socklen_t sa_size = m_addr_local->get_sockaddr_size();
  char* sa_remote_buffer = new char[sa_size];
  memset(sa_remote_buffer,0,sa_size);
  struct sockaddr* sa_remote = (struct sockaddr*)sa_remote_buffer;
  
  SOCKET socket = ::accept(m_socket,sa_remote,&sa_size);

  if (socket < 0) {
    delete [] sa_remote_buffer;
    return 1;
  }

  SocketAddress* addr_remote =
    dynamic_cast<SocketAddress*> (m_addr_local->new_copy());
  DEBUG_ASSERT(addr_remote!=0,"accept() SocketAddress cast failed");
  addr_remote->set_sockaddr(sa_remote);
  delete [] sa_remote_buffer;

  s->accept(socket,addr_remote);

  return 0;
}

//=============================================================================
Condition ListenerSocket::endpoint_read(void* buffer,int n,int& na)
{
  DESCRIPTOR_DEBUG_LOG("endpoint_read() Not valid for this type of socket!");
  return scx::Error;
}

//=============================================================================
Condition ListenerSocket::endpoint_write(const void* buffer,int n,int& na)
{
  DESCRIPTOR_DEBUG_LOG("endpoint_write() Not valid for this type of socket!");
  return scx::Error;
}

};
