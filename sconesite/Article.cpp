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

#include "Article.h"
#include "ArticleBody.h"
#include "Profile.h"
#include "Context.h"

#include <sconex/Stream.h>
#include <sconex/StreamTransfer.h>
#include <sconex/Date.h>
#include <sconex/Kernel.h>
#include <sconex/FileDir.h>
#include <sconex/FilePath.h>
#include <sconex/ScriptExpr.h>
#include <sconex/ScriptStatement.h>
#include <sconex/Log.h>

#define LOG(msg) scx::Log("sconesite").submit(msg);

//=========================================================================
bool ArticleSortDate(const Article* a, const Article* b)
{
  const scx::ScriptRef* a_time = a->get_meta("time");
  const scx::ScriptRef* b_time = b->get_meta("time");

  int a_int = a_time ? a_time->object()->get_int() : 0;
  int b_int = b_time ? b_time->object()->get_int() : 0;

  delete a_time;
  delete b_time;

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
  
  scx::ScriptRef* a_arg = a->get_meta(m_meta,true);
  scx::ScriptRef* b_arg = b->get_meta(m_meta,true);

  if (a_arg) {
    ret = m_reverse;
    if (b_arg) {
      scx::ScriptRef* result = a_arg->script_op(scx::ScriptAuth::Untrusted,
						scx::ScriptOp::LessThan,
						b_arg);
      if (!BAD_SCRIPTREF(result)) {
	ret ^= result->object()->get_int();
      }
    }
  }

  delete a_arg;
  delete b_arg;

  return ret;
}


scx::ProviderScheme<ArticleBody>* Article::s_article_providers = 0;

//=========================================================================
Article::Article(Profile& profile,
		 int id,
		 int parent_id,
		 const std::string& link)
  : m_profile(profile),
    m_id(id),
    m_parent_id(parent_id),
    m_link(link),
    m_access_time(scx::Date::now()),
    m_body(0)
{
  m_parent = &m_profile;
  DEBUG_ASSERT(id > 0,"Invalid article ID");
  DEBUG_ASSERT(id != parent_id,"Invalid article ID (same as parent ID)");

  scx::ScriptRef* a_name = m_profile.get_meta(m_id,"path");

  if (a_name) {
    m_name = a_name->object()->get_string();
    m_root = m_profile.get_path() + "art" + m_link;

    scx::ScriptMap::Ref args(new scx::ScriptMap());
    args.object()->give("name",a_name->ref_copy(scx::ScriptRef::ConstRef));
    args.object()->give("root",scx::ScriptString::new_ref(m_root.path()));

    for (scx::ProviderScheme<ArticleBody>::ProviderMap::const_iterator it = 
	   s_article_providers->providers().begin(); 
	 it != s_article_providers->providers().end(); ++it) {
      std::string file = "article." + it->first;
      if (scx::FileStat(m_root + file).is_file()) {
	ArticleBody* body = s_article_providers->provide(it->first, &args);
	m_body = new ArticleBody::Ref(body);
      }
    }
    if (!m_body) {
      DEBUG_LOG("No article body for '" << m_name << "' in '" << m_root.path() << "'");
    }
  } else {
    DEBUG_LOG("No article for id '" << m_id << "'");
  }

  delete a_name;
}

//=========================================================================
Article::~Article()
{
  delete m_body;
}

//=========================================================================
const std::string& Article::get_name() const
{
  return m_name;
}

//=========================================================================
const scx::FilePath& Article::get_root() const
{
  return m_root;
}

//=========================================================================
scx::FilePath Article::get_filepath() const
{
  if (m_body) return m_body->object()->get_filepath();
  return scx::FilePath();
}

//=========================================================================
bool Article::process(Context& context)
{
  if (m_body) return m_body->object()->process(context);
  return false;
}

//=========================================================================
bool Article::set_meta(const std::string& name,
		       scx::ScriptRef* value)
{
  return m_profile.set_meta(m_id,name,value);
}

//=========================================================================
scx::ScriptRef* Article::get_meta(const std::string& name,
				  bool recurse) const
{
  scx::ScriptRef* a = m_profile.get_meta(m_id,name);
  if (recurse && BAD_SCRIPTREF(a)) {
    delete a; 
    a = 0;
    Article::Ref* parent = m_profile.lookup_article(m_parent_id);
    if (parent) {
      a = parent->object()->get_meta(name,recurse);
      delete parent;
    }
  }
  return a;
}

//=========================================================================
const ArticleHeading& Article::get_headings() const
{
  return m_body->object()->get_headings();
}

//=========================================================================
std::string Article::get_href_path() const
{
  return m_link;
}

//=========================================================================
void Article::refresh(const scx::Date& purge_time)
{
  // Purge any loaded article data if it hasn't been accessed recently
  if (m_body) m_body->object()->purge(purge_time);
}

//=========================================================================
const scx::Date& Article::get_access_time() const
{
  return m_access_time;
}

//=========================================================================
void Article::reset_access_time()
{
  m_access_time = scx::Date::now();
}

//=========================================================================
void Article::register_article_type(const std::string& type,
				    scx::Provider<ArticleBody>* factory)
{
  init();
  s_article_providers->register_provider(type,factory);
}

//=========================================================================
void Article::unregister_article_type(const std::string& type,
				      scx::Provider<ArticleBody>* factory)
{
  init();
  s_article_providers->unregister_provider(type,factory);
}

//=========================================================================
std::string Article::get_string() const
{
  return m_name;
}

//=========================================================================
scx::ScriptRef* Article::script_op(const scx::ScriptAuth& auth,
				   const scx::ScriptRef& ref,
				   const scx::ScriptOp& op,
				   const scx::ScriptRef* right)
{
  if (op.type() == scx::ScriptOp::Lookup) {
    const std::string name = right->object()->get_string();

    // Methods
    if ("update" == name ||
	"add_file" == name ||
	"remove_file" == name ||
	"get_files" == name ||
	"set_meta" == name ||
	"get_meta" == name ||
	"lookup_meta" == name) {
      return new scx::ScriptMethodRef(ref,name);
    }

    // Properties
    if ("id" == name) 
      return scx::ScriptInt::new_ref(m_id);

    if ("link" == name) 
      return scx::ScriptString::new_ref(get_href_path());

    if ("title" == name) {
      scx::ScriptRef* a_title = get_meta("title",false);
      if (!BAD_SCRIPTREF(a_title)) {
	std::string title = a_title->object()->get_string();
	if (!title.empty()) {
	  return a_title;
	}
      }
      delete a_title;
      if (!m_name.empty()) {
	return scx::ScriptString::new_ref(m_name);
      }
      return scx::ScriptString::new_ref("");
    }

    if ("parent_id" == name) 
      return scx::ScriptInt::new_ref(m_parent_id);

    if ("parent" == name) {
      return m_profile.lookup_article(m_parent_id);
    }

    if ("headings" == name) {
      return get_headings().get_tree();
    }

    if ("access_time" == name) {
      return new scx::ScriptRef(m_access_time.new_copy());
    }

    if ("body" == name) {
      if (m_body) return m_body->ref_copy(ref.reftype());
      return scx::ScriptError::new_ref("No article body");
    }
  }

  return scx::ScriptObject::script_op(auth,ref,op,right);
}

//=========================================================================
scx::ScriptRef* Article::script_method(const scx::ScriptAuth& auth,
				       const scx::ScriptRef& ref,
				       const std::string& name,
				       const scx::ScriptRef* args)

{
  if (name == "update") {
    if (!auth.trusted()) return scx::ScriptError::new_ref("Not permitted");

    const scx::ScriptFile* a_file = 
      scx::get_method_arg<scx::ScriptFile>(args,0,"file");
    if (!a_file) 
      return scx::ScriptError::new_ref("No file specified");
    const scx::FilePath& srcpath = a_file->get_path();

    scx::FilePath dstpath = get_filepath();
    LOG("Update article moving '" + srcpath.path() + 
	"' to '" + dstpath.path() + "'");
    if (!scx::FilePath::move(srcpath,dstpath))
      return scx::ScriptError::new_ref("Could not replace article");

    return 0;
  }
  
  if (name == "add_file") {
    if (!auth.trusted()) return scx::ScriptError::new_ref("Not permitted");

    const scx::ScriptFile* a_file = 
      scx::get_method_arg<scx::ScriptFile>(args,0,"file");
    if (!a_file)
      return scx::ScriptError::new_ref("No file specified");
    const scx::FilePath& srcpath = a_file->get_path();

    if (!scx::FilePath::valid_filename(a_file->get_filename()))
      return scx::ScriptError::new_ref("Invalid filename");

    scx::FilePath dstpath = get_root() + a_file->get_filename();
    LOG("Add file moving '" + srcpath.path() + 
	"' to '" + dstpath.path() + "'");
    if (!scx::FilePath::move(srcpath,dstpath)) 
      return scx::ScriptError::new_ref("Could not move file");

    return 0;
  }

  if (name == "remove_file") {
    if (!auth.trusted()) return scx::ScriptError::new_ref("Not permitted");

    const scx::ScriptString* a_file = 
      scx::get_method_arg<scx::ScriptString>(args,0,"name");
    if (!a_file) 
      return scx::ScriptError::new_ref("No file specified");
    std::string file = a_file->get_string();

    if (!scx::FilePath::valid_filename(file))
      return scx::ScriptError::new_ref("Invalid filename");

    scx::FilePath path = get_root() + file;
    if (!scx::FilePath::rmfile(path)) {
      DEBUG_LOG_ERRNO("Cannot remove file '"+path.path()+"'");
      return scx::ScriptError::new_ref("Could not remove file");
    }
    return 0;
  }

  if (name == "get_files") {
    if (!auth.trusted()) return scx::ScriptError::new_ref("Not permitted");

    scx::ScriptList* filelist = new scx::ScriptList();
    scx::FileDir files(m_root);
    while (files.next()) {
      std::string file = files.name();
      if (file != "." &&
          file != ".." &&
          file != "article.xml" && file != "article.xml~" &&
          file != "meta.txt" && file != "meta.txt~") {
	if (files.stat().is_dir()) {
	  // Ignore directories for now?
	  // filelist->give(scx::ScriptString::new_ref(file+"/"));
	} else {
	  //            filelist->give(scx::ScriptString::new_ref(file));
	  scx::ScriptFile* sfile = new scx::ScriptFile(files.path(),file);
	  filelist->give(new scx::ScriptRef(sfile));
	}
      }
    }
    return new scx::ScriptRef(filelist);
  }

  if (name == "set_meta") {
    const scx::ScriptString* a_name = 
      scx::get_method_arg<scx::ScriptString>(args,0,"name");
    if (!a_name) 
      return scx::ScriptError::new_ref("No name specified");

    const scx::ScriptObject* a_value = 
      scx::get_method_arg<scx::ScriptObject>(args,1,"value");

    if (!set_meta(a_name->get_string(),
		  a_value ? new scx::ScriptRef(a_value->new_copy()) : 0)) {
      return scx::ScriptError::new_ref("Cannot set meta value");
    }
    
    return 0;
  }

  if (name == "get_meta" ||
      name == "lookup_meta") {
    const scx::ScriptString* a_name = 
      scx::get_method_arg<scx::ScriptString>(args,0,"name");
    if (!a_name) 
      return scx::ScriptError::new_ref("No name specified");

    return get_meta(a_name->get_string(),(name == "lookup_meta"));
  }

  return scx::ScriptObject::script_method(auth,ref,name,args);
}

void Article::init()
{
  if (!s_article_providers) {
    s_article_providers = new scx::ProviderScheme<ArticleBody>();
  }
}
