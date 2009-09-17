/* SconeServer (http://www.sconemad.com)

Connection forwarding moduile

Copyright (c) 2000-2007 Andrew Wedgbury <wedge@sconemad.com>

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

#include "ForwardModule.h"
//#include "ForwardStream.h"

#include "sconex/ModuleInterface.h"
#include "sconex/Module.h"
#include "sconex/Arg.h"
#include "sconex/StreamSocket.h"
#include "sconex/StreamTransfer.h"
#include "sconex/Kernel.h"

SCONESERVER_MODULE(ForwardModule);

//=========================================================================
ForwardModule::ForwardModule(
)
  : scx::Module("forward",scx::version())
{

}

//=========================================================================
ForwardModule::~ForwardModule()
{

}

//=========================================================================
std::string ForwardModule::info() const
{
  return "Copyright (c) 2000-2009 Andrew Wedgbury\n"
         "Connection forwarding\n";
}

//=========================================================================
bool ForwardModule::connect(
  scx::Descriptor* endpoint,
  scx::ArgList* args
)
{
  const scx::SocketAddress* sa =
    dynamic_cast<const scx::SocketAddress*>(args->get(0));
  if (!sa) {
    return new scx::ArgError("ForwardModule::connect() Address must be specified");
  }

  scx::StreamSocket* sock = new scx::StreamSocket();

  // Connect the socket
  scx::Condition err = sock->connect(sa);

  if (err != scx::Ok) {
    delete sock;
    return false;
  }
  
  // Create sconeserver <-- program transfer stream
  scx::StreamTransfer* xfer = new scx::StreamTransfer(sock);
  xfer->set_close_when_finished(true);
  endpoint->add_stream(xfer);
  
  scx::StreamTransfer* xfer2 =
    new scx::StreamTransfer(endpoint);
  sock->add_stream(xfer2);
  
  // Add socket to kernel table
  scx::Kernel::get()->connect(sock,0);
  
  return true;
}

//=============================================================================
scx::Arg* ForwardModule::arg_lookup(const std::string& name)
{
  // Methods

  return SCXBASE Module::arg_lookup(name);
}

//=============================================================================
scx::Arg* ForwardModule::arg_function(
  const std::string& name,
  scx::Arg* args
)
{
  scx::ArgList* l = dynamic_cast<scx::ArgList*>(args);

  return SCXBASE Module::arg_function(name,args);
}

