/* SconeServer (http://www.sconemad.com)

SMTP (Simple Mail Transfer Protocol) Module

Copyright (c) 2000-2010 Andrew Wedgbury <wedge@sconemad.com>

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

#include "smtp/SMTPModule.h"
#include "smtp/SMTPClient.h"

#include "sconex/Arg.h"
#include "sconex/Logger.h"
#include "sconex/ModuleInterface.h"

namespace smtp {

SCONESERVER_MODULE(SMTPModule);

//=========================================================================
SMTPModule::SMTPModule()
  : SCXBASE Module("smtp",scx::version())
{

}

//=========================================================================
SMTPModule::~SMTPModule()
{

}

//=========================================================================
std::string SMTPModule::info() const
{
  return "Simple Mail Transfer Protocol client";
}

//=========================================================================
int SMTPModule::init()
{
  return Module::init();
}

//=========================================================================
bool SMTPModule::connect(
  scx::Descriptor* endpoint,
  scx::ArgList* args
)
{
  return false;
}

//=============================================================================
scx::Arg* SMTPModule::arg_lookup(const std::string& name)
{
  // Methods
  
  if ("Client" == name) {
    return new_method(name);
  }

  return SCXBASE Module::arg_lookup(name);
}

//=============================================================================
scx::Arg* SMTPModule::arg_method(
  const scx::Auth& auth,
  const std::string& name,
  scx::Arg* args
)
{
  scx::ArgList* l = dynamic_cast<scx::ArgList*>(args);

  if ("Client" == name) {
    return new Client(*this);
  }

  return SCXBASE Module::arg_method(auth,name,args);
}

};
