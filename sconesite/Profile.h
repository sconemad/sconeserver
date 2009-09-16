/* SconeServer (http://www.sconemad.com)

Sconesite Profile

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

#ifndef sconesiteProfile_h
#define sconesiteProfile_h

#include "sconex/Stream.h"
#include "sconex/FilePath.h"

class SconesiteModule;
class Article;
class Template;

//=========================================================================
class Profile {

public:

  Profile(SconesiteModule& module,
          const scx::FilePath& path);
  
  ~Profile();

  void refresh();

  SconesiteModule& get_module();
  scx::FilePath& get_path();

  Article* lookup_article(const std::string& name);
  const std::list<Article*>& articles() const;
  Article* create_article(const std::string& name);

  Template* lookup_template(const std::string& name);
  
private:

  SconesiteModule& m_module;

  scx::FilePath m_path;
  
  std::list<Article*> m_articles;
  std::list<Template*> m_templates;
  
};



#endif
