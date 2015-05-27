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

#include <sconex/ModuleLoader.h>
#include <sconex/Log.h>
#include <sconex/File.h>
#include <sconex/FileStat.h>
#include <sconex/LineBuffer.h>
#include <sconex/ScriptBase.h>
#include <sconex/ScriptTypes.h>
#include <dlfcn.h>
#include <sys/types.h>
namespace scx {

typedef Module* (*PROC_SCONEX_MODULE) (void);
const char* SCONESERVER_PROC_NAME = "sconex_module";

#define LOG(msg) Log("loader").submit(msg);

  
//=============================================================================
ModuleLoader::ModuleLoader(const scx::FilePath& conf,
                           Module* parent)
  : m_name(),
    m_conf(conf),
    m_dll(0),
    m_module(0),
    m_depends(),
    m_mutex(),
    m_parent(parent)
{
  DEBUG_COUNT_CONSTRUCTOR(ModuleLoader);
  parse_conf();
}

//=============================================================================
ModuleLoader::ModuleLoader(const std::string& name,
                           Module* parent)
  : m_name(name),
    m_conf(),
    m_dll(0),
    m_module(0),
    m_depends(),
    m_mutex(),
    m_parent(parent)
{
  DEBUG_COUNT_CONSTRUCTOR(ModuleLoader);
}
	
//=============================================================================
ModuleLoader::~ModuleLoader()
{
  close();
  DEBUG_COUNT_DESTRUCTOR(ModuleLoader);
}

//=============================================================================
bool ModuleLoader::close()
{
  return unload_module();
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
Module::Ref ModuleLoader::get_module()
{
  MutexLocker locker(m_mutex);
  
  if (m_module==0) {
  
    load_module();

    if (m_module) {
      Module* m = m_module->object();
      LOG("Loaded module [" + m_name + "] " +
          m->name() + "-" + m->version().get_string());
      m->set_parent(m_parent);
      m->init();
      if (m_conf.path() != "") {
        m->load_config_file(m_conf);
      }

    } else {
      return Module::Ref(0);
    }

  }

  return *m_module;
}

//=============================================================================
const ModuleLoader::Depends& ModuleLoader::get_depends() const
{
  return m_depends;
}
  
//=============================================================================
bool ModuleLoader::is_loaded() const
{
  return (m_module != 0);
}

//=============================================================================
bool ModuleLoader::load_module()
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
	  while (mod && mod->m_parent_module) {
	    parent_path = FilePath(mod->name()) + parent_path;
	    mod = mod->m_parent_module;
	  }
	  path = get_path() +
	         FilePath(parent_path) +
	         FilePath(m_name) +
	         FilePath(".libs") +
	         FilePath(m_name + ".so");
	}
        break;

      default: // Can't find it anywhere
	LOG("Unable to locate module '"+m_name+"'");
	return false; 
    }
  } while (!FileStat(path).is_file());

  if (!load_dll(path.path())) {
    return false;
  }

  PROC_SCONEX_MODULE proc =
    (PROC_SCONEX_MODULE)locate_symbol(SCONESERVER_PROC_NAME);

  if (proc==0) {
    unload_dll();
    return false;
  }

  m_module = new Module::Ref(proc());
  return true;
}

//=============================================================================
bool ModuleLoader::unload_module()
{
  MutexLocker locker(m_mutex);
  
  if (m_module) {
    if (!m_module->object()->close()) {
      LOG("Unload '" + m_name + "' failed");
      return false;
    }
    int refs = m_module->object()->num_refs();
    if (refs > 1) {
      std::ostringstream oss;
      oss << "Deferring unload of '" << m_name <<"' (" << refs << " refs)";
      LOG(oss.str());
      return false;
    }
    delete m_module;
    m_module = 0;
  }

  unload_dll();
  return true;
}

//=============================================================================
void ModuleLoader::parse_conf()
{
  scx::File file;
  if (scx::Ok == file.open(m_conf, scx::File::Read)) {
    scx::LineBuffer* buffer = new scx::LineBuffer("modconf");
    file.add_stream(buffer);
    std::string line;
    while (scx::Ok == buffer->tokenize(line)) {
      if (line.length() < 2) continue;
      if (line[0] != '#') continue;
      int is = line.find_first_of(":");
      std::string param = line.substr(1,is-1);
      is = line.find_first_not_of(" ", is+1);
      std::string value = line.substr(is);
      parse_conf_param(param, value);
    }
  }
}

//=============================================================================
void ModuleLoader::parse_conf_param(const std::string& param,
                                    const std::string& value)
{
  if ("MODULE" == param) {
    m_name = value;
  } else if ("DEPENDS" == param) {
    m_depends.push_back(value);
  }
}
  
//=============================================================================
bool ModuleLoader::load_dll(const std::string& filename)
{
  if (m_dll!=0) {
    return false;
  }

  m_dll = dlopen(filename.c_str(),RTLD_LAZY | RTLD_GLOBAL); //RTLD_NOW);

  if (m_dll==0) {
    LOG(std::string("dlopen error: ") + dlerror());
    return false;
  }

  // Success
  return true;
}

//=============================================================================
bool ModuleLoader::unload_dll()
{
  if (m_dll==0) {
    return false;
  }

  dlclose(m_dll);
  m_dll=0;

  return true;
}

//=============================================================================
void* ModuleLoader::locate_symbol(const std::string& name) const
{
  DEBUG_ASSERT(m_dll!=0,"locate_symbol() no module loaded");

  return (void*)dlsym(m_dll,name.c_str());
}
  
};
