/* SconeServer (http://www.sconemad.com)

sconex Datagram channel

Copyright (c) 2000-2007 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef scxDatagramChannel_h
#define scxDatagramChannel_h

#include "sconex/sconex.h"
#include "sconex/Descriptor.h"
namespace scx {

class DatagramSocket;
class DatagramMultiplexer;
class SocketAddress;

//=============================================================================
class SCONEX_API DatagramChannel : public Descriptor {
public:

  DatagramChannel(
    DatagramSocket& socket,
    DatagramMultiplexer& multiplexer,
    SocketAddress* remote_addr);

  virtual ~DatagramChannel();

  std::string describe() const;
  
  Condition endpoint_read(void* buffer,int n,int& na);
  Condition endpoint_write(const void* buffer,int n,int& na);
  
  virtual void close(); 
  virtual int fd();

  const SocketAddress* get_remote_addr() const;
  // Get the remote address
  
protected:

  friend class DatagramMultiplexer;

  void recv_datagram(void* buffer,int n);

private:

  DatagramSocket& m_master;
  DatagramMultiplexer& m_multiplexer;
  SocketAddress* m_addr_remote;
  char* m_buffer;
  int m_buffer_size;
};


};
#endif
