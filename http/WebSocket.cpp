/* SconeServer (http://www.sconemad.com)

WebSocket server support

See RFC6455

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

#include <http/WebSocket.h>
#include <http/MessageStream.h>
#include <http/Request.h>

#include <sconex/sconex.h>
#include <sconex/Digest.h>
#include <sconex/Base64.h>
#include <sconex/Kernel.h>

#include <server/ServerModule.h>

#include <bitset>

namespace http {

// Uncomment to enable debug logging
//#define WEBSOCKET_DEBUG_LOG(m) STREAM_DEBUG_LOG(m)

#ifndef WEBSOCKET_DEBUG_LOG
#  define WEBSOCKET_DEBUG_LOG(m)
#endif

const char* WEBSOCKET_GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
const int WEBSOCKET_VERSION = 13;

#define OPCODE_CONT 0
#define OPCODE_TEXT 0x01
#define OPCODE_BINARY 0x02
#define OPCODE_CLOSE 0x08
#define OPCODE_PING 0x09
#define OPCODE_PONG 0x0A

//=========================================================================
WebSocketHandler::WebSocketHandler(HTTPModule* module,
                                   const std::string& chain)
  : m_module(module), m_chain(chain)
{
}

//=========================================================================
scx::Condition WebSocketHandler::handle_message(MessageStream* message)
{
  message->add_stream(new WebSocketStream(m_module.object(), message));

  // Lookup the server module
  scx::Module::Ref server = scx::Kernel::get()->get_module("server");
  if (!server.valid()) return scx::Close;

  scx::ScriptList::Ref args(new scx::ScriptList());
  args.object()->give(scx::ScriptString::new_ref(m_chain));

  ((ServerModule*)server.object())->connect(message->get_endpoint(),&args);
    
  return scx::Ok;
}
  
//=============================================================================
WebSocketStream::WebSocketStream(HTTPModule* module,
                                 MessageStream* message)
  : scx::Stream("http:websocket"),
    m_module(module),
    m_message(message),
    m_read_state(Header),
    m_read_buffer(140),
    m_read_pos(0),
    m_read_len(0),
    m_write_buffer(16)
{
}

//=============================================================================
WebSocketStream::~WebSocketStream()
{
}

//=============================================================================
scx::Condition WebSocketStream::read(void* buffer,int n,int& na)
{
  scx::Condition c = scx::Ok;
  na = 0;
  if (m_read_state == Header) {
    c = read_frame();
  }

  if (m_read_state == Data) {
    int rem = m_read_len - m_read_pos;
    if (n > rem) n = rem; 
    int a = m_read_buffer.used();
    if (a) {
      if (a > n) a = n;
      m_read_buffer.pop_to(buffer, a);
      na += a;
    }
    if (na < n) {
      int nr = 0;
      c = Stream::read((char*)buffer + na, n-na, nr);
      na += nr;
    }

    // Unmask
    if (m_read_usemask) {
      unsigned int ir = m_read_pos;
      for (int i=0; i<na; ++i, ++ir) {
        *((unsigned char*)buffer+i) ^= m_read_mask[ir%4];
      }
    }
    
    m_read_pos += na;
    if (m_read_pos == m_read_len) {
      m_read_state = Header;
      enable_event(scx::Stream::Readable,true);
    }
  }
  
  return c;
}

//=============================================================================
scx::Condition WebSocketStream::write(const void* buffer,int n,int& na)
{
  scx::Condition c = scx::Ok;
  na = 0;
  
  if (m_write_buffer.used()) {
    // Stuff still in the write buffer, try to write it
    c = Stream::write(m_write_buffer);
  }

  if (m_write_buffer.used() == 0) {
    // Write buffer is empty, start the next frame
    m_write_buffer.compact();
    scx::BufferWriter bw(m_write_buffer);

    build_frame(bw,OPCODE_TEXT,n);
    bw.done();
    c = Stream::write(m_write_buffer);
    
    if (m_write_buffer.used() == 0) {
      // Wrote the entire frame header, try to write the data
      c = Stream::write(buffer,n,na);
    }

    if (na < n) {
      // Couldn't write all of the data, buffer the rest
      int rem = n - na;
      m_write_buffer.resize(m_write_buffer.used() + rem);
      m_write_buffer.push_from((unsigned char*)buffer + na, rem);
      na += rem;
      enable_event(scx::Stream::Writeable,true);
    }
  }
  return c;
}

//=============================================================================
scx::Condition WebSocketStream::event(scx::Stream::Event e)
{
  scx::Condition c = scx::Ok;
  switch (e) {
    
    case scx::Stream::Opening: { // OPENING
      return process_handshake();
    } break;

    case scx::Stream::Closing: { // CLOSING
    } break;
    
    case scx::Stream::Readable: { // READABLE
      if (m_read_state == Header) {
        c = read_frame();
      }
    } break;

    case scx::Stream::Writeable: { // WRITEABLE
      if (m_write_buffer.used()) {
        c = Stream::write(m_write_buffer);
      }
      if (m_write_buffer.used() == 0) {
        enable_event(scx::Stream::Writeable,false);
      }
    } break;

    default:
      break;
  }
  
  return c;
}

//=============================================================================
bool WebSocketStream::has_readable() const
{
  return (m_read_state == Data &&
          m_read_buffer.used() > 0);
}
  
//=============================================================================
bool WebSocketStream::has_writeable() const
{
  return (m_write_buffer.used() > 0);
}
  
//=============================================================================
std::string WebSocketStream::stream_status() const
{
  std::ostringstream oss;
  oss << "rb:" << m_read_buffer.status_string()
      << " rf:" << m_read_pos << "/" << m_read_len
      << " (" << m_read_format << ")"
      << " wb:" << m_write_buffer.status_string();
  return oss.str();
}

//=============================================================================
scx::Condition WebSocketStream::process_handshake()
{
  const Request& req = m_message->get_request();
  Response& resp = m_message->get_response();
  
  if (req.get_header("Connection") != "Upgrade" &&
      req.get_header("Upgrade") != "websocket") {
    STREAM_DEBUG_LOG("Not a websocket request");
    resp.set_status(Status::BadRequest);
    return scx::Close;
  }

  std::string key = req.get_header("Sec-WebSocket-Key");
  std::string proto = req.get_header("Sec-WebSocket-Protocol");

  std::string ver = req.get_header("Sec-WebSocket-Version");
  int vn = atoi(ver.c_str());
  if (vn != WEBSOCKET_VERSION) {
    STREAM_DEBUG_LOG("Unsupported WebSocket-Version: " + ver);
    resp.set_status(Status::BadRequest);
    return scx::Close;
  }
  
  std::string ac = key + WEBSOCKET_GUID;
  scx::Digest* sha = scx::Digest::create("SHA1",0);
  if (!sha) {
    STREAM_DEBUG_LOG("SHA1 hashing not supported");
    resp.set_status(Status::NotImplemented);
    return scx::Close;
  }
  sha->update(ac.c_str(), ac.length());
  sha->finish();

  std::istringstream iss;
  iss.rdbuf()->pubsetbuf((char*)sha->get_digest().head(),
                         sha->get_digest().used());
  std::ostringstream oss;
  scx::Base64::encode(iss, oss, false);
  delete sha;

  resp.set_header("Sec-WebSocket-Accept", oss.str());
  resp.set_header("Connection", "Upgrade");
  resp.set_header("Upgrade", "websocket");
  resp.set_header("Sec-WebSocket-Protocol", proto);
  resp.set_status(Status::SwitchingProtocols);

  m_message->log("Upgrading HTTP connection to WebSocket");
  m_message->set_transparent();
  enable_event(scx::Stream::Readable,true);
  return scx::Ok;
}

//=============================================================================
scx::Condition WebSocketStream::read_frame()
{
  m_read_buffer.compact();
  scx::Condition c = Stream::read(m_read_buffer);
  if (c != scx::Ok) return c;

  try {
    scx::BufferReader br(m_read_buffer);
    
    uint8_t a = br.read_u8();
    bool fin = a & 0x80;
    unsigned short opcode = a & 0x0f;
    
    uint8_t b = br.read_u8();
    m_read_usemask = b & 0x80;
    m_read_len = b & 0x7f;
    m_read_pos = 0;
    
    if (m_read_len == 126) { // 16-bit length
      m_read_len = br.read_u16();
      
    } else if (m_read_len == 127) { // 64-bit length
      m_read_len = br.read_u64();
    }
    
    // Read masking key
    if (m_read_usemask) {
      for (int i=0; i<4; ++i) m_read_mask[i] = br.read_u8();
    }

    WEBSOCKET_DEBUG_LOG("FRAME fin:" << fin
                        << " op:" << opcode
                        << " usemask:" << m_read_usemask
                        << " len:" << m_read_len);

    if (opcode < 8) { // Data frame
      br.done();
      switch (opcode) {
        case OPCODE_CONT:
          if (m_read_format == None) c = scx::Error;
          break;
        case OPCODE_TEXT:
          m_read_format = Text;
          break;
        case OPCODE_BINARY:
          m_read_format = Binary;
          break;
        default:
          STREAM_DEBUG_LOG("Unsupported opcode " << opcode);
          c = scx::Error;
          break;
      }
      if (c == scx::Ok) {
        m_read_state = Data;
        enable_event(scx::Stream::Readable,false);
      }
      return c;
    }

    // Control frame
    if (!fin) return scx::Error;
    if (m_read_len > 125) return scx::Error;
    scx::Buffer data(128);
    if (m_read_len > 0) br.read_bytes((char*)data.tail(),m_read_len);
    data.push(m_read_len);
    br.done();
    
    // Unmask
    if (m_read_usemask) {
      for (unsigned int i=0; i<m_read_len; ++i) {
        *((unsigned char*)data.head()+i) ^= m_read_mask[i%4];
      }
    }

    switch (opcode) {
      case OPCODE_CLOSE: handle_close(data); break;
      case OPCODE_PING: handle_ping(data); break;
      case OPCODE_PONG: handle_pong(data); break;
      default:
        STREAM_DEBUG_LOG("Unsupported control opcode " << opcode);
        c = scx::Error;
        break;
    }
    
  } catch (scx::BufferReader::Underflow& e) {
    c = scx::Wait;
  }
  return c;
}

//=============================================================================
void WebSocketStream::handle_close(scx::Buffer& data)
{
  scx::BufferReader br(data);
  unsigned int code = br.read_u16();
  char msg[128];
  int msglen = data.used()-2;
  br.read_bytes(msg,msglen);
  br.done();
  msg[msglen]=0;
  WEBSOCKET_DEBUG_LOG("Close: " << code << " " << msg);

  // Place a close frame in the write buffer
  m_write_buffer.ensure_free(4+2+msglen);
  scx::BufferWriter bw(m_write_buffer);
  build_frame(bw,OPCODE_CLOSE,2+msglen);
  bw.write_u16(code);
  bw.write_bytes(msg,msglen);
  bw.done();
  enable_event(scx::Stream::Writeable,true);
}

//=============================================================================
void WebSocketStream::handle_ping(scx::Buffer& data)
{
  // Place a pong frame in the write buffer
  m_write_buffer.ensure_free(data.used()+4);
  scx::BufferWriter bw(m_write_buffer);
  build_frame(bw,OPCODE_PONG,data.used());
  bw.write_bytes((char*)data.head(),data.used());
  bw.done();
  enable_event(scx::Stream::Writeable,true);
}

//=============================================================================
void WebSocketStream::handle_pong(scx::Buffer& data)
{

}

//=============================================================================
void WebSocketStream::build_frame(scx::BufferWriter& bw,
                                  unsigned short opcode,
                                  unsigned int length)
{
  bw.write_u8(0x80 | opcode); // Don't support continuation or masking here
  if (length < 126) {
    bw.write_u8((uint8_t)length);
  } else if (length < 65536) {
    bw.write_u8(126);
    bw.write_u16((uint16_t)length);
  } else {
    bw.write_u8(127);
    bw.write_u64((uint64_t)length);
  }
}
  
};
