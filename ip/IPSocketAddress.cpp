/* SconeServer (http://www.sconemad.com)

Sconex socket address, holds address:port for a socket

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

#include "IPSocketAddress.h"
#include <sconex/ScriptTypes.h>

// Min/Max buffer size for gethostbyname_r/gethostbyaddr_r requests
#define BSIZE_MIN 1024
#define BSIZE_MAX 65536

//=============================================================================
IPSocketAddress::IPSocketAddress(scx::Module* module,
				 const scx::ScriptRef* args)
  : scx::SocketAddress(PF_INET,SOCK_STREAM),
    m_module(module)
{
  DEBUG_COUNT_CONSTRUCTOR(IPSocketAddress);
  memset(&m_addr,0,sizeof(m_addr));
  m_addr.sin_family = AF_INET;
  
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

  const scx::ScriptString* a_type =
    scx::get_method_arg<scx::ScriptString>(args,2,"type");
  if (a_type) {
    if (a_type->get_string() == "datagram") {
      m_type = SOCK_DGRAM;
    }
  }
  // type defaults to SOCK_STREAM
  
}

//=============================================================================
IPSocketAddress::IPSocketAddress(const IPSocketAddress& c)
  : scx::SocketAddress(c),
    m_module(c.m_module),
    m_host(c.m_host),
    m_service(c.m_service),
    m_valid(c.m_valid)
{
  DEBUG_COUNT_CONSTRUCTOR(IPSocketAddress);
  memcpy(&m_addr,&c.m_addr,sizeof(m_addr));
}

//=============================================================================
IPSocketAddress::~IPSocketAddress()
{
  DEBUG_COUNT_DESTRUCTOR(IPSocketAddress);
}

//=============================================================================
scx::ScriptObject* IPSocketAddress::new_copy() const
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
  DEBUG_ASSERT(sa!=0,"Invalid sockaddr pointer");
  DEBUG_ASSERT(sa->sa_family == m_domain,"Socket domain mismatch");

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
  if (service.empty()) oss << get_port();
  
  return oss.str();
}

//=============================================================================
scx::ScriptRef* IPSocketAddress::script_op(const scx::ScriptAuth& auth,
					   const scx::ScriptRef& ref,
					   const scx::ScriptOp& op,
					   const scx::ScriptRef* right)
{
  if (scx::ScriptOp::Lookup == op.type()) {
    std::string name = right->object()->get_string();
    if (name == "family") return scx::ScriptString::new_ref("ip");
    if (name == "host") return scx::ScriptString::new_ref(get_host());
    if (name == "address") return scx::ScriptString::new_ref(get_address());
    if (name == "service") return scx::ScriptString::new_ref(get_service());
    if (name == "port") return scx::ScriptInt::new_ref((int)get_port());
  }

  return scx::SocketAddress::script_op(auth,ref,op,right);
}

//=============================================================================
void IPSocketAddress::set_address(
  const std::string& addr
)
{
  m_host = std::string();
  
  if (addr.length()==0) {
    // Set the address invalid
    m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    m_valid = false;
    
  } else if (addr=="*") {
    // Wildcard address
    m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    m_host = addr;
    m_valid = true;
		
  } else {
    // ip address string (xxx.xxx.xxx.xxx)
    if (inet_pton(AF_INET, addr.c_str(), &m_addr.sin_addr.s_addr) > 0) {
      m_valid = true;
    } else {
      // Its not a valid ip address, so assume it is a host name and 
      // try and resolve it.
      int ret = -1;
      hostent he;
      hostent* phe = 0;
      int rerr = 0;
      int bsize = BSIZE_MIN;
      for (; bsize <= BSIZE_MAX; bsize *= 2) {
	char* buf = new char[bsize];
	ret = gethostbyname_r(addr.c_str(),
			      &he,buf,bsize,&phe,&rerr);
	if (0 == ret && phe) {
	  // Resolved so save it
	  memcpy(&m_addr.sin_addr,phe->h_addr,phe->h_length);
	  m_host = addr;
	  m_valid = true;
	}
	delete [] buf;
	if (ERANGE != ret) break;
      }
      if (0 != ret) {
	// Could not resolve, so leave invalid
	DEBUG_LOG("set_address() could not resolve address");
	m_valid = false;
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
  
  unsigned char* paddr = (unsigned char*)&m_addr.sin_addr.s_addr;
  
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
    if (0 == m_host.length()) {
      // We haven't got a host name set so try and resolve it
      int ret = -1;
      hostent he;
      hostent* phe = 0;
      int rerr = 0;
      int bsize = BSIZE_MIN;
      for (; bsize <= BSIZE_MAX; bsize *= 2) {
	char* buf = new char[bsize];
	ret = gethostbyaddr_r((const char*)&m_addr.sin_addr,
			      sizeof(m_addr.sin_addr),AF_INET,
			      &he,buf,bsize,&phe,&rerr);
	if (0 == ret && phe) {
	  // Resolved the host name so cache it
	  IPSocketAddress* unconst = const_cast<IPSocketAddress*>(this);
	  unconst->m_host = phe->h_name;
	}
	delete [] buf;
	if (ERANGE != ret) break;
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
      int ret = -1;
      servent se;
      servent* pse = 0;
      int bsize = BSIZE_MIN;
      for (; bsize <= BSIZE_MAX; bsize *= 2) {
	char* buf = new char[bsize];
	ret = getservbyname_r(port.c_str(),"tcp",
			      &se,buf,bsize,&pse);
	if (0 == ret && pse) {
	  // Resolved so save it
	  m_addr.sin_port = pse->s_port;
	  m_service = port;
	}
	delete [] buf;
	if (ERANGE != ret) break;
      }
      if (0 != ret) {
	// Could not resolve, so leave as invalid
        DEBUG_LOG("set_port() could not resolve port number");
        m_service = std::string();
      }
    }
  }
}

//=============================================================================
void IPSocketAddress::set_port(
  unsigned short port
)
{
  m_service = std::string();
  m_addr.sin_port = htons(port);
}

//=============================================================================
unsigned short IPSocketAddress::get_port() const
{
  return ntohs(m_addr.sin_port);
}
	
//=============================================================================
const std::string& IPSocketAddress::get_service() const
{
  if (0 == m_service.length()) {
    IPSocketAddress* unconst = const_cast<IPSocketAddress*>(this);
    // We haven't got a service name set so try and resolve it
    int ret = -1;
    servent se;
    servent* pse = 0;
    int bsize = BSIZE_MIN;
    for (; bsize <= BSIZE_MAX; bsize *= 2) {
      char* buf = new char[bsize];
      ret = getservbyport_r((int)m_addr.sin_port,
			    get_type_name().c_str(),
			    &se,buf,bsize,&pse);
      if (0 == ret && pse) {
	// Resolved the service name so cache it
	unconst->m_service = pse->s_name;
      }
      delete [] buf;
      if (ERANGE != ret) break;
    }
    if (ret != 0) {
      std::ostringstream oss;
      oss << ntohs(m_addr.sin_port);
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
