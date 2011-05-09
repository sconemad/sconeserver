/* SconeServer (http://www.sconemad.com)

External program execution Module

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

#include "ExecModule.h"
#include "ExecStream.h"

#include <sconex/ModuleInterface.h>
#include <sconex/Module.h>
#include <sconex/ScriptTypes.h>
#include <sconex/Process.h>

SCONESERVER_MODULE(ExecModule);

//=========================================================================
ExecModule::ExecModule()
  : scx::Module("exec",scx::version()),
    m_exec_user(scx::User::current())
{
  scx::Stream::register_stream("exec",this);

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
  scx::Stream::unregister_stream("exec",this);
}

//=========================================================================
std::string ExecModule::info() const
{
  return "External program execution and http CGI";
}

//=============================================================================
scx::ScriptRef* ExecModule::script_op(const scx::ScriptAuth& auth,
				      const scx::ScriptRef& ref,
				      const scx::ScriptOp& op,
				      const scx::ScriptRef* right)
{
  if (scx::ScriptOp::Lookup == op.type()) {
    std::string name = right->object()->get_string();

    // Methods
    if ("set_exec_user" == name ||
	"exec" == name ||
	"system" == name) {
      return new scx::ScriptMethodRef(ref,name);
    }
  }

  return scx::Module::script_op(auth,ref,op,right);
}

//=============================================================================
scx::ScriptRef* ExecModule::script_method(const scx::ScriptAuth& auth,
					  const scx::ScriptRef& ref,
					  const std::string& name,
					  const scx::ScriptRef* args)
{
  if ("set_exec_user" == name) {
    if (!auth.admin()) return scx::ScriptError::new_ref("Not permitted");

    const scx::ScriptString* a_user =
      scx::get_method_arg<scx::ScriptString>(args,0,"value");
    if (!a_user)
      return scx::ScriptError::new_ref("Username must be specified");
    m_exec_user = scx::User(a_user->get_string());

    return 0;
  }

  if ("exec" == name ||
      "system" == name) {
    if (!auth.trusted()) return scx::ScriptError::new_ref("Not permitted");

    const scx::ScriptList* argl = 
      dynamic_cast<const scx::ScriptList*>(args->object());

    const scx::ScriptString* a_prog = 
      dynamic_cast<const scx::ScriptString*>(argl->get(0));

    if (!a_prog) 
      return scx::ScriptError::new_ref("No program name specified");

    scx::Process process(a_prog->get_string());

    // Set arguments
    for (int i=0; i<argl->size(); ++i) {
      process.add_arg( argl->get(i)->object()->get_string() );
    }
    
    process.set_user(m_exec_user);
    
    // Launch the process
    if (!process.launch()) {
      return scx::ScriptError::new_ref("Failed to launch process");
    }

    if ("system" == name) {
      // For system, wait until the command exits
      int code=0;
      while (!process.get_exitcode(code)) {
        usleep(1000); // SPIN
      }
      return scx::ScriptInt::new_ref(code);
    }

    return 0;
  }
  
  return scx::Module::script_method(auth,ref,name,args);
}

//=============================================================================
const scx::User& ExecModule::get_exec_user() const
{
  return m_exec_user;
}

//=========================================================================
void ExecModule::provide(const std::string& type,
			 const scx::ScriptRef* args,
			 scx::Stream*& object)
{
  const scx::ScriptList* argl = 
    dynamic_cast<const scx::ScriptList*>(args->object());

  const int max_args = 64;
  if (argl->size() >= max_args) {
    log("Too many arguments to exec call",scx::Logger::Error);
    return;
  }

  int i;
  scx::ScriptList* eargs = new scx::ScriptList();
  for (i=0; i<argl->size(); ++i) {
    const scx::ScriptRef* cur = argl->get(i);
    eargs->give(cur->ref_copy());
  }

  // Create the exec stream
  object = new ExecStream(*this,eargs);
  object->add_module_ref(this);
}

