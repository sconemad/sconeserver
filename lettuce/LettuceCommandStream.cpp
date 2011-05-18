/* SconeServer (http://www.sconemad.com)

Lettuce command stream

Copyright (c) 2000-2009 Andrew Wedgbury <wedge@sconemad.com>

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

#include "LettuceCommandStream.h"
#include "LettuceModule.h"

#include <sconex/DatagramChannel.h>
#include <sconex/SocketAddress.h>
#include <sconex/Stream.h>
#include <sconex/ScriptTypes.h>

//=========================================================================
LettuceBuffer::LettuceBuffer(
) : m_type(LettuceBufferEmpty),
    m_size(0)
{

}

//=========================================================================
LettuceBuffer::~LettuceBuffer()
{

}

//=========================================================================
scx::Condition LettuceBuffer::read(scx::Stream& stream)
{
  int na = 0;
  scx::Condition c;

  // Read the type byte
  unsigned char b_type = 0;
  c = stream.read(&b_type,1,na);
  if (c != scx::Ok) {
    return c;
  }
  if (na != 1) {
    return scx::Error;
  }
  m_type = (Type)b_type;

  // Read the size byte
  unsigned char b_size = 0;
  c = stream.read(&b_size,1,na);
  if (c != scx::Ok) {
    return c;
  }
  if (na != 1) {
    return scx::Error;
  }
  if (b_size > 128) {
    return scx::Error;
  }
  m_size = (int)b_size;

  // Read the data
  c = stream.read(m_buffer,m_size,na);
  if (c != scx::Ok) {
    return c;
  }
  if (na != m_size) {
    return scx::Error;
  }

  // Ok
  return c;
}

//=========================================================================
scx::Condition LettuceBuffer::write(scx::Stream& stream)
{
  int na = 0;
  scx::Condition c;

  // Write the type byte
  unsigned char b_type = (unsigned char)m_type;
  c = stream.write(&b_type,1,na);
  if (c != scx::Ok) {
    return c;
  }
  if (na != 1) {
    return scx::Error;
  }

  // Write the size byte
  unsigned char b_size = (unsigned char)m_size;
  c = stream.write(&b_size,1,na);
  if (c != scx::Ok) {
    return c;
  }
  if (na != 1) {
    return scx::Error;
  }

  // Write the data (if there is any)
  if (m_size > 0) {
    c = stream.write(m_buffer,m_size,na);
    if (c != scx::Ok) {
      return c;
    }
    if (na != m_size) {
      return scx::Error;
    }
  }

  // Ok
  return c;
}

//=========================================================================
void LettuceBuffer::set_type(LettuceBuffer::Type type)
{
  m_type = type;
}

//=========================================================================
LettuceBuffer::Type LettuceBuffer::get_type()
{
  return m_type;
}

//=========================================================================
scx::ScriptObject* LettuceBuffer::new_obj()
{
  scx::ScriptObject* a = 0;

  switch (m_type) {

    case LettuceBufferEmpty:
      break;
      
    case LettuceBufferBool:
    case LettuceBufferUInt:
      {
        switch (m_size) {
          case 1:
            {
              unsigned char b = m_buffer[0];
              a = new scx::ScriptInt(b);
            }
            break;
            
          case 2:
            {
              unsigned short int w;
              memcpy(&w,m_buffer,2);
              a = new scx::ScriptInt(w);
            }
            break;
            
          case 4:
            {
              unsigned int l;
              memcpy(&l,m_buffer,4);
              a = new scx::ScriptInt(l);
            }
            break;
        }
      }
      break;
      
    case LettuceBufferString:
      m_buffer[m_size] = '\0';
      a = new scx::ScriptString(m_buffer);
      break;
      
    case LettuceBufferBinary:
    case LettuceBufferIPAddr:
      break;
  }
  
  return a;
}


//=========================================================================
LettuceCommandStream::LettuceCommandStream(LettuceModule* module)
  : Stream("lettuce-cmd"),
    m_module(module)
{
  enable_event(scx::Stream::Readable,true);
}

//=========================================================================
LettuceCommandStream::~LettuceCommandStream()
{

}
  
//=========================================================================
scx::Condition LettuceCommandStream::event(scx::Stream::Event e) 
{
  if (e == scx::Stream::Opening) {
    m_module.object()->log("Opening lettuce command stream");
    endpoint().set_timeout(scx::Time(125));
  } 

  if (e == scx::Stream::Readable) {
    int na=0;
    scx::Condition c;

    char prefix;
    c = read(&prefix,1,na);
    
    char cmd;
    c = read(&cmd,1,na);

    LettuceBuffer name_buffer;
    c = name_buffer.read(*this);
    scx::ScriptObject* a_name = name_buffer.new_obj();
    scx::ScriptInt* a_int_name = dynamic_cast<scx::ScriptInt*>(a_name);
    
    LettuceBuffer data_buffer;
    c = data_buffer.read(*this);
    scx::ScriptObject* a_data = data_buffer.new_obj();

    const scx::DatagramChannel* sock =
      dynamic_cast<const scx::DatagramChannel*>(&endpoint());
    const scx::SocketAddress* addr = sock->get_remote_addr();
  
    // log
    std::ostringstream oss;
    oss << addr->get_string() << " - ";
    if (a_int_name) {
      oss << std::hex << a_int_name->get_int();
    } else {
      oss << a_name->get_string();
    }
    oss << ": " << a_data->get_string();
    m_module.object()->log(oss.str());
    
    endpoint().reset_timeout();

    delete a_name;
    delete a_data;
  }
    
  return scx::Ok;
}
