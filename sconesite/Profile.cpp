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

#include <sconex/Stream.h>
#include <sconex/StreamTransfer.h>
#include <sconex/Date.h>
#include <sconex/Kernel.h>
#include <sconex/File.h>
#include <sconex/FileDir.h>
#include <sconex/ScriptTypes.h>
#include <sconex/Database.h>

#include <memory> // Using auto_ptr

const char* ARTDIR = "art";
const char* TPLDIR = "tpl";

//=========================================================================
Profile::Profile(
  SconesiteModule& module,
  const std::string& name,
  const scx::FilePath& path
) : m_module(module),
    m_name(name),
    m_path(path),
    m_db(0)
{
  m_parent = &m_module;

  // Open article database
  scx::ScriptMap::Ref args(new scx::ScriptMap());
  args.object()->give("profile",scx::ScriptString::new_ref(name));
  m_db = scx::Database::open("MySQL",&args);

  refresh();
}

//=========================================================================
Profile::~Profile()
{
  for (ArticleMap::iterator it_a = m_articles.begin();
       it_a != m_articles.end(); ++it_a) {
    delete it_a->second;
  }
  for (TemplateMap::iterator it_t = m_templates.begin();
       it_t != m_templates.end(); ++it_t) {
    delete it_t->second;
  }
}

//=========================================================================
void Profile::refresh()
{
  // Add new templates
  scx::FileDir dir(m_path + TPLDIR);
  while (dir.next()) {
    std::string file = dir.name();
    if (file != "." && file != "..") {
      std::string::size_type idot = file.find_first_of(".");
      if (idot != std::string::npos) {
        std::string name = file.substr(0,idot);
        std::string extn = file.substr(idot+1,std::string::npos);
        if (extn == "xml" && !lookup_template(name)) {
          Template* tpl = new Template(*this,name,dir.root());
          m_templates[name] = new Template::Ref(tpl);
          m_module.log("Adding template '" + name + "'");
        }
      }
    }
  }
  
  // Remove deleted templates
  for (TemplateMap::iterator it_t = m_templates.begin();
       it_t != m_templates.end();
       ++it_t) {
    Template::Ref* tpl_ref = it_t->second;
    Template* tpl = tpl_ref->object();
    if (!scx::FileStat(tpl->get_filepath()).is_file()) {
      m_module.log("Removing template '" + tpl->get_name() + "'");
      it_t = m_templates.erase(it_t);
      delete tpl_ref;
    }
  }

  //TODO: purge from article cache

  /*

  // Create index article if it doesn't exist
  if (m_index == 0) {
    m_index = new Article(*this,"",m_path + ARTDIR,0);
  }

  // Calculate purge time for article data
  scx::Date purge_time;
  if (m_purge_threshold.seconds() != 0) {
    purge_time = scx::Date::now() - scx::Time(m_purge_threshold);
  }
  
  // Scan articles
  m_index->refresh(purge_time);  

  */
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
Article* Profile::lookup_article(int id)
{
  // Look for article in cache
  ArticleMap::const_iterator id_it = m_articles.find(id);
  if (id_it != m_articles.end()) {
    DEBUG_LOG("Article id cache hit for '" << id << "'");
    return id_it->second->object();
  }

  // Look for article in database
  std::auto_ptr<scx::DbQuery> query(
    m_db->object()->new_query("SELECT path,parent FROM article WHERE id = ?"));

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

  ArticleLinkMap::iterator itL = m_article_links.find(link);
  if (itL == m_article_links.end()) {
    DEBUG_LOG("Article link cache add: " << id << " -> '" << link << "'");
    m_article_links[link] = id;
  }

  return load_article(id,pid,link);
}

//=========================================================================
Article* Profile::lookup_article(const std::string& href, std::string& extra)
{
  // Look for article in cache
  ArticleLinkMap::const_iterator it = m_article_links.find(href);
  if (it != m_article_links.end()) {
    DEBUG_LOG("Article link cache hit for '" << href << "'");
    return lookup_article(it->second);
  }

  // Look for article in database
  std::auto_ptr<scx::DbQuery> query(
    m_db->object()->new_query(
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
      ArticleLinkMap::iterator itL = m_article_links.find(link);
      if (itL == m_article_links.end()) {
	DEBUG_LOG("Article link cache add: " << id 
		  << " -> '" << link << "'");
	m_article_links[link] = id;
      }
      ArticleMap::iterator itA = m_articles.find(id);
      if (itA != m_articles.end()) {
	return itA->second->object();
      }
      return load_article(id,pid,link);

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

  /*
  std::auto_ptr<scx::DbQuery> query(
    m_db->object()->new_query("SELECT id FROM article WHERE link = ?"));
  
  std::string::size_type len = name.length();
  std::string key = name;

  if (len > 0 && key[len-1] != '/') {
    key += "/";
    ++len;
  }
    
  while (true) {

    ArticleLinkMap::iterator it = m_article_links.find(key);

    if (it == m_article_links.end()) {
      scx::ScriptList::Ref args(new scx::ScriptList());
      args.object()->give(scx::ScriptString::new_ref(key));

      query->exec(&args);
      if (query->next_result()) {
	scx::ScriptRef* row_ref = query->result();
	scx::ScriptMap* row = dynamic_cast<scx::ScriptMap*>(row_ref->object());
	if (row) {
	  scx::ScriptRef* a_id = row->lookup("id");
	  if (a_id) {
	    int id = a_id->object()->get_int();
	    m_article_links[key] = id;
	    Article* art = new Article(*this,id);
	    m_articles[id] = new Article::Ref(art);
	    it = m_article_links.find(key);
	    DEBUG_LOG("lookup(link) Cache add for " << id 
		      << " -> '" << key << "'");
	  }
	}
	delete row_ref;
      }
    }

    if (it != m_article_links.end()) {

      if (key.length() < name.length()) {
	extra = name.substr(key.length());
	std::string::size_type i = extra.find_first_not_of("/");
	if (i == std::string::npos) {
	  extra = "";
	} else {
	  extra = extra.substr(i);
	}
      }
      // Found!
      DEBUG_LOG("lookup(link) Cache hit for '" << key << "'");
      ArticleMap::iterator ita = m_articles.find(it->second);
      return ita->second->object();
    }

    len = key.length();
    if (len == 1 && key == "/") { 
      key = "";
    } else {
      std::string::size_type start = key.find_last_of("/",len-2);
      if (start == std::string::npos) {
	key = "";
      } else {
	key = key.substr(0,start+1);
      }
    }
  }
  */
  return 0;
}

//=========================================================================
Article* Profile::create_article(int pid, const std::string name)
{
  if (name.empty()) { //TODO: check name validity
    DEBUG_LOG("Invalid article name: " << name);
    return 0;
  }
  
  Article* parent = lookup_article(pid);
  if (!parent) {
    DEBUG_LOG("Invalid parent article: " << pid);
    return 0;
  }

  std::string link = parent->get_href_path() + name;
  std::string extra;
  if (lookup_article(link,extra) && extra.empty()) {
    // Aricle already exists
    DEBUG_LOG("Article '" << link << "' already exists");
    return 0;
  }

  scx::FilePath path = parent->get_root() + name;
  scx::FilePath::mkdir(path,false,00770);

  std::auto_ptr<scx::DbQuery> query(m_db->object()->new_query(
    "INSERT INTO article (parent,path) VALUES (?,?)"));

  scx::ScriptList::Ref args(new scx::ScriptList());
  args.object()->give(scx::ScriptInt::new_ref(pid));
  args.object()->give(scx::ScriptString::new_ref(name));
  bool result = query->exec(&args);

  int id = -1;
  std::auto_ptr<scx::DbQuery> query2(m_db->object()->new_query(
    "SELECT id FROM article where parent = ? AND path = ?"));

  result = query2->exec(&args);
  if (query2->next_result()) {
    scx::ScriptRef* row_ref = query2->result_list();
    std::cerr << "LII: " << row_ref->object()->get_string() << "\n";
    scx::ScriptList* row = dynamic_cast<scx::ScriptList*>(row_ref->object());
    scx::ScriptRef* a_id = row->get(0);
    if (a_id) id = a_id->object()->get_int();
    delete row_ref;
  }

  if (id <= 0) {
    DEBUG_LOG("Could not determine id for new article");
    return 0;
  }

  Article* article = load_article(id,pid,link);
  scx::FilePath apath = article->get_filepath();

  DEBUG_LOG("Article link cache add: " << id << " -> '" << link << "'");
  m_article_links[link] = id;

  scx::File file;
  if (scx::Ok != file.open(apath,scx::File::Write|scx::File::Create,00660)) {
    DEBUG_LOG_ERRNO("Unable to create new article xml file '" 
		    << apath.path() << "'");
    return 0;
  }

  file.write("<article>\n");
  file.write("\n<p>\nwrite something here...\n</p>\n\n");
  file.write("</article>\n");
  file.close();

  return article;
}

//=========================================================================
bool Profile::remove_article(int id)
{
  Article* article = lookup_article(id);
  if (!article) {
    DEBUG_LOG("Cannot find article");
    return false;
  }
  std::string link = article->get_href_path();
  scx::FilePath path = article->get_root();
  scx::FilePath::rmdir(path,true);
  
  std::auto_ptr<scx::DbQuery> query(m_db->object()->new_query(
    "DELETE FROM article WHERE id = ?"));

  scx::ScriptList::Ref args(new scx::ScriptList());
  args.object()->give(scx::ScriptInt::new_ref(id));
  bool result = query->exec(&args);

  // Remove from caches
  m_articles.erase(id);
  m_article_links.erase(link);
  delete article;

  return result;
}

//=========================================================================
Template* Profile::lookup_template(const std::string& name)
{
  TemplateMap::iterator it = m_templates.find(name);
  if (it != m_templates.end()) {
    return it->second->object();
  }
  return 0;
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
	"remove_article" == name) {
      return new scx::ScriptMethodRef(ref,name);
    }

    // Properties
    if ("purge_threshold" == name) 
      return new scx::ScriptRef(m_purge_threshold.new_copy());

    if ("path" == name) 
      return scx::ScriptString::new_ref(m_path.path());
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
      Article* art = lookup_article(a_name->get_string(),extra);
      if (!art) return 0;
      return new Article::Ref(art);
    }

    const scx::ScriptInt* a_id = 
      scx::get_method_arg<scx::ScriptInt>(args,0,"id");
    if (a_id) {
      std::string extra;
      Article* art = lookup_article(a_id->get_int());
      if (!art) return 0;
      return new Article::Ref(art);
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

    Article* article = create_article(a_pid->get_int(),
				      a_name->get_string());
    if (!article) return scx::ScriptError::new_ref("Create article failed");

    return new scx::ScriptRef(article);
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

  return scx::ScriptObject::script_method(auth,ref,name,args);
}

//=========================================================================
bool Profile::set_meta(int id,
		       const std::string& property,
		       scx::ScriptRef* value)
{
  std::auto_ptr<scx::DbQuery> query(
    m_db->object()->new_query("UPDATE article set "+property+" = ? WHERE id = ?"));

  scx::ScriptList::Ref args(new scx::ScriptList());
  args.object()->give(value);
  args.object()->give(scx::ScriptInt::new_ref(id));
  bool result = query->exec(&args);

  DEBUG_LOG("PROFILE set_meta "<<id<<":"<<property<<"="
	    <<value->object()->get_string()
	    <<(result?" OK":" FAILED"));
  return result;
}

//=========================================================================
scx::ScriptRef* Profile::get_meta(int id,
				  const std::string& property) const
{
  std::auto_ptr<scx::DbQuery> query(
    m_db->object()->new_query("SELECT "+property+" FROM article WHERE id = ?"));

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
  
  DEBUG_LOG("PROFILE get_meta "<<id<<":"<<property<<"="
	    <<(result?result->object()->get_string():"NULL"));
  return result;
}

//=========================================================================
Article* Profile::load_article(int id, int pid, const std::string& link)
{
  DEBUG_LOG("Loading article [" << id << "] p=" << pid << " " << link);
  Article* art = new Article(*this,id,pid,link);
  m_articles[id] = new Article::Ref(art);
  return art;
}
