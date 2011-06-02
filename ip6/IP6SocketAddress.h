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

#ifndef ip6SocketAddress_h
#define ip6SocketAddress_h

#include <sconex/SocketAddress.h>
#include <sconex/Module.h>

//=============================================================================
class IP6SocketAddress : public scx::SocketAddress {
public:

  IP6SocketAddress(scx::Module* module,
		   const scx::ScriptRef* args);

  IP6SocketAddress(const IP6SocketAddress& c);
  
  virtual ~IP6SocketAddress();
  
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
				    const scx::ScriptRef* right=0);
  
  // Address/host methods

  void set_address(const std::string& addr);

  std::string get_address() const;
  // Get ip6 address string

  const std::string& get_host() const;
  // Get host name (resolves if required)

  
  // Port/service methods

  void set_port(const std::string& port);
  void set_port(short port);

  short get_port() const;
  // Get the port in numerical form
	
  const std::string& get_service() const;
  // Get the service name (resolves if required)

protected:

  scx::Module::Ref m_module;

  struct sockaddr_in6 m_addr;
  std::string m_host;
  std::string m_service;
  bool m_valid;
  
};

#endif
