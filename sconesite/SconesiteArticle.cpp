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


#include "SconesiteArticle.h"
#include "SconesiteModule.h"

#include "sconex/Stream.h"
#include "sconex/StreamTransfer.h"
#include "sconex/Date.h"
#include "sconex/Kernel.h"
#include "sconex/FileDir.h"

//=========================================================================
SconesiteArticle::SconesiteArticle(
  SconesiteModule& module,
  const scx::FilePath& path,
  const std::string& name
) : m_module(module),
    m_path(path),
    m_name(name)
{

}

//=========================================================================
SconesiteArticle::~SconesiteArticle()
{
  
}

//=========================================================================
const std::string& SconesiteArticle::get_name() const
{
  return m_name;
}



//=========================================================================
SconesiteArticleManager::SconesiteArticleManager(
  SconesiteModule& module,
  const scx::FilePath& path
) : m_module(module),
    m_path(path)
{
  refresh();
}

//=========================================================================
SconesiteArticleManager::~SconesiteArticleManager()
{
  for (std::list<SconesiteArticle*>::iterator it = m_articles.begin();
       it != m_articles.end(); ++it) {
    delete (*it);
  }
}

//=========================================================================
void SconesiteArticleManager::refresh()
{
  scx::FileDir dir(m_path);
  while (dir.next()) {
    std::string name = dir.name();
    if (name != "." && name != "..") {
      if (!lookup_article(name)) {
        SconesiteArticle* article = new SconesiteArticle(m_module,dir.path(),name);
        m_articles.push_back(article);
      }
    }
  }
}

//=========================================================================
SconesiteArticle* SconesiteArticleManager::lookup_article(const std::string& name)
{
  for (std::list<SconesiteArticle*>::iterator it = m_articles.begin();
       it != m_articles.end(); ++it) {
    SconesiteArticle* article = (*it);
    if (article->get_name() == name) {
      return article;
    }
  }
  
  return 0;
}

//=========================================================================
const std::list<SconesiteArticle*>& SconesiteArticleManager::articles() const
{
  return m_articles;
}
