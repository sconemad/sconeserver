/* SconeServer (http://www.sconemad.com)

Sconesite Template manager

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

#include <sconesite/TemplateManager.h>
#include <sconex/File.h>
#include <sconex/FileDir.h>
#include <sconex/ScriptTypes.h>
#include <sconex/Log.h>
namespace scs {

//=========================================================================
TemplateStore::TemplateStore(const scx::FilePath& path)
  : m_path(path)
{
  refresh();
}

//=========================================================================
TemplateStore::~TemplateStore()
{
  for (TemplateMap::iterator it_t = m_templates.begin();
       it_t != m_templates.end(); ++it_t) {
    delete it_t->second;
  }
}

//=========================================================================
const scx::FilePath& TemplateStore::get_path() const
{
  return m_path;
}
 
//=========================================================================
void TemplateStore::refresh()
{
  scx::RWLocker locker(m_lock, true, scx::RWLock::Write);

  // Add new templates
  scx::FileDir dir(m_path);
  while (dir.next()) {
    std::string file = dir.name();
    if (file != "." && file != "..") {
      std::string::size_type idot = file.find_first_of(".");
      if (idot != std::string::npos) {
        std::string name = file.substr(0,idot);
        std::string extn = file.substr(idot+1,std::string::npos);
        if (extn == "xml" &&
            m_templates.find(name) == m_templates.end()) {
          Template* tpl = new Template(name,m_path);
          m_templates[name] = new Template::Ref(tpl);
        }
      }
    }
  }
  
  // Remove deleted templates
  TemplateMap::iterator it_t = m_templates.begin();
  while (it_t != m_templates.end()) {
    Template::Ref* tpl_ref = it_t->second;
    Template* tpl = tpl_ref->object();
    if (!scx::FileStat(tpl->get_filepath()).is_file()) {
      it_t = m_templates.erase(it_t);
      delete tpl_ref;
    } else {
      ++it_t;
    }
  }
}

//=========================================================================
Template* TemplateStore::lookup(const std::string& name)
{
  scx::RWLocker locker(m_lock, true, scx::RWLock::Read);

  TemplateMap::iterator it = m_templates.find(name);
  if (it != m_templates.end()) {
    return it->second->object();
  }
  return 0;
}


//=========================================================================
TemplateManager::TemplateManager()
{
}

//=========================================================================
TemplateManager::~TemplateManager()
{
  for (TemplateStoreList::iterator it = m_templates.begin();
       it != m_templates.end(); ++it) {
    delete (*it);
  }
}

//=========================================================================
void TemplateManager::add(const scx::FilePath& path)
{
  m_templates.push_back(new TemplateStore(path));
}

//=========================================================================
void TemplateManager::refresh()
{
  for (TemplateStoreList::iterator it = m_templates.begin();
       it != m_templates.end(); ++it) {
    (*it)->refresh();
  }
}
  
//=========================================================================
Template* TemplateManager::lookup(const std::string& name)
{
  for (TemplateStoreList::iterator it = m_templates.begin();
       it != m_templates.end(); ++it) {
    Template* t = (*it)->lookup(name);
    if (t) return t;
  }
  return 0;
}
  
};
