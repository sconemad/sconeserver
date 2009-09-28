/* SconeServer (http://www.sconemad.com)

Socket address for Local (UNIX domain) protocol

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

#ifndef localSocketAddress_h
#define localSocketAddress_h

#include "sconex/SocketAddress.h"
#include "sys/un.h"

//=============================================================================
class LocalSocketAddress : public scx::SocketAddress {

public:

  LocalSocketAddress(scx::Arg* args);

  LocalSocketAddress(const LocalSocketAddress& c);
  
  virtual ~LocalSocketAddress();

  
  // SocketAddress methods

  virtual scx::Arg* new_copy() const;

  virtual bool valid_for_bind() const;
  virtual bool valid_for_connect() const;

  virtual void set_sockaddr(const struct sockaddr* sa);
  virtual const struct sockaddr* get_sockaddr() const;
  virtual socklen_t get_sockaddr_size() const;

  virtual int socket_bind(SOCKET s) const;
  
  // Arg methods

  virtual std::string get_string() const;
  virtual scx::Arg* op(
    const scx::Auth& auth,
    scx::Arg::OpType optype,
    const std::string& opname,
    scx::Arg* right
  );

  bool set_path(const std::string& path);
  std::string get_path() const;
  
protected:

  struct sockaddr_un m_addr;
  
};

#endif
