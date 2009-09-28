/* SconeServer (http://www.sconemad.com)

Router IP6 protocol module (TCP/IP version 6)

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
#include "IP6SocketAddress.h"

//=========================================================================
class IP6Module : public scx::Module {
public:

  IP6Module();
  virtual ~IP6Module();

  virtual std::string info() const;

  virtual int init();
  
  virtual scx::Arg* arg_lookup(const std::string& name);
  virtual scx::Arg* arg_function(const scx::Auth& auth,const std::string& name,scx::Arg* args);

protected:

private:

};

SCONESERVER_MODULE(IP6Module);

//=========================================================================
IP6Module::IP6Module(
) : scx::Module("router:ip6",scx::version())
{

}

//=========================================================================
IP6Module::~IP6Module()
{

}

//=========================================================================
std::string IP6Module::info() const
{
  return "Copyright (c) 2000-2005 Andrew Wedgbury\n"
         "Router protocol module for TCP/IP version 6\n";
}

//=========================================================================
int IP6Module::init()
{
  return Module::init();
}

//=============================================================================
scx::Arg* IP6Module::arg_lookup(const std::string& name)
{
  // Methods
  if ("addr" == name) {
    return new scx::ArgObjectFunction(new scx::ArgModule(ref()),name);
  }      

  return SCXBASE Module::arg_lookup(name);
}

//=============================================================================
scx::Arg* IP6Module::arg_function(
  const scx::Auth& auth, 
  const std::string& name,
  scx::Arg* args
)
{
  if ("addr" == name) {
    if (!auth.trusted()) return new scx::ArgError("Not permitted");
    return new IP6SocketAddress(args);
  }
  
  return SCXBASE Module::arg_function(auth,name,args);
}

