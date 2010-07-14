/* SconeServer (http://www.sconemad.com)

Sconesite RSS module

Copyright (c) 2000-2010 Andrew Wedgbury <wedge@sconemad.com>

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

#include "sconex/ModuleInterface.h"
#include "sconex/Module.h"
#include "sconex/Kernel.h"
#include "sconex/Job.h"

SCONESERVER_MODULE(RSSModule);

// This is the period for running the RSS feed refresh job, and sets the
// minumum refresh time for feeds.
#define RSS_JOB_PERIOD 10

//=========================================================================
class RSSJob : public scx::PeriodicJob {

public:

  RSSJob(RSSModule& module, const scx::Time& period)
    : scx::PeriodicJob("sconesite::rss Feed updates",period),
      m_module(module) {};

  virtual bool run()
  {
    m_module.refresh(false);
    reset_timeout();
    return false;
  };

protected:
  RSSModule& m_module;
};

//=========================================================================
RSSModule::RSSModule(
) : scx::Module("rss",scx::version())
{
#ifndef DISABLE_JOBS
  m_job = scx::Kernel::get()->add_job(new RSSJob(*this,scx::Time(RSS_JOB_PERIOD)));
#endif
}

//=========================================================================
RSSModule::~RSSModule()
{
#ifndef DISABLE_JOBS  
  scx::Kernel::get()->end_job(m_job);
#endif

  for (FeedMap::const_iterator it = m_feeds.begin();
       it != m_feeds.end();
       ++it) {
    delete it->second;
  }
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

//=============================================================================
scx::Arg* RSSModule::arg_lookup(const std::string& name)
{  
  // Methods

  if ("add" == name ||
      "remove" == name ||
      "refresh" == name) {
    return new_method(name);
  }

  // Properties

  if ("list" == name) {
    scx::ArgList* list = new scx::ArgList();
    for (FeedMap::const_iterator it = m_feeds.begin();
         it != m_feeds.end();
         ++it) {
      list->give(new scx::ArgObject(it->second));
    }
    return list;
  }

  // Sub-objects

  FeedMap::const_iterator it = m_feeds.find(name);
  if (it != m_feeds.end()) {
    Feed* f = it->second;
    return new scx::ArgObject(f);
  }

  return SCXBASE Module::arg_lookup(name);
}

//=============================================================================
scx::Arg* RSSModule::arg_method(
  const scx::Auth& auth,
  const std::string& name,
  scx::Arg* args
)
{
  scx::ArgList* l = dynamic_cast<scx::ArgList*>(args);

  if (name == "add") {
    if (!auth.admin()) return new scx::ArgError("Not permitted");

    const scx::ArgString* a_id = dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_id) return new scx::ArgError("add() No feed id specified");
    std::string s_id = a_id->get_string();
    
    const scx::ArgString* a_url = dynamic_cast<const scx::ArgString*>(l->get(1));
    if (!a_url) return new scx::ArgError("add() No feed URL specified");
    std::string s_url = a_url->get_string();

    FeedMap::const_iterator it = m_feeds.find(s_id);
    if (it != m_feeds.end()) return new scx::ArgError("add() Feed already exists");
        
    log("Adding feed '" + s_id + "' URL '" + s_url + "'");
    m_feeds[s_id] = new Feed(*this,s_id,s_url);

    return 0;
  }

  if (name == "remove") {
    if (!auth.admin()) return new scx::ArgError("Not permitted");

    const scx::ArgString* a_id = dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_id) return new scx::ArgError("remove() No feed id specified");
    std::string s_id = a_id->get_string();

    FeedMap::const_iterator it = m_feeds.find(s_id);
    if (it == m_feeds.end()) return new scx::ArgError("remove() Feed not found");
        
    log("Removing feed '" + s_id + "'");
    Feed* feed = it->second;
    m_feeds.erase(it);
    delete feed;

    return 0;
  }

  if (name == "refresh") {
    if (!auth.trusted()) return new scx::ArgError("Not permitted");

    refresh(true);
    return 0;
  }

  return SCXBASE Module::arg_method(auth,name,args);
}

//=========================================================================
bool RSSModule::connect(
  scx::Descriptor* endpoint,
  scx::ArgList* args
)
{
  return true;
}

//=========================================================================
void RSSModule::refresh(bool force)
{
  for (FeedMap::const_iterator it = m_feeds.begin();
       it != m_feeds.end();
       ++it) {
    Feed* f = it->second;
    f->refresh(force);
  }
}
