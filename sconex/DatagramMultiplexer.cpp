/* SconeServer (http://www.sconemad.com)

sconex Datagram Multiplexer

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

#include "sconex/DatagramMultiplexer.h"
#include "sconex/DatagramSocket.h"
#include "sconex/DatagramChannel.h"
#include "sconex/Kernel.h"
namespace scx {
 
//=============================================================================
DatagramMultiplexer::DatagramMultiplexer(
  Module& router, 
  const std::string& chain
)
  : Stream("datagram_mplex"),
    m_router(router),
    m_chain(chain)
{
  enable_event(Stream::Readable,true);
}
  
//=============================================================================
DatagramMultiplexer::~DatagramMultiplexer()
{

}

//=============================================================================
scx::Condition DatagramMultiplexer::event(scx::Stream::Event e)
{ 
  if (e == scx::Stream::Readable) {
    
    DatagramSocket* sock = dynamic_cast<DatagramSocket*>(&endpoint());

    DatagramChannel* channel = 0;
    SocketAddress* sa = 0;
    char* buffer = new char[65536];
    int na=0;
    sock->endpoint_readfrom(buffer,65536,na,sa);
    
    std::map<std::string,DatagramChannel*>::iterator iter = 
      m_channels.find(sa->get_string());
    if (iter!=m_channels.end()) {
      // Existing channel
      channel = (*iter).second;
      delete sa;
    } else {
      // New channel
      channel = new DatagramChannel(*sock,*this,sa);
      scx::ArgList* args = new scx::ArgList();
      args->give(new scx::ArgString(m_chain));

      if (m_router.connect(channel,args)) {
	m_channels[sa->get_string()] = channel;
	scx::Kernel::get()->connect(channel,args);
      } else {
	delete channel;
      }

      delete args;
    }
    
    channel->recv_datagram(buffer,na);
  }
  
  return scx::Ok;
}
  
//=============================================================================
std::string DatagramMultiplexer::stream_status() const
{
  std::ostringstream oss;
  oss << m_channels.size() << " active channels";
  return oss.str();
}

//=============================================================================
void DatagramMultiplexer::notify_close(const std::string& addr_remote)
{
  std::map<std::string,DatagramChannel*>::iterator iter = 
    m_channels.find(addr_remote);
  if (iter!=m_channels.end()) {
    m_channels.erase(iter);
  }
}


};
