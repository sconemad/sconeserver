/* SconeServer (http://www.sconemad.com)

Sconesite Article Heading

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

#ifndef sconesiteArticleHeading_h
#define sconesiteArticleHeading_h

#include <sconex/ScriptBase.h>

//=========================================================================
// ArticleHeading - Represents a heading in a document, can be arranged in
// a tree structure
//
class ArticleHeading {
public:
  ArticleHeading(int level, const std::string& name, int index);
  ~ArticleHeading();

  int level() const;
  const std::string& name() const;
  int index() const;
  
  void clear();
  void add(int level, const std::string& name, int index);

  const ArticleHeading* lookup_index(int index) const;
  std::string lookup_anchor(int index) const;
  std::string lookup_section(int index) const;

  scx::ScriptRef* get_tree(const std::string& anchor_prefix = "",
			   const std::string& section_prefix = "") const;

private:
  
  int m_level;
  std::string m_name;
  int m_index;
  
  typedef std::vector<ArticleHeading*> ArticleHeadingList;
  ArticleHeadingList m_subs;
};

#endif
