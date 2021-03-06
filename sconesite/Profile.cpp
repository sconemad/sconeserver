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

#include <sconesite/Profile.h>
#include <sconesite/Article.h>
#include <sconesite/Template.h>
#include <sconesite/SconesiteModule.h>
#include <sconex/Stream.h>
#include <sconex/StreamTransfer.h>
#include <sconex/Date.h>
#include <sconex/Kernel.h>
#include <sconex/File.h>
#include <sconex/FileDir.h>
#include <sconex/ScriptTypes.h>
#include <sconex/Database.h>
#include <sconex/Log.h>
#include <memory> // Using unique_ptr
namespace scs {

// Uncomment to enable debug logging
//#define SCONESITEPROFILE_DEBUG_LOG(m) DEBUG_LOG(m)

#ifndef SCONESITEPROFILE_DEBUG_LOG
#  define SCONESITEPROFILE_DEBUG_LOG(m)
#endif

#define LOG(msg) scx::Log("sconesite").attach("id",m_name).submit(msg);

const char* ARTDIR = "art";
const char* TPLDIR = "tpl";

//=========================================================================
Profile::Profile(
  SconesiteModule& module,
  const std::string& name,
  http::Host* host,
  scx::Database* db
) : m_module(module),
    m_name(name),
    m_host(new http::Host::Ref(host)),
    m_db(new scx::Database::Ref(db)),
    m_use_default_templates(true)
{
  m_parent = &m_module;

  check_database();

  // Map all requests to the sconesite module
  scx::ScriptList* ml = new scx::ScriptList();
  ml->give(scx::ScriptString::new_ref(m_name));
  m_host->object()->add_path_map("/",m_module.name(),new scx::ScriptRef(ml));
  
  // Enable automation session allocation
  m_host->object()->set_param("auto_session", scx::ScriptInt::new_ref(1));
  
  m_templates.add(get_path() + TPLDIR);

  refresh();
}

//=========================================================================
Profile::~Profile()
{
  for (ArticleMap::iterator it_a = m_articles.begin();
       it_a != m_articles.end(); ++it_a) {
    delete it_a->second;
  }

  delete m_db;
  delete m_host;
}

//=========================================================================
void Profile::refresh()
{
  m_templates.refresh();

  // Calculate purge time for cached articles
  scx::Date purge_time;
  if (m_purge_threshold.seconds() != 0) {
    purge_time = scx::Date::now() - scx::Time(m_purge_threshold);
  }
  
  // Scan cached articles and purge
  scx::RWLocker locker(m_cache_lock,true,scx::RWLock::Write);
  for (ArticleMap::iterator it_a = m_articles.begin();
       it_a != m_articles.end(); ) {
    Article::Ref* article = it_a->second;
    if (article->object()->get_access_time() < purge_time) {
      LOG("Purging article: /" + article->object()->get_href_path());
      m_articles.erase(it_a++);
      delete article;
    } else {
      ++it_a;
    }
  }
}

//=========================================================================
SconesiteModule& Profile::get_module()
{
  return m_module;
}

//=========================================================================
const scx::FilePath& Profile::get_path()
{
  return m_host->object()->get_hostroot();
}

//=========================================================================
Article::Ref* Profile::lookup_article(int id)
{
  // Look for article in cache
  scx::RWLocker locker(m_cache_lock);
  ArticleMap::const_iterator id_it = m_articles.find(id);
  if (id_it != m_articles.end()) {
    SCONESITEPROFILE_DEBUG_LOG("Article id cache hit for '" + id + "'");
    Article::Ref* article = id_it->second;
    article->object()->reset_access_time();
    return article->ref_copy();
  }
  locker.unlock();

  // Look for article in database
  std::unique_ptr<scx::DbQuery> query(m_db->object()->new_query(
    "SELECT path,parent FROM article WHERE id = ?"));

  std::string link;
  int cid = id;
  int pid = 0;

  while (cid != 1) {

    scx::ScriptList::Ref args(new scx::ScriptList());
    args.object()->give(scx::ScriptInt::new_ref(cid));
    query->exec(&args);
    
    if (!query->next_result()) {
      return 0;

    } else {
      scx::ScriptRef* row_ref = query->result();
      scx::ScriptMap* row = dynamic_cast<scx::ScriptMap*>(row_ref->object());
      if (row) {
	scx::ScriptRef* a_path = row->lookup("path");
	if (a_path) {
	  link = a_path->object()->get_string() + "/" + link;
	}
	
	scx::ScriptRef* a_parent = row->lookup("parent");
	if (a_parent) {
	  // Save initial parent ID as it is needed when constructing article
	  if (cid == id) pid = a_parent->object()->get_int();
	  cid = a_parent->object()->get_int();
	}
      }
      delete row_ref;
    }
  }

  locker.lock();
  ArticleLinkMap::iterator itL = m_article_links.find(link);
  if (itL == m_article_links.end()) {
    SCONESITEPROFILE_DEBUG_LOG("Article link cache add: " + id + 
			       " -> '" + link + "'");
    locker.lock(scx::RWLock::Write); // Relock for writing
    m_article_links[link] = id;
  }

  locker.lock(scx::RWLock::Write);
  return new Article::Ref(load_article(id,pid,link));
}

//=========================================================================
Article::Ref* Profile::lookup_article(const std::string& href,
				      std::string& extra)
{
  // Look for article in cache
  scx::RWLocker locker(m_cache_lock);
  ArticleLinkMap::const_iterator it = m_article_links.find(href);
  if (it != m_article_links.end()) {
    SCONESITEPROFILE_DEBUG_LOG("Article link cache hit for '" + href + "'");
    locker.unlock();
    return lookup_article(it->second);
  }
  locker.unlock();

  // Look for article in database
  std::unique_ptr<scx::DbQuery> query(m_db->object()->new_query(
    "SELECT id FROM article WHERE parent = ? AND path = ?"));

  std::string key = href;
  std::string path;
  std::string link;
  int id = 1;
  int pid = 0;
  while (true) {
    std::string::size_type is = key.find_first_of("/");
    path = key.substr(0,is);
    scx::ScriptList::Ref args(new scx::ScriptList());
    args.object()->give(scx::ScriptInt::new_ref(id));
    args.object()->give(scx::ScriptString::new_ref(path));
    query->exec(&args);
    if (!query->next_result()) {
      extra = key;
      locker.lock();
      ArticleLinkMap::iterator itL = m_article_links.find(link);
      if (itL == m_article_links.end()) {
	SCONESITEPROFILE_DEBUG_LOG("Article link cache add: " + id + 
				   " -> '" + link + "'");
	locker.lock(scx::RWLock::Write);
	m_article_links[link] = id;
      }
      ArticleMap::iterator itA = m_articles.find(id);
      if (itA != m_articles.end()) {
	Article::Ref* article = itA->second;
	article->object()->reset_access_time();
	return article->ref_copy();
      }
      locker.lock(scx::RWLock::Write);
      return new Article::Ref(load_article(id,pid,link));

    } else {
      scx::ScriptRef* row_ref = query->result();
      scx::ScriptMap* row = dynamic_cast<scx::ScriptMap*>(row_ref->object());
      if (row) {
	scx::ScriptRef* a_id = row->lookup("id");
	if (a_id) {
	  pid = id; id = a_id->object()->get_int();
	}
      }
      delete row_ref;
    }
    if (is == std::string::npos) {
      key = "";
    } else {
      key = key.substr(is+1);
    }
    link += path + "/";
  }

  return 0;
}

//=========================================================================
Article::Ref* Profile::create_article(int pid,
				      const std::string& name,
                                      const std::string& type)
{
  scx::Log("sconesite").attach("id",m_name)
    .attach("name",name).attach("pid",pid).attach("type",type)
    .submit("Create article");
  
  if (name.empty()) { //TODO: check name validity
    LOG("Invalid article name '" + name + "'");
    return 0;
  }
  
  Article::Ref* parent = lookup_article(pid);
  if (!parent) {
    LOG("Invalid parent article: " + pid);
    return 0;
  }

  std::string link = parent->object()->get_href_path() + name + "/";
  scx::FilePath path = parent->object()->get_root() + name;
  delete parent;

  std::string extra;
  Article::Ref* existing = lookup_article(link,extra);
  delete existing;
  if (existing && extra.empty()) {
    // Article already exists
    LOG("Article '" + link + "' already exists");
    return 0;
  }

  // Create article directory
  if (!scx::FilePath::mkdir(path,false,00770)) {
    LOG("Failed to create article directory '" + path.path() + "'");
    return 0;
  }

  std::unique_ptr<scx::DbQuery> query(m_db->object()->new_query(
    "INSERT INTO article (parent,path) VALUES (?,?)"));

  scx::ScriptList::Ref args(new scx::ScriptList());
  args.object()->give(scx::ScriptInt::new_ref(pid));
  args.object()->give(scx::ScriptString::new_ref(name));
  if (!query->exec(&args)) {
    LOG("Failed to insert article metadata into db");
    return 0;
  }

  int id = query->insert_id();
  if (id <= 0) {
    LOG("Could not determine id for new article");
    return 0;
  }

  scx::RWLocker locker(m_cache_lock,true,scx::RWLock::Write);

  Article* article = load_article(id,pid,link,type);

  SCONESITEPROFILE_DEBUG_LOG("Article link cache add: " << id <<
			     " -> '" << link << "'");
  m_article_links[link] = id;

  locker.unlock();

  scx::Log("sconesite").attach("id",m_name)
    .attach("aid",id).attach("link",link)
    .submit("Article created");
  
  return new Article::Ref(article);
}

//=========================================================================
bool Profile::remove_article(int id)
{
  scx::Log("sconesite").attach("id",m_name)
    .attach("aid",id)
    .submit("Remove article");

  Article::Ref* article = lookup_article(id);
  if (!article) {
    LOG("Cannot find article");
    return false;
  }
  std::string link = article->object()->get_href_path();
  scx::FilePath path = article->object()->get_root();
  delete article;
  
  if (!scx::FilePath::rmdir(path,true)) {
    LOG("Failed to remove article directory '" + path.path() + "'");
    return false;
  }
  
  std::unique_ptr<scx::DbQuery> query(m_db->object()->new_query(
    "DELETE FROM article WHERE id = ?"));

  scx::ScriptList::Ref args(new scx::ScriptList());
  args.object()->give(scx::ScriptInt::new_ref(id));
  if (!query->exec(&args)) {
    LOG("Failed to remove article from db");
  }

  // Remove from caches
  scx::RWLocker locker(m_cache_lock,true,scx::RWLock::Write);

  article = m_articles[id];
  m_articles.erase(id);
  m_article_links.erase(link);

  locker.unlock();

  delete article;

  scx::Log("sconesite").attach("id",m_name)
    .attach("aid",id).attach("link",link)
    .submit("Article removed");

  return true;
}

//=========================================================================
bool Profile::rename_article(int id,
			     const std::string& new_name,
			     int new_pid)
{
  scx::Log("sconesite").attach("id",m_name)
    .attach("aid",id).attach("new_name",new_name).attach("new_pid",new_pid)
    .submit("Rename article");
  
  Article::Ref* article = lookup_article(id);
  if (!article) {
    LOG("Cannot find article");
    return false;
  }
  std::string link = article->object()->get_href_path();
  scx::FilePath old_path = article->object()->get_root();
  std::string old_name = article->object()->get_name();
  delete article;

  scx::FilePath new_path = old_path;
  new_path.pop();
  Article::Ref* new_parent = lookup_article(new_pid);
  if (new_parent) {
    new_path = new_parent->object()->get_root();
    delete new_parent;
  }

  if (!new_name.empty()) {
    new_path += new_name;
  } else {
    new_path += old_name;
  }

  if (old_path == new_path) {
    LOG("Nothing to do");
    return false;
  }

  if (!scx::FilePath::move(old_path, new_path)) {
    LOG("Failed to move article directory '" + old_path.path() +
        "' to '" + new_path.path() + "'");
    return false;
  }

  if (new_parent) {
    // Update parent id field
    std::unique_ptr<scx::DbQuery> query(m_db->object()->new_query(
      "UPDATE article SET parent = ? WHERE id = ?"));
    scx::ScriptList::Ref args(new scx::ScriptList());
    args.object()->give(scx::ScriptInt::new_ref(new_pid));
    args.object()->give(scx::ScriptInt::new_ref(id));
    if (!query->exec(&args)) {
      LOG("Failed to update parent id in db");
      return false;
    }
  }

  if (!new_name.empty()) {
    // Update path field
    std::unique_ptr<scx::DbQuery> query(m_db->object()->new_query(
      "UPDATE article SET path = ? WHERE id = ?"));
    scx::ScriptList::Ref args(new scx::ScriptList());
    args.object()->give(scx::ScriptString::new_ref(new_name));
    args.object()->give(scx::ScriptInt::new_ref(id));
    if (!query->exec(&args)) {
      LOG("Failed to update path in db");
      return false;
    }
  }  

  // Remove from caches
  scx::RWLocker locker(m_cache_lock,true,scx::RWLock::Write);
  article = m_articles[id];
  m_articles.erase(id);
  m_article_links.erase(link);
  delete article;

  // Child articles also need to be removed and deleted
  for (ArticleLinkMap::iterator itl = m_article_links.begin();
       itl != m_article_links.end();
       ++itl) {
    if (itl->first.find(link) == 0) {
      // i.e. if the link starts with our moved article link, it is a child
      SCONESITEPROFILE_DEBUG_LOG("Removing " << itl->second << 
				 ": " << itl->first);
      article = m_articles[itl->second];
      m_articles.erase(itl->second);
      m_article_links.erase(itl);
      delete article;
    }
  }

  locker.unlock();

  scx::Log("sconesite").attach("id",m_name)
    .attach("aid",id)
    .submit("Article renamed");

  return true;
}

//=========================================================================
Template* Profile::lookup_template(const std::string& name)
{
  Template* t = m_templates.lookup(name);
  if (t) return t;
  if (m_use_default_templates) t = m_module.lookup_template(name);
  return t;
}

//=========================================================================
std::string Profile::get_string() const
{
  return m_name;
}

//=========================================================================
scx::ScriptRef* Profile::script_op(const scx::ScriptAuth& auth,
				   const scx::ScriptRef& ref,
				   const scx::ScriptOp& op,
				   const scx::ScriptRef* right)
{
  if (op.type() == scx::ScriptOp::Lookup) {
    const std::string name = right->object()->get_string();

    // Methods
    if ("set_purge_threshold" == name ||
	"lookup" == name ||
	"create_article" == name ||
	"remove_article" == name ||
	"rename_article" == name ||
        "add_templates" == name ||
        "set_use_default_templates" == name) {
      return new scx::ScriptMethodRef(ref,name);
    }

    // Properties
    if ("purge_threshold" == name) 
      return new scx::ScriptRef(m_purge_threshold.new_copy());

    if ("path" == name) 
      return scx::ScriptString::new_ref(get_path().path());

    if ("db" == name) 
      return m_db->ref_copy();
    
    if ("article_cache" == name) {
      scx::ScriptList::Ref* list = 
	new scx::ScriptList::Ref(new scx::ScriptList());
      scx::RWLocker locker(m_cache_lock);
      for (ArticleMap::iterator it_a = m_articles.begin();
	   it_a != m_articles.end();
	   ++it_a) {
	scx::ScriptMap::Ref* map =
	  new scx::ScriptMap::Ref(new scx::ScriptMap());
	Article* article = it_a->second->object();
	map->object()->give("last_access",
          new scx::ScriptRef(article->get_access_time().new_copy()));
	map->object()->give("article",
			    new Article::Ref(article));
	list->object()->give(map);
      }
      return list;
    }

    if ("use_default_templates" == name) 
      return scx::ScriptInt::new_ref(m_use_default_templates);
  }

  return scx::ScriptObject::script_op(auth,ref,op,right);
}

//=========================================================================
scx::ScriptRef* Profile::script_method(const scx::ScriptAuth& auth,
				       const scx::ScriptRef& ref,
				       const std::string& name,
				       const scx::ScriptRef* args)
{
  if ("set_purge_threshold" == name) {
    if (!auth.admin()) return scx::ScriptError::new_ref("Not permitted");

    const scx::ScriptInt* a_thr = 
      scx::get_method_arg<scx::ScriptInt>(args,0,"value");
    if (!a_thr) 
      return scx::ScriptError::new_ref("Must specify value");
    int n_thr = a_thr->get_int();
    if (n_thr < 0) 
      return scx::ScriptError::new_ref("Value must be >= 0");
    
    m_purge_threshold = scx::Time(n_thr);
    return 0;
  }

  if ("lookup" == name) {

    const scx::ScriptString* a_name = 
      scx::get_method_arg<scx::ScriptString>(args,0,"name");
    if (a_name) {
      std::string extra;
      Article::Ref* art = lookup_article(a_name->get_string(),extra);
      return art;
    }

    const scx::ScriptInt* a_id = 
      scx::get_method_arg<scx::ScriptInt>(args,0,"id");
    if (a_id) {
      std::string extra;
      Article::Ref* art = lookup_article(a_id->get_int());
      return art;
    }

    return scx::ScriptError::new_ref("No name or id specified");
  }

  if ("create_article" == name) {
    const scx::ScriptInt* a_pid = 
      scx::get_method_arg<scx::ScriptInt>(args,0,"pid");
    if (!a_pid) return scx::ScriptError::new_ref("No parent id specified");

    const scx::ScriptString* a_name = 
      scx::get_method_arg<scx::ScriptString>(args,1,"name");
    if (!a_name) return scx::ScriptError::new_ref("No name specified");

    const scx::ScriptString* a_type = 
      scx::get_method_arg<scx::ScriptString>(args,2,"type");

    Article::Ref* art = create_article(a_pid->get_int(),
				       a_name->get_string(),
                                       a_type ? a_type->get_string() : "");
    if (!art) 
      return scx::ScriptError::new_ref("Create article failed");
    
    return art;
  }

  if ("remove_article" == name) {
    const scx::ScriptInt* a_id = 
      scx::get_method_arg<scx::ScriptInt>(args,0,"id");
    if (!a_id) return scx::ScriptError::new_ref("No id specified");

    if (!remove_article(a_id->get_int())) {
      return scx::ScriptError::new_ref("Remove article failed");
    }

    return 0;
  }

  if ("rename_article" == name) {
    const scx::ScriptInt* a_id = 
      scx::get_method_arg<scx::ScriptInt>(args,0,"id");
    if (!a_id) return scx::ScriptError::new_ref("No id specified");

    int i=1;
    const scx::ScriptString* a_new_name = 
      scx::get_method_arg<scx::ScriptString>(args,i,"name");
    if (a_new_name) ++i;

    const scx::ScriptInt* a_new_pid = 
      scx::get_method_arg<scx::ScriptInt>(args,i,"pid");

    if (!a_new_pid && !a_new_name) 
      return scx::ScriptError::new_ref("No new name or parent id specified");

    if (!rename_article(a_id->get_int(),
			(a_new_name ? a_new_name->get_string() : ""),
			(a_new_pid ? a_new_pid->get_int() : -1) )) {
      return scx::ScriptError::new_ref("Rename article failed");
    }

    return 0;
  }

  if ("add_templates" == name) {
    if (!auth.admin()) return scx::ScriptError::new_ref("Not permitted");

    const scx::ScriptString* a_path =
      scx::get_method_arg<scx::ScriptString>(args,0,"path");
    if (!a_path) {
      return scx::ScriptError::new_ref("No path specified");
    }
    m_templates.add(a_path->get_string());

    return 0;
  }

  if ("set_use_default_templates" == name) {
    if (!auth.admin()) return scx::ScriptError::new_ref("Not permitted");

    const scx::ScriptInt* a_value =
      scx::get_method_arg<scx::ScriptInt>(args,0,"value");
    if (!a_value) {
      return scx::ScriptError::new_ref("No value specified");
    }

    m_use_default_templates = (a_value->get_int() != 0);
    
    return 0;
  }
  
  return scx::ScriptObject::script_method(auth,ref,name,args);
}

//=========================================================================
bool Profile::set_meta(int id,
		       const std::string& property,
		       scx::ScriptRef* value)
{
  std::unique_ptr<scx::DbQuery> query(m_db->object()->new_query(
    "UPDATE article set "+property+" = ? WHERE id = ?"));

  scx::ScriptList::Ref args(new scx::ScriptList());
  args.object()->give(value);
  args.object()->give(scx::ScriptInt::new_ref(id));
  bool result = query->exec(&args);

  SCONESITEPROFILE_DEBUG_LOG("set_meta " << id << 
   ":" << property << "=" << 
   (value ? value->object()->get_string() : "NULL") <<
   (result?" OK":" FAILED"));
  return result;
}

//=========================================================================
scx::ScriptRef* Profile::get_meta(int id,
				  const std::string& property) const
{
  std::unique_ptr<scx::DbQuery> query(m_db->object()->new_query(
    "SELECT "+property+" FROM article WHERE id = ?"));

  scx::ScriptList::Ref args(new scx::ScriptList());
  args.object()->give(scx::ScriptInt::new_ref(id));
  query->exec(&args);

  scx::ScriptRef* result = 0;
  if (query->next_result()) {
    scx::ScriptRef* row_ref = query->result_list();
    scx::ScriptList* row = dynamic_cast<scx::ScriptList*>(row_ref->object());
    if (row) result = row->take(0);
    delete row_ref;
  }
  
  SCONESITEPROFILE_DEBUG_LOG("get_meta " << id << 
			     ":" << property << "=" <<
			     (result?result->object()->get_string():"NULL"));
  return result;
}

//=========================================================================
Article* Profile::load_article(int id, int pid, const std::string& link,
                               const std::string& type)
{
  SCONESITEPROFILE_DEBUG_LOG("Loading article [" << id << "] p=" << pid << 
			     " " << link << " t=" << type);
  Article* art = new Article(*this,id,pid,link,type);
  // Article cache must be locked for writing before calling this method!
  m_articles[id] = new Article::Ref(art);
  return art;
}

//=============================================================================
void Profile::check_database()
{
  if (!m_db) throw std::exception();

  // Attempt to create article table if not present
  m_db->object()->simple_query(
    "CREATE TABLE IF NOT EXISTS article ( "
    "id        INTEGER PRIMARY KEY AUTOINCREMENT, "
    "parent    INTEGER, "
    "path      VARCHAR(128) )");

  // Create root article if not present
  if (m_db->object()->simple_query_num(
        "SELECT COUNT(*) FROM article WHERE id = 1") != 1) {
    m_db->object()->simple_query(
      "INSERT INTO article (id,parent,path) VALUES (1,0,\"\")");
  }
}
  
};
