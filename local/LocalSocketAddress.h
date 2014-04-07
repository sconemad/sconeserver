/* SconeServer (http://www.sconemad.com)

Socket address for Local (UNIX domain) protocol

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

#ifndef localSocketAddress_h
#define localSocketAddress_h

#include <sconex/SocketAddress.h>
#include <sconex/Module.h>
#include "sys/un.h"

//=============================================================================
class LocalSocketAddress : public scx::SocketAddress {
public:

  LocalSocketAddress(scx::Module* module,
		     const scx::ScriptRef* args);

  LocalSocketAddress(const LocalSocketAddress& c);
  
  virtual ~LocalSocketAddress();
  
  // SocketAddress methods

  virtual scx::ScriptObject* new_copy() const;

  virtual bool valid_for_bind() const;
  virtual bool valid_for_connect() const;

  virtual void set_sockaddr(const struct sockaddr* sa);
  virtual const struct sockaddr* get_sockaddr() const;
  virtual socklen_t get_sockaddr_size() const;

  virtual int socket_bind(SOCKET s) const;
  
  // ScriptObject methods

  virtual std::string get_string() const;

  virtual scx::ScriptRef* script_op(const scx::ScriptAuth& auth,
				    const scx::ScriptRef& ref,
				    const scx::ScriptOp& op,
				    const scx::ScriptRef* right=0);


  bool set_path(const std::string& path);
  std::string get_path() const;
  
  void set_mode(mode_t mode);
  mode_t get_mode() const;

protected:

  scx::Module::Ref m_module;

  struct sockaddr_un m_addr;
  mode_t m_mode;
  
};

#endif
