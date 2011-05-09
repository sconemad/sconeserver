/* SconeServer (http://www.sconemad.com)

File MIME type lookup module

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


#include "MIMEModule.h"

#include <sconex/ModuleInterface.h>
#include <sconex/MimeType.h>
#include <sconex/ScriptTypes.h>

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
scx::ScriptRef* MIMEModule::script_op(const scx::ScriptAuth& auth,
				      const scx::ScriptRef& ref,
				      const scx::ScriptOp& op,
				      const scx::ScriptRef* right)
{
  if (op.type() == scx::ScriptOp::Lookup) {
    const std::string name = right->object()->get_string();

    // Methods
    if ("lookup" == name ||
	"add" == name ||
	"remove" == name) {
      return new scx::ScriptMethodRef(ref,name);
    }      
    
    if ("list" == name) {
      scx::ScriptMap* map = new scx::ScriptMap();
      for (MimeMap::const_iterator it = m_mimemap.begin();
	   it != m_mimemap.end();
	   it++) {
	map->give(it->first,new scx::ScriptRef(new scx::MimeType(it->second)));
      }
      return new scx::ScriptRef(map);
    }
  }

  return scx::Module::script_op(auth,ref,op,right);
}

//=============================================================================
scx::ScriptRef* MIMEModule::script_method(const scx::ScriptAuth& auth,
					  const scx::ScriptRef& ref,
					  const std::string& name,
					  const scx::ScriptRef* args)
{
  if ("lookup" == name) {
    const scx::ScriptString* a_filename =
      scx::get_method_arg<scx::ScriptString>(args,0,"filename");
    if (!a_filename)
      return scx::ScriptError::new_ref("Filename must be specified");
    std::string key=a_filename->get_string();
    
    int bailout=100;
    std::string::size_type idot;
    while (--bailout > 0) {
      
      MimeMap::const_iterator it = m_mimemap.find(key);
      if (it != m_mimemap.end()) {
        return new scx::ScriptRef(new scx::MimeType(it->second));
      }
      
      if (key.size()<=0 || key=="*") {
        return new scx::ScriptRef(new scx::MimeType("")); // No match
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
    return scx::ScriptError::new_ref("Pattern match bailout");
  }

  if ("add" == name) {
    if (!auth.admin()) return scx::ScriptError::new_ref("Not permitted");

    const scx::ScriptString* a_pattern =
      scx::get_method_arg<scx::ScriptString>(args,0,"pattern");
    if (!a_pattern)
      return scx::ScriptError::new_ref("Pattern must be specified");
    std::string s_pattern = a_pattern->get_string();

    const scx::ScriptString* a_type =
      scx::get_method_arg<scx::ScriptString>(args,1,"type");
    if (!a_type)
      return scx::ScriptError::new_ref("Type must be specified");
    std::string s_type = a_type->get_string();

    m_mimemap[s_pattern] = s_type;
    return 0;
  }
  
  if ("remove" == name) {
    if (!auth.admin()) return scx::ScriptError::new_ref("Not permitted");

    const scx::ScriptString* a_pattern =
      scx::get_method_arg<scx::ScriptString>(args,0,"pattern");
    if (!a_pattern)
      return scx::ScriptError::new_ref("Pattern must be specified");
    std::string s_pattern = a_pattern->get_string();

    m_mimemap.erase(s_pattern);
    return 0;
  }

  return scx::Module::script_method(auth,ref,name,args);
}
