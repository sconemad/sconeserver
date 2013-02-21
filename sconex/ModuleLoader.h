/* SconeServer (http://www.sconemad.com)

Sconex module loader

Copyright (c) 2000-2013 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef scxModuleLoader_h
#define scxModuleLoader_h

#include <sconex/sconex.h>
#include <sconex/FilePath.h>
#include <sconex/Module.h>
#include <sconex/Mutex.h>
namespace scx {

class Logger;
class ScriptRef;

//=============================================================================
class SCONEX_API ModuleLoader {
public:

  // Load module given a modconf file
  ModuleLoader(const scx::FilePath& conf,
               Module* parent);

  // Load module given a name
  ModuleLoader(const std::string& name,
               Module* parent);

  ~ModuleLoader();

  bool close();

  const std::string& get_name() const;
  
  FilePath get_path() const;
  
  Module::Ref get_module();

  typedef std::vector<std::string> Depends;
  const Depends& get_depends() const;
  
  bool is_loaded() const;

protected:

  bool load_module();
  bool unload_module();

  void parse_conf();
  void parse_conf_param(const std::string& param,
                        const std::string& value);
  
  // Load/unload the dll or shared object
  bool load_dll(const std::string& filename);
  bool unload_dll();

  // Locate the named symbol, returning a pointer to
  // its entry point
  void* locate_symbol(const std::string& name) const;

  void log(const std::string& message);

  std::string m_name;
  FilePath m_conf;

  void* m_dll;

  Module::Ref* m_module;

  Depends m_depends;

  Mutex m_mutex;

  Module* m_parent;
};

};
#endif
