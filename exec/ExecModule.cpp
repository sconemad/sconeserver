/* SconeServer (http://www.sconemad.com)

External program execution Module

Copyright (c) 2000-2006 Andrew Wedgbury <wedge@sconemad.com>

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

#include "ExecModule.h"
#include "ExecStream.h"

#include "sconex/ModuleInterface.h"
#include "sconex/Module.h"
#include "sconex/Arg.h"
#include "sconex/Process.h"

SCONESERVER_MODULE(ExecModule);

//=========================================================================
ExecModule::ExecModule(
)
  : scx::Module("exec",scx::version()),
    m_exec_user(scx::User::current())
{
  // Default to "nobody" if running as root
  if (m_exec_user.get_user_id() == 0) {
    if (!m_exec_user.set_user_name("nobody")) {
      DEBUG_LOG("Failed to set user");
    }
  }
}

//=========================================================================
ExecModule::~ExecModule()
{

}

//=========================================================================
std::string ExecModule::info() const
{
  return "External program execution and http CGI";
}

//=========================================================================
bool ExecModule::connect(
  scx::Descriptor* endpoint,
  scx::ArgList* args
)
{
  const int max_args = 64;
  if (args->size() >= max_args) {
    log("Too many arguments to exec call",scx::Logger::Error);
    return false;
  }

  int i;
  scx::ArgList* eargs = new scx::ArgList();
  for (i=0; i<args->size(); ++i) {
    const scx::Arg* cur = args->get(i);
    eargs->give(cur->new_copy());
  }

  ExecStream* s = new ExecStream(*this,eargs);
  s->add_module_ref(ref());

  endpoint->add_stream(s);
  return true;
}

//=============================================================================
scx::Arg* ExecModule::arg_lookup(const std::string& name)
{
  // Methods

  if ("set_exec_user" == name ||
      "exec" == name ||
      "system" == name) {
    return new_method(name);
  }      

  return SCXBASE Module::arg_lookup(name);
}

//=============================================================================
scx::Arg* ExecModule::arg_method(
  const scx::Auth& auth,
  const std::string& name,
  scx::Arg* args
)
{
  scx::ArgList* l = dynamic_cast<scx::ArgList*>(args);

  if ("set_exec_user" == name) {
    if (!auth.admin()) return new scx::ArgError("Not permitted");

    const scx::ArgString* a_user =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_user) {
      return new scx::ArgError("exec::set_exec_user() "
                               "Username must be specified");
    }
    m_exec_user = scx::User(a_user->get_string());

    return 0;
  }

  if ("exec" == name ||
      "system" == name) {
    if (!auth.trusted()) return new scx::ArgError("Not permitted");

    const scx::ArgString* a_prog = dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_prog) return new scx::ArgError("No program name specified");

    scx::Process process(a_prog->get_string());

    // Set arguments
    for (int i=0; i<l->size(); ++i) {
      process.add_arg( l->get(i)->get_string() );
    }
    
    process.set_user(m_exec_user);
    
    // Launch the process
    if (!process.launch()) {
      return new scx::ArgError("Failed to launch process");
    }

    if ("system" == name) {
      // For system, wait until the command exits
      int code=0;
      while (!process.get_exitcode(code)) {
        usleep(1000); // SPIN
      }
      return new scx::ArgInt(code);
    }

    return 0;
  }
  
  return SCXBASE Module::arg_method(auth,name,args);
}

//=============================================================================
const scx::User& ExecModule::get_exec_user() const
{
  return m_exec_user;
}
