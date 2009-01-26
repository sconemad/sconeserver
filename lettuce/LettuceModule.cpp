/* SconeServer (http://www.sconemad.com)

Lettuce module

Copyright (c) 2000-2008 Andrew Wedgbury <wedge@sconemad.com>

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

#include "LettuceModule.h"
#include "LettuceCommandStream.h"
#include "LettuceMediaStream.h"

#include "sconex/ModuleInterface.h"
#include "sconex/Arg.h"

SCONESERVER_MODULE(LettuceModule);

//=========================================================================
LettuceModule::LettuceModule(
) : scx::Module("lettuce",scx::version())
{

}

//=========================================================================
LettuceModule::~LettuceModule()
{

}

//=========================================================================
std::string LettuceModule::info() const
{
  return "Copyright (c) 2000-2008 Andrew Wedgbury\n"
         "Services for the Lettuce automation platform\n"
         "See http://sconemad.com/lettuce for more details\n";
}

//=========================================================================
int LettuceModule::init()
{
  return Module::init();
}

//=========================================================================
bool LettuceModule::connect(
  scx::Descriptor* endpoint,
  scx::ArgList* args
)
{
  const scx::ArgString* a_service =
    dynamic_cast<const scx::ArgString*>(args->get(0));
  std::string service;
  if (a_service) {
    service = a_service->get_string();
  }

  if ("stream" == service) {
    LettuceMediaStream* s = new LettuceMediaStream(*this);
    s->add_module_ref(ref());
    endpoint->set_timeout(scx::Time(60));
    endpoint->add_stream(s);
    return true;

  } else {
    LettuceCommandStream* s = new LettuceCommandStream(*this);
    s->add_module_ref(ref());
    endpoint->set_timeout(scx::Time(60));
    endpoint->add_stream(s);
    return true;

  }

  return false;
}
