/* SconeServer (http://www.sconemad.com)

Sconesite Document Heading

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

#include "Heading.h"
#include <sconex/ScriptTypes.h>

//=========================================================================
Heading::Heading(int level, const std::string& name, int index)
  : m_level(level),
    m_name(name),
    m_index(index)
{

}

//=========================================================================
Heading::~Heading()
{
  clear();
}

//=========================================================================
int Heading::level() const
{
  return m_level;
}

//=========================================================================
const std::string& Heading::name() const
{
  return m_name;
}

//=========================================================================
int Heading::index() const
{
  return m_index;
}

//=========================================================================
void Heading::clear()
{
  for (HeadingList::iterator it = m_subs.begin();
       it != m_subs.end();
       ++it) {
    Heading* h = *it;
    delete h;
  }
  m_subs.clear();
}

//=========================================================================
void Heading::add(int level, const std::string& name, int index)
{
  if (m_subs.size() == 0 || m_subs.back()->level() >= level) {
    m_subs.push_back(new Heading(level,name,index));
  } else {
    m_subs.back()->add(level,name,index);
  }
}

//=========================================================================
const Heading* Heading::lookup_index(int index) const
{
  if (index == m_index) return this;
  
  for (HeadingList::const_iterator it = m_subs.begin();
       it != m_subs.end();
       ++it) {
    const Heading* f = (*it)->lookup_index(index);
    if (f) return f;
  }
  
  return 0;
}

//=========================================================================
std::string name_encode(const std::string& name)
{
  std::string ret;
  bool first = true;
  for (unsigned int i=0; i<name.size(); ++i) {
    char c = name[i];
    if (isalnum(c)) {
      ret += first ? toupper(c) : c;
      first = false;
    } else {
      first = true;
    }
  }
  return ret;
}

//=========================================================================
std::string Heading::lookup_anchor(int index) const
{
  if (index == m_index) {
    return name_encode(m_name);
  }

  for (HeadingList::const_iterator it = m_subs.begin();
       it != m_subs.end();
       ++it) {
    std::string p = (*it)->lookup_anchor(index);
    if (!p.empty()) {
      if (m_index == 0) return p;
      return (name_encode(m_name) + "_" + p);
    }
  }
  
  return "";
}

//=========================================================================
std::string Heading::lookup_section(int index) const
{
  int sec = 0;
  for (HeadingList::const_iterator it = m_subs.begin();
       it != m_subs.end();
       ++it) {
    const Heading* h = *it;
    ++sec;
    std::ostringstream oss;
    if (h->index() == index) {
      oss << sec;
      return oss.str();
      
    } else {
      std::string str = h->lookup_section(index);
      if (!str.empty()) {
        oss << sec << "." << str;
        return oss.str();
      }
    }
  }
  
  return "";
}

//=========================================================================
scx::ScriptRef* Heading::get_tree(const std::string& anchor_prefix,
				  const std::string& section_prefix) const
{
  scx::ScriptList* list = new scx::ScriptList();
  int s=0;
  for (HeadingList::const_iterator it = m_subs.begin();
       it != m_subs.end();
       ++it) {
    const Heading* h = *it;
    std::string anchor = name_encode(h->name());

    std::ostringstream oss;
    oss << (++s);
    std::string section = oss.str();

    if (m_index != 0) {
      anchor = anchor_prefix + "_" + anchor;
      section = section_prefix + "." + section;
    }
    
    scx::ScriptMap* map = new scx::ScriptMap();
    map->give("level", scx::ScriptInt::new_ref(h->level()));
    map->give("name",scx::ScriptString::new_ref(h->name()));
    map->give("anchor",scx::ScriptString::new_ref(anchor));
    map->give("section",scx::ScriptString::new_ref(section));
    map->give("subsection",scx::ScriptInt::new_ref(s));
    map->give("subs",h->get_tree(anchor,section));
    list->give(new scx::ScriptRef(map));
  }

  return new scx::ScriptRef(list);
}
