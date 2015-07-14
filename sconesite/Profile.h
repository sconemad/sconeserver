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

#include <sconesite/Article.h>
#include <sconesite/Template.h>
#include <http/Host.h>
#include <sconex/Stream.h>
#include <sconex/FilePath.h>
#include <sconex/ScriptBase.h>
#include <sconex/Database.h>
#include <sconex/Mutex.h>
namespace scs {
  
class SconesiteModule;

//=========================================================================
// Profile - A site profile for Sconesite, provides access to the templates
// and articles associated with this site.
//
class Profile : public scx::ScriptObject {
public:

  Profile(SconesiteModule& module,
          const std::string& name,
          http::Host* host,
          const std::string& dbtype);
  
  ~Profile();

  void refresh();

  SconesiteModule& get_module();
  const scx::FilePath& get_path();

  // Find an article by id or name
  // the article is loaded into the cache if it is not already cached
  Article::Ref* lookup_article(int id);
  Article::Ref* lookup_article(const std::string& href,
			       std::string& extra);

  // Create a new article under the specified parent.
  // pid identifies the parent article.
  // name is used for the path component from parent.
  // type is the document type.
  // Returns a ref to the new article on success, 0 on failure.
  Article::Ref* create_article(int pid, 
			       const std::string& name,
                               const std::string& type);

  // Remove the specified article and any associated data.
  // id identifies the article to remove.
  // Returns true on success, false on failure.
  bool remove_article(int id);

  // Rename and/or move article and any associated data.
  // id identifies the article to rename/move.
  // new_name specifies the new path component.
  // new_pid identifies the new parent (or -1 to keep the same).
  // Returns true on success, false on failure.
  bool rename_article(int id, 
		      const std::string& new_name,
		      int new_pid = -1);

  // Find a template by name
  Template* lookup_template(const std::string& name);

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

  typedef scx::ScriptRefTo<Profile> Ref;

protected:  

  friend class Article;

  // Article metadata
  bool set_meta(int id,
		const std::string& property,
		scx::ScriptRef* value);

  scx::ScriptRef* get_meta(int id,
			   const std::string& property) const;

  Article* load_article(int id,
			int pid,
			const std::string& link,
                        const std::string& type="");

  void configure_docroot(const std::string& docroot);
  
private:
  
  SconesiteModule& m_module;

  std::string m_name;
  http::Host::Ref* m_host;

  scx::Database::Ref* m_db;

  // Lock for article cache
  scx::RWLock m_cache_lock;

  // Caches loaded articles, accessed by article ID
  typedef HASH_TYPE<int,Article::Ref*> ArticleMap;
  ArticleMap m_articles;

  // Caches article link to ID mappings
  typedef HASH_TYPE<std::string,int> ArticleLinkMap;
  ArticleLinkMap m_article_links;

  // Templates
  typedef HASH_TYPE<std::string,Template::Ref*> TemplateMap;
  TemplateMap m_templates;

  scx::Time m_purge_threshold;
  
};

};
#endif
