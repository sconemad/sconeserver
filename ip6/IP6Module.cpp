/* SconeServer (http://www.sconemad.com)

Router IP6 protocol module (TCP/IP version 6)

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


#include <sconex/ModuleInterface.h>
#include <sconex/Module.h>
#include <sconex/ScriptTypes.h>
#include <sconex/ScriptContext.h>
#include "IP6SocketAddress.h"

//=========================================================================
class IP6Module : public scx::Module,
                  public scx::Provider<scx::ScriptObject> {
public:

  IP6Module();
  virtual ~IP6Module();

  virtual std::string info() const;

  // Provider<ScriptObject> method
  virtual void provide(const std::string& type,
		       const scx::ScriptRef* args,
		       scx::ScriptObject*& object);
};

SCONEX_MODULE(IP6Module);

//=========================================================================
IP6Module::IP6Module(
) : scx::Module("ip6",scx::version())
{
  scx::StandardContext::register_type("IP6Addr",this);
}

//=========================================================================
IP6Module::~IP6Module()
{
  scx::StandardContext::unregister_type("IP6Addr",this);
}

//=========================================================================
std::string IP6Module::info() const
{
  return "Internet Protocol (TCP/IP version 6) connectivity module";
}

//=============================================================================
void IP6Module::provide(const std::string& type,
			const scx::ScriptRef* args,
			scx::ScriptObject*& object)
{
  if (type == "IP6Addr") {
    object = new IP6SocketAddress(this,args);
  }
}
