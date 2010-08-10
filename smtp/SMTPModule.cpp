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
#include "sconex/Process.h"

namespace smtp {

SCONESERVER_MODULE(SMTPModule);

//=========================================================================
SMTPModule::SMTPModule()
  : SCXBASE Module("smtp",scx::version()),
    m_server(0)
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
void SMTPModule::close()
{
  if (m_server) {
    delete m_server;
    m_server = 0;
  }
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
  
  if ("Client" == name ||
      "set_server" == name) {
    return new_method(name);
  }

  // Properties
  if ("server" == name) {
    if (!m_server) return 0;
    return m_server->ref_copy(scx::Arg::ConstRef);
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
    return new Client(*this,l);
  }

  if ("set_server" == name) {
    scx::Arg* new_server = l->take(0);
    delete m_server;
    m_server = new_server;
    return 0;
  }

  return SCXBASE Module::arg_method(auth,name,args);
}

//=============================================================================
const scx::Arg* SMTPModule::get_server() const
{
  return m_server;
}

//=============================================================================
scx::StreamSocket* SMTPModule::new_server_connection()
{
  const scx::SocketAddress* server_addr = dynamic_cast<const scx::SocketAddress*>(m_server);
  const scx::ArgString* server_str = dynamic_cast<const scx::ArgString*>(m_server);

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
  
};
