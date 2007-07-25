/* SconeServer (http://www.sconemad.com)

DSO Module loader

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

#include "sconex/ModuleLoader.h"
#include "sconex/Module.h"
#include "sconex/Logger.h"
namespace scx {

typedef Module* (*PROC_SCONESERVER_MODULE) (void);

//=============================================================================
ModuleLoader::ModuleLoader(
  const std::string& name,
  Module* parent,
  Module* mod
) 
  : m_name(name),
    m_autoload_config(true),
    m_module(mod),
    m_parent(parent)
{
  DEBUG_COUNT_CONSTRUCTOR(ModuleLoader);
}
	
//=============================================================================
ModuleLoader::~ModuleLoader()
{
  DEBUG_COUNT_DESTRUCTOR(ModuleLoader);
}

//=============================================================================
const std::string& ModuleLoader::get_name() const
{
  return m_name;
}

//=============================================================================
FilePath ModuleLoader::get_path() const
{
  if (m_parent) {
    return m_parent->get_mod_path();
  }
  return FilePath("");
}
  
//=============================================================================
ModuleRef ModuleLoader::ref()
{
  MutexLocker locker(m_mutex);
  
  if (m_module==0) {
  
    load_module();

    if (m_module) {
      log("Loaded module [" + m_name + "] " +
          m_module->name() + "-" + m_module->version().get_string());
      m_module->set_parent(m_parent);
      m_module->set_autoload_config(m_autoload_config);
      m_module->set_conf_path(m_config_path);
      m_module->init();

    } else {
      return ModuleRef();
    }

  }

  return m_module->ref();
}

//=============================================================================
bool ModuleLoader::is_loaded() const
{
  return (m_module != 0);
}

//=============================================================================
void ModuleLoader::set_autoload_config(bool onoff)
{
  m_autoload_config = onoff;
}

//=============================================================================
void ModuleLoader::set_conf_path(const FilePath& path)
{
  m_config_path = path;
}

//=============================================================================
void ModuleLoader::load_module()
{
}

//=============================================================================
void ModuleLoader::unload_module()
{
}

//=============================================================================
void ModuleLoader::log(const std::string& message)
{
  if (m_parent) {
    m_parent->log(message);
  }
}

};
