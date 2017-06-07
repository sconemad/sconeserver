/* SconeServer (http://www.sconemad.com)

WebSocket server support

Copyright (c) 2000-2017 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef httpWebSocket_h
#define httpWebSocket_h

#include <http/HTTPModule.h>
#include <http/Status.h>
#include <http/Response.h>
#include <http/Handler.h>
#include <sconex/Stream.h>
#include <sconex/Buffer.h>

namespace http {

//=========================================================================
class HTTP_API WebSocketHandler : public Handler {
public:

  WebSocketHandler(HTTPModule* module,
                   const std::string& chain);
  virtual ~WebSocketHandler() {}

  virtual scx::Condition handle_message(MessageStream* message);

private:

  HTTPModule::Ref m_module;
  std::string m_chain;
  
};
  
//=============================================================================
class HTTP_API WebSocketStream : public scx::Stream {
public:

  WebSocketStream(HTTPModule* module, MessageStream* message);
  virtual ~WebSocketStream();

  // scx::Stream interface:
  virtual scx::Condition read(void* buffer,int n,int& na);
  virtual scx::Condition write(const void* buffer,int n,int& na);
  virtual scx::Condition event(scx::Stream::Event e);
  virtual bool has_readable() const;
  virtual std::string stream_status() const;
 
private:

  scx::Condition process_handshake();
  scx::Condition read_frame();

  void handle_close(scx::Buffer& data);
  void handle_ping(scx::Buffer& data);
  void handle_pong(scx::Buffer& data);
  
  void build_frame(scx::BufferWriter& bw,
                   unsigned short opcode,
                   unsigned int length);
  
  HTTPModule::Ref m_module;
  MessageStream* m_message;

  enum ReadState { Header, Data };
  ReadState m_read_state;
  enum DataFormat { None, Text, Binary };
  DataFormat m_read_format;
  scx::Buffer m_read_buffer;
  unsigned long m_read_pos;
  unsigned long m_read_len;
  bool m_read_usemask;
  uint8_t m_read_mask[4];

  scx::Buffer m_write_buffer;
};

};
#endif
