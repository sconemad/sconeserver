/* SconeServer (http://www.sconemad.com)

Router chain

Copyright (c) 2000-2009 Andrew Wedgbury <wedge@sconemad.com>

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

#include "RouterChain.h"
#include "RouterListener.h"

#include "sconex/Kernel.h" 
#include "sconex/ConfigStream.h"
#include "sconex/TermBuffer.h"
#include "sconex/ListenerSocket.h"
#include "sconex/DatagramSocket.h"
#include "sconex/DatagramMultiplexer.h"
#include "sconex/StreamBuffer.h"
#include "sconex/StreamDebugger.h"
#include "sconex/Descriptor.h"
#include "sconex/ModuleRef.h"
#include "sconex/Module.h"
#include "sconex/ArgScript.h"

//=============================================================================
RouterChain::RouterChain(
  const std::string& name,
  scx::Module& module
)
  : m_name(name),
    m_module(module)
{

}

//=============================================================================
RouterChain::~RouterChain()
{
  for (RouterNodeList::iterator it = m_nodes.begin();
       it != m_nodes.end();
       ++it) {
    delete (*it);
  }
}

//=============================================================================
bool RouterChain::connect(
  scx::Descriptor* d
)
{
  for (RouterNodeList::iterator it = m_nodes.begin();
       it != m_nodes.end();
       ++it) {
    if (!(*it)->connect(d)) {
      return false;
    }
  }
  return true;
}

//=============================================================================
void RouterChain::add(RouterNode* n)
{
  m_nodes.push_back(n);
}

//=============================================================================
std::string RouterChain::name() const
{
  return m_name;
}

//=============================================================================
scx::Arg* RouterChain::arg_lookup(
  const std::string& name
)
{
  // Methods
  
  if ("add" == name ||
      "remove" == name ||
      "listen" == name ||
      "listen_all" == name) {
    return new_method(name);
  }

  // Properties
  
  if ("list" == name) {
    scx::ArgList* nodelist = new scx::ArgList();
    for (RouterNodeList::const_iterator it = m_nodes.begin();
	 it != m_nodes.end();
	 ++it) {
      scx::ArgMap* map = new scx::ArgMap();
      map->give("module",new scx::ArgString((*it)->get_name()));
      const scx::ArgList* args = (*it)->get_args();
      if (args) {
	map->give("args",args->new_copy());
      } else {
	map->give("args",new scx::ArgList());
      }
      nodelist->give(map);
    }
    return nodelist;
  }
  
  return SCXBASE ArgObjectInterface::arg_lookup(name);
}

//=============================================================================
scx::Arg* RouterChain::arg_method(
  const scx::Auth& auth,
  const std::string& name,
  scx::Arg* args
)
{
  scx::ArgList* l = dynamic_cast<scx::ArgList*>(args);

  if (!auth.admin()) return new scx::ArgError("Not permitted");

  if ("add" == name) {
    // Module name
    std::string s_name;
    const scx::ArgString* a_name =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (a_name) {
      s_name = a_name->get_string();
    } else {
      return new scx::ArgError("route::add() Module name must be specified");
    }

    // Transfer the remaining arguments to a new list, to be stored with
    // the route node.
    std::string logargs;
    scx::ArgList* ml = new scx::ArgList();
    while (l->size() > 1) {
      scx::Arg* a = l->take(1);
      DEBUG_ASSERT(a!=0,"add() NULL argument in list");
      ml->give(a);
      if (!logargs.empty()) logargs += ",";
      logargs += a->get_string();
    }
    
    log("Adding " + s_name + "(" + logargs + ")");
    add(new RouterNode(s_name,ml,m_module) );

    return 0;
  }

  if ("remove" == name) {

    // Router name
    std::string s_name;
    const scx::ArgString* a_name =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (a_name) {
      s_name = a_name->get_string();
    } else {
      return new scx::ArgError("route::remove() Name must be specified");
    }

    std::list<RouterNode*>::iterator it = m_nodes.begin();
    while (it != m_nodes.end()) {
      if (s_name == (*it)->get_name()) {
        log("Removing " + s_name);
        delete (*it);
        m_nodes.erase(it);
        return 0;
      }
      it++;
    }

    return new scx::ArgError("route::remove() Route '" +
                             s_name + "' not found");
  }

  if ("listen" == name) {

    // Socket address
    const scx::SocketAddress* sa =
      dynamic_cast<const scx::SocketAddress*>(l->get(0));
    if (!sa) {
      return new scx::ArgError("route::listen() Address must be specified");
    }

    if (sa->socket_type() == SOCK_STREAM) {
      // STREAM socket

      RouterListener* rl = new RouterListener(m_name,m_module);
      rl->add_module_ref(m_module.ref());
    
      const int bl = 5;
      scx::ListenerSocket* ls = new scx::ListenerSocket(sa,bl);
      ls->add_stream(rl);
      
      if (ls->listen()) {
	delete ls;
	return new scx::ArgError("route::listen() Unable to bind " +
				 sa->get_string());
      }

      log("Listening on " + sa->get_string());
      
      // Construct argument list with route name
      scx::ArgList* listener_args = new scx::ArgList();
      listener_args->give(new scx::ArgString(m_name));
      
      scx::Kernel::get()->connect(ls,listener_args);
      
      delete listener_args;

    } else {
      // DATAGRAM socket

      scx::DatagramSocket* ds = new scx::DatagramSocket();

      if (ds->listen(sa)) {
	delete ds;
	return new scx::ArgError("route::listen() Unable to bind " +
				 sa->get_string());
      }

      log("Listening on " + sa->get_string());

      scx::DatagramMultiplexer* mp = 
	new scx::DatagramMultiplexer(m_module,m_name);
      mp->add_module_ref(m_module.ref());
      ds->add_stream(mp);
  
      scx::ArgList* listener_args = new scx::ArgList();
      scx::Kernel::get()->connect(ds,listener_args);
    }
    return 0;
  }

  if ("listen_all" == name) {
    // Socket address
    const scx::SocketAddress* sa =
      dynamic_cast<const scx::SocketAddress*>(l->get(0));
    if (!sa) {
      return new scx::ArgError("route::listen_all() Address must be specified");
    }
    scx::DatagramSocket* ds = new scx::DatagramSocket();
    
    if (ds->listen(sa)) {
      delete ds;
      return new scx::ArgError("route::listen_all() Unable to bind " +
			       sa->get_string());
    }

    if (connect(ds)) {
      log("Listening (all) on " + sa->get_string());
      scx::ArgList* listener_args = new scx::ArgList();
      scx::Kernel::get()->connect(ds,listener_args);
    } else {
      delete ds;
      return new scx::ArgError("route::listen_all() Failed to create chain");
    }
  }
  
  return SCXBASE ArgObjectInterface::arg_method(auth,name,args);
}

//###---

//=============================================================================
RouterNode::RouterNode(
  const std::string& name,
  scx::ArgList* args,
  scx::Module& module
)
  : m_name(name),
    m_args(args),
    m_module(module)
{

}

//=============================================================================
RouterNode::~RouterNode()
{
  delete m_args;
}

//=============================================================================
bool RouterNode::connect(
  scx::Descriptor* d
)
{
  scx::ModuleRef ref = scx::Kernel::get()->get_module(m_name);
  if (ref.valid()) {
    // Connect module
    scx::Module* cmod = ref.module();
    if (!cmod->connect(d,m_args)) {
      // Error occured, disconnect
      return false;
    }
    return true;
  }

  if ("buffer" == m_name) {
    const int max = 10*1048576;
    const scx::ArgInt* a_read =
      dynamic_cast<const scx::ArgInt*>(m_args->get(0));
    if (!a_read) return false;
    int read_size = (int)a_read->get_int();
    if (read_size > max) return false;

    const scx::ArgInt* a_write =
      dynamic_cast<const scx::ArgInt*>(m_args->get(1));
    if (!a_write) return false;
    int write_size = (int)a_write->get_int();
    if (write_size > max) return false;

    d->add_stream( new scx::StreamBuffer(read_size,write_size) );
    return true;  
  }

  if ("config" == m_name) {
    d->add_stream( new scx::ConfigStream(scx::Kernel::get()->ref()) );
    return true;
  }

  if ("term" == m_name) {
    d->add_stream( new scx::TermBuffer("term") );
    return true;
  }

  if ("sconescript" == m_name) {
    scx::ArgModule* ctx = new scx::ArgModule(scx::Kernel::get()->ref());
    d->add_stream( new scx::ArgScript(scx::Auth::Admin,ctx) );
    return true;
  }

  if ("debug" == m_name) {
    const scx::ArgString* a_name =
      dynamic_cast<const scx::ArgString*>(m_args->get(0));
    std::string name;
    if (a_name) {
      name  = a_name->get_string();
    }
    d->add_stream( new scx::StreamDebugger(name) );
    return true;
  }

  return false;
}

//=============================================================================
const std::string& RouterNode::get_name() const
{
  return m_name;
}

//=============================================================================
const scx::ArgList* RouterNode::get_args() const
{
  return m_args;
}
