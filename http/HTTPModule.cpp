/* SconeServer (http://www.sconemad.com)

HTTP (HyperText Transfer Protocol) Module

Copyright (c) 2000-2004 Andrew Wedgbury <wedge@sconemad.com>

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

#include "http/HTTPModule.h"
#include "http/ConnectionStream.h"
#include "http/Host.h"
#include "http/Client.h"

#include "sconex/Arg.h"
#include "sconex/Logger.h"
#include "sconex/ModuleInterface.h"
#include "sconex/ModuleLoaderDLL.h"
#include "sconex/StreamDebugger.h"

namespace http {

SCONESERVER_MODULE(HTTPModule);

//=========================================================================
HTTPModule::HTTPModule()
  : SCXBASE Module("http",scx::version()),
    m_hosts(*this),
    m_realms(*this),
    m_sessions(*this),
    m_idle_timeout(30)
{

}

//=========================================================================
HTTPModule::~HTTPModule()
{

}

//=========================================================================
std::string HTTPModule::info() const
{
  return "Hypertext Transfer Protocol (Web server)";
}

//=========================================================================
int HTTPModule::init()
{
  return Module::init();
}

//=========================================================================
bool HTTPModule::connect(
  scx::Descriptor* endpoint,
  scx::ArgList* args
)
{
  const scx::ArgString* a_profile =
    dynamic_cast<const scx::ArgString*>(args->get(0));
  std::string profile = (a_profile ? a_profile->get_string() : "default");

  ConnectionStream* s = new ConnectionStream(*this,profile);
  s->add_module_ref(ref());

  // Set idle timeout for connections
  endpoint->set_timeout(scx::Time(m_idle_timeout));

  endpoint->add_stream(s);

  //  endpoint->add_stream(new scx::StreamDebugger("https-con"));
  return true;
}

//=============================================================================
HostMapper& HTTPModule::get_hosts()
{
  return m_hosts;
}

//=========================================================================
AuthRealmManager& HTTPModule::get_realms()
{
  return m_realms;
}

//=========================================================================
SessionManager& HTTPModule::get_sessions()
{
  return m_sessions;
}

//=============================================================================
scx::Arg* HTTPModule::arg_lookup(const std::string& name)
{
  // Methods
  
  if ("set_idle_timeout" == name ||
      "set_client_proxy" == name ||
      "Client" == name) {
    return new_method(name);
  }

  // Properties
  
  if ("idle_timeout" == name) return new scx::ArgInt(m_idle_timeout);
  if ("client_proxy" == name) return m_client_proxy.ref_copy(scx::Arg::ConstRef);
  
  // Sub-objects
  
  if ("hosts" == name) return new scx::ArgObject(&m_hosts);
  if ("realms" == name) return new scx::ArgObject(&m_realms);
  if ("sessions" == name) return new scx::ArgObject(&m_sessions);

  return SCXBASE Module::arg_lookup(name);
}

//=============================================================================
scx::Arg* HTTPModule::arg_method(
  const scx::Auth& auth,
  const std::string& name,
  scx::Arg* args
)
{
  scx::ArgList* l = dynamic_cast<scx::ArgList*>(args);

  if ("set_idle_timeout" == name) {
    const scx::ArgInt* a_timeout = dynamic_cast<const scx::ArgInt*>(l->get(0));
    if (!a_timeout) return new scx::ArgError("set_timeout() Must specify timeout value");
    int n_timeout = a_timeout->get_int();
    if (n_timeout < 0) return new scx::ArgError("set_timeout() Timeout value must be >= 0");
    m_idle_timeout = n_timeout;
    return 0;
  }

  if ("set_client_proxy" == name) {
    const scx::Uri* a_proxy = dynamic_cast<const scx::Uri*>(l->get(0));
    if (!a_proxy) return new scx::ArgError("set_client_proxy() Must specify proxy Uri");
    m_client_proxy = *a_proxy;
    return 0;
  }
  
  if ("Client" == name) {
    const scx::ArgString* method = dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!method) return new scx::ArgError("No method specified");
    
    const scx::Uri* uri = dynamic_cast<const scx::Uri*>(l->get(1));
    if (!uri) return new scx::ArgError("No uri specified");
    
    return new Client(*this,method->get_string(),*uri);
  }

  return SCXBASE Module::arg_method(auth,name,args);
}

//=============================================================================
unsigned int HTTPModule::get_idle_timeout() const
{
  return m_idle_timeout;
}

//=============================================================================
const scx::Uri& HTTPModule::get_client_proxy() const
{
  return m_client_proxy;
}
  
};
