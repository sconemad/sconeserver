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

#ifndef scxModule_h
#define scxModule_h

#include <sconex/sconex.h>
#include <sconex/VersionTag.h>
#include <sconex/ScriptBase.h>
#include <sconex/FilePath.h>
#include <sconex/Date.h>
#include <sconex/Logger.h>

namespace scx {

class ModuleLoader;
class Descriptor;

//=============================================================================
class SCONEX_API Module : public ScriptObject {
public:

  typedef ScriptRefTo<Module> Ref;

  Module(
    const std::string& name,
    const VersionTag& ver
  );

  virtual ~Module();

  // Access module information
  const VersionTag& version() const;
  virtual std::string name() const;
  virtual std::string copyright() const;
  virtual std::string info() const =0;

  // Initialize the module
  virtual int init();

  // Close the module
  virtual bool close();
  
  // Get sub-module
  Module::Ref get_module(const std::string& name);

  // Automatically load config or not
  void set_autoload_config(bool onoff);
  
  // Set/get path for sub-modules
  void set_mod_path(const FilePath& path);
  FilePath get_mod_path() const;

  // Set/get path for configuration files
  void set_conf_path(const FilePath& path);
  FilePath get_conf_path() const;

  // Set/get path for variable files  
  void set_var_path(const FilePath& path);
  FilePath get_var_path() const;
  
  // Log message with module name
  virtual void log(
    const std::string& message,
    Logger::Level level = Logger::Info
  );
  
  // Set logger to use
  void log_string(const std::string& str,Logger::Level level);
  virtual void set_logger(Logger* logger);

  // ScriptObject interface:
  virtual std::string get_string() const;

  virtual ScriptRef* script_op(const ScriptAuth& auth,
			       const ScriptRef& ref,
			       const ScriptOp& op,
			       const ScriptRef* right);
  
  virtual ScriptRef* script_method(const ScriptAuth& auth,
				   const ScriptRef& ref,
				   const std::string& name,
				   const ScriptRef* args);

protected:

  void add_module(ModuleLoader* loader);
  void remove_module(ModuleLoader* loader);
  ModuleLoader* find_module(const std::string& name);

  bool load_config_file(FilePath path = FilePath());
  bool load_config_dir(FilePath path = FilePath());
  
private:

  // Parent module (ModuleLoader can set this)
  void set_parent(Module* parent);
  friend class ModuleLoader;
  friend class ModuleLoaderDLL;

  // Module name
  std::string m_name;

  // Module version
  VersionTag m_version;
  
  // Load time
  Date m_loadtime;
  
  // Sub modules
  std::list<ModuleLoader*> m_modules;

  // Automatically load config
  bool m_autoload_config;
  
  // Path to locate sub-modules
  FilePath m_mod_path;

  // Path to configuration files
  FilePath m_conf_path;
  std::string m_conf_file;

  // Path to variable files
  FilePath m_var_path;
  
  // Parent module
  Module* m_parent_module;
  
  // Logger instance
  Logger* m_logger;

};

};
#endif
