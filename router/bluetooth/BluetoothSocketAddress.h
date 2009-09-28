/* SconeServer (http://www.sconemad.com)

Socket address for Bluetooth

Copyright (c) 2000-2005 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef bluetoothSocketAddress_h
#define bluetoothSocketAddress_h

#include "sconex/SocketAddress.h"

#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

//=============================================================================
class BluetoothSocketAddress : public scx::SocketAddress {

public:

  BluetoothSocketAddress(scx::Arg* args);

  BluetoothSocketAddress(const BluetoothSocketAddress& c);
  
  virtual ~BluetoothSocketAddress();

  
  // SocketAddress methods

  virtual scx::Arg* new_copy() const;

  virtual bool valid_for_bind() const;
  virtual bool valid_for_connect() const;

  virtual void set_sockaddr(const struct sockaddr* sa);
  virtual const struct sockaddr* get_sockaddr() const;
  virtual socklen_t get_sockaddr_size() const;
  
  // Arg methods

  virtual std::string get_string() const;
  virtual scx::Arg* op(
    const scx::Auth& auth,
    scx::Arg::OpType optype,
    const std::string& opname,
    scx::Arg* right
  );

  
  // Address
  void set_address(const std::string& addr);
  std::string get_address() const;
  
  // Channel
  void set_channel(short channel);
  short get_channel() const;

protected:

  struct sockaddr_rc m_addr;
  bool m_valid;
  
};

#endif
