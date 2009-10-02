/* SconeServer (http://www.sconemad.com)

Router module

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

#include "RouterModule.h"
#include "RouterChain.h"

#include "sconex/ModuleInterface.h"
#include "sconex/Descriptor.h"
#include "sconex/Kernel.h"

SCONESERVER_MODULE(RouterModule);

//=============================================================================
RouterModule::RouterModule()
  : scx::Module("router",scx::version())
{

}

//=============================================================================
RouterModule::~RouterModule()
{
  for (RouterChainMap::const_iterator it = m_chains.begin();
       it != m_chains.end();
       ++it) {
    delete it->second;
  }
}

//=========================================================================
std::string RouterModule::info() const
{
  return "Connection router";
}

//=============================================================================
bool RouterModule::connect(
  scx::Descriptor* endpoint,
  scx::ArgList* args
)
{
  const scx::ArgString* a_chain =
    dynamic_cast<const scx::ArgString*>(args->get(0));
  if (!a_chain) {
    log("router::connect() No router chain specified");
    return false;
  }

  RouterChain* chain = find(a_chain->get_string());
  
  if (!chain || !chain->connect(endpoint)) {
    // Connection was not mapped
    return false;
  }

  return true;
}

//=============================================================================
void RouterModule::add(const std::string& name, RouterChain* c)
{
  m_chains[name] = c;
}

//=============================================================================
RouterChain* RouterModule::find(const std::string& name)
{
  RouterChainMap::const_iterator it = m_chains.find(name);
  if (it != m_chains.end()) {
    return it->second;
  }
  
  return 0;
}

//=============================================================================
scx::Arg* RouterModule::arg_lookup(
  const std::string& name
)
{
  // Methods

  if ("add" == name ||
      "remove" == name) {
    return new scx::ArgObjectFunction(new scx::ArgModule(ref()),name);
  }

  // Properties

  if ("list" == name) {
    scx::ArgList* list = new scx::ArgList();
    for (RouterChainMap::const_iterator it = m_chains.begin();
	 it != m_chains.end();
	 ++it) {
      list->give(new scx::ArgObject(it->second));
    }
    return list;
  }
  
  // Sub-objects

  RouterChain* c = find(name);
  if (c) {
    return new scx::ArgObject(c);
  }

  return SCXBASE Module::arg_lookup(name);
}

//=============================================================================
scx::Arg* RouterModule::arg_function(
  const scx::Auth& auth,
  const std::string& name,
  scx::Arg* args
)
{
  scx::ArgList* l = dynamic_cast<scx::ArgList*>(args);

  if (!auth.admin()) return new scx::ArgError("Not permitted");

  if ("add" == name) {
    // Route name
    const scx::ArgString* a_name =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_name) {
      return new scx::ArgError("router::add() Name must be specified");
    }
    std::string s_name = a_name->get_string();

    // Check route doesn't already exist
    if (find(s_name)) {
      return new scx::ArgError("router::add() {ROUTE:" + s_name +
                               "} already exists");
    }

    log("Adding {ROUTE:" + s_name + "}");
    add(s_name, new RouterChain(s_name,*this) );

    return 0;
  }

  if ("remove" == name) {

    // Route name
    const scx::ArgString* a_name =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_name) {
      return new scx::ArgError("router::remove() Name must be specified");
    }
    std::string s_name = a_name->get_string();

    RouterChainMap::iterator it = m_chains.find(s_name);
    if (it == m_chains.end()) {
      return new scx::ArgError("router::remove() {ROUTE:" + s_name + "} not found");
    }
    
    log("Removing {ROUTE:" + s_name + "}");
    delete (*it).second;
    m_chains.erase(it);

    return 0;
  }

  return SCXBASE Module::arg_function(auth,name,args);
}
