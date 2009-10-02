/* SconeServer (http://www.sconemad.com)

File MIME type lookup module

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


#include "MIMEModule.h"

#include "sconex/ModuleInterface.h"
#include "sconex/Module.h"
#include "sconex/Arg.h"
#include "sconex/MimeType.h"

SCONESERVER_MODULE(MIMEModule);

//=========================================================================
MIMEModule::MIMEModule(
)
  : scx::Module("mime",scx::version())
{

}

//=========================================================================
MIMEModule::~MIMEModule()
{

}

//=========================================================================
std::string MIMEModule::info() const
{
  return "File mimetype lookup module";
}

//=============================================================================
scx::Arg* MIMEModule::arg_lookup(const std::string& name)
{
  // Methods
  if ("lookup" == name ||
      "add" == name ||
      "remove" == name) {
    return new scx::ArgObjectFunction(new scx::ArgModule(ref()),name);
  }      

  if ("list" == name) {
    scx::ArgMap* map = new scx::ArgMap();
    for (MimeMap::const_iterator it = m_mimemap.begin();
	 it != m_mimemap.end();
	 it++) {
      map->give(it->first,new scx::MimeType(it->second));
    }
    return map;
  }

  return SCXBASE Module::arg_lookup(name);
}

//=============================================================================
scx::Arg* MIMEModule::arg_function(
  const scx::Auth& auth,
  const std::string& name,
  scx::Arg* args
)
{
  scx::ArgList* l = dynamic_cast<scx::ArgList*>(args);
  
  if ("lookup" == name) {
    const scx::ArgString* a_filename =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_filename) {
      return new scx::ArgError("mime::lookup() File name must be specified");
    }
    std::string key=a_filename->get_string();
    
    int bailout=100;
    std::string::size_type idot;
    while (--bailout > 0) {
      
      MimeMap::const_iterator it = m_mimemap.find(key);
      if (it != m_mimemap.end()) {
        return new scx::MimeType(it->second);
      }
      
      if (key.size()<=0 || key=="*") {
        return new scx::MimeType(""); // No match
      }
      
      if (key[0]=='*') {
        idot = key.find(".",2);
      } else {
        idot = key.find_first_of(".");
      }
      
      if (idot==key.npos) {
      key="*";
      } else {
        key = "*" + key.substr(idot);
      }
      
    }
    return new scx::ArgError("mime::lookup() Pattern match bailout");
  }

  if ("add" == name) {
    if (!auth.admin()) return new scx::ArgError("Not permitted");

    const scx::ArgString* a_pattern =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_pattern) {
      return new scx::ArgError("mime::add() Pattern must be specified");
    }
    std::string s_pattern = a_pattern->get_string();

    const scx::ArgString* a_type =
      dynamic_cast<const scx::ArgString*>(l->get(1));
    if (!a_type) {
      return new scx::ArgError("mime::add() Type must be specified");
    }
    std::string s_type = a_type->get_string();

    m_mimemap[s_pattern] = s_type;
    return 0;
  }
  
  if ("remove" == name) {
    if (!auth.admin()) return new scx::ArgError("Not permitted");

    const scx::ArgString* a_pattern =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_pattern) {
      return new scx::ArgError("mime::remove() Pattern must be specified");
    }
    std::string s_pattern = a_pattern->get_string();

    m_mimemap.erase(s_pattern);
    return 0;
  }

  return SCXBASE Module::arg_function(auth,name,args);
}
