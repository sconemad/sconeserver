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

#include "XMLDoc.h"

#include "sconex/Stream.h"
#include "sconex/FilePath.h"
#include "sconex/Arg.h"
#include "sconex/ArgStore.h"

class SconesiteModule;
class Profile;
class Article;

// Sort predicates:
bool ArticleSortDate(const Article* a, const Article* b);
bool ArticleSortName(const Article* a, const Article* b);

//=========================================================================
class ArticleMetaSorter {
public:
  ArticleMetaSorter(const std::string& meta, bool reverse);
  bool operator()(const Article* a, const Article* b);

private:
  std::string m_meta;
  bool m_reverse;
};


//=========================================================================
class XMLArticleBody : public XMLDoc {

public:
  XMLArticleBody(const std::string& name,
                 const scx::FilePath& path);

  virtual ~XMLArticleBody();

  virtual const ArticleHeading& get_headings() const;
  
protected:

  virtual void handle_open();
  virtual void handle_close();

  void scan_headings(xmlNode* start,int& index);

  ArticleHeading m_headings;
  
};


//=========================================================================
class Article : public scx::ArgObjectInterface {

public:

  Article(Profile& profile,
          const std::string& name,
          const scx::FilePath& path,
          Article* parent);
  
  virtual ~Article();

  const std::string& get_name() const;
  const scx::FilePath& get_root() const;
  scx::FilePath get_filepath() const;

  bool allow_access(Context& context);
  bool allow_upload(Context& context);
  
  bool process(Context& context);
  
  scx::Arg* get_meta(const std::string& name,bool recurse=false) const;
  const ArticleHeading& get_headings() const;
  std::string get_href_path() const;
  
  // Sub-articles
  void refresh(const scx::Date& purge_time);

  Article* lookup_article(const std::string& name);
  Article* find_article(const std::string& name,std::string& extra_path);
  void get_articles(std::list<Article*>& articles, bool recurse=false);

  Article* create_article(const std::string& name);
  bool remove_article(const std::string& name);
  
  // ArgObject interface
  virtual std::string name() const;
  virtual scx::Arg* arg_resolve(const std::string& name);
  virtual scx::Arg* arg_lookup(const std::string& name);
  virtual scx::Arg* arg_method(const scx::Auth& auth,const std::string& name,scx::Arg* args);

protected:

  bool evaluate_rule(Context& context, const std::string& meta);

  std::string m_name;
  scx::FilePath m_root;

  Profile& m_profile;

  scx::ArgStore m_metastore;

  Article* m_parent;
  std::list<Article*> m_articles;

  ArticleBody* m_body;
  
};

#endif