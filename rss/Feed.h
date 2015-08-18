/* SconeServer (http://www.sconemad.com)

RSS Feed

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

#ifndef rssFeed_h
#define rssFeed_h

#include <sconex/ScriptBase.h>
#include <sconex/ScriptTypes.h>
#include <sconex/Date.h>
#include <sconex/Uri.h>
#include <sconex/Mutex.h>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/HTMLtree.h>

class RSSModule;

#define Feed_DEFAULT_REFRESH_PERIOD 600

//=========================================================================
class Feed : public scx::ScriptObject {

public:

  Feed(RSSModule& module,
       const std::string& id,
       const scx::Uri& url,
       const scx::Time& period = scx::Time(Feed_DEFAULT_REFRESH_PERIOD));
  
  ~Feed();

  // Refresh the feed
  // force : If true, refresh the feed without checking timeout
  //         If false, only refresh if timeout period has been reached
  void refresh(bool force);

  void set_refresh_period(const scx::Time& period);

  RSSModule& get_module();
  const scx::Uri& get_url();

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

  typedef scx::ScriptRefTo<Feed> Ref;

protected:

  bool process(xmlNode* root);

  // RSS processing
  void process_rss(xmlNode* root);
  void process_rss_channel(xmlNode* root);
  void process_rss_channel_image(xmlNode* root);
  void process_rss_channel_item(xmlNode* root);

  // Atom processing
  void process_feed(xmlNode* root);
  void process_feed_entry(xmlNode* root);

  void clear_items();

private:

  RSSModule& m_module;

  std::string m_id;
  scx::Uri m_url;
  scx::Uri m_image_url;

  std::string m_title;
  std::string m_description;

  scx::ScriptList::Ref m_items;

  scx::Date m_refresh_time;
  scx::Date m_modified_time;
  scx::Time m_period;

  scx::Mutex m_mutex;
};

#endif
