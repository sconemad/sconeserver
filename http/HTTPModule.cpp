/* SconeServer (http://www.sconemad.com)

HTTP (HyperText Transfer Protocol) Module

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

#include "http/HTTPModule.h"
#include "http/ConnectionStream.h"
#include "http/Host.h"
#include "http/Client.h"

#include "sconex/ScriptTypes.h"
#include "sconex/Logger.h"
#include "sconex/ModuleInterface.h"
#include "sconex/ModuleLoaderDLL.h"
#include "sconex/StreamDebugger.h"
#include "sconex/ScriptExpr.h"

namespace http {

SCONESERVER_MODULE(HTTPModule);

//=========================================================================
HTTPModule::HTTPModule()
  : scx::Module("http",scx::version()),
    m_hosts(new HostMapper(*this)),
    m_realms(new AuthRealmManager(*this)),
    m_sessions(new SessionManager(*this)),
    m_idle_timeout(30)
{
  scx::Stream::register_stream("http",this);
  scx::ScriptExpr::register_type("HTTPClient",this);
}

//=========================================================================
HTTPModule::~HTTPModule()
{
  scx::Stream::unregister_stream("http",this);
  scx::ScriptExpr::unregister_type("HTTPClient",this);
}

//=========================================================================
std::string HTTPModule::info() const
{
  return "HyperText Transfer Protocol (WWW)";
}

//=========================================================================
int HTTPModule::init()
{
  return Module::init();
}

//=============================================================================
HostMapper& HTTPModule::get_hosts()
{
  return *m_hosts.object();
}

//=========================================================================
AuthRealmManager& HTTPModule::get_realms()
{
  return *m_realms.object();
}

//=========================================================================
SessionManager& HTTPModule::get_sessions()
{
  return *m_sessions.object();
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

//=============================================================================
scx::ScriptRef* HTTPModule::script_op(const scx::ScriptAuth& auth,
				      const scx::ScriptRef& ref,
				      const scx::ScriptOp& op,
				      const scx::ScriptRef* right)
{
  if (op.type() == scx::ScriptOp::Lookup) {
    const std::string name = right->object()->get_string();

    // Methods
    if ("set_idle_timeout" == name ||
	"set_client_proxy" == name ||
	"Client" == name) {
      return new scx::ScriptMethodRef(ref,name);
    }

    // Properties
    if ("idle_timeout" == name) 
      return scx::ScriptInt::new_ref(m_idle_timeout);
    if ("client_proxy" == name) 
      return new scx::ScriptRef(m_client_proxy.new_copy());
  
    // Sub-objects
    if ("hosts" == name) return m_hosts.ref_copy();
    if ("realms" == name) return m_realms.ref_copy();
    if ("sessions" == name) return m_sessions.ref_copy();
  }

  return scx::Module::script_op(auth,ref,op,right);
}

//=============================================================================
scx::ScriptRef* HTTPModule::script_method(const scx::ScriptAuth& auth,
					  const scx::ScriptRef& ref,
					  const std::string& name,
					  const scx::ScriptRef* args)
{
  if ("set_idle_timeout" == name) {
    if (!auth.admin()) return scx::ScriptError::new_ref("Not permitted");

    const scx::ScriptInt* a_timeout = 
      scx::get_method_arg<scx::ScriptInt>(args,0,"value");
    if (!a_timeout) 
      return scx::ScriptError::new_ref("Must specify value");
    int n_timeout = a_timeout->get_int();
    if (n_timeout < 0) 
      return scx::ScriptError::new_ref("Timeout must be >= 0");
    m_idle_timeout = n_timeout;
    return 0;
  }

  if ("set_client_proxy" == name) {
    if (!auth.admin()) return scx::ScriptError::new_ref("Not permitted");

    const scx::Uri* a_proxy = 
      scx::get_method_arg<scx::Uri>(args,0,"value");
    if (!a_proxy) 
      return scx::ScriptError::new_ref("Must specify proxy Uri");
    m_client_proxy = *a_proxy;
    return 0;
  }
  
  if ("Client" == name) {
    scx::ScriptObject* object = 0;
    provide("HTTPClient",args,object);
    return new scx::ScriptRef(object);
  }

  return scx::Module::script_method(auth,ref,name,args);
}

//=========================================================================
void HTTPModule::provide(const std::string& type,
			 const scx::ScriptRef* args,
			 scx::Stream*& object)
{
  const scx::ScriptString* a_profile =
    scx::get_method_arg<scx::ScriptString>(args,0,"profile");
  std::string profile = (a_profile ? a_profile->get_string() : "default");

  object = new ConnectionStream(*this,profile);
  object->add_module_ref(this);
}

//=========================================================================
void HTTPModule::provide(const std::string& type,
			 const scx::ScriptRef* args,
			 scx::ScriptObject*& object)
{
  const scx::ScriptString* method = 
    scx::get_method_arg<scx::ScriptString>(args,0,"method");
  if (!method) {
    object = new scx::ScriptError("No method specified");
    return;
  }
    
  const scx::Uri* uri = 
    scx::get_method_arg<scx::Uri>(args,1,"uri");
  if (!uri) { 
    object = new scx::ScriptError("No uri specified");
    return;
  }
  
  object = new Client(*this,method->get_string(),*uri);
}
  
};
