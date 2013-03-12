/* SconeServer (http://www.sconemad.com)

Connection chain

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

#include "ConnectionChain.h"
#include "ServerListener.h"
#include "ServerDatagramMultiplexer.h"
#include "ServerModule.h"

#include <sconex/Kernel.h> 
#include <sconex/ListenerSocket.h>
#include <sconex/DatagramSocket.h>
#include <sconex/Descriptor.h>
#include <sconex/Module.h>
#include <sconex/ScriptTypes.h>
#include <sconex/Stream.h>
#include <sconex/Log.h>

#define LOG(msg) scx::Log("server").attach("id", m_name).submit(msg);

//=============================================================================
ConnectionChain::ConnectionChain(ServerModule* module,
				 const std::string& name)
  : m_module(module),
    m_name(name)
{
  m_parent = module;
}

//=============================================================================
ConnectionChain::~ConnectionChain()
{
  for (ConnectionNodeList::iterator it = m_nodes.begin();
       it != m_nodes.end();
       ++it) {
    delete (*it);
  }
}

//=============================================================================
bool ConnectionChain::connect(scx::Descriptor* d)
{
  for (ConnectionNodeList::iterator it = m_nodes.begin();
       it != m_nodes.end();
       ++it) {
    if (!(*it)->connect(d)) {
      return false;
    }
  }
  return true;
}

//=============================================================================
void ConnectionChain::add(ConnectionNode* n)
{
  m_nodes.push_back(n);
}

//=============================================================================
std::string ConnectionChain::get_string() const
{
  return m_name;
}

//=============================================================================
scx::ScriptRef* ConnectionChain::script_op(const scx::ScriptAuth& auth,
					   const scx::ScriptRef& ref,
					   const scx::ScriptOp& op,
					   const scx::ScriptRef* right)
{
  if (scx::ScriptOp::Lookup == op.type()) {
    const std::string name = right->object()->get_string();

    // Methods
    if ("add" == name ||
	"remove" == name ||
	"listen" == name ||
	"listen_all" == name) {
      return new scx::ScriptMethodRef(ref,name);
    }

    // Properties
 
    if ("list" == name) {
      scx::ScriptList* nodelist = new scx::ScriptList();
      scx::ScriptRef* nodelist_ref = new scx::ScriptRef(nodelist);
      for (ConnectionNodeList::const_iterator it = m_nodes.begin();
	   it != m_nodes.end();
	   ++it) {
	scx::ScriptMap* map = new scx::ScriptMap();
	scx::ScriptRef* map_ref = new scx::ScriptRef(map);
	map->give("type",scx::ScriptString::new_ref((*it)->get_name()));
	const scx::ScriptRef* args = (*it)->get_args();
	if (args) {
	  map->give("args",args->new_copy());
	} else {
	  map->give("args",new scx::ScriptRef(new scx::ScriptList()));
	}
	nodelist->give(map_ref);
      }
      return nodelist_ref;
    }
  }
  
  return scx::ScriptObject::script_op(auth,ref,op,right);
}

//=============================================================================
scx::ScriptRef* ConnectionChain::script_method(const scx::ScriptAuth& auth,
					       const scx::ScriptRef& ref,
					       const std::string& name,
					       const scx::ScriptRef* args)
{
  if (!auth.admin()) return scx::ScriptError::new_ref("Not permitted");

  if ("add" == name) {
    // Stream type
    const scx::ScriptString* a_type =
      scx::get_method_arg<scx::ScriptString>(args,0,"type");
    if (!a_type)
      return scx::ScriptError::new_ref("Stream type must be specified");
    std::string s_type = a_type->get_string();

    // Add the remaining arguments to a new list, to be stored with
    // the connection node.
    std::string logargs;
    scx::ScriptList::Ref* ml = new scx::ScriptList::Ref(new scx::ScriptList());
    int pos=1;
    const scx::ScriptRef* arg = 0;
    while (0 != (arg = scx::get_method_arg_ref(args,pos++))) {
      ml->object()->give(arg->ref_copy());
      if (!logargs.empty()) logargs += ",";
      logargs += arg->object()->get_string();
    }
    
    LOG("Adding " + s_type + "(" + logargs + ")");
    add(new ConnectionNode(m_module.object(), s_type, ml));

    return 0;
  }

  if ("remove" == name) {

    // Stream type
    const scx::ScriptString* a_type =
      scx::get_method_arg<scx::ScriptString>(args,0,"type");
    if (!a_type)
      return scx::ScriptError::new_ref("Stream type must be specified");
    std::string s_type = a_type->get_string();

    ConnectionNodeList::iterator it = m_nodes.begin();
    while (it != m_nodes.end()) {
      if (s_type == (*it)->get_name()) {
        LOG("Removing " + s_type);
        delete (*it);
        m_nodes.erase(it);
        return 0;
      }
      it++;
    }

    return scx::ScriptError::new_ref("Stream '" + s_type + "' not found");
  }

  if ("listen" == name) {

    // Socket address
    const scx::SocketAddress* sa =
      scx::get_method_arg<scx::SocketAddress>(args,0,"address");
    if (!sa) 
      return scx::ScriptError::new_ref("Address must be specified");

    if (sa->socket_type() == SOCK_STREAM) {
      // STREAM socket

      ServerListener* rl = new ServerListener(m_module.object(), m_name);
    
      const int bl = 5;
      scx::ListenerSocket* ls = new scx::ListenerSocket(sa, bl);
      ls->add_stream(rl);
      
      if (ls->listen()) {
	delete ls;
	return scx::ScriptError::new_ref("Unable to bind " + sa->get_string());
      }

      LOG("Listening on " + sa->get_string());
      
      scx::Kernel::get()->connect(ls);

    } else {
      // DATAGRAM socket

      scx::DatagramSocket* ds = new scx::DatagramSocket();

      if (ds->listen(sa)) {
	delete ds;
	return scx::ScriptError::new_ref("Unable to bind " + sa->get_string());
      }
      
      LOG("Listening on " + sa->get_string());
      
      ServerDatagramMultiplexer* mp = 
	new ServerDatagramMultiplexer(m_module.object(), m_name);
      ds->add_stream(mp);
  
      scx::Kernel::get()->connect(ds);
    }
    return 0;
  }

  if ("listen_all" == name) {

    // Socket address
    const scx::SocketAddress* sa =
      scx::get_method_arg<scx::SocketAddress>(args,0,"address");
    if (!sa) 
      return scx::ScriptError::new_ref("Address must be specified");

    scx::DatagramSocket* ds = new scx::DatagramSocket();
    
    if (ds->listen(sa)) {
      delete ds;
      return scx::ScriptError::new_ref("Unable to bind " + sa->get_string());
    }

    if (connect(ds)) {
      LOG("Listening (all) on " + sa->get_string());
      scx::Kernel::get()->connect(ds);
    } else {
      delete ds;
      return scx::ScriptError::new_ref("Failed to create chain");
    }
  }
  
  return scx::ScriptObject::script_method(auth,ref,name,args);
}

//###---

//=============================================================================
ConnectionNode::ConnectionNode(ServerModule* module,
			       const std::string& type,
			       scx::ScriptRef* args)
  : m_module(module),
    m_type(type),
    m_args(args)
{

}

//=============================================================================
ConnectionNode::~ConnectionNode()
{
  delete m_args;
}

//=============================================================================
bool ConnectionNode::connect(
  scx::Descriptor* d
)
{
  if (m_type == "chain") {

    const scx::ScriptString* chain_name = 
      scx::get_method_arg<scx::ScriptString>(m_args,0,"chain");
    if (!chain_name) {
      DEBUG_LOG("No chain name specified");
      return false;
    }

    ConnectionChain::Ref* chain = 
      m_module.object()->find(chain_name->get_string());
    if (!chain) {
      DEBUG_LOG("Unknown chain '" << chain_name << "'");
      return false;
    }
    return chain->object()->connect(d);
  }

  scx::Stream* stream = scx::Stream::create_new(m_type,m_args);
  if (!stream) {
    DEBUG_LOG("Failed to create stream of type " << m_type <<
	      " args " << m_args->object()->get_string());
    return false;
  }

  d->add_stream(stream);
  return true;
}

//=============================================================================
const std::string& ConnectionNode::get_name() const
{
  return m_type;
}

//=============================================================================
const scx::ScriptRef* ConnectionNode::get_args() const
{
  return m_args;
}
