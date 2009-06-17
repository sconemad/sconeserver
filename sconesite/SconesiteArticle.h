/* SconeServer (http://www.sconemad.com)

Sconesite Article

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

#ifndef sconesiteArticle_h
#define sconesiteArticle_h

#include "sconex/Stream.h"
#include "sconex/FilePath.h"

class SconesiteModule;

//=========================================================================
class SconesiteArticle {

public:

  SconesiteArticle(SconesiteModule& module,
                   const scx::FilePath& path,
                   const std::string& name);

  ~SconesiteArticle();

  const std::string& get_name() const;
  
protected:

private:
  
  SconesiteModule& m_module;

  scx::FilePath m_path;

  std::string m_name;
  
};

//=========================================================================
class SconesiteArticleManager {

public:

  SconesiteArticleManager(SconesiteModule& module,
                          const scx::FilePath& path);

  ~SconesiteArticleManager();

  void refresh();

  SconesiteArticle* lookup_article(const std::string& name);
  const std::list<SconesiteArticle*>& articles() const;
  
private:

  SconesiteModule& m_module;

  scx::FilePath m_path;
  
  std::list<SconesiteArticle*> m_articles;
  
};



#endif
