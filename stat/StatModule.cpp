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
#include "StatChannel.h"
#include "StatStream.h"

#include "sconex/ModuleInterface.h"
#include "sconex/Module.h"
#include "sconex/Arg.h"
#include "sconex/Stream.h"

SCONESERVER_MODULE(StatModule);

//=========================================================================
StatModule::StatModule(
)
  : scx::Module("stat",scx::version())
{

}

//=========================================================================
StatModule::~StatModule()
{
  for (ChannelMap::iterator it = m_channels.begin();
       it != m_channels.end();
       ++it) {
    delete (*it).second;
  }
}

//=========================================================================
std::string StatModule::info() const
{
  return "Copyright (c) 2000-2009 Andrew Wedgbury\n"
         "Connection and I/O statistics\n";
}

//=========================================================================
bool StatModule::connect(
  scx::Descriptor* endpoint,
  scx::ArgList* args
)
{
  const scx::ArgString* channel =
    dynamic_cast<const scx::ArgString*>(args->get(0));
  if (!channel) {
    log("No channel specified, stat counter not deployed");
    return true;
  }

  StatStream* s = new StatStream(*this,channel->get_string());
  s->add_module_ref(ref());
  
  endpoint->add_stream(s);
  return true;
}

//=========================================================================
void StatModule::add_stats(
  const std::string& channel,
  StatChannel::StatType type,
  long count
)
{
  StatChannel* c = find_channel(channel);
  if (c) {
    c->add_stats(type,count);
  }
}

//=========================================================================
StatChannel* StatModule::find_channel(const std::string& name)
{
  ChannelMap::const_iterator it = m_channels.find(name);
  if (it != m_channels.end()) {
    return (*it).second;
  }
  
  return 0;
}

//=========================================================================
StatChannel* StatModule::add_channel(const std::string& name)
{
  if (m_channels.count(name) == 0) {
    m_channels[name] = new StatChannel(name);
  }
  
  return m_channels[name];
}

//=========================================================================
bool StatModule::remove_channel(const std::string& name)
{
  StatChannel* channel = find_channel(name);

  if (channel) {
    delete channel;
    m_channels.erase(name);
    return true;
  }

  return false;
}

//=============================================================================
scx::Arg* StatModule::arg_lookup(const std::string& name)
{
  // Methods

  if ("add" == name ||
      "remove" == name) {
    return new scx::ArgObjectFunction(
      new scx::ArgModule(ref()),name);
  }      

  // Properties
  
  if ("list" == name) {
    std::ostringstream oss;
    for (ChannelMap::const_iterator it = m_channels.begin();
	 it != m_channels.end();
	 ++it) {
      oss << (*it).first << "\n";
    }
    return new scx::ArgString(oss.str());
  }

  if ("print" == name) {
    std::ostringstream oss;
    oss << "   CHANNEL    NUM-CON   BYTES-IN  BYTES-OUT\n";
    for (ChannelMap::const_iterator it = m_channels.begin();
	 it != m_channels.end();
	 ++it) {

      StatChannel* channel = (*it).second;
      scx::Arg* connections = channel->arg_lookup("connections");
      scx::Arg* input = channel->arg_lookup("input");
      scx::Arg* output = channel->arg_lookup("output");

      oss << std::setw(10) << (*it).first << " : "
          << std::setw(8) << connections->get_string() << " "
          << std::setw(10) << input->get_string() << " "
          << std::setw(10) << output->get_string() << "\n";
      
      delete connections;
      delete input;
      delete output;
    }
    return new scx::ArgString(oss.str());
  }

  // Sub-objects
  
  StatChannel* channel = find_channel(name);
  if (channel) {
    return new scx::ArgObject(channel);
  }
  
  return SCXBASE Module::arg_lookup(name);
}

//=============================================================================
scx::Arg* StatModule::arg_function(
  const std::string& name,
  scx::Arg* args
)
{
  scx::ArgList* l = dynamic_cast<scx::ArgList*>(args);
  
  if ("add" == name) {
    std::string s_name;
    const scx::ArgString* a_name =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (a_name) {
      s_name = a_name->get_string();
    } else {
      return new scx::ArgError("stat::add() Name must be specified");
    }

    // Check channel doesn't already exist
    if (find_channel(s_name)) {
      return new scx::ArgError("stat::add() Channel '" + s_name +
                               "' already exists");
    }
   
    add_channel(s_name);
    return 0;
  }
  
  if ("remove" == name) {
    std::string s_name;
    const scx::ArgString* a_name =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (a_name) {
      s_name = a_name->get_string();
    } else {
      return new scx::ArgError("stat::remove() Name must be specified");
    }

    // Remove channel
      if (!remove_channel(s_name)) {
      return new scx::ArgError("stat::remove() Channel '" + s_name +
                               "' does not exist");
    }
    return 0;
  }

  return SCXBASE Module::arg_function(name,args);
}
