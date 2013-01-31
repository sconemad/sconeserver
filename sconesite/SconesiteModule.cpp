/* SconeServer (http://www.sconemad.com)

Sconesite Module

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


#include "SconesiteModule.h"
#include "SconesiteStream.h"
#include "Profile.h"
#include "XMLArticleBody.h"
#include "WikiTextArticleBody.h"

#include <sconex/ModuleInterface.h>
#include <sconex/Module.h>
#include <sconex/ScriptTypes.h>
#include <sconex/Stream.h>
#include <sconex/Kernel.h>

SCONEX_MODULE(SconesiteModule);

#define SCONESITE_JOB_PERIOD 7

//=========================================================================
class SconesiteJob : public scx::PeriodicJob {
public:

  SconesiteJob(SconesiteModule* module, 
	       const scx::Time& period)
    : scx::PeriodicJob("sconesite Refresh",period),
      m_module(module) {};

  virtual bool run()
  {
    m_module.object()->refresh();
    reset_timeout();
    return false;
  };

protected:
  SconesiteModule::Ref m_module;
};

//=========================================================================
SconesiteModule::SconesiteModule()
  : scx::Module("sconesite",scx::version())
{
  scx::Stream::register_stream("sconesite",this);
  Article::register_article_type("xml",this);
  Article::register_article_type("wtx",this);

  m_job = scx::Kernel::get()->add_job(
    new SconesiteJob(this,scx::Time(SCONESITE_JOB_PERIOD)));
}

//=========================================================================
SconesiteModule::~SconesiteModule()
{
  scx::Stream::unregister_stream("sconesite",this);
  Article::unregister_article_type("xml",this);
  Article::unregister_article_type("wtx",this);
}

//=========================================================================
std::string SconesiteModule::info() const
{
  return "Sconesite content management system";
}

//=========================================================================
bool SconesiteModule::close()
{
  if (!scx::Module::close()) return false;

  scx::Kernel::get()->end_job(m_job);

  for (ProfileMap::const_iterator it = m_profiles.begin();
       it != m_profiles.end();
       ++it) {
    delete it->second;
  }
  m_profiles.clear();
  return true;
}

//=========================================================================
void SconesiteModule::provide(const std::string& type,
			      const scx::ScriptRef* args,
			      scx::Stream*& object)
{
  const scx::ScriptString* profile_name =
    scx::get_method_arg<scx::ScriptString>(args,0,"profile");
  if (!profile_name) {
    log("No profile specified, aborting connection");
  }

  Profile* profile = lookup_profile(profile_name->get_string());
  if (!profile) {
    log("Unknown profile '"+profile_name->get_string()+
	"' specified, aborting connection");
  }
  
  object = new SconesiteStream(this,*profile);
}

//=========================================================================
void SconesiteModule::provide(const std::string& type,
			      const scx::ScriptRef* args,
			      ArticleBody*& object)
{
  const scx::ScriptString* name =
    scx::get_method_arg<scx::ScriptString>(args,0,"name");
  if (!name) {
    log("No article name specified");
    return;
  }  

  const scx::ScriptString* root =
    scx::get_method_arg<scx::ScriptString>(args,1,"root");
  if (!root) {
    log("No article root specified");
    return;
  }  

  if ("xml" == type) {
    object = new XMLArticleBody(name->get_string(), root->get_string());

  } else if ("wtx" == type) {
    object = new WikiTextArticleBody(name->get_string(), root->get_string());
  }
}

//=========================================================================
void SconesiteModule::refresh()
{
  for (ProfileMap::iterator it = m_profiles.begin();
       it != m_profiles.end();
       ++it) {
    Profile* profile = it->second->object();
    profile->refresh();
  }
}

//=========================================================================
Profile* SconesiteModule::lookup_profile(const std::string& profile)
{
  ProfileMap::const_iterator it = m_profiles.find(profile);
  if (it != m_profiles.end()) {
    return it->second->object();
  }

  return 0;
}

//=============================================================================
scx::ScriptRef* SconesiteModule::script_op(const scx::ScriptAuth& auth,
					   const scx::ScriptRef& ref,
					   const scx::ScriptOp& op,
					   const scx::ScriptRef* right)
{
  if (op.type() == scx::ScriptOp::Lookup) {
    const std::string name = right->object()->get_string();

    // Methods
    if ("add" == name) {
      return new scx::ScriptMethodRef(ref,name);
    }      

    // Sub-objects
    
    ProfileMap::const_iterator it = m_profiles.find(name);
    if (it != m_profiles.end()) {
      return it->second->ref_copy(ref.reftype());
    }
  }

  return scx::Module::script_op(auth,ref,op,right);
}

//=============================================================================
scx::ScriptRef* SconesiteModule::script_method(const scx::ScriptAuth& auth,
					       const scx::ScriptRef& ref,
					       const std::string& name,
					       const scx::ScriptRef* args)
{
  if ("add" == name) {
    if (!auth.admin()) return scx::ScriptError::new_ref("Not permitted");

    const scx::ScriptString* a_profile =
      scx::get_method_arg<scx::ScriptString>(args,0,"name");
    if (!a_profile) {
      return scx::ScriptError::new_ref("No profile name specified");
    }
    std::string s_profile = a_profile->get_string();

    scx::Module::Ref httpmod = scx::Kernel::get()->get_module("http");
    http::HTTPModule* http = dynamic_cast<http::HTTPModule*>(httpmod.object());
    http::Host* host = http ? http->get_hosts().lookup_host(s_profile) : 0;
    if (!host)
      return scx::ScriptError::new_ref("No http host for this profile");

    ProfileMap::const_iterator it = m_profiles.find(s_profile);
    if (it != m_profiles.end())
      return scx::ScriptError::new_ref("Profile already exists");

    std::string s_dbtype = "MySQL";
    const scx::ScriptString* a_dbtype =
      scx::get_method_arg<scx::ScriptString>(args,1,"dbtype");
    if (a_dbtype) s_dbtype = a_dbtype->get_string();
    
    scx::FilePath path = host->get_path();
    log("Adding profile '" + s_profile + "' dir '" + path.path() +
        "' dbtype '" + s_dbtype + "'");
    Profile* profile = new Profile(*this,s_profile,path,s_dbtype);
    m_profiles[s_profile] = new Profile::Ref(profile);

    return new Profile::Ref(profile);
  }
  
  return scx::Module::script_method(auth,ref,name,args);
}
