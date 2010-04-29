/* SconeServer (http://www.sconemad.com)

Sconesite XML document

Copyright (c) 2000-2009 Andrew Wedgbury <wedge@sconemad.com>

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


#include "ArticleBody.h"
#include "Context.h"

//=========================================================================
ArticleHeading::ArticleHeading(int level, const std::string& name, int index)
  : m_level(level),
    m_name(name),
    m_index(index)
{

}

//=========================================================================
ArticleHeading::~ArticleHeading()
{
  clear();
}

//=========================================================================
int ArticleHeading::level() const
{
  return m_level;
}

//=========================================================================
const std::string& ArticleHeading::name() const
{
  return m_name;
}

//=========================================================================
int ArticleHeading::index() const
{
  return m_index;
}

//=========================================================================
void ArticleHeading::clear()
{
  for (ArticleHeadingList::iterator it = m_subs.begin();
       it != m_subs.end();
       ++it) {
    ArticleHeading* h = *it;
    delete h;
  }
  m_subs.clear();
}

//=========================================================================
void ArticleHeading::add(int level, const std::string& name, int index)
{
  if (m_subs.size() == 0 || m_subs.back()->level() >= level) {
    m_subs.push_back(new ArticleHeading(level,name,index));
  } else {
    m_subs.back()->add(level,name,index);
  }
}

//=========================================================================
const ArticleHeading* ArticleHeading::lookup_index(int index) const
{
  if (index == m_index) return this;
  
  for (ArticleHeadingList::const_iterator it = m_subs.begin();
       it != m_subs.end();
       ++it) {
    const ArticleHeading* f = (*it)->lookup_index(index);
    if (f) return f;
  }
  
  return 0;
}

//=========================================================================
std::string ArticleHeading::lookup_anchor(int index) const
{
  if (index == m_index) return m_name;

  for (ArticleHeadingList::const_iterator it = m_subs.begin();
       it != m_subs.end();
       ++it) {
    std::string p = (*it)->lookup_anchor(index);
    if (!p.empty()) {
      if (m_index == 0) return p;
      return (m_name + "+" + p);
    }
  }
  
  return "";
}

//=========================================================================
std::string ArticleHeading::lookup_section(int index) const
{
  int sec = 0;
  for (ArticleHeadingList::const_iterator it = m_subs.begin();
       it != m_subs.end();
       ++it) {
    const ArticleHeading* h = *it;
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
scx::Arg* ArticleHeading::get_arg(
  const std::string& anchor_prefix,
  const std::string& section_prefix
) const
{
  scx::ArgList* list = new scx::ArgList();
  int s=0;
  for (ArticleHeadingList::const_iterator it = m_subs.begin();
       it != m_subs.end();
       ++it) {
    const ArticleHeading* h = *it;
    std::string anchor = h->name();

    std::ostringstream oss;
    oss << (++s);
    std::string section = oss.str();

    if (m_index != 0) {
      anchor = anchor_prefix + "+" + anchor;
      section = section_prefix + "." + section;
    }
    
    scx::ArgMap* map = new scx::ArgMap();
    map->give("level", new scx::ArgInt(h->level()));
    map->give("name",new scx::ArgString(h->name()));
    map->give("anchor",new scx::ArgString(anchor));
    map->give("section",new scx::ArgString(section));
    map->give("subsection",new scx::ArgInt(s));
    map->give("subs",h->get_arg(anchor,section));
    list->give(map);
  }
  return list;  
}
