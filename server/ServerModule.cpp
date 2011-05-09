/* SconeServer (http://www.sconemad.com)

Server module

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

#include "ServerModule.h"

#include <sconex/ModuleInterface.h>
#include <sconex/Descriptor.h>
#include <sconex/Kernel.h>
#include <sconex/ScriptTypes.h>
#include <sconex/ConfigStream.h>
#include <sconex/TermBuffer.h>
#include <sconex/StreamBuffer.h>
#include <sconex/StreamDebugger.h>
#include <sconex/ScriptEngine.h>

SCONESERVER_MODULE(ServerModule);

//=============================================================================
ServerModule::ServerModule()
  : scx::Module("server",scx::version())
{
  scx::Stream::register_stream("buffer",this);
  scx::Stream::register_stream("config",this);
  scx::Stream::register_stream("term",this);
  scx::Stream::register_stream("sconescript",this);
  scx::Stream::register_stream("debug",this);
}

//=============================================================================
ServerModule::~ServerModule()
{
  scx::Stream::unregister_stream("buffer",this);
  scx::Stream::unregister_stream("config",this);
  scx::Stream::unregister_stream("term",this);
  scx::Stream::unregister_stream("sconescript",this);
  scx::Stream::unregister_stream("debug",this);

  for (ConnectionChainMap::const_iterator it = m_chains.begin();
       it != m_chains.end();
       ++it) {
    delete it->second;
  }
}

//=========================================================================
std::string ServerModule::info() const
{
  return "Handles incoming connections";
}

//=============================================================================
bool ServerModule::connect(scx::Descriptor* endpoint,
			   const scx::ScriptRef* args)
{
  const scx::ScriptString* a_chain =
    scx::get_method_arg<scx::ScriptString>(args,0,"chain");
  if (!a_chain) {
    log("No connection chain specified");
    return false;
  }
  
  ConnectionChain::Ref* chain = find(a_chain->get_string());
  
  if (!chain || !chain->object()->connect(endpoint)) {
    // Connection was not mapped
    return false;
  }
  
  return true;
}

//=============================================================================
void ServerModule::add(const std::string& name, ConnectionChain* c)
{
  m_chains[name] = new ConnectionChain::Ref(c);
}

//=============================================================================
ConnectionChain::Ref* ServerModule::find(const std::string& name)
{
  ConnectionChainMap::const_iterator it = m_chains.find(name);
  if (it != m_chains.end()) {
    return it->second;
  }
  
  return 0;
}

//=============================================================================
scx::ScriptRef* ServerModule::script_op(const scx::ScriptAuth& auth,
					const scx::ScriptRef& ref,
					const scx::ScriptOp& op,
					const scx::ScriptRef* right)
{
  if (scx::ScriptOp::Lookup == op.type()) {
    std::string name = right->object()->get_string();

    // Methods
    if ("add" == name ||
	"remove" == name) {
      return new scx::ScriptMethodRef(ref,name);
    }

    // Properties
    if ("list" == name) {
      scx::ScriptList* list = new scx::ScriptList();
      scx::ScriptRef* list_ref = new scx::ScriptRef(list);
      for (ConnectionChainMap::const_iterator it = m_chains.begin();
	   it != m_chains.end();
	   ++it) {
	list->give(it->second->ref_copy(ref.reftype()));
      }
      return list_ref;
    }
  
    // Sub-objects
    ConnectionChain::Ref* c = find(name);
    if (c) return c->ref_copy(ref.reftype());
  }
	
  return scx::Module::script_op(auth,ref,op,right);
}

//=============================================================================
scx::ScriptRef* ServerModule::script_method(const scx::ScriptAuth& auth,
					    const scx::ScriptRef& ref,
					    const std::string& name,
					    const scx::ScriptRef* args)
{
  if (!auth.admin()) return scx::ScriptError::new_ref("Not permitted");

  if ("add" == name) {
    // Route name
    const scx::ScriptString* a_name =
      scx::get_method_arg<scx::ScriptString>(args,0,"name");
    if (!a_name) {
      return scx::ScriptError::new_ref("server::add() Name must be given");
    }
    std::string s_name = a_name->get_string();

    // Check route doesn't already exist
    if (find(s_name)) {
      return scx::ScriptError::new_ref("server::add() {CHAIN:" + s_name +
				       "} already exists");
    }

    log("Adding {CHAIN:" + s_name + "}");
    add(s_name, new ConnectionChain(s_name,*this) );

    return 0;
  }

  if ("remove" == name) {
    // Route name
    const scx::ScriptString* a_name =
      scx::get_method_arg<scx::ScriptString>(args,0,"name");
    if (!a_name) {
      return scx::ScriptError::new_ref("server::remove() Name must be given");
    }
    std::string s_name = a_name->get_string();

    ConnectionChainMap::iterator it = m_chains.find(s_name);
    if (it == m_chains.end()) {
      return scx::ScriptError::new_ref("server::remove() {CHAIN:" + 
				       s_name + "} not found");
    }
    
    log("Removing {CHAIN:" + s_name + "}");
    delete (*it).second;
    m_chains.erase(it);

    return 0;
  }

  return scx::Module::script_method(auth,ref,name,args);
}

//=============================================================================
void ServerModule::provide(const std::string& type,
			   const scx::ScriptRef* args,
			   scx::Stream*& object)
{
  if ("buffer" == type) {
    const int max = 10*1048576;
    const scx::ScriptInt* a_read =
      scx::get_method_arg<scx::ScriptInt>(args,0,"read");
    if (!a_read) return;
    int read_size = (int)a_read->get_int();
    if (read_size > max) return;

    const scx::ScriptInt* a_write =
      scx::get_method_arg<scx::ScriptInt>(args,1,"write");
    if (!a_write) return;
    int write_size = (int)a_write->get_int();
    if (write_size > max) return;

    object = new scx::StreamBuffer(read_size,write_size);

  } else if ("config" == type) {
    object = new scx::ConfigStream(scx::Kernel::get()->ref());

  } else if ("term" == type) {
    object = new scx::TermBuffer("term");

  } else if ("sconescript" == type) {
    scx::ScriptRef* ctx = new scx::ScriptRef(scx::Kernel::get());
    object = new scx::ScriptEngineExec(scx::ScriptAuth::Admin,ctx);

  } else if ("debug" == type) {
    const scx::ScriptString* a_name =
      scx::get_method_arg<scx::ScriptString>(args,0,"name");
    if (!a_name) return;
    std::string name  = a_name->get_string();
    object = new scx::StreamDebugger(name);
  }
}
