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

#include <sconex/ScriptTypes.h>
#include <sconex/Logger.h>
#include <sconex/ModuleInterface.h>
#include <sconex/Process.h>
#include <sconex/ScriptExpr.h>

namespace smtp {

SCONESERVER_MODULE(SMTPModule);

//=========================================================================
SMTPModule::SMTPModule()
  : SCXBASE Module("smtp",scx::version()),
    m_server(0)
{
  scx::ScriptExpr::register_type("SMTPClient",this);
}

//=========================================================================
SMTPModule::~SMTPModule()
{
  scx::ScriptExpr::unregister_type("SMTPClient",this);
  delete m_server;
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
void SMTPModule::close()
{

}
  
//=============================================================================
const scx::ScriptRef* SMTPModule::get_server() const
{
  return m_server;
}

//=============================================================================
scx::StreamSocket* SMTPModule::new_server_connection()
{
  if (!m_server) {
    DEBUG_LOG("Mail server not configured");
    return 0;
  }

  // The server can be:
  // - A socket address, in which case we connect via a socket and return it.
  // - A string, in which case we launch the specified process and return a 
  //   pipe to it.
  const scx::SocketAddress* server_addr = 
    dynamic_cast<const scx::SocketAddress*>(m_server->object());

  const scx::ScriptString* server_str = 
    dynamic_cast<const scx::ScriptString*>(m_server->object());

  if (server_addr) {
    // Server is specifed with a socket address
    scx::StreamSocket* sock = new scx::StreamSocket();

    // Initiate socket connection
    scx::Condition err = sock->connect(server_addr);
    if (err != scx::Ok && err != scx::Wait) {
      DEBUG_LOG("Unable to initiate connection to mail server");
      delete sock;
      return false;
    }
    return sock;
    
  } else if (server_str) {
    // Server is specified by a local process
    scx::Process* proc = new scx::Process();
    proc->parse_command_line(server_str->get_string());
    proc->set_user(scx::User::current());
    
    // Launch process
    if (!proc->launch()) {
      DEBUG_LOG("Unable to launch mail server process");
      delete proc;
      return false;
    }

    return proc;
  }

  return 0;
}
  
//=============================================================================
scx::ScriptRef* SMTPModule::script_op(const scx::ScriptAuth& auth,
				      const scx::ScriptRef& ref,
				      const scx::ScriptOp& op,
				      const scx::ScriptRef* right)
{
  if (op.type() == scx::ScriptOp::Lookup) {
    const std::string name = right->object()->get_string();

    // Methods
    if ("Client" == name ||
	"set_server" == name) {
      return new scx::ScriptMethodRef(ref,name);
    }
    
    // Properties
    if ("server" == name) {
      if (!m_server) return 0;
      return m_server->ref_copy(scx::ScriptRef::ConstRef);
    }
  }

  return scx::Module::script_op(auth,ref,op,right);
}

//=============================================================================
scx::ScriptRef* SMTPModule::script_method(const scx::ScriptAuth& auth,
					  const scx::ScriptRef& ref,
					  const std::string& name,
					  const scx::ScriptRef* args)
{
  if ("Client" == name) {
    scx::ScriptObject* object = 0;
    provide("SMTPClient",args,object);
    return new scx::ScriptRef(object);
  }

  if ("set_server" == name) {
    const scx::ScriptRef* new_server = 
      scx::get_method_arg_ref(args,0,"value");
    delete m_server;
    m_server = new_server->new_copy();
    return 0;
  }

  return scx::Module::script_method(auth,ref,name,args);
}

//=============================================================================
void SMTPModule::provide(const std::string& type,
			 const scx::ScriptRef* args,
			 scx::ScriptObject*& object)
{
  object = new SMTPClient(*this,args);
}

};
