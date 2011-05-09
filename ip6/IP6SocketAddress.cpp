/* SconeServer (http://www.sconemad.com)

Socket address for TCP/IP version 6 (host:port)

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

#include "IP6SocketAddress.h"
#include <sconex/ScriptTypes.h>

//=============================================================================
IP6SocketAddress::IP6SocketAddress(const scx::ScriptRef* args)
  : scx::SocketAddress(PF_INET6,SOCK_STREAM)
{
  DEBUG_COUNT_CONSTRUCTOR(IP6SocketAddress);
  memset(&m_addr,0,sizeof(m_addr));
  m_addr.sin6_family = AF_INET6;
  
  const scx::ScriptString* a_host =
    scx::get_method_arg<scx::ScriptString>(args,0,"host");
  if (a_host) {
    set_address(a_host->get_string());
  }

  // Could be string or numeric
  const scx::ScriptInt* a_service_num =
    scx::get_method_arg<scx::ScriptInt>(args,1,"port");
  const scx::ScriptString* a_service_str =
    scx::get_method_arg<scx::ScriptString>(args,1,"service");
  
  if (a_service_num) {
    int p = a_service_num->get_int();
    if (p > 0 && p < 32768) {
      set_port(p);
    }

  } else if (a_service_str) {
    set_port(a_service_str->get_string());
    
  }
  
}

//=============================================================================
IP6SocketAddress::IP6SocketAddress(const IP6SocketAddress& c)
  : scx::SocketAddress(c),
    m_host(c.m_host),
    m_service(c.m_service),
    m_valid(c.m_valid)
{
  DEBUG_COUNT_CONSTRUCTOR(IP6SocketAddress);
  memcpy(&m_addr,&c.m_addr,sizeof(m_addr));
}

//=============================================================================
IP6SocketAddress::~IP6SocketAddress()
{
  DEBUG_COUNT_DESTRUCTOR(IP6SocketAddress);
}

//=============================================================================
scx::ScriptObject* IP6SocketAddress::new_copy() const
{
  return new IP6SocketAddress(*this);
}

//=============================================================================
bool IP6SocketAddress::valid_for_bind() const
{
  return m_valid && (get_port() > 0);
}

//=============================================================================
bool IP6SocketAddress::valid_for_connect() const
{
  return m_valid;
}

//=============================================================================
void IP6SocketAddress::set_sockaddr(const struct sockaddr* sa)
{
  DEBUG_ASSERT(sa!=0,"Invalid sockaddr pointer");
  DEBUG_ASSERT(sa->sa_family == m_domain,"Socket domain mismatch");

  memcpy(&m_addr,sa,sizeof(m_addr));
  m_host = "";
  m_service = "";
}

//=============================================================================
const struct sockaddr* IP6SocketAddress::get_sockaddr() const
{
  return (const struct sockaddr*)&m_addr;
}

//=============================================================================
socklen_t IP6SocketAddress::get_sockaddr_size() const
{
  return sizeof(m_addr);
}

//=============================================================================
std::string IP6SocketAddress::get_string() const
{
  std::ostringstream oss;
  switch (m_type) {
    case SOCK_STREAM: oss << "tcp6:"; break;
    case SOCK_DGRAM: oss << "udp6:"; break;
    default:
      oss << "unknown_ip6:";
      DEBUG_LOG("get_string() Unknown socket type");
      break;
  }
  
  if (!m_host.empty()) {
    // This commented out line results in the host being resolved automatically,
    // which isn't always the right thing to do.
    //    std::string host = get_host();
    oss << m_host;
  } else {
    oss << get_address();
  }
  oss << ":";

  std::string service = get_service();
  oss << service;
  if (service.empty()) oss << (int)get_port();
  
  return oss.str();
}

//=============================================================================
scx::ScriptRef* IP6SocketAddress::script_op(const scx::ScriptAuth& auth,
					    const scx::ScriptRef& ref,
					    const scx::ScriptOp& op,
					    scx::ScriptRef* right)
{
  if (scx::ScriptOp::Lookup == op.type()) {
    std::string name = right->object()->get_string();
    if (name == "family") return scx::ScriptString::new_ref("ip6");
    if (name == "host") return scx::ScriptString::new_ref(get_host());
    if (name == "address") return scx::ScriptString::new_ref(get_address());
    if (name == "service") return scx::ScriptString::new_ref(get_service());
    if (name == "port") return scx::ScriptInt::new_ref((int)get_port());
  }

  return scx::SocketAddress::script_op(auth,ref,op,right);
}

//=============================================================================
void IP6SocketAddress::set_address(
  const std::string& addr
)
{
  m_host = std::string();
  
  if (addr.length()==0) {
    // Set the address invalid
    m_addr.sin6_addr = in6addr_any;
    m_valid = false;
    
  } else if (addr=="*") {
    // Wildcard address
    m_addr.sin6_addr = in6addr_any;
    m_host = addr;
    m_valid = true;
		
  } else {
    // ip6 address string (xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx)
    if (inet_pton(AF_INET6, addr.c_str(), &m_addr.sin6_addr) > 0) {
      m_valid = true;
    } else {
      // Its not a valid ip address, so assume it is a host name and 
      // try and resolve it.
      hostent* phe = gethostbyname2(addr.c_str(),AF_INET6);
      if (phe) {
        // Resolved so save it
        memcpy(&m_addr.sin6_addr,phe->h_addr,phe->h_length);
        m_host = addr;
        m_valid = true;
      } else {
        // Could not resolve, so leave invalid
        m_valid = false;
        DEBUG_LOG("set_address() could not resolve address");
      }
    }
  }
}

//=============================================================================
std::string IP6SocketAddress::get_address() const
{
  if (m_valid) {
    char str[INET6_ADDRSTRLEN];
    const char* a = inet_ntop(AF_INET6, &m_addr.sin6_addr, str, sizeof(str));
    if (a) {
      return std::string(str);
    }
  } 
  
  return std::string();
}

//=============================================================================
const std::string& IP6SocketAddress::get_host() const
{
  if (m_valid) {
    
    if (m_host.length()==0) {
      // We haven't got a host name set so try and resolve it
      hostent* phe = gethostbyaddr((const char*)&m_addr.sin6_addr,
                                   sizeof(m_addr.sin6_addr),AF_INET6);
      if (phe) {
        // Resolved the host name so cache it
        IP6SocketAddress* unconst = (IP6SocketAddress*)this; // CAC!!!
        unconst->m_host = phe->h_name;
      }
    }
    
  }
  return m_host;
}

//=============================================================================
void IP6SocketAddress::set_port(
  const std::string& port
)
{
  if (port.length()==0) {
    // Set the port invalid
    m_addr.sin6_port = 0;
    
  } else if (port=="*") {
    // Wildcard address
    m_addr.sin6_port = 0;
    m_service = port;
    
  } else {
    // See if its a port number
    m_addr.sin6_port = htons((short)atoi(port.c_str()));
    
    if (m_addr.sin6_port == 0) {
      // Its not a valid port number, so assume it is a service name and 
      // try and resolve it.
      servent* pse = getservbyname(port.c_str(),"tcp");
      if (pse) {
        // Resolved so save it
        m_addr.sin6_port = pse->s_port;
        m_service = port;
      } else {
        // Could not resolve, so leave as invalid
        DEBUG_LOG("set_port() could not resolve port number");
        m_service = std::string();
      }
    }
  }
}

//=============================================================================
void IP6SocketAddress::set_port(
  short port
)
{
  m_service = std::string();
  m_addr.sin6_port = htons(port);
}

//=============================================================================
short IP6SocketAddress::get_port() const
{
  return ntohs(m_addr.sin6_port);
}
	
//=============================================================================
const std::string& IP6SocketAddress::get_service() const
{
  if (m_service.length()==0) {
    // We haven't got a service name set so try and resolve it
    servent* pse = getservbyport((int)m_addr.sin6_port,"tcp");
    IP6SocketAddress* unconst = (IP6SocketAddress*)this; // CAC!!!
    if (pse) {
      // Resolved the service name so cache it
      unconst->m_service = pse->s_name;
    } else {
      std::ostringstream oss;
      oss << (int)ntohs(m_addr.sin6_port);
      unconst->m_service = oss.str();
    }
  }
  
  return m_service;
}
