/* SconeServer (http://www.sconemad.com)

sconex Datagram Multiplexer

Copyright (c) 2000-2011 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef scxDatagramMultiplexer_h
#define scxDatagramMultiplexer_h

#include "sconex/sconex.h"
#include "sconex/Stream.h"
namespace scx {

class DatagramSocket;
class DatagramChannel;
class Module;

//=============================================================================
// DatagramMultiplexer - Multiplexes a single datagram socket into multiple
// virtual 'channels', each representing a connection to an  individual remote
// socket.
//
class DatagramMultiplexer : public Stream {
public:
  
  DatagramMultiplexer();
  virtual ~DatagramMultiplexer();
  
  virtual scx::Condition event(scx::Stream::Event e);
  virtual std::string stream_status() const;

protected:

  // Called when a new channel is opened
  // Return true to accept, false to reject (default is to accept)
  virtual bool channel_open(DatagramChannel* channel);

  // Called when a channel is closed
  virtual void channel_close(DatagramChannel* channel);

  // Notification from DatagramChannel
  void notify_channel_closing(DatagramChannel* channel);

  friend class DatagramChannel;

private:

  typedef HASH_TYPE<std::string,DatagramChannel*> DatagramChannelMap;
  DatagramChannelMap m_channels;

};


};
#endif
