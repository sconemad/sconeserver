/* SconeServer (http://www.sconemad.com)

SconeX module

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

#ifndef scxModule_h
#define scxModule_h

#include "sconex/sconex.h"
#include "sconex/ModuleRef.h"
#include "sconex/VersionTag.h"
#include "sconex/Arg.h"
#include "sconex/ArgObject.h"
#include "sconex/FilePath.h"
#include "sconex/Date.h"

namespace scx {

class ModuleLoader;
class Logger;
class Descriptor;

//=============================================================================
class SCONEX_API Module : public ArgObjectInterface {

public:

  Module(
    const std::string& name,
    const VersionTag& ver
  );

  virtual ~Module();

  const VersionTag& version() const;
  virtual std::string name() const;
  virtual std::string copyright() const;
  virtual std::string info() const =0;
  // Access module information

  virtual int init();
  // Initialize the module

  virtual void close();
  // Close the module
  
  ModuleRef ref();
  // Get a reference to ourself

  ModuleRef get_module(const std::string& name);
  // Get sub-module

  void set_autoload_config(bool onoff);
  // Automatically load config or not
  
  void set_mod_path(const FilePath& path);
  FilePath get_mod_path() const;
  // Set/get path for sub-modules

  void set_conf_path(const FilePath& path);
  FilePath get_conf_path() const;
  // Set/get path for configuration files

  void set_var_path(const FilePath& path);
  FilePath get_var_path() const;
  // Set/get path for variable files  
  
  virtual void log(
    const std::string& message,
    Logger::Level level = Logger::Info
  );
  // Log message with module name
  
  void log_string(const std::string& str,Logger::Level level);
  virtual void set_logger(Logger* logger);
  // Set logger to use

  virtual bool connect(
    Descriptor* endpoint,
    ArgList* args
  );
  //
  // Use this module on the specified endpoint with the specified argument
  //
  // RETURNS: true  - ok, module used
  //          false - failed
  //
  
  virtual Arg* arg_lookup(const std::string& name);
  virtual Arg* arg_method(const Auth& auth, const std::string& name,Arg* args);

protected:

  void add_module(ModuleLoader* loader);
  void remove_module(ModuleLoader* loader);
  ModuleLoader* find_module(const std::string& name);

  bool load_config_file(FilePath path = FilePath());
  
private:

  void set_parent(Module* parent);
  friend class ModuleLoader;
  friend class ModuleLoaderDLL;
  // Parent module (ModuleLoader can set this)

  std::string m_name;
  // Module name

  VersionTag m_version;
  // Module version
  
  Date m_loadtime;
  // Load time
  
  std::list<ModuleLoader*> m_modules;
  // Sub modules

  bool m_autoload_config;
  // Automatically load config
  
  FilePath m_mod_path;
  // Path to locate sub-modules

  FilePath m_conf_path;
  std::string m_conf_file;
  // Path to configuration files

  FilePath m_var_path;
  // Path to variable files
  
  Module* m_parent;
  // Parent module
  
  Logger* m_logger;
  // Logger instance

};


//=============================================================================
class SCONEX_API ArgModule : public ArgObject {

public:
  
  ArgModule(ModuleRef ref);
  virtual ~ArgModule();

protected:

  ModuleRef m_ref;
  
};

};
#endif
