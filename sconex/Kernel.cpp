/* SconeServer (http://www.sconemad.com)

SconeServer kernel

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

#include "Kernel.h"
#include "ConfigStream.h"
#include "Console.h"
#include "Logger.h"
#include "Debug.h"
#include <pwd.h>

namespace scx {

Kernel* Kernel::s_kernel = 0;
  
//=============================================================================
Kernel* Kernel::get()
{
  if (!s_kernel) {
    s_kernel = new Kernel();
  }
  return s_kernel;
}

//=============================================================================
std::string Kernel::info() const
{
  return "Copyright (c) 2000-2005 Andrew Wedgbury\n"
         "An object orientated network server framework\n";
}

//=============================================================================
int Kernel::init() 
{
  // Set default logger
  FilePath path = get_var_path() + std::string(name() + ".log");
  set_logger(new scx::Logger(path.path()));
  
  log("Init " + name() + "-" + version().get_string() + " "
      + scx::build_type() + " built " + scx::build_time().code());
  
  Module::init();
  
  return 0;
}

//=============================================================================
int Kernel::run()
{
  m_state = Run;
  
  while (true) {

    if (m_spinner.spin() < 0) {
      log("Exiting (descriptor table empty)");
      return 1;
    }
    
    if (m_state == Shutdown) {
      log("Exiting");
      return 1;
    }

    if (m_state == Restart) {
      log("Restarting");
      return 0;
    }
  }
  
  return 0;
}

//=============================================================================
void Kernel::connect_config_console()
{
  Console* c = new Console();
  c->add_stream( new ConfigStream(ref(),true) );
  connect(c,0);
}

//=============================================================================
void Kernel::set_logger(Logger* logger)
{
  Module::set_logger(logger);
  Debug::get()->set_logger(logger);
}

//=============================================================================
bool Kernel::connect(
  Descriptor* d,
  ArgList* args
)
{
  m_spinner.add(d);
  //  if (!args || args->size() == 0) return false;
  return true;
}

//=============================================================================
Arg* Kernel::arg_lookup(const std::string& name)
{
  // Methods

  if ("restart" == name ||
      "shutdown" == name ||
      "set_user" == name) {
    return new ArgObjectFunction(
      new ArgModule(ref()),name);
  }      

  // Properties

  if ("lsdes" == name) {
    return new scx::ArgString(m_spinner.describe());
  }
  if ("root" == name) {
    return new scx::ArgInt(geteuid() == 0);
  }
  
  return Module::arg_lookup(name);
}

//=============================================================================
Arg* Kernel::arg_function(
  const std::string& name,
  Arg* args
)
{
  scx::ArgList* l = dynamic_cast<scx::ArgList*>(args);

  if ("restart" == name) {
    log("Restart requested via command");
    m_state = Restart;
    return 0;
  }

  if ("shutdown" == name) {
    log("Shutdown requested via command");
    m_state = Shutdown;
    return 0;
  }

  if ("set_user" == name) {
    struct passwd* pwent = 0;
    const scx::ArgString* a_name =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    const scx::ArgInt* a_uid =
      dynamic_cast<const scx::ArgInt*>(l->get(0));
    if (a_name) {
      pwent = getpwnam(a_name->get_string().c_str());
      if (!pwent) {
        return new ArgError("set_user() Unknown username '" +
                            a_name->get_string() + "'");
      }
    } else if (a_uid) {
      pwent = getpwuid(a_uid->get_int());
      if (!pwent) {
        return new ArgError("set_user() Unknown user id '" +
                            a_uid->get_string() + "'");
      }
    } else {
      return new ArgError("set_user() Invalid argument");
    }

    std::ostringstream oss;
    oss << "Setting effective user and group ids to " << 
      pwent->pw_uid << ":" << pwent->pw_gid;
    log(oss.str());
    
    if (0 != ::seteuid(pwent->pw_uid)) {
      log("Cannot set user id",Logger::Error);
    }
    if (0 != ::setegid(pwent->pw_gid)) {
      log("Cannot set group id",Logger::Error);
    }
    return 0;
  }
  
  return Module::arg_function(name,args);
}

//=============================================================================
Kernel::Kernel() 
  : Module("sconeserver",scx::version()),
    m_state(Init)
{

}

//=============================================================================
Kernel::~Kernel()
{

}

};
