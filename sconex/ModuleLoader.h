/* SconeServer (http://www.sconemad.com)

Module loader base class

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

#ifndef scxModuleLoader_h
#define scxModuleLoader_h

#include "sconex/sconex.h"
#include "sconex/ModuleRef.h"
#include "sconex/FilePath.h"
namespace scx {

class Logger;

//=============================================================================
class SCONEX_API ModuleLoader {

public:

  ModuleLoader(
    const std::string& name,
    Module* parent,
    Module* mod = 0
  );
  // Construct a module loader
  
  virtual ~ModuleLoader();

  const std::string& get_name() const;
  
  FilePath get_path() const;
  
  ModuleRef ref();

  bool is_loaded() const;

  void set_autoload_config(bool onoff);
  void set_conf_path(const FilePath& path);
  
protected:

  virtual void load_module();
  virtual void unload_module();

  std::string m_name;
  FilePath m_config_path;
  bool m_autoload_config;

  Module* m_module;

  void log(const std::string& message);

  Module* m_parent;
};

};
#endif
