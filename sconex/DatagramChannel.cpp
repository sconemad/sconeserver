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

#include <sconex/DatagramChannel.h>
#include <sconex/DatagramSocket.h>
#include <sconex/DatagramMultiplexer.h>
#include <sconex/SocketAddress.h>
namespace scx {

//=============================================================================
DatagramChannel::DatagramChannel(
  DatagramSocket& socket,
  DatagramMultiplexer& multiplexer,
  SocketAddress* remote_addr
)
  : m_master(socket),
    m_multiplexer(multiplexer),
    m_addr_remote(remote_addr),
    m_buffer(0),
    m_buffer_size(0)
{
  m_state = Descriptor::Connected;
  set_timeout(Time(15));
  m_virtual_events = (1<<Stream::Writeable);
}

//=============================================================================
DatagramChannel::~DatagramChannel()
{
  m_multiplexer.notify_channel_closing(this);
  delete m_addr_remote;
  delete[] m_buffer;
}

//=============================================================================
std::string DatagramChannel::describe() const
{
  std::ostringstream oss;
  oss << Descriptor::describe()
      << " Datagram channel (" << m_master.get_local_addr()->get_string() << ")"
      << " <-- (" << (m_addr_remote!=0 ? m_addr_remote->get_string() : "")
      << ")";
  return oss.str();
}

//=============================================================================
Condition DatagramChannel::endpoint_read(void* buffer,int n,int& na)
{
  if (m_buffer) {
    na = std::min(n,m_buffer_size);
    memcpy(buffer,m_buffer,na);
    delete[] m_buffer;
    m_buffer = 0;
    m_buffer_size = 0;
    m_virtual_events &= ~(1<<Stream::Readable);
    return scx::Ok;
  }
  
  return scx::Wait;
}

//=============================================================================
Condition DatagramChannel::endpoint_write(const void* buffer,int n,int& na)
{
  return m_master.endpoint_writeto(buffer,n,na,*m_addr_remote);
}

//=============================================================================
void DatagramChannel::close() 
{ 
  
}
  
//=============================================================================
int DatagramChannel::fd() 
{ 
  return -1; 
}

//=============================================================================
const SocketAddress* DatagramChannel::get_remote_addr() const
{
  return m_addr_remote;
}

//=============================================================================
void DatagramChannel::recv_datagram(void* buffer,int n)
{
  if (m_buffer) {
    delete[] m_buffer;
    m_buffer = 0;
    m_buffer_size = 0;
  }
  m_buffer = (char*)buffer;
  m_buffer_size = n;
  m_virtual_events |= (1<<Stream::Readable);
}


};
