/* SconeServer (http://www.sconemad.com)

Router IP protocol module (TCP/IP version 4)

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


#include "sconex/ModuleInterface.h"
#include "sconex/Module.h"
#include "sconex/Arg.h"
#include "IPSocketAddress.h"

//=========================================================================
class IPModule : public scx::Module {
public:

  IPModule();
  virtual ~IPModule();

  virtual std::string info() const;

  virtual int init();
  
  virtual scx::Arg* arg_lookup(const std::string& name);
  virtual scx::Arg* arg_method(const scx::Auth& auth,const std::string& name,scx::Arg* args);

protected:

private:

};

SCONESERVER_MODULE(IPModule);

//=========================================================================
IPModule::IPModule(
) : scx::Module("router:ip",scx::version())
{

}

//=========================================================================
IPModule::~IPModule()
{

}

//=========================================================================
std::string IPModule::info() const
{
  return "Router protocol module for TCP/IP version 4";
}

//=========================================================================
int IPModule::init()
{
  return Module::init();
}

//=============================================================================
scx::Arg* IPModule::arg_lookup(const std::string& name)
{
  // Methods
  if ("addr" == name) {
    return new_method(name);
  }      

  return SCXBASE Module::arg_lookup(name);
}

//=============================================================================
scx::Arg* IPModule::arg_method(
  const scx::Auth& auth, 
  const std::string& name,
  scx::Arg* args
)
{
  if ("addr" == name) {
    if (!auth.trusted()) return new scx::ArgError("Not permitted");
    return new IPSocketAddress(args);
  }

  return SCXBASE Module::arg_method(auth,name,args);
}

