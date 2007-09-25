/* SconeServer (http://www.sconemad.com)

Sconex socket address, holds address:port for a socket

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

#include "IPSocketAddress.h"

#define m_s_addr m_addr.sin_addr.s_addr

#ifndef HAVE_INET_PTON
//=============================================================================
// IP4 only inet_pton for systems that don't have it
int inet_pton(
  int family,
  const char* strptr,
  void* addrptr
)
{
  if (family == AF_INET) {
#ifdef HAVE_INET_ATON
    struct in_addr in_val;
    if (inet_aton(strptr, &in_val)) {
      memcpy(addrptr, &in_val, sizeof(struct in_addr));
      return 1;
    }
    return 0;
#else
    // We'll just have to use the crap one
    *((unsigned long*)addrptr) = inet_addr(strptr);
    // (hope in worked!)
    return 1;
#endif
  }
  //  errno = EAFNOSUPPORT;
  return -1;
}
#endif

#ifndef HAVE_INET_NTOP
#define INET_ADDRSTRLEN 16
//=============================================================================
// IP4 only inet_ntop for systems that don't have it
const char* inet_ntop(
  int family,
  const void* addrptr,
  char* strptr,
  size_t len
)
{
  const unsigned char *p = (const unsigned char*)addrptr;
  if (family == AF_INET) {
    char temp[INET_ADDRSTRLEN];
    std::ostringstream oss;
    oss << (int)p[0] << "." << (int)p[1] << "."
        << (int)p[2] << "." << (int)p[2];
    if (oss.str().length() >= len) {
      return 0;
    }
    strcpy(strptr,oss.str().c_str());
    return strptr;
  }
  return 0;
}
#endif


//=============================================================================
IPSocketAddress::IPSocketAddress(scx::Arg* args)
  : scx::SocketAddress(PF_INET,SOCK_STREAM)
{
  DEBUG_COUNT_CONSTRUCTOR(IPSocketAddress);
  memset(&m_addr,0,sizeof(m_addr));
  m_addr.sin_family = AF_INET;
  
  scx::ArgList* l = dynamic_cast<scx::ArgList*>(args);
  
  const scx::ArgString* a_host =
    dynamic_cast<const scx::ArgString*>(l->get(0));
  if (a_host) {
    set_address(a_host->get_string());
  }

  // Could be string or numeric
  const scx::ArgInt* a_service_num =
    dynamic_cast<const scx::ArgInt*>(l->get(1));
  const scx::ArgString* a_service_str =
    dynamic_cast<const scx::ArgString*>(l->get(1));
  
  if (a_service_num) {
    int p = a_service_num->get_int();
    if (p > 0 && p < 32768) {
      set_port(p);
    }

  } else if (a_service_str) {
    set_port(a_service_str->get_string());
    
  }

  const scx::ArgString* a_type =
    dynamic_cast<const scx::ArgString*>(l->get(2));
  if (a_type) {
    if (a_type->get_string() == "datagram") {
      m_type = SOCK_DGRAM;
    }
  }
  // type defaults to SOCK_STREAM
  
}

//=============================================================================
IPSocketAddress::IPSocketAddress(const IPSocketAddress& c)
  : scx::SocketAddress(c)
{
  DEBUG_COUNT_CONSTRUCTOR(IPSocketAddress);
  memcpy(&m_addr,&c.m_addr,sizeof(m_addr));
  m_host = c.m_host;
  m_service = c.m_service;
  m_valid = c.m_valid;
}

//=============================================================================
IPSocketAddress::~IPSocketAddress()
{
  DEBUG_COUNT_DESTRUCTOR(IPSocketAddress);
}

//=============================================================================
scx::Arg* IPSocketAddress::new_copy() const
{
  return new IPSocketAddress(*this);
}

//=============================================================================
bool IPSocketAddress::valid_for_bind() const
{
  return m_valid && (get_port() > 0);
}

//=============================================================================
bool IPSocketAddress::valid_for_connect() const
{
  return m_valid;
}

//=============================================================================
void IPSocketAddress::set_sockaddr(const struct sockaddr* sa)
{
  DEBUG_ASSERT(sa!=0,"set_sockaddr() Invalid sockaddr pointer");
  DEBUG_ASSERT(sa->sa_family == m_domain,"set_sockaddr() Socket domain mismatch");

  memcpy(&m_addr,sa,sizeof(m_addr));
  m_host = "";
  m_service = "";
}

//=============================================================================
const struct sockaddr* IPSocketAddress::get_sockaddr() const
{
  return (const struct sockaddr*)&m_addr;
}

//=============================================================================
socklen_t IPSocketAddress::get_sockaddr_size() const
{
  return sizeof(m_addr);
}

//=============================================================================
std::string IPSocketAddress::get_string() const
{
  std::ostringstream oss;
  oss << get_type_name() << ":";
  
  std::string host = get_host();
  oss << host;
  if (host.empty()) oss << get_address();
  oss << ":";

  std::string service = get_service();
  oss << service;
  if (service.empty()) oss << (int)get_port();
  
  return oss.str();
}

//=============================================================================
scx::Arg* IPSocketAddress::op(
  scx::Arg::OpType optype,
  const std::string& opname,
  scx::Arg* right
)
{
  if (scx::Arg::Binary == optype && "." == opname) {
    std::string name = right->get_string();
    if (name == "family") return new scx::ArgString("ip");
    if (name == "host") return new scx::ArgString(get_host());
    if (name == "address") return new scx::ArgString(get_address());
    if (name == "service") return new scx::ArgString(get_service());
    if (name == "port") return new scx::ArgInt((int)get_port());
  }
  return SCXBASE SocketAddress::op(optype,opname,right);
}

//=============================================================================
void IPSocketAddress::set_address(
  const std::string& addr
)
{
  m_host = std::string();
  
  if (addr.length()==0) {
    // Set the address invalid
    m_s_addr = htonl(INADDR_ANY);
    m_valid = false;
    
  } else if (addr=="*") {
    // Wildcard address
    m_s_addr = htonl(INADDR_ANY);
    m_host = addr;
    m_valid = true;
		
  } else {
    // ip address string (xxx.xxx.xxx.xxx)
    if (inet_pton(AF_INET, addr.c_str(), &m_s_addr) > 0) {
      m_valid = true;
    } else {
      // Its not a valid ip address, so assume it is a host name and 
      // try and resolve it.
      hostent* phe = gethostbyname(addr.c_str());
      if (phe) {
        // Resolved so save it
        memcpy(&m_addr.sin_addr,phe->h_addr,phe->h_length);
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
void IPSocketAddress::set_address(
  int ip1,
  int ip2,
  int ip3,
  int ip4
)
{
  DEBUG_ASSERT(ip1>=0 && ip1<=255,"set_address() ip1 invalid");
  DEBUG_ASSERT(ip2>=0 && ip2<=255,"set_address() ip2 invalid");
  DEBUG_ASSERT(ip3>=0 && ip3<=255,"set_address() ip3 invalid");
  DEBUG_ASSERT(ip4>=0 && ip4<=255,"set_address() ip4 invalid");
  
  m_host = std::string();
  
  unsigned char* paddr = (unsigned char*)&m_s_addr;
  
  paddr[0] = (unsigned char)ip1;
  paddr[1] = (unsigned char)ip2;
  paddr[2] = (unsigned char)ip3;
  paddr[3] = (unsigned char)ip4;
  m_valid = true;
}

//=============================================================================
std::string IPSocketAddress::get_address() const
{
  if (m_valid) {
    char str[INET_ADDRSTRLEN];
    const char* a = inet_ntop(AF_INET, &m_addr.sin_addr, str, sizeof(str));
    if (a) {
      return std::string(str);
    }
  } 
  
  return std::string();
}

//=============================================================================
const std::string& IPSocketAddress::get_host() const
{
  if (m_valid) {
    
    if (m_host.length()==0) {
      // We haven't got a host name set so try and resolve it
      hostent* phe = gethostbyaddr((const char*)&m_addr.sin_addr,
                                   sizeof(m_addr.sin_addr),AF_INET);
      if (phe) {
        // Resolved the host name so cache it
        IPSocketAddress* unconst = (IPSocketAddress*)this; // CAC!!!
        unconst->m_host = phe->h_name;
      }
    }
    
  }
  return m_host;
}

//=============================================================================
void IPSocketAddress::set_port(
  const std::string& port
)
{
  if (port.length()==0) {
    // Set the port invalid
    m_addr.sin_port = 0;
    
  } else if (port=="*") {
    // Wildcard address
    m_addr.sin_port = 0;
    m_service = port;
    
  } else {
    // See if its a port number
    m_addr.sin_port = htons((short)atoi(port.c_str()));
    
    if (m_addr.sin_port == 0) {
      // Its not a valid port number, so assume it is a service name and 
      // try and resolve it.
      servent* pse = getservbyname(port.c_str(),
				   get_type_name().c_str());
      if (pse) {
        // Resolved so save it
        m_addr.sin_port = pse->s_port;
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
void IPSocketAddress::set_port(
  short port
)
{
  m_service = std::string();
  m_addr.sin_port = htons(port);
}

//=============================================================================
short IPSocketAddress::get_port() const
{
  return ntohs(m_addr.sin_port);
}
	
//=============================================================================
const std::string& IPSocketAddress::get_service() const
{
  if (m_service.length()==0) {
    // We haven't got a service name set so try and resolve it
    servent* pse = getservbyport((int)m_addr.sin_port,
				 get_type_name().c_str());
    IPSocketAddress* unconst = (IPSocketAddress*)this; // CAC!!!
    if (pse) {
      // Resolved the service name so cache it
      unconst->m_service = pse->s_name;
    } else {
      std::ostringstream oss;
      oss << (int)ntohs(m_addr.sin_port);
      unconst->m_service = oss.str();
    }
  }
  
  return m_service;
}

//=============================================================================
std::string IPSocketAddress::get_type_name() const
{
  switch (m_type) {
  case SOCK_STREAM: return "tcp";
  case SOCK_DGRAM: return "udp";
  }
  return "unknown";
}
