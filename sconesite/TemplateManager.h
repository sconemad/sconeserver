/* SconeServer (http://www.sconemad.com)

Sconesite Template store

Copyright (c) 2000-2015 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef scsTemplateStore_h
#define scsTemplateStore_h

#include <sconesite/Template.h>
#include <sconex/FilePath.h>
#include <sconex/ScriptBase.h>
#include <sconex/Mutex.h>
namespace scs {
  
class SconesiteModule;

//=========================================================================
// TemplateStore
//
class TemplateStore {
public:

  TemplateStore(const scx::FilePath& path);

  ~TemplateStore();

  const scx::FilePath& get_path() const;
  
  // Refresh the templates
  void refresh();

  // Lookup template by name
  Template* lookup(const std::string& name);

private:

  // Store directory
  scx::FilePath m_path;

  // Lock
  scx::RWLock m_lock;
  
  // Template map
  typedef HASH_TYPE<std::string,Template::Ref*> TemplateMap;
  TemplateMap m_templates;

};

//=========================================================================
// TemplateManager
//
class TemplateManager {
public:
  
  TemplateManager();

  ~TemplateManager();

  void add(const scx::FilePath& path);

  void refresh();
  
  Template* lookup(const std::string& name);

private:

  typedef std::list<TemplateStore*> TemplateStoreList;
  TemplateStoreList m_templates;
  
};
  
};
#endif
