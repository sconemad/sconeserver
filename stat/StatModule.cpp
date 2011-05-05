/* SconeServer (http://www.sconemad.com)

Statistics Module

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


#include "StatModule.h"
#include "StatStream.h"

#include "sconex/ModuleInterface.h"
#include "sconex/Module.h"
#include "sconex/ScriptTypes.h"
#include "sconex/Stream.h"

SCONESERVER_MODULE(StatModule);

//=========================================================================
StatModule::StatModule()
  : scx::Module("stat",scx::version())
{
  scx::Stream::register_stream("stat",this);
}

//=========================================================================
StatModule::~StatModule()
{
  scx::Stream::unregister_stream("stat",this);

  for (ChannelMap::iterator it = m_channels.begin();
       it != m_channels.end();
       ++it) {
    delete it->second;
  }
}

//=========================================================================
std::string StatModule::info() const
{
  return "Connection and I/O statistics";
}

//=========================================================================
StatChannel* StatModule::find_channel(const std::string& name)
{
  ChannelMap::const_iterator it = m_channels.find(name);
  if (it != m_channels.end()) {
    return it->second->object();
  }
  
  return 0;
}

//=========================================================================
StatChannel* StatModule::add_channel(const std::string& name)
{
  StatChannel* channel = find_channel(name);

  if (!channel) {
    channel = new StatChannel(name);
    m_channels[name] = new StatChannel::Ref(channel);
  }
  
  return channel;
}

//=========================================================================
bool StatModule::remove_channel(const std::string& name)
{
  ChannelMap::iterator it = m_channels.find(name);
  if (it != m_channels.end()) {
    delete it->second;
    m_channels.erase(it);
    return true;
  }

  return false;
}

//=============================================================================
scx::ScriptRef* StatModule::script_op(const scx::ScriptAuth& auth,
				      const scx::ScriptRef& ref,
				      const scx::ScriptOp& op,
				      const scx::ScriptRef* right)
{
  if (op.type() == scx::ScriptOp::Lookup) {
    const std::string name = right->object()->get_string();
    
    // Methods
    if ("add" == name ||
	"remove" == name ||
	"print" == name) {
      return new scx::ScriptMethodRef(ref,name);
    }      

    // Properties
    if ("channels" == name) {
      scx::ScriptList* list = new scx::ScriptList();
      for (ChannelMap::const_iterator it = m_channels.begin();
	   it != m_channels.end();
	   ++it) {
	list->give(it->second->ref_copy(ref.reftype()));
      }
      return new scx::ScriptRef(list);
    }
    
    // Sub-objects
    
    StatChannel* channel = find_channel(name);
    if (channel)
      return new scx::ScriptRef(channel);
  }
    
  return scx::Module::script_op(auth,ref,op,right);
}

//=============================================================================
scx::ScriptRef* StatModule::script_method(const scx::ScriptAuth& auth,
					  const scx::ScriptRef& ref,
					  const std::string& name,
					  const scx::ScriptRef* args)
{
  if (!auth.admin()) return scx::ScriptError::new_ref("Not permitted");

  if ("add" == name) {
    const scx::ScriptString* a_name =
      scx::get_method_arg<scx::ScriptString>(args,0,"name");
    if (!a_name) 
      return scx::ScriptError::new_ref("Name must be specified");
    std::string s_name = a_name->get_string();

    // Check channel doesn't already exist
    if (find_channel(s_name))
      return scx::ScriptError::new_ref("Channel '" + s_name + 
				       "' already exists");

    // Add the channel
    StatChannel* channel = add_channel(s_name);
    return new scx::ScriptRef(channel);
  }
  
  if ("remove" == name) {
    const scx::ScriptString* a_name =
      scx::get_method_arg<scx::ScriptString>(args,0,"name");
    if (!a_name) 
      return scx::ScriptError::new_ref("Name must be specified");
    std::string s_name = a_name->get_string();

    // Remove channel
    if (!remove_channel(s_name)) {
      return scx::ScriptError::new_ref("Channel '" + s_name +
				       "' does not exist");
    }
    return 0;
  }

  if ("print" == name) {
    const scx::ScriptList* argl =
      dynamic_cast<const scx::ScriptList*>(args->object());
    scx::ScriptList default_argl;
    if (!argl || argl->size() == 0) {
      // Use these types as default if none were given
      default_argl.give(scx::ScriptString::new_ref("conn"));
      default_argl.give(scx::ScriptString::new_ref("in"));
      default_argl.give(scx::ScriptString::new_ref("out"));
      argl = &default_argl;
    }

    int wc = 14;
    int ws = 8;
    std::ostringstream oss;
    // Heading
    oss << "\n" 
	<< std::right
	<< std::setw(wc) 
	<< "CHANNEL" << " | ";
    for (int i=0; i<argl->size(); ++i) {
      const scx::ScriptRef* type = argl->get(i);
      oss << std::left
	  << std::setw(ws) 
	  << type->object()->get_string() << " ";
    }
    oss << "\n";

    // Stats
    for (ChannelMap::const_iterator it = m_channels.begin();
	 it != m_channels.end();
	 ++it) {
      
      StatChannel* channel = (*it).second->object();
      oss << std::right
	  << std::setw(wc)
	  << (*it).first << " | ";
      for (int i=0; i<argl->size(); ++i) {
	const scx::ScriptRef* type = argl->get(i);
	long stat = channel->get_stat(type->object()->get_string());
	oss << std::left
	    << std::setw(ws)
	    << stat << " ";
      }
      oss << "\n";
    }
    return scx::ScriptString::new_ref(oss.str());
  }
    
  return scx::Module::script_method(auth,ref,name,args);
}

//=========================================================================
void StatModule::provide(const std::string& type,
			 const scx::ScriptRef* args,
			 scx::Stream*& object)
{
  const scx::ScriptString* s_channel =
    scx::get_method_arg<scx::ScriptString>(args,0,"channel");
  if (!s_channel) {
    log("No channel specified, stat counter not deployed");
    return;
  }

  // Make sure there is a channel
  StatChannel* channel = add_channel(s_channel->get_string());

  object = new StatStream(*this,channel);
  object->add_module_ref(this);
}

