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
#include "Build.h"
#include "BuildStep.h"

//=========================================================================
BuildProfile::BuildProfile(
  TestBuilderModule& module,
  const std::string& name
) : m_module(module),
    m_name(name)
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
      "set_make_targets" == name) {
    return new_method(name);
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
    return new scx::ArgString(get_make_targets());
  }
  
  return SCXBASE ArgObjectInterface::arg_lookup(name);
}

//=============================================================================
scx::Arg* BuildProfile::arg_method(
  const scx::Auth& auth,
  const std::string& name,
  scx::Arg* args
)
{
  scx::ArgList* l = dynamic_cast<scx::ArgList*>(args);

  if (!auth.trusted()) return new scx::ArgError("Not permitted");

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
    if ("set_make_targets" == name) {
      set_make_targets(value->get_string());
      return 0;
    }
  }
  
  return SCXBASE ArgObjectInterface::arg_method(auth,name,args);
}

//=============================================================================
Build* BuildProfile::create_build(const scx::FilePath& root_dir)
{
  Build* build = new Build(m_module,m_name);

  // Add source aquisition step
  scx::FilePath source_script =
    m_module.get_dir() + "scripts" + m_source_method;
  build->add_step(new BuildStep(m_module,"source",
                                source_script.path() + " " + m_source_uri)
  );

  // Add configure step
  if (!m_configure_command.empty()) {
    build->add_step(new BuildStep(m_module,"configure",m_configure_command));
  }

  // Add make steps
  for (std::list<std::string>::iterator it = m_make_targets.begin();
       it != m_make_targets.end();
       it++) {
    scx::FilePath make_script =
      m_module.get_dir() + "scripts" + "make";
    build->add_step(new BuildStep(m_module,"make_" + (*it),
                                  make_script.path() + " " + (*it)));
  }

  build->save();
  
  return build;
}

//=============================================================================
const std::string& BuildProfile::get_name() const
{
  return m_name;
}
  
//=============================================================================
const std::string& BuildProfile::get_source_method() const
{
  return m_source_method;
}

//=============================================================================
void BuildProfile::set_source_method(const std::string& source_method)
{
  m_source_method = source_method;
}

//=============================================================================
const std::string& BuildProfile::get_source_uri() const
{
  return m_source_uri;
}

//=============================================================================
void BuildProfile::set_source_uri(const std::string& source_uri)
{
  m_source_uri = source_uri;
}

//=============================================================================
const std::string& BuildProfile::get_configure_command() const
{
  return m_configure_command;
}

//=============================================================================
void BuildProfile::set_configure_command(const std::string& configure_command)
{
  m_configure_command = configure_command;
}

//=============================================================================
std::string BuildProfile::get_make_targets() const
{
  std::string s;
  for (std::list<std::string>::const_iterator it = m_make_targets.begin();
       it != m_make_targets.end();
       it++) {
    s += (it == m_make_targets.begin() ? "" : " ") + (*it);
  }
  return s;
}

//=============================================================================
void BuildProfile::set_make_targets(const std::string& make_targets)
{
  m_make_targets.clear();
  std::string::size_type start = 0;
  while (true) {
    start = make_targets.find_first_not_of(" ",start);
    std::string::size_type end = make_targets.find_first_of(" ",start);
    if (start == std::string::npos) {
      break;
    }
    std::string target;
    if (end == std::string::npos) {
      target = std::string(make_targets,start);
    } else {
      target = std::string(make_targets,start,end-start);
    }
    m_make_targets.push_back(target);
    start = end;
  }
}

//=============================================================================
void BuildProfile::save(scx::Descriptor& d)
{
  d.write("add('" + m_name + "');\n");
  d.write(" " + m_name + ".set_source_method('" + m_source_method + "');\n");
  d.write(" " + m_name + ".set_source_uri('" + m_source_uri + "');\n");
  d.write(" " + m_name + ".set_configure_command('" + m_configure_command + "');\n");
  d.write(" " + m_name + ".set_make_targets('" + get_make_targets() + "');\n");
}
