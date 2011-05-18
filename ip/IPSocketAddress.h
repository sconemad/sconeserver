/* SconeServer (http://www.sconemad.com)

Socket address for TCP/IP version 4 (host:port)

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

#ifndef ipSocketAddress_h
#define ipSocketAddress_h

#include <sconex/SocketAddress.h>
#include <sconex/Module.h>

//=============================================================================
class IPSocketAddress : public scx::SocketAddress {
public:

  IPSocketAddress(scx::Module* module,
		  const scx::ScriptRef* args);

  IPSocketAddress(const IPSocketAddress& c);
  
  virtual ~IPSocketAddress();
  
  // SocketAddress methods

  virtual scx::ScriptObject* new_copy() const;

  virtual bool valid_for_bind() const;
  virtual bool valid_for_connect() const;

  virtual void set_sockaddr(const struct sockaddr* sa);
  virtual const struct sockaddr* get_sockaddr() const;
  virtual socklen_t get_sockaddr_size() const;
  
  // ScriptObject methods

  virtual std::string get_string() const;

  virtual scx::ScriptRef* script_op(const scx::ScriptAuth& auth,
				    const scx::ScriptRef& ref,
				    const scx::ScriptOp& op,
				    scx::ScriptRef* right=0);
  
  // Address/host methods

  void set_address(const std::string& addr);
  void set_address(int ip1,int ip2,int ip3,int ip4);

  std::string get_address() const;
  // Get ip address in form xxx.xxx.xxx.xxx

  const std::string& get_host() const;
  // Get host name (resolves if required)

  
  // Port/service methods

  void set_port(const std::string& port);
  void set_port(short port);

  short get_port() const;
  // Get the port in numerical form
	
  const std::string& get_service() const;
  // Get the service name (resolves if required)

  std::string get_type_name() const;
  // Get a string describing the socket type, e.g. "tcp" or "udp"

protected:

  scx::Module::Ref m_module;

  struct sockaddr_in m_addr;
  std::string m_host;
  std::string m_service;
  bool m_valid;
  
};

#endif
