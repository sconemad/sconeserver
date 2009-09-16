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


#include "Profile.h"
#include "Article.h"
#include "Template.h"
#include "SconesiteModule.h"

#include "sconex/Stream.h"
#include "sconex/StreamTransfer.h"
#include "sconex/Date.h"
#include "sconex/Kernel.h"
#include "sconex/FileDir.h"

const char* ARTDIR = "art";
const char* TPLDIR = "tpl";

//=========================================================================
Profile::Profile(
  SconesiteModule& module,
  const scx::FilePath& path
) : m_module(module),
    m_path(path)
{
  refresh();
}

//=========================================================================
Profile::~Profile()
{
  for (std::list<Article*>::iterator it_a = m_articles.begin();
       it_a != m_articles.end(); ++it_a) {
    delete (*it_a);
  }
  for (std::list<Template*>::iterator it_t = m_templates.begin();
       it_t != m_templates.end(); ++it_t) {
    delete (*it_t);
  }
}

//=========================================================================
void Profile::refresh()
{
  // Add new articles
  scx::FileDir dir(m_path + ARTDIR);
  while (dir.next()) {
    std::string name = dir.name();
    if (name != "." && name != "..") {
      if (!lookup_article(name)) {
        scx::FilePath path = dir.path();
        Article* article = new Article(*this,name,path);
        m_articles.push_back(article);
        m_module.log("Adding article '" + name + "'");
      }
    }
  }

  // Remove deleted articles
  for (std::list<Article*>::iterator it_a = m_articles.begin();
       it_a != m_articles.end();
       ++it_a) {
    Article* article = (*it_a);
    if (!scx::FileStat(article->get_path()).is_file()) {
      it_a = m_articles.erase(it_a);
      delete article;
    }
  }

  // Add new templates
  dir = scx::FileDir(m_path + TPLDIR);
  while (dir.next()) {
    std::string file = dir.name();
    if (file != "." && file != "..") {
      std::string::size_type idot = file.find_first_of(".");
      if (idot != std::string::npos) {
	std::string name = file.substr(0,idot);
	std::string extn = file.substr(idot+1,std::string::npos);
	if (extn == "xml" && !lookup_template(name)) {
	  scx::FilePath path = dir.path();
	  Template* tpl = new Template(*this,name,path);
	  m_templates.push_back(tpl);
	  m_module.log("Adding template '" + name + "'");
	}
      }
    }
  }

  // Remove deleted templates
  for (std::list<Template*>::iterator it_t = m_templates.begin();
       it_t != m_templates.end();
       ++it_t) {
    Template* tpl = (*it_t);
    if (!scx::FileStat(tpl->get_path()).is_file()) {
      it_t = m_templates.erase(it_t);
      delete tpl;
    }
  }
}

//=========================================================================
SconesiteModule& Profile::get_module()
{
  return m_module;
}

//=========================================================================
scx::FilePath& Profile::get_path()
{
  return m_path;
}

//=========================================================================
Article* Profile::lookup_article(const std::string& name)
{
  for (std::list<Article*>::iterator it = m_articles.begin();
       it != m_articles.end(); ++it) {
    Article* article = (*it);
    if (article->get_name() == name) {
      return article;
    }
  }
  
  return 0;
}

//=========================================================================
const std::list<Article*>& Profile::articles() const
{
  return m_articles;
}

//=========================================================================
Article* Profile::create_article(const std::string& name)
{
  if (lookup_article(name)) {
    // Aricle already exists
    return 0;
  }

  scx::FilePath path = m_path + ARTDIR + name;
  scx::FilePath::mkdir(path,false,0777);
  scx::FilePath::mkdir(path + "files",false,0777);

  Article* article = new Article(*this,name,path);
  m_articles.push_back(article);

  return article;
}

//=========================================================================
Template* Profile::lookup_template(const std::string& name)
{
  for (std::list<Template*>::iterator it = m_templates.begin();
       it != m_templates.end(); ++it) {
    Template* tpl = (*it);
    if (tpl->get_name() == name) {
      return tpl;
    }
  }
  
  return 0;
}
