/* SconeServer (http://www.sconemad.com)

Simple TCP/IP services module

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

#include "SimpleModule.h"
#include "SimpleStreams.h"

#include "sconex/ModuleInterface.h"
#include "sconex/Socket.h"
#include "sconex/Date.h"
#include "sconex/Response.h"
#include "sconex/Arg.h"

SCONESERVER_MODULE(SimpleModule);

//=========================================================================
SimpleModule::SimpleModule()
  : scx::Module("simple",scx::version())
{

}

//=========================================================================
SimpleModule::~SimpleModule()
{

}

//=========================================================================
std::string SimpleModule::info() const
{
  return "Copyright (c) 2000-2005 Andrew Wedgbury\n"
         "Simple TCP/IP services\n"
         "Implements: echo, discard, daytime, chargen and time\n";
}

//=========================================================================
bool SimpleModule::connect(
  scx::Descriptor* endpoint,
  scx::ArgList* args
)
{
  const scx::ArgString* a_service =
    dynamic_cast<const scx::ArgString*>(args->get(0));
  std::string service;
  if (a_service) {
    // Use service name if one was passed
    service = a_service->get_string();
  } else {
    // Otherwise try to determine the service associated with the local port
    scx::Socket* sock = dynamic_cast<scx::Socket*>(endpoint);
    if (sock) {
      scx::SocketAddress* addr =
        const_cast<scx::SocketAddress*>(sock->get_local_addr());
      scx::ArgString a_right("service");
      scx::Arg* a_addr_service = addr->op(scx::Arg::Binary,".",&a_right);
      service = a_addr_service->get_string();
    }
  }

  if ("echo" == service) {
    EchoStream* s = new EchoStream();
    s->add_module_ref(ref());
    endpoint->set_timeout(scx::Time(60));
    endpoint->add_stream(s);
    return true;

  } else if ("discard" == service) {
    DiscardStream* s = new DiscardStream();
    s->add_module_ref(ref());
    endpoint->set_timeout(scx::Time(60));
    endpoint->add_stream(s);
    return true;

  } else if ("daytime" == service) {
    // RFC867
    std::string str = scx::Date::now(true).string() + std::string("\r\n");
    endpoint->add_stream( new scx::Response(str) );
    return true;
    
  } else if ("chargen" == service) {
    ChargenStream* s = new ChargenStream();
    s->add_module_ref(ref());
    endpoint->set_timeout(scx::Time(60));
    endpoint->add_stream(s);
    return true;

  } else if ("time" == service) {
    // RFC868
    uint32_t t = htonl(scx::Date::now().epoch_seconds() + 2208988800u);
    endpoint->add_stream( new scx::Response(&t,4) );
    return true;
   
  }
  
  return false;
}

//=========================================================================
bool SimpleModule::unload()
{
  return true;
}

