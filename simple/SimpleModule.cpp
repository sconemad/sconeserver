/* SconeServer (http://www.sconemad.com)

Simple TCP/IP services module

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

#include "SimpleModule.h"
#include "SimpleStreams.h"

#include <sconex/ModuleInterface.h>
#include <sconex/Socket.h>
#include <sconex/Date.h>
#include <sconex/Response.h>
#include <sconex/ScriptTypes.h>

SCONEX_MODULE(SimpleModule);

//=========================================================================
SimpleModule::SimpleModule()
  : scx::Module("simple",scx::version())
{
  scx::Stream::register_stream("echo",this);
  scx::Stream::register_stream("discard",this);
  scx::Stream::register_stream("daytime",this);
  scx::Stream::register_stream("chargen",this);
  scx::Stream::register_stream("time",this);
}

//=========================================================================
SimpleModule::~SimpleModule()
{
  scx::Stream::unregister_stream("echo",this);
  scx::Stream::unregister_stream("discard",this);
  scx::Stream::unregister_stream("daytime",this);
  scx::Stream::unregister_stream("chargen",this);
  scx::Stream::unregister_stream("time",this);
}

//=========================================================================
std::string SimpleModule::info() const
{
  return "Simple TCP/IP services\n"
         "Implements: echo, discard, daytime, chargen and time";
}

//=========================================================================
void SimpleModule::provide(const std::string& type,
			   const scx::ScriptRef* args,
			   scx::Stream*& object)
{
  if ("echo" == type) {
    const scx::ScriptInt* a_buffer_size =
      scx::get_method_arg<scx::ScriptInt>(args,0,"buffer_size");
    int buffer_size = (a_buffer_size ? a_buffer_size->get_int() : 1);
    object = new EchoStream(this, buffer_size);
    
  } else if ("discard" == type) {
    object = new DiscardStream(this);

  } else if ("daytime" == type) {
    // RFC867
    std::string str = scx::Date::now(true).string() + std::string("\r\n");
    object = new scx::Response(str);
    
  } else if ("chargen" == type) {
    object = new ChargenStream(this);

  } else if ("time" == type) {
    // RFC868
    uint32_t t = htonl(scx::Date::now().epoch_seconds() + 2208988800u);
    object = new scx::Response(&t,4);

  }
}

/* -- this doesn't fit into the new provider fw, could use an "auto" stream?
// Otherwise try to determine the type associated with the local port
scx::Socket* sock = dynamic_cast<scx::Socket*>(endpoint);
if (sock) {
  scx::SocketAddress* addr =
    const_cast<scx::SocketAddress*>(sock->get_local_addr());
  scx::ArgString a_right("service");
  scx::Arg* a_addr_service = addr->op(scx::Auth::Untrusted,scx::Arg::Binary,".",&a_right);
  type = a_addr_service->get_string();
 }
}
*/
