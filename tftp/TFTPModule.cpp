/* SconeServer (http://www.sconemad.com)

Trivial File Transfer Protocol (TFTP) module

See RFC1350

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


#include "TFTPModule.h"
#include "TFTPStream.h"
#include "TFTPProfile.h"

#include "sconex/ModuleInterface.h"
#include "sconex/Arg.h"

SCONESERVER_MODULE(TFTPModule);

//=========================================================================
TFTPModule::TFTPModule()
  : scx::Module("tftp",scx::version())
{

}

//=========================================================================
TFTPModule::~TFTPModule()
{  
  for (ProfileMap::const_iterator it = m_profiles.begin();
       it != m_profiles.end();
       it++) {
    delete it->second;
  }
}

//=========================================================================
std::string TFTPModule::info() const
{
  return "Copyright (c) 2000-2007 Andrew Wedgbury\n"
         "Trivial File Transfer Protocol (TFTP)\n";
}

//=========================================================================
int TFTPModule::init()
{
  return Module::init();
}

//=========================================================================
bool TFTPModule::connect(
  scx::Descriptor* endpoint,
  scx::ArgList* args
)
{
  const scx::ArgString* channel =
    dynamic_cast<const scx::ArgString*>(args->get(0));
  if (!channel) {
    log("No TFTP profile specified, aborting connection");
    return false;
  }

  TFTPStream* s = new TFTPStream(*this,channel->get_string());
  s->add_module_ref(ref());
  
  endpoint->add_stream(s);
  return true;
}

//=========================================================================
TFTPProfile* TFTPModule::find_profile(const std::string& name)
{
  ProfileMap::const_iterator it = m_profiles.find(name);
  
  if (it != m_profiles.end()) {
    return it->second;
  }
  
  return 0;
}

//=============================================================================
scx::Arg* TFTPModule::arg_lookup(const std::string& name)
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
    for (ProfileMap::const_iterator it = m_profiles.begin();
	 it != m_profiles.end();
	 ++it) {
      oss << (*it).first << "\n";
    }
    return new scx::ArgString(oss.str());
  }

  // Sub-objects
  
  TFTPProfile* profile = find_profile(name);
  if (profile) {
    return new scx::ArgObject(profile);
  }
  
  return SCXBASE Module::arg_lookup(name);
}

//=============================================================================
scx::Arg* TFTPModule::arg_function(
  const scx::Auth& auth,
  const std::string& name,
  scx::Arg* args
)
{
  scx::ArgList* l = dynamic_cast<scx::ArgList*>(args);
  
  if (!auth.admin()) return new scx::ArgError("Not permitted");

  if ("add" == name) {
    std::string s_name;
    const scx::ArgString* a_name =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (a_name) {
      s_name = a_name->get_string();
    } else {
      return new scx::ArgError("tftp::add() Name must be specified");
    }

    // Check profile doesn't already exist
    if (find_profile(s_name)) {
      return new scx::ArgError("ssl::add() Profile '" + s_name +
                               "' already exists");
    }

    m_profiles[s_name] = new TFTPProfile(*this,s_name);
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

    // Remove profile
    TFTPProfile* profile = find_profile(s_name);
    if (!profile) {
      return new scx::ArgError("ssl::remove() Profile '" + s_name +
                               "' does not exist");
    }
      
    delete profile;
    m_profiles.erase(s_name);
    return 0;
  }

  return SCXBASE Module::arg_function(auth,name,args);
}
