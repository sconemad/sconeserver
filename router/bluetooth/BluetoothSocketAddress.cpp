/* SconeServer (http://www.sconemad.com)

Socket address for Bluetooth

Copyright (c) 2000-2005 Andrew Wedgbury <wedge@sconemad.com>

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

#include "BluetoothSocketAddress.h"
namespace scx {

//=============================================================================
BluetoothSocketAddress::BluetoothSocketAddress(scx::Arg* args)
  : scx::SocketAddress(PF_BLUETOOTH,SOCK_STREAM,BTPROTO_RFCOMM)
{
  DEBUG_COUNT_CONSTRUCTOR(BluetoothSocketAddress);
  memset(&m_addr,0,sizeof(m_addr));
  m_addr.rc_family = AF_BLUETOOTH;
  
  scx::ArgList* l = dynamic_cast<scx::ArgList*>(args);
  
  const scx::ArgString* a_addr =
    dynamic_cast<const scx::ArgString*>(l->get(0));
  if (a_addr) {
    set_address(a_addr->get_string());
  }

  const scx::ArgInt* a_channel =
    dynamic_cast<const scx::ArgInt*>(l->get(1));
  
  if (a_channel) {
    int ch = a_channel->get_int();
    if (ch > 0 && ch < 32768) {
      set_channel(ch);
    }
  }
  
}

//=============================================================================
BluetoothSocketAddress::BluetoothSocketAddress(const BluetoothSocketAddress& c)
  : scx::SocketAddress(c)
{
  DEBUG_COUNT_CONSTRUCTOR(BluetoothSocketAddress);
  memcpy(&m_addr,&c.m_addr,sizeof(m_addr));
  m_valid = c.m_valid;
}

//=============================================================================
BluetoothSocketAddress::~BluetoothSocketAddress()
{
  DEBUG_COUNT_DESTRUCTOR(BluetoothSocketAddress);
}

//=============================================================================
scx::Arg* BluetoothSocketAddress::new_copy() const
{
  return new BluetoothSocketAddress(*this);
}

//=============================================================================
bool BluetoothSocketAddress::valid_for_bind() const
{
  return m_valid;
}

//=============================================================================
bool BluetoothSocketAddress::valid_for_connect() const
{
  return m_valid;
}

//=============================================================================
void BluetoothSocketAddress::set_sockaddr(const struct sockaddr* sa)
{
  DEBUG_ASSERT(sa!=0,"set_sockaddr() Invalid sockaddr pointer");
  DEBUG_ASSERT(sa->sa_family == m_domain,"set_sockaddr() Socket domain mismatch");

  memcpy(&m_addr,sa,sizeof(m_addr));
}

//=============================================================================
const struct sockaddr* BluetoothSocketAddress::get_sockaddr() const
{
  return (const struct sockaddr*)&m_addr;
}

//=============================================================================
socklen_t BluetoothSocketAddress::get_sockaddr_size() const
{
  return sizeof(m_addr);
}

//=============================================================================
std::string BluetoothSocketAddress::get_string() const
{
  std::ostringstream oss;
  switch (m_type) {
    case SOCK_STREAM: oss << "rfcomm:"; break;
    default:
      oss << "unknown_rfcomm:";
      DEBUG_LOG("get_string() Unknown socket type");
      break;
  }
  
  oss << get_address() << ":" << (int)get_channel();
  
  return oss.str();
}

//=============================================================================
scx::Arg* BluetoothSocketAddress::op(
  scx::Arg::OpType optype,
  const std::string& opname,
  scx::Arg* right
)
{
  if (scx::Arg::Binary == optype && "." == opname) {
    std::string name = right->get_string();
    if (name == "family") return new scx::ArgString("bluetooth");
    if (name == "address") return new scx::ArgString(get_address());
    if (name == "channel") return new scx::ArgInt((int)get_channel());
  }
  return SCXBASE SocketAddress::op(optype,opname,right);
}

//=============================================================================
void BluetoothSocketAddress::set_address(
  const std::string& addr
)
{
  if (addr.length()==0) {
    // Set the address invalid
    bacpy(&m_addr.rc_bdaddr,BDADDR_ANY);
    m_valid = false;
    
  } else if (addr=="*") {
    // Wildcard address
    bacpy(&m_addr.rc_bdaddr,BDADDR_ANY);
    m_valid = true;
		
  } else {
    // bd address string
    str2ba(addr.c_str(),&m_addr.rc_bdaddr);
    m_valid = true;
  }
}

//=============================================================================
std::string BluetoothSocketAddress::get_address() const
{
  if (m_valid) {
    char str[20]; // Is this defined anywhere?
    ba2str(&m_addr.rc_bdaddr,str);
    return std::string(str);
  } 
  
  return std::string();
}

//=============================================================================
void BluetoothSocketAddress::set_channel(
  short channel
)
{
  //  m_addr.rc_channel = htobs(channel);
  m_addr.rc_channel = htobs(4);
}

//=============================================================================
short BluetoothSocketAddress::get_channel() const
{
  return btohs(m_addr.rc_channel);
}

};
