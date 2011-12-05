/* SconeServer (http://www.sconemad.com)

Sconesite Article

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

#ifndef sconesiteArticle_h
#define sconesiteArticle_h

#include <sconex/ScriptBase.h>
#include <sconex/Stream.h>
#include <sconex/FilePath.h>
#include "ArticleBody.h"

class SconesiteModule;
class Profile;
class Article;

//=========================================================================
// Sort predicates
//
bool ArticleSortDate(const Article* a, const Article* b);
bool ArticleSortName(const Article* a, const Article* b);

//=========================================================================
// ArticleMetaSorter - A functor predicate to sort articles using metadata
//
class ArticleMetaSorter {
public:
  ArticleMetaSorter(const std::string& meta, bool reverse);
  bool operator()(const Article* a, const Article* b);

private:
  std::string m_meta;
  bool m_reverse;
};

//=========================================================================
// Article - A SconeSite article, consisting of metadata and a body
//
class Article : public scx::ScriptObject {
public:

  Article(Profile& profile, 
	  int id, 
	  int parent_id,
	  const std::string& link);
  
  virtual ~Article();

  const std::string& get_name() const;
  const scx::FilePath& get_root() const;
  scx::FilePath get_filepath() const;

  bool process(Context& context);

  // Metadata
  bool set_meta(const std::string& name,
		scx::ScriptRef* value);

  scx::ScriptRef* get_meta(const std::string& name,
			   bool recurse=false) const;

  const ArticleHeading& get_headings() const;
  std::string get_href_path() const;
  
  void refresh(const scx::Date& purge_time);
  const scx::Date& get_access_time() const;
  void reset_access_time();

  static void register_article_type(const std::string& type,
				    scx::Provider<ArticleBody>* factory);
  static void unregister_article_type(const std::string& type,
				      scx::Provider<ArticleBody>* factory);

  // ScriptObject methods
  virtual std::string get_string() const;

  virtual scx::ScriptRef* script_op(const scx::ScriptAuth& auth,
				    const scx::ScriptRef& ref,
				    const scx::ScriptOp& op,
				    const scx::ScriptRef* right=0);

  virtual scx::ScriptRef* script_method(const scx::ScriptAuth& auth,
					const scx::ScriptRef& ref,
					const std::string& name,
					const scx::ScriptRef* args);

  typedef scx::ScriptRefTo<Article> Ref;

protected:

  Profile& m_profile;

  int m_id;
  int m_parent_id;
  std::string m_link;

  std::string m_name;
  scx::FilePath m_root;

  scx::Date m_access_time;

  ArticleBody::Ref* m_body;

  static void init();

  static scx::ProviderScheme<ArticleBody>* s_article_providers;
  
};

#endif
