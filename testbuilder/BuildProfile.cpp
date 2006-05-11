/* SconeServer (http://www.sconemad.com)

Test Build Profile

Copyright (c) 2000-2006 Andrew Wedgbury <wedge@sconemad.com>

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


#include "BuildProfile.h"

//=========================================================================
BuildProfile::BuildProfile(
  const std::string& name
)
  : m_name(name)
{
  
}

//=========================================================================
BuildProfile::~BuildProfile()
{

}

//=============================================================================
std::string BuildProfile::name() const
{
  return std::string("PROFILE:") + m_name;
}

//=============================================================================
scx::Arg* BuildProfile::arg_lookup(
  const std::string& name
)
{
  // Methods

  if ("set_source_method" == name ||
      "set_source_uri" == name ||
      "set_configure_command" == name ||
      "add_make_target" == name ||
      "remove_make_target" == name) {
    return new scx::ArgObjectFunction(new scx::ArgObject(this),name);
  }

  // Properties
  
  if ("source_method" == name) {
    return new scx::ArgString(m_source_method);
  }
  if ("source_uri" == name) {
    return new scx::ArgString(m_source_uri);
  }
  if ("configure_command" == name) {
    return new scx::ArgString(m_configure_command);
  }
  if ("make_targets" == name) {
    scx::ArgList* trgs = new scx::ArgList();
    for (std::list<std::string>::iterator it = m_make_targets.begin();
	 it != m_make_targets.end();
	 it++) {
      trgs->give( new scx::ArgString(*it) );
    }
    return trgs;
  }
  
  return SCXBASE ArgObjectInterface::arg_lookup(name);
}

//=============================================================================
scx::Arg* BuildProfile::arg_function(
  const std::string& name,
  scx::Arg* args
)
{
  scx::ArgList* l = dynamic_cast<scx::ArgList*>(args);

  const scx::Arg* value = l->get(0);
  if (value) {
    if ("set_source_method" == name) {
      m_source_method = value->get_string();
      return 0;
    }
    if ("set_source_uri" == name) {
      m_source_uri = value->get_string();
      return 0;
    }
    if ("set_configure_command" == name) {
      m_configure_command = value->get_string();
      return 0;
    }
    if ("add_make_target" == name) {
      m_make_targets.push_back(value->get_string());
      return 0;
    }
  }
  
  return SCXBASE ArgObjectInterface::arg_function(name,args);
}
