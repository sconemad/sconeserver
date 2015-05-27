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
#include <sconex/ConfigFile.h>
#include <sconex/ScriptExpr.h>
#include <sconex/Log.h>
#include <sconex/FileStat.h>
#include <sconex/FileDir.h>
#include <sconex/Kernel.h>

namespace scx {

//=============================================================================
static void get_sorted_conf_files(const FilePath& path,
  std::list<std::string>& files)
// Find all conf files under the specified directory path, and order by name
{
  files.clear();

  const char* filepattern =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_-.";
  const char* fileext = ".conf";
  
  FileDir dir(path);
  while (dir.next()) {
    size_t extpos = dir.name().length() - strlen(fileext);
    if (dir.stat().is_file() &&
	dir.name().find(fileext,extpos) != std::string::npos &&
	dir.name().find_first_not_of(filepattern) == std::string::npos) {
      files.push_back(dir.name());
    }
  }
  files.sort();
}

//=============================================================================
Module::Module(
  const std::string& name,
  const VersionTag& version
) : m_name(name),
    m_version(version),
    m_loadtime(),
    m_modules(),
    m_mod_path(""),
    m_var_path(""),
    m_parent_module(0)
{
  DEBUG_COUNT_CONSTRUCTOR(Module);
  m_loadtime = Date::now();
}
	
//=============================================================================
Module::~Module()
{
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
  return sconex_copyright();
}

//=============================================================================
int Module::init()
{
  return 0;
}

//=============================================================================
bool Module::close()
{
  bool done = true;
  m_modules.reverse();
  std::list<ModuleLoader*>::iterator it = m_modules.begin();
  while (it != m_modules.end()) {
    ModuleLoader* loader = *it;
    if (loader->close()) {
      delete loader;
      it = m_modules.erase(it);
    } else {
      done = false;
      ++it;
    }
  }
  m_modules.reverse();
  return done;
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
    if ("log" == name ||
	"insmod" == name ||
	"rmmod" == name ||
	"load_config" == name ||
	"load_config_dir" == name ||
	"load_module_dir" == name ||
	"run_parts" == name ||
	"set_mod_path" == name ||
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
  if ("log" == name) {
    const ScriptString* message = 
      get_method_arg<ScriptString>(args,0,"message");
    if (message) {
      // Use the module name as the category
      Log(m_name).submit(message->get_string());
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
        return ScriptError::new_ref("Module [" + name +
				    "] already loaded and in use");
      }
      // The old module is not loaded - probably invalid,
      // remove to let the new one take its place.
      remove_module(existing_loader);
    }

    Log("loader").submit("Adding module [" + name + "]");
    
    ModuleLoader* loader = new ModuleLoader(name,this);
    add_module(loader);
    
    // Load the module and check it's valid
    Module::Ref mod = loader->get_module();
    if (!mod.valid()) {
      remove_module(loader);
      return ScriptError::new_ref("Cannot load module [" + name + "]");
    }

    return mod.ref_copy(ref.reftype());
  }
  
  if ("rmmod" == name) {
    if (!auth.admin()) return ScriptError::new_ref("Not permitted");

    const ScriptString* a_name = 
      get_method_arg<ScriptString>(args,0,"name");
    if (!a_name) {
      return ScriptError::new_ref("No module specified");
    }

    std::string name = a_name->get_string();
    ModuleLoader* loader = find_module(name);
    if (!loader) {
      return ScriptError::new_ref("Module [" + name + "] not found");
    }

    if (loader->is_loaded()) {
      Module::Ref mod = loader->get_module();
      if (!mod.object()->close()) {
        return ScriptError::new_ref("Module [" + name + "] in use");
      }
      int nr = mod.object()->num_refs() - 2;
      if (nr > 0) {
        return ScriptError::new_ref("Module [" + name + "] in use");
      }
    }
    
    Log("loader").submit("Removing module [" + name + "]");
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
      return ScriptError::new_ref("Error occured");
    }
    return 0;
  }

  if ("load_config_dir" == name || "run_parts" == name) {
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

  if ("load_module_dir" == name) {
    if (!auth.admin()) return ScriptError::new_ref("Not permitted");

    const ScriptString* a_path = get_method_arg<ScriptString>(args,0,"path");
    FilePath path;
    if (a_path) {
      path = a_path->get_string();
    }
    if (false == load_module_dir(path)) {
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
  ConfigFile config(path);
  ScriptRef ctx(this);
  return config.load(&ctx);
}

//=============================================================================
bool Module::load_config_dir(FilePath path)
{
  path = Kernel::get()->get_conf_path() + path;

  std::list<std::string> files;
  get_sorted_conf_files(path, files);

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
bool Module::load_module_dir(FilePath path)
{
  path = Kernel::get()->get_conf_path() + path;
  
  std::list<std::string> files;
  get_sorted_conf_files(path, files);

  // Create a module loader for each conf file
  for (std::list<std::string>::const_iterator it = files.begin();
       it != files.end(); ++it) {
    /*
    int i = (*it).find_last_of(".");
    std::string name((*it), 0, i);
    */
    ModuleLoader* loader = new ModuleLoader(path + (*it), this);
    if (loader->get_name() == "") {
      delete loader;
      continue;
    }
    add_module(loader);
  }

  // Now try and load the modules
  // Loop until we can't load any more (hopefully because they've all been
  // loaded), recording any errors that occur for reporting later.
  std::map<std::string,std::string> errors;
  int prev_loaded = 0;
  int loaded = 0;
  do {
    prev_loaded = loaded;

    // Loop through and load any modules that have satisfied dependencies
    for (std::list<ModuleLoader*>::iterator itM = m_modules.begin();
         itM != m_modules.end(); ++itM) {
      ModuleLoader* loader = (*itM);
      if (!loader->is_loaded()) {
        // Determine if we can we load this module
        bool loadable = true;
        const ModuleLoader::Depends& deps = loader->get_depends();
        for (ModuleLoader::Depends::const_iterator itD = deps.begin();
             itD != deps.end(); ++itD) {
          ModuleLoader* dl = find_module(*itD);
          if (!dl) {
            errors[loader->get_name()] =
              std::string("missing dependency [") + (*itD) + "]";
            loadable = false;
          } else if (!dl->is_loaded()) {
            // Module is just not loaded yet
            errors[loader->get_name()] =
              std::string("dependency [") + (*itD) + "] not loaded";
            loadable = false;
          }
        }

        if (loadable) {
          // Try and load the module
          Module::Ref mod = loader->get_module();
          if (mod.valid()) {
            // Module has loaded, great
            ++loaded;
            errors.erase(loader->get_name());
          } else {
            errors[loader->get_name()] = std::string("failed to load");
          }
        }
      }
    }
    
  } while (loaded > prev_loaded);

  // Log a summary of what happened
  if (errors.size() == 0) {
    Log("loader").submit("All modules loaded succesfully");
  } else {
    Log("loader").submit("Some module(s) failed to load:");
    for (std::map<std::string,std::string>::const_iterator it = errors.begin();
         it != errors.end(); ++it) {
      Log("loader").submit(std::string("Module [") + it->first + "] " +
                           it->second);
    }
  }
  
  return true;
}

//=============================================================================
void Module::set_parent(Module* parent)
{
  m_parent_module = parent;
  m_parent = parent;
}

};
