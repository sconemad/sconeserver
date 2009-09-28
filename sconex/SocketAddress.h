/* SconeServer (http://www.sconemad.com)

Sconex socket address

Copyright (c) 2000-2006 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef scxSocketAddress_h
#define scxSocketAddress_h

#include "sconex/sconex.h"
#include "sconex/Arg.h"
namespace scx {

class Socket;

//=============================================================================
class SCONEX_API SocketAddress : public Arg {

public:

  SocketAddress(
    int domain,
    int type,
    int protocol = 0
  );
  // Default constructor

  SocketAddress(const SocketAddress& c);
  // Copy constructor
  
  virtual ~SocketAddress();
  // Destructor

  
  virtual bool valid_for_bind() const;
  // Is this socket valid for binding, i.e. does it represent local
  // interface(s) and specify a valid port/channel?
  // NOTE: This does not gaurantee that a call to bind() will succeed, just
  // that the address 'looks right'.
  
  virtual bool valid_for_connect() const;
  // Is this socket valid for connecting. i.e. does it represent a valid
  // address and port/channel and the socket type can be connected?
  // NOTE: As above, this does not gaurantee that a call to connect() will
  // succeed.


  virtual void set_sockaddr(const struct sockaddr* sa) =0;
  // Set address using a generic sockaddr pointer, which must point to a
  // sockaddr_xxx structure of the appropriate type for this socket type.
  // NOTE: Ownership of sa is not taken.

  virtual const struct sockaddr* get_sockaddr() const =0;
  // Build and return a generic sockaddr structure for this socket address.
  // This always has network byte ordering
  // NOTE: Ownership is not returned.

  virtual socklen_t get_sockaddr_size() const =0;
  // Get the size of the sockaddr structure used by this address type.

  
  int socket_domain() const;
  int socket_type() const;
  int socket_protocol() const;
  // Get parameters that should be used to create a socket for use with this
  // type of address.

  virtual SOCKET socket_create() const;
  virtual int socket_bind(SOCKET s) const;
  
  // Arg methods
  virtual std::string get_string() const =0;
  virtual int get_int() const;
  virtual Arg* op(const Auth& auth, OpType optype, const std::string& opname, Arg* right);


protected:

  int m_domain;
  int m_type;
  int m_protocol;
  
private:

};

//=============================================================================
class AnonSocketAddress : public SocketAddress {

public:

  AnonSocketAddress(const std::string& name);
  AnonSocketAddress(const AnonSocketAddress& c);
  virtual ~AnonSocketAddress();
  
  // SocketAddress methods
  virtual scx::Arg* new_copy() const;

  virtual bool valid_for_bind() const;
  virtual bool valid_for_connect() const;

  virtual void set_sockaddr(const struct sockaddr* sa);
  virtual const struct sockaddr* get_sockaddr() const;
  virtual socklen_t get_sockaddr_size() const;

  // Arg methods

  virtual std::string get_string() const;
  
protected:

  std::string m_name;

};
  
};
#endif
