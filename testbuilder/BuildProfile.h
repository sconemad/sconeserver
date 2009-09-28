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

#ifndef testbuilderBuildProfile_h
#define testbuilderBuildProfile_h

#include "TestBuilderModule.h"

#include "sconex/Stream.h"
#include "sconex/Module.h"
#include "sconex/ArgObject.h"

class Build;

//#########################################################################
class BuildProfile : public scx::ArgObjectInterface {

public:

  BuildProfile(
    TestBuilderModule& module,
    const std::string& name
  );

  ~BuildProfile();

  virtual std::string name() const;
  virtual scx::Arg* arg_lookup(const std::string& name);
  virtual scx::Arg* arg_function(const scx::Auth& auth,const std::string& name,scx::Arg* args);

  const std::string& get_name() const;
  // Get profile name (cannot be set once constructed)
  
  const std::string& get_source_method() const;
  void set_source_method(const std::string& source_method);
  // Get/set source method
  
  const std::string& get_source_uri() const;
  void set_source_uri(const std::string& source_uri);
  // Get/set source URI

  const std::string& get_configure_command() const;
  void set_configure_command(const std::string& configure_command);
  // Get/set configure command
  
  std::string get_make_targets() const;
  void set_make_targets(const std::string& make_targets);
  // Get/set make targets

  Build* create_build(const scx::FilePath& root_dir);
  // Create a runnable build object from this profile

  void save(scx::Descriptor& d);
  // Save the config to the specified descriptor
  
protected:

private:

  TestBuilderModule& m_module;

  std::string m_name;

  std::string m_source_method;
  std::string m_source_uri;

  std::string m_configure_command;
  std::list<std::string> m_make_targets;
  
};

#endif
