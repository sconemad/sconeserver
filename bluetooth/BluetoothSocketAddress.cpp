/* SconeServer (http://www.sconemad.com)

Socket address for Bluetooth

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

#include "BluetoothSocketAddress.h"
#include <sconex/ScriptTypes.h>

//=============================================================================
BluetoothSocketAddress::BluetoothSocketAddress(scx::Module* module,
					       const scx::ScriptRef* args)
  : scx::SocketAddress(PF_BLUETOOTH,SOCK_STREAM,BTPROTO_RFCOMM),
    m_module(module)
{
  DEBUG_COUNT_CONSTRUCTOR(BluetoothSocketAddress);
  memset(&m_addr,0,sizeof(m_addr));
  m_addr.rc_family = AF_BLUETOOTH;
  
  const scx::ScriptString* a_addr =
    scx::get_method_arg<scx::ScriptString>(args,0,"address");
  if (a_addr) {
    set_address(a_addr->get_string());
  }

  const scx::ScriptInt* a_channel =
    scx::get_method_arg<scx::ScriptInt>(args,1,"channel");
  
  if (a_channel) {
    int ch = a_channel->get_int();
    if (ch > 0 && ch < 32768) {
      set_channel(ch);
    }
  }
  
}

//=============================================================================
BluetoothSocketAddress::BluetoothSocketAddress(const BluetoothSocketAddress& c)
  : scx::SocketAddress(c),
    m_module(c.m_module),
    m_valid(c.m_valid)
{
  DEBUG_COUNT_CONSTRUCTOR(BluetoothSocketAddress);
  memcpy(&m_addr,&c.m_addr,sizeof(m_addr));
}

//=============================================================================
BluetoothSocketAddress::~BluetoothSocketAddress()
{
  DEBUG_COUNT_DESTRUCTOR(BluetoothSocketAddress);
}

//=============================================================================
scx::ScriptObject* BluetoothSocketAddress::new_copy() const
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
  DEBUG_ASSERT(sa!=0,"Invalid sockaddr pointer");
  DEBUG_ASSERT(sa->sa_family == m_domain,"Socket domain mismatch");

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
scx::ScriptRef* BluetoothSocketAddress::script_op(const scx::ScriptAuth& auth,
						  const scx::ScriptRef& ref,
						  const scx::ScriptOp& op,
						  const scx::ScriptRef* right)
{
  if (op.type() == scx::ScriptOp::Lookup) {
    const std::string name = right->object()->get_string();

    if (name == "family") 
      return scx::ScriptString::new_ref("bluetooth");
    if (name == "address") 
      return scx::ScriptString::new_ref(get_address());
    if (name == "channel") 
      return scx::ScriptInt::new_ref((int)get_channel());
  }
  return scx::SocketAddress::script_op(auth,ref,op,right);
}

//=============================================================================
void BluetoothSocketAddress::set_address(
  const std::string& addr
)
{
  // N.B. Using BDADDR_ANY results in "taking address of temporary"
  // compiler errors. We can squash the errors using -fpermissive, but that
  // would also mask other, unrelated problems; hence this ugly workaround.
  const bdaddr_t workaround_bdaddr_any = {0, 0, 0, 0, 0, 0};
  if (addr.length()==0) {
    // Set the address invalid
    bacpy(&m_addr.rc_bdaddr,&workaround_bdaddr_any);
    m_valid = false;
    
  } else if (addr=="*") {
    // Wildcard address
    bacpy(&m_addr.rc_bdaddr,&workaround_bdaddr_any);
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
  m_addr.rc_channel = htobs(channel);
}

//=============================================================================
short BluetoothSocketAddress::get_channel() const
{
  return btohs(m_addr.rc_channel);
}
