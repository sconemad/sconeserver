/* SconeServer (http://www.sconemad.com)

HTTP filesystem directory node

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

#include "http/FSDirectory.h"
#include "http/FSLink.h"
#include "http/FSFile.h"
#include "sconex/FilePath.h"
#include "sconex/FileStat.h"
#include "sconex/FileDir.h"
#include "sconex/ConfigFile.h"
namespace http {

//=============================================================================
FSDirectory::FSDirectory(
  const std::string& name
) : FSNode(FSNode::Directory,name)
{
  DEBUG_COUNT_CONSTRUCTOR(FSDirectory);
}

//=============================================================================
FSDirectory::~FSDirectory()
{
  std::list<FSNode*>::iterator it = m_dir.begin();

  while (it != m_dir.end()) {
    delete (*it);
    it++;
  }

  DEBUG_COUNT_DESTRUCTOR(FSDirectory);
}

//=============================================================================
void FSDirectory::add(FSNode* entry)
{
  if (entry) {
    entry->m_parent = this;
    m_dir.push_back(entry);
  }
}

//=============================================================================
void FSDirectory::remove(FSNode* entry)
{
  if (entry) {
    m_dir.remove(entry);
    entry->m_parent = 0;
    delete entry;
  }
}

//=============================================================================
const FSNode* FSDirectory::find(const std::string& name) const
{
  std::list<FSNode*>::const_iterator it = m_dir.begin();

  while (it != m_dir.end()) {
    FSNode* entry = (*it);
    if (entry && entry->name() == name) {
      return entry;
    }
    it++;
  }
  
  return 0;
}

//=============================================================================
const std::list<FSNode*>& FSDirectory::dir() const
{
  return m_dir;
}

//=============================================================================
const FSNode* FSDirectory::lookup(const std::string& name) const
{
  FSNode* uc = (FSNode*)this;
  uc->build();
  
  std::string::size_type nsep = name.find_first_of("/");
  if (nsep == std::string::npos) {
    return find(name);

  } else if (nsep == 0) {
    const FSNode* c = parent();
    if (c && c->type() == FSNode::Directory) {
      return ((FSDirectory*)c)->lookup(name);
    }
    std::string rest(name.substr(1));
    if (rest.empty()) {
      return this;
    }
    return lookup(rest);

  } else {
    std::string first(name.substr(0,nsep));
    std::string rest(name.substr(nsep+1));
    const FSNode* next = find(first);

    if (rest.empty()) {
      return next;
    } else if (next && next->type() == FSNode::Directory) {
      return ((FSDirectory*)next)->lookup(rest);
    }
  }

  return 0;
}

//=============================================================================
bool FSDirectory::build()
{
  std::list<FSNode*> exist;
  
  scx::FileDir dir(path());
  while (dir.next()) {
    std::string filename = dir.name().path();
    if (filename != "." && filename != "..") {

      FSNode* ne = (FSNode*)find(filename);
      if (ne) {
//        std::cerr << "[=] " << path().path() << "/" << filename << "\n";
        m_dir.remove(ne);
        exist.push_back(ne);
        
      } else {
//        std::cerr << "[+] " << path().path() << "/" << filename << "\n";
        FSNode* n = 0;
        if (dir.stat().is_dir()) {
          n = new FSDirectory(filename);
        } else {
          n = new FSFile(filename);
        }
        n->m_parent = this;
        exist.push_back(n);
        n->build();
        ne = n;
      }

      // Handle modified files
      if (ne->modified()) {
        if (filename == ".scone") {
          scx::ConfigFile config(path() + filename);
          scx::ArgObject* ctx = new scx::ArgObject(this);
          config.load(ctx);
        }
        ne->set_unmodified();
      }
    }
  }

  std::list<FSNode*>::iterator it = m_dir.begin();
  while (it != m_dir.end()) {
    FSNode* entry = (*it);  
//    std::cerr << "[-] " << entry->path().path() << "\n";
    it = m_dir.erase(it);
    entry->m_parent = 0;
    delete entry;
  }

  it = exist.begin();
  while (it != exist.end()) {
     m_dir.push_back(*it);
    it++;
  }
  
  return true;
}

//=============================================================================
scx::Arg* FSDirectory::arg_lookup(const std::string& name)
{
  // Methods
  if ("map" == name ||
      "lookup" == name) {
    return new scx::ArgObjectFunction(new scx::ArgObject(this),name);
  }

  return FSNode::arg_lookup(name);
}

//=============================================================================
scx::Arg* FSDirectory::arg_function(
  const std::string& name,
  scx::Arg* args
)
{
  scx::ArgList* l = dynamic_cast<scx::ArgList*>(args);

  if ("map" == name) {
    const scx::ArgString* a_pattern =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_pattern) {
      return new scx::ArgError("map() No pattern specified");
    }
    std::string s_pattern = a_pattern->get_string();

    const scx::ArgString* a_module =
      dynamic_cast<const scx::ArgString*>(l->get(1));
    if (!a_module) {
      return new scx::ArgError("map() No module specified");
    }
    std::string s_module = a_module->get_string();

    log("Mapping '" + s_pattern + "' --> " + s_module);
    m_mods[s_pattern] = s_module;

    return 0;
  }

  if ("lookup" == name) {
    const scx::ArgString* a_pattern =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_pattern) {
      return new scx::ArgError("map() No pattern specified");
    }
    std::string s_pattern = a_pattern->get_string();

    return new scx::ArgObject((FSNode*)lookup(s_pattern));
  }
  
  return FSNode::arg_function(name,args);
}

//=============================================================================
std::string FSDirectory::lookup_mod(const std::string& name) const
{
  int bailout=100;
  std::string::size_type idot;
  std::string key=name;
  while (--bailout > 0) {

    std::map<std::string,std::string>::const_iterator it = m_mods.find(key);
    if (it != m_mods.end()) {
      return (*it).second;
    }
    
    if (key.size()<=0 || key=="*") {
      if (parent()) {
        return parent()->lookup_mod(name);
      } else {
        return "";
      }
    }

    if (key[0]=='*') {
      idot = key.find(".",2);
    } else {
      idot = key.find_first_of(".");
    }
    
    if (idot==key.npos) {
      key="*";
    } else {
      key = "*" + key.substr(idot);
    }
    
  }
  DEBUG_LOG("lookup_mod() Pattern match bailout");
  return ""; // Bailed out
}

};
