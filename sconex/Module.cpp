/* SconeServer (http://www.sconemad.com)

Module

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

#include "sconex/Module.h"
#include "sconex/ModuleLoader.h"
#include "sconex/ModuleLoaderDLL.h"
#include "sconex/ConfigFile.h"
#include "sconex/ArgProc.h"
#include "sconex/Logger.h"

namespace scx {

//=============================================================================
Module::Module(
  const std::string& name,
  const VersionTag& version
) : m_name(name),
    m_version(version),
    m_autoload_config(true),
    m_mod_path(""),
    m_conf_path(""),
    m_var_path(""),
    m_parent(0),
    m_logger(0)
{
  DEBUG_COUNT_CONSTRUCTOR(Module);
  m_loadtime = Date::now();
}
	
//=============================================================================
Module::~Module()
{
  for (std::list<ModuleLoader*>::iterator it = m_modules.begin();
       it != m_modules.end();
       it++) {
    delete (*it);
  }

  delete m_logger;

  DEBUG_COUNT_DESTRUCTOR(Module);
}

//=============================================================================
const VersionTag& Module::version() const
{
  return m_version;
}

//=============================================================================
std::string Module::name() const
{
  return m_name;
}

//=============================================================================
int Module::init()
{
  if (m_autoload_config) {
    load_config_file();
  }
  
  return 0;
}

//=============================================================================
ModuleRef Module::ref()
{
  return ModuleRef(this);
}

//=============================================================================
void Module::set_autoload_config(bool onoff)
{
  m_autoload_config = onoff;
}

//=============================================================================
ModuleRef Module::get_module(const std::string& name)
{
  if (name == ".") {
    return ref();
  }
  
  ModuleLoader* loader = find_module(name);
  if (loader) {
    return loader->ref();
  }

  if (m_parent) {
    return m_parent->get_module(name);
  }
  
  return ModuleRef();
}

//=============================================================================
void Module::set_mod_path(const FilePath& path)
{
  m_mod_path = path;
}

//=============================================================================
FilePath Module::get_mod_path() const
{
  if (m_parent) {
    return FilePath(m_parent->get_mod_path() + m_mod_path);
  }
  return m_mod_path;
}

//=============================================================================
void Module::set_conf_path(const FilePath& path)
{
  m_conf_path = path;
}

//=============================================================================
FilePath Module::get_conf_path() const
{
  if (m_parent) {
    return FilePath(m_parent->get_conf_path() + m_conf_path);
  }
  return m_conf_path;
}

//=============================================================================
void Module::set_var_path(const FilePath& path)
{
  m_var_path = path;
}

//=============================================================================
FilePath Module::get_var_path() const
{
  if (m_parent) {
    return FilePath(m_parent->get_var_path() + m_var_path);
  }
  return m_var_path;
}

//=============================================================================
bool Module::connect(Descriptor* endpoint,ArgList* args)
{
  log("ERROR: Module does not support stream connections");
  
  return false;
}

//=============================================================================
Arg* Module::arg_lookup(const std::string& name)
{
  // See if its a function
  if ("set_log" == name ||
      "log" == name ||
      "insmod" == name ||
      "rmmod" == name ||
      "load_config" == name ||
      "set_mod_path" == name ||
      "set_conf_path" == name ||
      "set_var_path" == name ||
      "VER" == name) {
    return new ArgObjectFunction(new ArgModule(ref()),name);
  }

  // See if its a property
  if ("name" == name) {
    return new ArgString(Module::name());
  }

  if ("version" == name) {
    return new VersionTag(version());
  }

  if ("info" == name) {
    std::ostringstream oss;
    oss << m_name << "-" << m_version.get_string() << "\n" << info();
    return new ArgString(oss.str());
  }

  if ("loadtime" == name) {
    return new ArgString(m_loadtime.code());
  }

  if ("lsmod" == name) {
    std::ostringstream oss;
    for (std::list<ModuleLoader*>::const_iterator it = m_modules.begin();
         it != m_modules.end();
         it++) {
      ModuleLoader* loader = (*it);
      oss << loader->get_name() << " ";
      if (loader->is_loaded()) {
        ModuleRef ref = loader->ref();
        oss << ref.module()->get_num_refs() - 1;
      } else {
        oss << "X";
      }
      oss << "\n";
    }
    return new scx::ArgString(oss.str());
  }

  if ("mod_path" == name) {
    return new ArgString(get_mod_path().path());
  }

  if ("conf_path" == name) {
    return new ArgString(get_conf_path().path());
  }

  if ("var_path" == name) {
    return new ArgString(get_var_path().path());
  }
  
  // See if its a sub-module
  ModuleLoader* loader = find_module(name);
  if (loader) {
    return new ArgModule(loader->ref());
  }
  
  return ArgObjectInterface::arg_lookup(name);
}

//=============================================================================
Arg* Module::arg_resolve(const std::string& name)
{
  return ArgObjectInterface::arg_resolve(name);
}

//=============================================================================
Arg* Module::arg_function(
  const std::string& name,
  Arg* args
)
{
  ArgList* l = dynamic_cast<ArgList*>(args);
  DEBUG_ASSERT(l,"arg_function() Invalid arg list");

  if ("set_log" == name) {
    const ArgString* a_file = dynamic_cast<const ArgString*>(l->get(0));
    if (a_file) {
      FilePath path = get_var_path() + a_file->get_string();
      log("Setting log file to '" + path.path() + "'");
      set_logger(new scx::Logger(path.path()));
    } else {
      log("Closing log file");
      set_logger(0);
    }
    return 0;
  }

  if ("log" == name) {
    const ArgString* a_message = dynamic_cast<const ArgString*>(l->get(0));
    const ArgInt* a_level = dynamic_cast<const ArgInt*>(l->get(1));
    Logger::Level level = a_level ?
      (Logger::Level)a_level->get_int() : Logger::Info;
    if (a_message) {
      log(a_message->get_string(),level);
    }
    return 0;
  }

  if ("insmod" == name) {
    const ArgString* a_name = dynamic_cast<const ArgString*>(l->get(0));
    if (!a_name) {
      return new ArgError("insmod() No module specified");
    }
    std::string name = a_name->get_string();

    // Check module entry doesn't already exist
    ModuleLoader* existing_loader = find_module(name);
    if (existing_loader) {
      if (existing_loader->is_loaded()) {
        return new ArgError("insmod() Module [" + name +
                            "] already loaded and in use");
      }
      // The old module is not loaded - probably invalid,
      // remove to let the new one take its place.
      remove_module(existing_loader);
    }

    // Parse any options
    std::string opts;
    const ArgString* a_opts = dynamic_cast<const ArgString*>(l->get(1));
    if (a_opts) {
      opts = a_opts->get_string();
    }
    bool opt_noconfig = (opts.find("noconfig") != opts.npos);
    bool opt_delay = (opts.find("delay") != opts.npos);

    log(
      "Adding module [" + name + "]"
      + (opt_noconfig ? " (no configuration)" : "")
      + (opt_delay ? " (delayed loading)" : "")
    );
    
    ModuleLoaderDLL* loader = new ModuleLoaderDLL(name,this);
    add_module(loader);
    
    const ArgString* a_conf_path = dynamic_cast<const ArgString*>(l->get(2));
    if (a_conf_path) {
      loader->set_conf_path(a_conf_path->get_string());
    }
    
    if (opt_noconfig) {
      // Surpress autoloading of config during init
      loader->set_autoload_config(false);
    }
    
    if (!opt_delay) {
      // Cause the module to be loaded immediately
      ModuleRef r = loader->ref();
      if (!r.valid()) {
        return new ArgError("insmod() Cannot load module [" + name + "]");      
      }
    }
    return 0;
  }
  
  if ("rmmod" == name) {
    const ArgString* a_name = dynamic_cast<const ArgString*>(l->get(0));
    if (!a_name) {
      return new ArgError("rmmod() No module specified");
    }

    std::string name = a_name->get_string();
    ModuleLoader* loader = find_module(name);
    if (!loader) {
      return new ArgError("rmmod() Module [" + name + "] not found");
    }

    if (loader->is_loaded()) {
      ModuleRef ref = loader->ref();
      int nr = ref.module()->get_num_refs() - 1;
      if (nr > 0) {
        return new ArgError("rmmod() Module [" + name + "] in use");
      }
    }
    
    log("Removing module [" + name + "]");
    remove_module(loader);
    return 0;
  }

  if ("load_config" == name) {
    const ArgString* a_path = dynamic_cast<const ArgString*>(l->get(0));
    FilePath path;
    if (a_path) {
      path = a_path->get_string();
    }
    if (false == load_config_file(path)) {
      return new ArgError("load_config() Error occured");
    }
    return 0;
  }

  if ("set_mod_path" == name) {
    const ArgString* a_path = dynamic_cast<const ArgString*>(l->get(0));
    if (!a_path) {
      return new ArgString(get_mod_path().path());
    }
    m_mod_path = a_path->get_string();
    return 0;
  }

  if ("set_conf_path" == name) {
    const ArgString* a_path = dynamic_cast<const ArgString*>(l->get(0));
    if (!a_path) {
      return new ArgString(get_conf_path().path());
    }
    m_conf_path = a_path->get_string();
    return 0;
  }

  if ("set_var_path" == name) {
    const ArgString* a_path = dynamic_cast<const ArgString*>(l->get(0));
    if (!a_path) {
      return new ArgString(get_var_path().path());
    }
    m_var_path = a_path->get_string();
    return 0;
  }

  if ("VER" == name) {
    return new VersionTag(args);
  }

  return ArgObjectInterface::arg_function(name,args);
}

//=============================================================================
Arg* Module::arg_eval(const std::string& expr)
{
  ArgModule argmod(ref());
  ArgProc evaluator(&argmod);
  return evaluator.evaluate(expr);
}

//=============================================================================
void Module::add_module(ModuleLoader* loader)
{
  if (loader) {
    m_modules.push_back(loader);
  }
}

//=============================================================================
void Module::remove_module(ModuleLoader* loader)
{
  if (loader) {
    m_modules.remove(loader);
    delete loader;
  }
}

//=============================================================================
ModuleLoader* Module::find_module(const std::string& name)
{
  for (std::list<ModuleLoader*>::iterator it = m_modules.begin();
       it != m_modules.end();
       it++) {
    ModuleLoader* loader = (*it);
    if (loader && loader->get_name() == name) {
      return loader;
    }
  }
  return 0;
}

//=============================================================================
bool Module::load_config_file(FilePath path)
{
  if (path.path().empty()) {
    // Look for config file in standard locations
    int i=0;
    do {
      switch (++i) {
        case 1:
          // confpath/module.conf
          path = get_conf_path() +
                 FilePath(m_name + ".conf");
          break;
        case 2:
          // confpath/module/module.conf (DEV)
          path = get_conf_path() +
                 FilePath(m_name) + 
                 FilePath(m_name + ".conf");
          break;
        case 3:
          // confpath/parent/module/module.conf (DEV)
          if (!m_parent) continue;
          path = get_conf_path() +
                 FilePath(m_parent->name()) + 
                 FilePath(m_name) + 
                 FilePath(m_name + ".conf");
          break;
          
        default: return false; // Can't find conf file
      }
    } while (!FileStat(path).is_file());
  }

  ConfigFile config(path);
  ModuleRef modref = ref();
  ArgModule* ctx = new ArgModule(modref);
  return config.load(ctx);
}

//=============================================================================
void Module::log(const std::string& message,Logger::Level level)
{
  log_string("[" + m_name + "] " + message,level);
}

//=============================================================================
void Module::log_string(const std::string& str,Logger::Level level)
{
  if (m_logger) {
    m_logger->log(str,level);
  } else if (m_parent && m_parent != this) {
    m_parent->log_string(str,level);
  }
}

//=============================================================================
void Module::set_logger(Logger* logger)
{
  delete m_logger;
  m_logger = logger;
}

//=============================================================================
void Module::set_parent(Module* parent)
{
  m_parent = parent;
}

//=============================================================================
ArgModule::ArgModule(
  ModuleRef ref
)
  : ArgObject(ref.module()),
    m_ref(ref)
{
  DEBUG_ASSERT(ref.valid(),"ArgModule::ArgModule() Invalid module reference");
}

//=============================================================================
ArgModule::~ArgModule()
{

}

//=============================================================================
std::string ArgModule::get_string() const
{
  std::ostringstream oss;
  oss << "MODULE: "
      << (m_ref.valid() ? m_ref.module()->name() : "NULL");

  return oss.str();
}

//===========================================================================
int ArgModule::get_int() const
{
  return (m_ref.valid() ? 1 : 0);
}

};
