/* SconeServer (http://www.sconemad.com)

Connection forwarding moduile

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

#include "ForwardModule.h"
//#include "ForwardStream.h"

#include <sconex/ModuleInterface.h>
#include <sconex/Module.h>
#include <sconex/ScriptTypes.h>
#include <sconex/StreamSocket.h>
#include <sconex/StreamTransfer.h>
#include <sconex/Kernel.h>

SCONEX_MODULE(ForwardModule);

//=========================================================================
class ForwardStream : public scx::StreamTransfer {
public:
  ForwardStream(ForwardModule* module,
		const scx::SocketAddress* dest)
    : scx::StreamTransfer(0),
      m_module(module),
      m_connected(false),
      m_dest((scx::SocketAddress*)dest->new_copy())
  {
    set_close_when_finished(true);
  };

  virtual ~ForwardStream()
  {
    delete m_dest;
  };

  virtual scx::Condition event(scx::Stream::Event e)
  {
    if (!m_connected && e == scx::Stream::Opening) {
      
      scx::StreamSocket* sock = new scx::StreamSocket();

      // Connect the socket
      scx::Condition err = sock->connect(m_dest);
      
      if (err != scx::Ok && err != scx::Wait) {
	delete sock;
	DEBUG_LOG("Unable to connect to " << m_dest->get_string());
	return err;
      }

      // Hook up the transfer source
      sock->add_stream(m_manager->get_source());
  
      // Create sconeserver --> dest transfer stream
      scx::StreamTransfer* xfer2 = new scx::StreamTransfer(&endpoint());
      sock->add_stream(xfer2);
      
      // Add socket to kernel
      scx::Kernel::get()->connect(sock);
  
      m_connected = true;
    }

    if (m_connected)
      return scx::StreamTransfer::event(e);

    return scx::Wait;
  }

private:

  scx::ScriptRefTo<ForwardModule> m_module;

  bool m_connected;
  scx::SocketAddress* m_dest;

};


//=========================================================================
ForwardModule::ForwardModule()
  : scx::Module("forward",scx::version())
{
  scx::Stream::register_stream("forward",this);
}

//=========================================================================
ForwardModule::~ForwardModule()
{
  scx::Stream::unregister_stream("forward",this);
}

//=========================================================================
std::string ForwardModule::info() const
{
  return "Connection forwarding";
}

//=========================================================================
void ForwardModule::provide(const std::string& type,
			    const scx::ScriptRef* args,
			    scx::Stream*& object)
{
  const scx::SocketAddress* sa =
    scx::get_method_arg<scx::SocketAddress>(args,0,"address");
  if (!sa) {
    DEBUG_LOG("Address must be specified");
    return;
  }

  object = new ForwardStream(this,sa);
}
