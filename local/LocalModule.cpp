/* SconeServer (http://www.sconemad.com)

Local (UNIX domain) socket address module

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
#include <sconex/ScriptExpr.h>
#include "LocalSocketAddress.h"

//=========================================================================
class LocalModule : public scx::Module,
                    public scx::Provider<scx::ScriptObject> {
public:

  LocalModule();
  virtual ~LocalModule();

  virtual std::string info() const;

  // Provider<ScriptObject> method
  virtual void provide(const std::string& type,
		       const scx::ScriptRef* args,
		       scx::ScriptObject*& object);
};

SCONESERVER_MODULE(LocalModule);

//=========================================================================
LocalModule::LocalModule(
) : scx::Module("local",scx::version())
{
  scx::ScriptExpr::register_type("LocalAddr",this);
}

//=========================================================================
LocalModule::~LocalModule()
{
  scx::ScriptExpr::unregister_type("LocalAddr",this);
}

//=========================================================================
std::string LocalModule::info() const
{
  return "Local (UNIX domain) connectivity module";
}

//=============================================================================
void LocalModule::provide(const std::string& type,
			  const scx::ScriptRef* args,
			  scx::ScriptObject*& object)
{
  if (type == "LocalAddr") {
    object = new LocalSocketAddress(args);
  }
}
