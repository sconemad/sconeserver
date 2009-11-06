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


#include "Article.h"
#include "Profile.h"
#include "Context.h"
#include "ArgFile.h"

#include "sconex/Stream.h"
#include "sconex/StreamTransfer.h"
#include "sconex/Date.h"
#include "sconex/Kernel.h"
#include "sconex/FileDir.h"
#include "sconex/ArgProc.h"

//=========================================================================
bool ArticleSortDate(const Article* a, const Article* b)
{
  const scx::Arg* a_time = a->get_meta("time");
  const scx::Arg* b_time = b->get_meta("time");

  int a_int = a_time ? a_time->get_int() : 0;
  int b_int = b_time ? b_time->get_int() : 0;

  return (a_int > b_int);
}

//=========================================================================
bool ArticleSortName(const Article* a, const Article* b)
{
  return (a->get_name() < b->get_name());
}

//=========================================================================
ArticleMetaSorter::ArticleMetaSorter(const std::string& meta, bool reverse)
  : m_meta(meta),
    m_reverse(reverse)
{

}

//=========================================================================
bool ArticleMetaSorter::operator()(const Article* a, const Article* b)
{
  bool ret = !m_reverse;
  
  const scx::Arg* a_arg = a->get_meta(m_meta);
  const scx::Arg* b_arg = b->get_meta(m_meta);

  if (a_arg) {
    ret = m_reverse;
    if (b_arg) {
      if (dynamic_cast<const scx::ArgInt*>(a_arg) != 0) {
	int a_int = a_arg ? a_arg->get_int() : 0;
	int b_int = b_arg ? b_arg->get_int() : 0;
	ret = (m_reverse ? (a_int >= b_int) : (a_int < b_int));
      } else {
	ret = (m_reverse ? (a_arg->get_string() >= b_arg->get_string()) : (a_arg->get_string() < b_arg->get_string()));
      }
    }
  }

  delete a_arg;
  delete b_arg;

  return ret;
}

//=========================================================================
Article::Article(
  Profile& profile,
  const std::string& name,
  const scx::FilePath& path,
  Article* parent
) : XMLDoc(name,path,"article.xml"),
    m_profile(profile),
    m_metastore(path + "meta.txt"),
    m_parent(parent)
{
  m_metastore.load();
}

//=========================================================================
Article::~Article()
{
  for (std::list<Article*>::iterator it_a = m_articles.begin();
       it_a != m_articles.end(); ++it_a) {
    delete (*it_a);
  }
}

//=========================================================================
scx::Arg* Article::get_meta(const std::string& name,bool recurse) const
{
  Article* uc = const_cast<Article*>(this);
  scx::Arg* a = uc->m_metastore.arg_lookup(name);
  if (recurse && BAD_ARG(a) && m_parent) {
    delete a;
    a = uc->m_parent->get_meta(name,recurse);
  }
  return a;
}

//=========================================================================
std::string Article::get_href_path() const
{
  if (m_parent) {
    return m_parent->get_href_path() + m_name + "/";
  }
  return m_name;
}

//=========================================================================
void Article::refresh()
{
  // Add new articles
  scx::FileDir dir(m_root);
  while (dir.next()) {
    std::string name = dir.name();
    if (name != "." && name != "..") {
      if (!lookup_article(name)) {
        scx::FilePath path = dir.path();
        scx::FileStat stat(path + "article.xml");
        if (stat.is_file()) {
          Article* article = new Article(m_profile,name,path,this);
          m_articles.push_back(article);
          //          m_module.log("Adding article '" + name + "'");
          DEBUG_LOG("Adding article '" << name << "'");
        }
      }
    }
  }

  // Loop over existing articles
  for (std::list<Article*>::iterator it_a = m_articles.begin();
       it_a != m_articles.end();
       ++it_a) {
    Article* article = (*it_a);
    if (!scx::FileStat(article->get_filepath()).is_file()) {
      // Remove deleted article
      //        m_module.log("Removing article '" + article->get_name() + "'");
      DEBUG_LOG("Removing article '" << article->get_name() << "'");

      it_a = m_articles.erase(it_a);
      delete article;
    } else {
      // Refresh article
      article->refresh();
    }
  }
}

//=========================================================================
Article* Article::lookup_article(const std::string& name)
{
  if (name.empty()) return this;
  
  std::string::size_type is = name.find_first_of("/");
  std::string first = name;
  std::string second;
  if (is != std::string::npos) {
    first = name.substr(0,is);
    second = name.substr(is+1);
  }

  //  DEBUG_LOG("LOOKUP: " << name << "(" << first << "," << second << ")");
  
  for (std::list<Article*>::iterator it = m_articles.begin();
       it != m_articles.end(); ++it) {
    Article* article = (*it);
    if (article->get_name() == first) {
      if (second.empty()) {
        return article;
      } else {
        return article->lookup_article(second);
      }
    }
  }

  return 0;
}

//=========================================================================
Article* Article::find_article(const std::string& name,std::string& extra_path)
{
  if (name.empty()) return this;
  
  std::string::size_type is = name.find_first_of("/");
  std::string first = name;
  std::string second;
  if (is != std::string::npos) {
    first = name.substr(0,is);
    second = name.substr(is+1);
  }

  //  DEBUG_LOG("LOOKUP: " << name << "(" << first << "," << second << ")");
  
  for (std::list<Article*>::iterator it = m_articles.begin();
       it != m_articles.end(); ++it) {
    Article* article = (*it);
    if (article->get_name() == first) {
      if (second.empty()) {
        extra_path = "";
        return article;
      } else {
        return article->find_article(second,extra_path);
      }
    }
  }

  extra_path = first;
  if (!second.empty()) {
    extra_path += "/" + second;
  }
  return this;
}

//=========================================================================
void Article::get_articles(std::list<Article*>& articles, bool recurse)
{
  for (std::list<Article*>::iterator it_a = m_articles.begin();
       it_a != m_articles.end();
       ++it_a) {
    Article* article = (*it_a);
    articles.push_back(article);
    if (recurse) {
      article->get_articles(articles,recurse);
    }
  }
}

//=========================================================================
Article* Article::create_article(const std::string& name)
{
  if (lookup_article(name)) {
    // Aricle already exists
    return 0;
  }

  scx::FilePath path = m_root + name;
  scx::FilePath::mkdir(path,false,0777);
  scx::FilePath::mkdir(path + "files",false,0777);

  Article* article = new Article(m_profile,name,path,this);
  scx::FilePath apath = article->get_filepath();

  scx::File file;
  if (scx::Ok != file.open(apath,scx::File::Write|scx::File::Create)) {
    delete article;
    return 0;
  }

  file.write("<article>\n");
  file.write("\n<p>\nwrite something here...\n</p>\n\n");
  file.write("</article>\n");
  file.close();
  
  m_articles.push_back(article);
  return article;
}

//=========================================================================
bool Article::remove_article(const std::string& name)
{
  for (std::list<Article*>::iterator it_a = m_articles.begin();
       it_a != m_articles.end();
       ++it_a) {
    Article* article = (*it_a);
    if (article->get_name() == name) {
      DEBUG_LOG("Removing article '" << article->get_root().path() << "'");
      m_articles.erase(it_a);
      scx::FilePath::rmdir(article->get_root(),true);
      delete article;
      return true;

    }
  }
  return false;
}

//=========================================================================
scx::Arg* Article::arg_resolve(const std::string& name)
{
  return XMLDoc::arg_resolve(name);
}

//=========================================================================
scx::Arg* Article::arg_lookup(const std::string& name)
{
  // Methods
  if ("create" == name ||
      "remove" == name ||
      "update" == name ||
      "get_articles" == name ||
      "get_all_articles" == name ||
      "lookup_article" == name ||
      "add_file" == name ||
      "remove_file" == name ||
      "get_files" == name ||
      "lookup_meta" == name) {
    return new_method(name);
  }

  // Properties
  if ("link" == name) return new scx::ArgString(get_href_path());
  if ("meta" == name) return new scx::ArgObject(&m_metastore);
  if ("title" == name) {
    scx::Arg* a_title = m_metastore.arg_lookup("title");
    if (!BAD_ARG(a_title)) {
      std::string title = a_title->get_string();
      if (!title.empty()) {
        return new scx::ArgString(title);
      }
    }
    delete a_title;
    if (!m_name.empty()) {
      return new scx::ArgString(m_name);
    }
    return new scx::ArgString("");
  }
  if ("parent" == name) {
    if (m_parent) {
      return new scx::ArgObject(m_parent);
    }
    return 0;
  }

  return XMLDoc::arg_lookup(name);
}

//=========================================================================
scx::Arg* Article::arg_method(const scx::Auth& auth,const std::string& name,scx::Arg* args)
{
  scx::ArgList* l = dynamic_cast<scx::ArgList*>(args);

  if (name == "create") {
    if (!auth.trusted()) return new scx::ArgError("Not permitted");

    const scx::Arg* a_name = l->get(0);
    if (!a_name) return new scx::ArgError("No article name specified");
    const std::string article_name = a_name->get_string();
    if (article_name.empty()) return new scx::ArgError("Empty article name");
    Article* article = create_article(article_name);
    if (!article) return new scx::ArgError("Could not create article");
    return new scx::ArgObject(article);
  }

  if (name == "remove") {
    if (!auth.trusted()) return new scx::ArgError("Not permitted");

    const scx::Arg* a_name = l->get(0);
    if (!a_name) return new scx::ArgError("No article name specified");
    const std::string article_name = a_name->get_string();
    if (article_name.empty()) return new scx::ArgError("Empty article name");
    if (!remove_article(article_name)) {
      return new scx::ArgError("Failed to remove article");
    }
    return 0;
  }

  if (name == "update") {
    if (!auth.trusted()) return new scx::ArgError("Not permitted");

    scx::Arg* a_file = l->get(0);
    ArgFile* f_file = dynamic_cast<ArgFile*>(a_file);
    if (!f_file) {
      return new scx::ArgError("No file specified");
    }
    const scx::FilePath& srcpath = f_file->get_path();

    scx::FilePath dstpath = get_filepath();
    log("Update article moving '" + srcpath.path() + "' to '" + dstpath.path() + "'");
    if (!scx::FilePath::move(srcpath,dstpath)) {
      return new scx::ArgError("Could not replace article");
    }
    return 0;
  }

  if (name == "get_articles" ||
      name == "get_all_articles") {
    if (!auth.trusted()) return new scx::ArgError("Not permitted");

    std::list<Article*> articles;
    get_articles(articles,(name == "get_all_articles"));

    const scx::Arg* a_sort = l->get(0);
    if (a_sort) {
      std::string sort = a_sort->get_string();
      bool reverse = false;
      if (sort[0] == '!') {
        sort = sort.substr(1);
        reverse = true;
      }
      articles.sort(ArticleMetaSorter(sort,reverse));
    }
    int count = 9999;
    const scx::ArgInt* a_max = dynamic_cast<const scx::ArgInt*>(l->get(1));
    if (a_max) {
      count = a_max->get_int();
    }
      
    scx::ArgList* artlist = new scx::ArgList();
    for (std::list<Article*>::const_iterator it = articles.begin();
	 it != articles.end();
	 ++it) {
      artlist->give(new scx::ArgObject(*it));
      if (--count == 0) {
	break;
      }
    }
    return artlist;
  }

  if (name == "lookup_article") {
    scx::Arg* a_name = l->get(0);
    if (!a_name) return new scx::ArgError("No article name specified");
    std::string s_name = a_name->get_string();
    Article* art = lookup_article(s_name);
    if (art) {
      return new scx::ArgObject(art);
    }
    return 0;
  }

  
  if (name == "add_file") {
    if (!auth.trusted()) return new scx::ArgError("Not permitted");

    scx::Arg* a_file = l->get(0);
    ArgFile* f_file = dynamic_cast<ArgFile*>(a_file);
    if (!f_file) {
      return new scx::ArgError("No file specified");
    }
    const scx::FilePath& srcpath = f_file->get_path();

    scx::FilePath dstpath = get_root() + f_file->get_filename();
    log("Add file moving '" + srcpath.path() + "' to '" + dstpath.path() + "'");
    if (!scx::FilePath::move(srcpath,dstpath)) {
      return new scx::ArgError("Could not move file");
    }
    return 0;
  }

  if (name == "remove_file") {
    if (!auth.trusted()) return new scx::ArgError("Not permitted");

    scx::Arg* a_file = l->get(0);
    if (!a_file) return new scx::ArgError("No file specified");
    std::string file = a_file->get_string();
    if (file.empty()) return new scx::ArgError("No file specified");
    if (file.find("/") != std::string::npos ||
	file.find("..") != std::string::npos) {
      return new scx::ArgError("Could not remove file");
    }
    scx::FilePath path = get_root() + file;
    if (!scx::FilePath::rmfile(path)) {
      DEBUG_LOG_ERRNO("Cannot remove file '"+path.path()+"'");
      return new scx::ArgError("Could not remove file");
    }
    return 0;
  }

  if (name == "get_files") {
    if (!auth.trusted()) return new scx::ArgError("Not permitted");

    scx::ArgList* filelist = new scx::ArgList();
    scx::FileDir files(m_root);
    while (files.next()) {
      std::string file = files.name();
      if (file != "." &&
          file != ".." &&
          file != "article.xml" && file != "article.xml~" &&
          file != "meta.txt" && file != "meta.txt~") {
        if (!lookup_article(file)) {
          if (files.stat().is_dir()) {
	    // Ignore directories for now?
	    // filelist->give(new scx::ArgString(file+"/"));
          } else {
            //            filelist->give(new scx::ArgString(file));
            filelist->give(new ArgFile(files.path(),file));
          }
        }
      }
    }
    return filelist;
  }

  if (name == "lookup_meta") {
    scx::Arg* a_name = l->get(0);
    if (!a_name) return new scx::ArgError("No name specified");
    std::string s_name = a_name->get_string();
    return get_meta(s_name,true);
  }

  
  return XMLDoc::arg_method(auth,name,args);
}
