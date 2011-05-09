/* SconeServer (http://www.sconemad.com)

Sconex socket address

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

#include <sconex/SocketAddress.h>
#include <sconex/ScriptTypes.h>
namespace scx {

//=============================================================================
SocketAddress::SocketAddress(
  int domain,
  int type,
  int protocol
) : m_domain(domain),
    m_type(type),
    m_protocol(protocol)
{
  DEBUG_COUNT_CONSTRUCTOR(SocketAddress);
}

//=============================================================================
SocketAddress::SocketAddress(const SocketAddress& c)
  : ScriptObject(c),
    m_domain(c.m_domain),
    m_type(c.m_type),
    m_protocol(c.m_protocol)
{
  DEBUG_COUNT_CONSTRUCTOR(SocketAddress);
}
  
//=============================================================================
SocketAddress::~SocketAddress()
{
  DEBUG_COUNT_DESTRUCTOR(SocketAddress);
}

//=============================================================================
bool SocketAddress::valid_for_bind() const
{
  return true;
}
  
//=============================================================================
bool SocketAddress::valid_for_connect() const
{
  return true;
}

//=============================================================================
int SocketAddress::socket_domain() const
{
  return m_domain;
}

//=============================================================================
int SocketAddress::socket_type() const
{
  return m_type;
}

//=============================================================================
int SocketAddress::socket_protocol() const
{
  return m_protocol;
}

//=============================================================================
SOCKET SocketAddress::socket_create() const
{
  return ::socket(
    m_domain,
    m_type,
    m_protocol
  );
}

//=============================================================================
int SocketAddress::socket_bind(SOCKET s) const
{
  return ::bind(
    s,
    get_sockaddr(),
    get_sockaddr_size()
  );
}

//=============================================================================
int SocketAddress::get_int() const
{
  return (valid_for_bind() || valid_for_connect());
}

//=============================================================================
ScriptRef* SocketAddress::script_op(const ScriptAuth& auth,
				    const ScriptRef& ref,
				    const ScriptOp& op,
				    ScriptRef* right)
{
  if (ScriptOp::Lookup == op.type()) {
    std::string name = right->object()->get_string();
    if (name == "type") {
      switch (m_type) {
      case SOCK_STREAM: return ScriptString::new_ref("stream"); 
      case SOCK_DGRAM: return ScriptString::new_ref("datagram");
      case SOCK_RAW: return ScriptString::new_ref("raw");
      default: return ScriptString::new_ref("unknown");
      }
    }
  }

  return ScriptObject::script_op(auth,ref,op,right);
}


//=============================================================================
AnonSocketAddress::AnonSocketAddress(const std::string& name)
  : SocketAddress(0,0,0),
    m_name(name)
{
  DEBUG_COUNT_CONSTRUCTOR(AnonSocketAddress);
}

//=============================================================================
AnonSocketAddress::AnonSocketAddress(const AnonSocketAddress& c)
  : SocketAddress(c),
    m_name(c.m_name)
{
  DEBUG_COUNT_CONSTRUCTOR(AnonSocketAddress);
}

//=============================================================================
AnonSocketAddress::~AnonSocketAddress()
{
  DEBUG_COUNT_DESTRUCTOR(AnonSocketAddress);
}

//=============================================================================
ScriptObject* AnonSocketAddress::new_copy() const
{
  return new AnonSocketAddress(*this);
}

//=============================================================================
bool AnonSocketAddress::valid_for_bind() const
{
  return false;
}

//=============================================================================
bool AnonSocketAddress::valid_for_connect() const
{
  return false;
}

//=============================================================================
void AnonSocketAddress::set_sockaddr(const struct sockaddr* sa)
{
  DEBUG_LOG("Method not implemented for this address type");
}

//=============================================================================
const struct sockaddr* AnonSocketAddress::get_sockaddr() const
{
  DEBUG_LOG("Method not implemented for this address type");
  return 0;
}

//=============================================================================
socklen_t AnonSocketAddress::get_sockaddr_size() const
{
  DEBUG_LOG("Method not implemented for this address type");
  return 0;
}

//=============================================================================
std::string AnonSocketAddress::get_string() const
{
  return m_name;
}

};
