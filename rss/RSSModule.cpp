/* SconeServer (http://www.sconemad.com)

RSS client module

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

#include "RSSModule.h"

#include <sconex/ModuleInterface.h>
#include <sconex/Module.h>
#include <sconex/Kernel.h>
#include <sconex/Job.h>
#include <sconex/Log.h>

SCONEX_MODULE(RSSModule);

#define LOG(msg) scx::Log("rss").submit(msg);

// This is the period for running the RSS feed refresh job, and sets the
// minumum refresh time for feeds.
#define RSS_JOB_PERIOD 10

//=========================================================================
class RSSJob : public scx::PeriodicJob {
public:

  RSSJob(RSSModule* module,
	 const scx::Time& period)
    : scx::PeriodicJob("rss Feed updates",period),
      m_module(module) {};

  virtual bool run()
  {
    m_module.object()->refresh(false);
    reset_timeout();
    return false;
  };

protected:
  RSSModule::Ref m_module;
};

//=========================================================================
RSSModule::RSSModule(
) : scx::Module("rss",scx::version())
{
  m_job = scx::Kernel::get()->add_job(
    new RSSJob(this,scx::Time(RSS_JOB_PERIOD)));
}

//=========================================================================
RSSModule::~RSSModule()
{

}

//=========================================================================
std::string RSSModule::info() const
{
  return "RSS/Atom feed client";
}

//=========================================================================
int RSSModule::init()
{
  return Module::init();
}

//=========================================================================
bool RSSModule::close()
{
  if (!scx::Module::close()) return false;

  scx::Kernel::get()->end_job(m_job);

  for (FeedMap::const_iterator it = m_feeds.begin();
       it != m_feeds.end();
       ++it) {
    delete it->second;
  }
  m_feeds.clear();

  return true;
}

//=========================================================================
void RSSModule::refresh(bool force)
{
  if (force) {
    LOG("Forcing refresh of all feeds");
  }

  for (FeedMap::const_iterator it = m_feeds.begin();
       it != m_feeds.end();
       ++it) {
    it->second->object()->refresh(force);
  }
}

//=============================================================================
scx::ScriptRef* RSSModule::script_op(const scx::ScriptAuth& auth,
				     const scx::ScriptRef& ref,
				     const scx::ScriptOp& op,
				     const scx::ScriptRef* right)
{
  if (op.type() == scx::ScriptOp::Lookup) {
    const std::string name = right->object()->get_string();

    // Methods
    if ("add" == name ||
	"remove" == name ||
	"refresh" == name) {
      return new scx::ScriptMethodRef(ref,name);
    }

    // Properties

    if ("list" == name) {
      scx::ScriptList::Ref* list = 
	new scx::ScriptList::Ref(new scx::ScriptList());
      for (FeedMap::const_iterator it = m_feeds.begin();
	   it != m_feeds.end();
         ++it) {
	list->object()->give(it->second->ref_copy(ref.reftype()));
      }
      return list;
    }
    
    // Sub-objects
    
    FeedMap::const_iterator it = m_feeds.find(name);
    if (it != m_feeds.end()) {
      return it->second->ref_copy(ref.reftype());
    }
  }

  return scx::Module::script_op(auth,ref,op,right);
}

//=============================================================================
scx::ScriptRef* RSSModule::script_method(const scx::ScriptAuth& auth,
					 const scx::ScriptRef& ref,
					 const std::string& name,
					 const scx::ScriptRef* args)
{
  if (name == "add") {
    if (!auth.admin()) return scx::ScriptError::new_ref("Not permitted");

    const scx::ScriptString* a_id = 
      scx::get_method_arg<scx::ScriptString>(args,0,"id");
    if (!a_id) {
      return scx::ScriptError::new_ref("No feed id specified");
    }
    std::string s_id = a_id->get_string();
    
    const scx::ScriptString* a_url = 
      scx::get_method_arg<scx::ScriptString>(args,1,"url");
    if (!a_url) {
      return scx::ScriptError::new_ref("No feed URL specified");
    }
    std::string s_url = a_url->get_string();

    FeedMap::const_iterator it = m_feeds.find(s_id);
    if (it != m_feeds.end()) {
      return scx::ScriptError::new_ref("Feed already exists");
    }
        
    LOG("Adding feed '" + s_id + "' URL '" + s_url + "'");
    Feed* feed = new Feed(*this,s_id,scx::Uri(s_url));
    m_feeds[s_id] = new Feed::Ref(feed);

    return new Feed::Ref(feed);
  }

  if (name == "remove") {
    if (!auth.admin()) return scx::ScriptError::new_ref("Not permitted");

    const scx::ScriptString* a_id = 
      scx::get_method_arg<scx::ScriptString>(args,0,"id");
    if (!a_id) {
      return scx::ScriptError::new_ref("No feed id specified");
    }
    std::string s_id = a_id->get_string();

    FeedMap::const_iterator it = m_feeds.find(s_id);
    if (it == m_feeds.end()) {
      return scx::ScriptError::new_ref("Feed not found");
    }
        
    LOG("Removing feed '" + s_id + "'");
    Feed::Ref* fr = it->second;
    m_feeds.erase(it);
    delete fr;

    return 0;
  }

  if (name == "refresh") {
    if (!auth.trusted()) return scx::ScriptError::new_ref("Not permitted");

    refresh(true);
    return 0;
  }

  return scx::Module::script_method(auth,ref,name,args);
}

