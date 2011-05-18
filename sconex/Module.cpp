/* SconeServer (http://www.sconemad.com)

Module

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

#include <sconex/Module.h>
#include <sconex/ModuleLoader.h>
#include <sconex/ModuleLoaderDLL.h>
#include <sconex/ConfigFile.h>
#include <sconex/ScriptExpr.h>
#include <sconex/Logger.h>
#include <sconex/FileStat.h>
#include <sconex/FileDir.h>

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
    m_parent_module(0),
    m_logger(0)
{
  DEBUG_COUNT_CONSTRUCTOR(Module);
  m_loadtime = Date::now();
}
	
//=============================================================================
Module::~Module()
{
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
std::string Module::copyright() const
{
  return sconeserver_copyright();
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
bool Module::close()
{
  bool done = true;
  m_modules.reverse();
  std::list<ModuleLoader*>::iterator it;
  for (it = m_modules.begin(); it != m_modules.end(); it++) {
    ModuleLoader* loader = *it;
    if (loader->close()) {
      delete loader;
      it = m_modules.erase(it);
    } else {
      done = false;
    }
  }
  m_modules.reverse();
  return done;
}

//=============================================================================
void Module::set_autoload_config(bool onoff)
{
  m_autoload_config = onoff;
}

//=============================================================================
Module::Ref Module::get_module(const std::string& name)
{
  ModuleLoader* loader = find_module(name);
  if (loader) {
    return loader->get_module();
  }

  if (m_parent_module) {
    return m_parent_module->get_module(name);
  }
  
  return Module::Ref(0);
}

//=============================================================================
void Module::set_mod_path(const FilePath& path)
{
  m_mod_path = path;
}

//=============================================================================
FilePath Module::get_mod_path() const
{
  if (m_parent_module) {
    return FilePath(m_parent_module->get_mod_path() + m_mod_path);
  }
  return m_mod_path;
}

//=============================================================================
void Module::set_conf_path(const FilePath& path)
{
  if (FileStat(path).is_file()) {
    std::string pathstr = path.path();
    std::string::size_type is = pathstr.find_last_of("/");
    if (is != std::string::npos) {
      m_conf_path = pathstr.substr(0,is);
      m_conf_file = pathstr.substr(is+1);
    }
  } else {
    m_conf_path = path;
  }
}

//=============================================================================
FilePath Module::get_conf_path() const
{
  if (m_parent_module) {
    return FilePath(m_parent_module->get_conf_path() + m_conf_path);
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
  if (m_parent_module) {
    return FilePath(m_parent_module->get_var_path() + m_var_path);
  }
  return m_var_path;
}

//=============================================================================
std::string Module::get_string() const
{
  return m_name;
}

//=============================================================================
ScriptRef* Module::script_op(const ScriptAuth& auth,
			     const ScriptRef& ref,
			     const ScriptOp& op,
			     const ScriptRef* right)
{
   if (op.type() == ScriptOp::Lookup) {
    std::string name = right->object()->get_string();

    // See if its a method
    if ("set_log" == name ||
	"log" == name ||
	"insmod" == name ||
	"rmmod" == name ||
	"load_config" == name ||
	"run_parts" == name ||
	"set_mod_path" == name ||
	"set_conf_path" == name ||
	"set_var_path" == name) {
      return new ScriptMethodRef(ref,name);
    }

    // See if its a property
    if ("version" == name) 
      return new ScriptRef(new VersionTag(version()));
    if ("name" == name) 
      return ScriptString::new_ref(Module::name());
    if ("copyright" == name) 
      return ScriptString::new_ref(copyright());
    if ("info" == name) 
      return ScriptString::new_ref(info());
    if ("loadtime" == name) 
      return new ScriptRef(m_loadtime.new_copy());
    if ("mod_path" == name) 
      return ScriptString::new_ref(get_mod_path().path());
    if ("conf_path" == name) 
      return ScriptString::new_ref(get_conf_path().path());
    if ("var_path" == name) 
      return ScriptString::new_ref(get_var_path().path());

    if ("description" == name) {
      std::ostringstream oss;
      oss << m_name << "-" << m_version.get_string() << "\n" 
	  << copyright() << "\n"
	<< info() << "\n";
      return ScriptString::new_ref(oss.str());
    }
    
    if ("modules" == name) {
      ScriptMap* modmap = new ScriptMap();
      ScriptRef* modmap_ref = new ScriptRef(modmap);
      for (std::list<ModuleLoader*>::const_iterator it = m_modules.begin();
	   it != m_modules.end();
	   it++) {
	ModuleLoader* loader = (*it);
	ScriptMap* entry = new ScriptMap();
	ScriptRef* entry_ref = new ScriptRef(entry);
	modmap->give(loader->get_name(),entry_ref);
	
	bool loaded = loader->is_loaded();
	entry->give("loaded",ScriptInt::new_ref(loaded));

	if (loaded) {
	  Module::Ref mod = loader->get_module();
	  entry->give("refs",
	  	      ScriptInt::new_ref(mod.object()->num_refs() - 2));
	  entry->give("object",mod.ref_copy(ref.reftype()));
	}
      }
      return modmap_ref;
    }
  
    // See if its a sub-module
    ModuleLoader* loader = find_module(name);
    if (loader) {
      return loader->get_module().ref_copy(ref.reftype());
    }
  }
  
  return ScriptObject::script_op(auth,ref,op,right);
}

//=============================================================================
ScriptRef* Module::script_method(const ScriptAuth& auth,
				 const ScriptRef& ref,
				 const std::string& name,
				 const ScriptRef* args)
{
  if ("set_log" == name) {
    if (!auth.admin()) return ScriptError::new_ref("Not permitted");

    const ScriptString* file = get_method_arg<ScriptString>(args,0,"file");
    if (file) {
      FilePath path = get_var_path() + file->get_string();
      log("Setting log file to '" + path.path() + "'");
      set_logger(new scx::Logger(path.path()));
    } else {
      log("Closing log file");
      set_logger(0);
    }
    return 0;
  }

  if ("log" == name) {
    const ScriptString* message = 
      get_method_arg<ScriptString>(args,0,"message");
    const ScriptInt* a_level = 
      get_method_arg<ScriptInt>(args,1,"level");
    Logger::Level level = a_level ?
      (Logger::Level)a_level->get_int() : Logger::Info;
    if (message) {
      log(message->get_string(),level);
    }
    return 0;
  }

  if ("insmod" == name) {
    if (!auth.admin()) return ScriptError::new_ref("Not permitted");

    const ScriptString* a_name = 
      get_method_arg<ScriptString>(args,0,"name");
    if (!a_name)
      return ScriptError::new_ref("insmod() No module specified");
    std::string name = a_name->get_string();

    // Check module entry doesn't already exist
    ModuleLoader* existing_loader = find_module(name);
    if (existing_loader) {
      if (existing_loader->is_loaded()) {
        return ScriptError::new_ref("insmod() Module [" + name +
				    "] already loaded and in use");
      }
      // The old module is not loaded - probably invalid,
      // remove to let the new one take its place.
      remove_module(existing_loader);
    }

    // Parse any options
    std::string opts;
    const ScriptString* a_opts = 
      get_method_arg<ScriptString>(args,1,"options");
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
    
    const ScriptString* a_conf_path = 
      get_method_arg<ScriptString>(args,2,"conf_path");
    if (a_conf_path) {
      loader->set_conf_path(a_conf_path->get_string());
    }
    
    if (opt_noconfig) {
      // Surpress autoloading of config during init
      loader->set_autoload_config(false);
    }
    
    if (!opt_delay) {
      // Cause the module to be loaded immediately
      Module::Ref mod = loader->get_module();
      if (!mod.valid()) {
        remove_module(loader);
      }
      return mod.ref_copy(ref.reftype());
    }
    return 0;
  }
  
  if ("rmmod" == name) {
    if (!auth.admin()) return ScriptError::new_ref("Not permitted");

    const ScriptString* a_name = 
      get_method_arg<ScriptString>(args,0,"name");
    if (!a_name) {
      return ScriptError::new_ref("rmmod() No module specified");
    }

    std::string name = a_name->get_string();
    ModuleLoader* loader = find_module(name);
    if (!loader) {
      return ScriptError::new_ref("rmmod() Module [" + name + "] not found");
    }

    if (loader->is_loaded()) {
      Module::Ref mod = loader->get_module();
      int nr = mod.object()->num_refs() - 2;
      if (nr > 0) {
        return ScriptError::new_ref("rmmod() Module [" + name + "] in use");
      }
    }
    
    log("Removing module [" + name + "]");
    remove_module(loader);
    return 0;
  }

  if ("load_config" == name) {
    if (!auth.admin()) return ScriptError::new_ref("Not permitted");

    const ScriptString* a_path = get_method_arg<ScriptString>(args,0,"path");
    FilePath path;
    if (a_path) {
      path = a_path->get_string();
    }
    if (false == load_config_file(path)) {
      return ScriptError::new_ref("load_config() Error occured");
    }
    return 0;
  }

  if ("run_parts" == name) {
    if (!auth.admin()) return ScriptError::new_ref("Not permitted");

    const ScriptString* a_path = get_method_arg<ScriptString>(args,0,"path");
    FilePath path;
    if (a_path) {
      path = a_path->get_string();
    }
    if (false == load_config_dir(path)) {
      return ScriptError::new_ref("Error occured");
    }
    return 0;
  }

  if ("set_mod_path" == name) {
    if (!auth.admin()) return ScriptError::new_ref("Not permitted");

    const ScriptString* a_path = get_method_arg<ScriptString>(args,0,"path");
    if (!a_path) {
      return ScriptString::new_ref(get_mod_path().path());
    }
    m_mod_path = a_path->get_string();
    return 0;
  }

  if ("set_conf_path" == name) {
    if (!auth.admin()) return ScriptError::new_ref("Not permitted");

    const ScriptString* a_path = get_method_arg<ScriptString>(args,0,"path");
    if (!a_path) {
      return ScriptString::new_ref(get_conf_path().path());
    }
    m_conf_path = a_path->get_string();
    return 0;
  }

  if ("set_var_path" == name) {
    if (!auth.admin()) return ScriptError::new_ref("Not permitted");

    const ScriptString* a_path = get_method_arg<ScriptString>(args,0,"path");
    if (!a_path) {
      return ScriptString::new_ref(get_var_path().path());
    }
    m_var_path = a_path->get_string();
    return 0;
  }

  return ScriptObject::script_method(auth,ref,name,args);
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
          // confpath/conf_file
          path = get_conf_path() + m_conf_file;
          break;
        case 2:
          // confpath/module.conf
          path = get_conf_path() +
                 FilePath(m_name + ".conf");
          break;
        case 3:
          // confpath/module/module.conf (DEV)
          path = get_conf_path() +
                 FilePath(m_name) + 
                 FilePath(m_name + ".conf");
          break;
        case 4:
          // confpath/parent/module/module.conf (DEV)
          if (!m_parent_module) continue;
          path = get_conf_path() +
                 FilePath(m_parent_module->name()) + 
                 FilePath(m_name) + 
                 FilePath(m_name + ".conf");
          break;
          
        default: return false; // Can't find conf file
      }
    } while (!FileStat(path).is_file());
  }

  ConfigFile config(path);
  ScriptRef ctx(this);
  return config.load(&ctx);
}

//=============================================================================
bool Module::load_config_dir(FilePath path)
{
  const char* filepattern =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_-";

  FileDir dir(path);
  std::list<std::string> files;
  while (dir.next()) {
    if (dir.stat().is_file() &&
	dir.name().find_first_not_of(filepattern) == std::string::npos) {
      files.push_back(dir.name());
    }
  }
  files.sort();

  bool ok = true;
  ScriptRef ctx(this);
  for (std::list<std::string>::const_iterator it = files.begin();
       it != files.end();
       ++it) {
    ConfigFile config(path + *it);
    if (!config.load(&ctx)) {
      ok = false;
    }
  }
  return ok;
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
  } else if (m_parent_module && m_parent_module != this) {
    m_parent_module->log_string(str,level);
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
  m_parent_module = parent;
  m_parent = parent;
}

};
