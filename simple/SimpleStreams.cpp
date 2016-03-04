/* SconeServer (http://www.sconemad.com)

Simple streams

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

#include "SimpleStreams.h"
#include <sconex/Stream.h>

//=========================================================================
EchoStream::EchoStream(scx::Module* module, int buffer_size)
  : scx::Stream("echo"),
    m_module(module),
    m_buffer(buffer_size),
    m_read(0)
{
  enable_event(scx::Stream::Readable,true);
}

//=========================================================================
EchoStream::~EchoStream()
{

}

//=========================================================================
scx::Condition EchoStream::event(scx::Stream::Event e)
{
  scx::Condition c = scx::Ok;
  int p;
  switch (e) {

    case scx::Stream::Opening:
      endpoint().set_timeout(scx::Time(60));
      break;

    case scx::Stream::Closing:
      if (m_buffer.used() > 0) return scx::Wait;
      break;

    case scx::Stream::Readable:
      p = m_buffer.used();
      c = Stream::read(m_buffer);
      m_read += (m_buffer.used() - p);
      // Fall through to write...
    case scx::Stream::Writeable:
      if (m_buffer.used() > 0) c = Stream::write(m_buffer);

      enable_event(scx::Stream::Readable, m_buffer.free() > 0);
      enable_event(scx::Stream::Writeable, m_buffer.used() > 0);
      break;
  }
  if (c == scx::Ok) endpoint().reset_timeout();
  return c;
}

//=============================================================================
std::string EchoStream::stream_status() const
{
  std::ostringstream oss;
  oss << "r:" << m_read
      << " b:" << m_buffer.status_string();
  return oss.str();
}


//=========================================================================
DiscardStream::DiscardStream(scx::Module* module)
  : scx::Stream("discard"),
    m_module(module)
{
  enable_event(scx::Stream::Readable,true);
}

//=========================================================================
DiscardStream::~DiscardStream()
{
}

//=========================================================================
scx::Condition DiscardStream::event(scx::Stream::Event e)
{
  if (e == scx::Stream::Opening) {
    endpoint().set_timeout(scx::Time(60));
  }

  if (e == scx::Stream::Readable) {
    endpoint().reset_timeout();

    char b;
    int na;
    scx::Condition c = Stream::read(&b,1,na);
    if (c == scx::End) {
      return scx::Close;
    }
    return c;
  }

  return scx::Ok;
}

std::string ChargenStream::s_chargen_str("!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~ ");

//=========================================================================
ChargenStream::ChargenStream(scx::Module* module)
  : scx::Stream("chargen"),
    m_module(module),
    m_x(0), m_s(0)
{
  enable_event(scx::Stream::Readable,true);
  enable_event(scx::Stream::Writeable,true);
}

//=========================================================================
ChargenStream::~ChargenStream()
{

}

//=========================================================================
scx::Condition ChargenStream::event(scx::Stream::Event e)
{
  if (e == scx::Stream::Opening) {
    endpoint().set_timeout(scx::Time(60));
  }

  if (e == scx::Stream::Readable) {
    endpoint().reset_timeout();

    char b;
    int na;
    scx::Condition c = Stream::read(&b,1,na);
    if (c == scx::End) {
      return scx::Close;
    }
    return c;
  }

  if (e == scx::Stream::Writeable) {
    char b;
    if (m_x < 72) {
      int p = m_s + m_x;
      if (p >= 95) p -= 95;
      b = s_chargen_str[p];
    } else if (m_x == 72) {
      b = '\r';
    } else if (m_x == 73) {
      b = '\n';
    }

    int na;
    scx::Condition c = Stream::write(&b,1,na);
    if (c == scx::End) {
      return scx::Close;
    } else if (na == 1) {
      if (m_x == 73) {
        m_x = -1;
        ++m_s;
        if (m_s == 95) {
          m_s = 0;
        }
      }
      ++m_x;
    }
    return c;
  }
  
  return scx::Ok;
}
