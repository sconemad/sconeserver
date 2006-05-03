/* SconeServer (http://www.sconemad.com)

SSL connection module

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


#include "SSLModule.h"
#include "SSLStream.h"
#include "SSLChannel.h"

#include "sconex/ModuleInterface.h"
#include "sconex/StreamDebugger.h"
#include "sconex/Arg.h"

#include <openssl/ssl.h>

SCONESERVER_MODULE(SSLModule);

//=========================================================================
SSLModule::SSLModule(
)
  : scx::Module("ssl",scx::version())
{

}

//=========================================================================
SSLModule::~SSLModule()
{  
  for (
    std::map<std::string,SSLChannel*>::const_iterator it = m_channels.begin();
    it != m_channels.end();
    it++) {
    delete it->second;
  }
}

//=========================================================================
std::string SSLModule::info() const
{
  std::ostringstream oss;
  oss << "Copyright (c) 2000-2004 Andrew Wedgbury\n"
      << "Secure socket layer\n"
      << "Using " << SSLeay_version(SSLEAY_VERSION) << "\n";
  return oss.str();
}

//=========================================================================
int SSLModule::init()
{
  SSL_load_error_strings();
  SSL_library_init();

  return Module::init();
}

//=========================================================================
bool SSLModule::connect(
  scx::Descriptor* endpoint,
  scx::ArgList* args
)
{
  const scx::ArgString* channel =
    dynamic_cast<const scx::ArgString*>(args->get(0));
  if (!channel) {
    log("No SSL channel specified, aborting connection");
    return false;
  }

  SSLStream* s = new SSLStream(*this,channel->get_string());
  s->add_module_ref(ref());
  
  endpoint->add_stream(s);
  return true;
}

//=========================================================================
SSLChannel* SSLModule::find_channel(const std::string& name)
{
  std::map<std::string,SSLChannel*>::const_iterator it =
    m_channels.find(name);
  
  if (it != m_channels.end()) {
    return it->second;
  }
  
  return 0;
}

//=============================================================================
scx::Arg* SSLModule::arg_lookup(const std::string& name)
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
    std::map<std::string,SSLChannel*>::const_iterator it = m_channels.begin();
    while (it != m_channels.end()) {
      oss << (*it).first << "\n";
      it++;
    }
    return new scx::ArgString(oss.str());
  }

  // Sub-objects
  
  SSLChannel* channel = find_channel(name);
  if (channel) {
    return new scx::ArgObject(channel);
  }
  
  return SCXBASE Module::arg_lookup(name);
}

//=============================================================================
scx::Arg* SSLModule::arg_function(
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
      return new scx::ArgError("ssl::add() Name must be specified");
    }

    // Check channel doesn't already exist
    if (find_channel(s_name)) {
      return new scx::ArgError("ssl::add() Channel '" + s_name +
                               "' already exists");
    }

    m_channels[s_name] = new SSLChannel(*this,s_name);
    return 0;
  }
  
  if ("remove" == name) {
    std::string s_name;
    const scx::ArgString* a_name =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (a_name) {
      s_name = a_name->get_string();
    } else {
      return new scx::ArgError("ssl::remove() Name must be specified");
    }

    // Remove channel
    SSLChannel* channel = find_channel(s_name);
    if (!channel) {
      return new scx::ArgError("ssl::remove() Channel '" + s_name +
                               "' does not exist");
    }
      
    delete channel;
    m_channels.erase(s_name);
    return 0;
  }

  return SCXBASE Module::arg_function(name,args);
}
