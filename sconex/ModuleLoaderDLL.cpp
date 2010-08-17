/* SconeServer (http://www.sconemad.com)

SconeX dynamic module loader

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

#include "sconex/ModuleLoaderDLL.h"
#include "sconex/Module.h"
#include "sconex/FileStat.h"
#include "sconex/FilePath.h"

#include <dlfcn.h>
#include <sys/types.h>

namespace scx {

typedef Module* (*PROC_SCONESERVER_MODULE) (void);
const char* SCONESERVER_PROC_NAME = "SconeServer_module";

//=============================================================================
ModuleLoaderDLL::ModuleLoaderDLL(
  const std::string& name,
  Module* parent
) : ModuleLoader(name,parent),
    m_dll(0)
{

}

//=============================================================================
ModuleLoaderDLL::~ModuleLoaderDLL()
{
  unload_module();
}

//=============================================================================
void ModuleLoaderDLL::load_module()
{
  // Try and locate the module
  FilePath path;
  int i=0;
  do {
    switch (++i) {

      case 1:
        // modpath/module.so
        path = get_path() +
               FilePath(m_name + ".so");
        break;

      case 2:
        // modpath/module/.libs/module.so (DEV)
        path = get_path() +
               FilePath(m_name) +
               FilePath(".libs") +
               FilePath(m_name + ".so");
        break;
        
      case 3:
        // modpath/parent/module/.libs/module.so (DEV)
        if (m_parent) {
	  Module* mod = m_parent;
	  FilePath parent_path;
	  while (mod && mod->m_parent) {
	    parent_path = FilePath(mod->name()) + parent_path;
	    mod = mod->m_parent;
	  }
	  path = get_path() +
	         FilePath(parent_path) +
	         FilePath(m_name) +
	         FilePath(".libs") +
	         FilePath(m_name + ".so");
	}
        break;

      default: return; // Can't find it anywhere
    }
  } while (!FileStat(path).is_file());

  if (!load_dll(path.path())) {
    return;
  }

  PROC_SCONESERVER_MODULE proc =
    (PROC_SCONESERVER_MODULE)locate_symbol(SCONESERVER_PROC_NAME);

  if (proc==0) {
    unload_dll();
    return;
  }

  m_module = proc();
}

//=============================================================================
void ModuleLoaderDLL::unload_module()
{
  MutexLocker locker(m_mutex);
  
  if (m_module) {
    DEBUG_ASSERT(m_module->get_num_refs() == 0,
                "unload_module() Deleting referenced module");

    m_module->close();
    delete m_module;
    m_module=0;
  }

  unload_dll();
}

//=============================================================================
bool ModuleLoaderDLL::load_dll(
  const std::string& filename
)
{
  if (m_dll!=0) {
    return false;
  }

  m_dll = dlopen(filename.c_str(),RTLD_LAZY | RTLD_GLOBAL); //RTLD_NOW);

  if (m_dll==0) {
    return false;
  }

  // Success
  return true;
}

//=============================================================================
bool ModuleLoaderDLL::unload_dll()
{
  if (m_dll==0) {
    return false;
  }

  dlclose(m_dll);
  m_dll=0;

  return true;
}

//=============================================================================
void* ModuleLoaderDLL::locate_symbol(
  const std::string& name
) const
{
  DEBUG_ASSERT(m_dll!=0,"locate_symbol() no module loaded");

  return (void*)dlsym(m_dll,name.c_str());
}

};
