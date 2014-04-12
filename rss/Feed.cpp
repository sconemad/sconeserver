/* SconeServer (http://www.sconemad.com)

RSS Feed

Copyright (c) 2000-2014 Andrew Wedgbury <wedge@sconemad.com>

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

// Uncomment to enable debug info
//#define RSSFeed_DEBUG_LOG(m) DEBUG_LOG(m)

#ifndef RSSFeed_DEBUG_LOG
#  define RSSFeed_DEBUG_LOG(m)
#endif

#include "Feed.h"
#include "RSSModule.h"
#include <sconex/Log.h>
#include <sconex/Kernel.h>
#include <http/Client.h>

#define LOG(msg) scx::Log("rss").attach("id",m_id).submit(msg);

//=========================================================================
void RSSFeed_ErrorHandler(void* vcx,const char* str,...)
{
  va_list vl;
  va_start(vl,str);
  char* msg = va_arg(vl,char*);
  va_end(vl);

  //  xmlParserCtxt* cx = (xmlParserCtxt*)vcx;
  //  Feed* feed = (Feed*)cx->_private;
  RSSFeed_DEBUG_LOG("Feed error: " << msg);
}

//=========================================================================
Feed::Feed(
  RSSModule& module,
  const std::string& id,
  const scx::Uri& url,
  const scx::Time& period
) : m_module(module),
    m_id(id),
    m_url(url),
    m_items(new scx::ScriptList()),
    m_period(period)
{
  m_parent = &m_module;
}

//=========================================================================
Feed::~Feed()
{

}

//=========================================================================
void Feed::refresh(bool force)
{
  if (force || scx::Date::now() >= (m_refresh_time + m_period)) {

    scx::Module::Ref httpmod = scx::Kernel::get()->get_module("http");
    http::HTTPModule* http = dynamic_cast<http::HTTPModule*>(httpmod.object());
    http::Client hc(http, "GET", m_url);

    // use an If-Modified-Since header to detect if the feed has changed
    hc.set_header("If-Modified-Since", m_refresh_time.string());

    if (!hc.run()) {
      LOG("Unable to refresh feed - network error");
    }
    
    if (http::Status::NotModified == hc.get_response().get_status().code()) {
      LOG("Not refreshing feed - not modified");
      return;
    }

    xmlParserCtxt* cx;
    cx = xmlNewParserCtxt();
    cx->_private = this;
    cx->sax->error = RSSFeed_ErrorHandler;
    cx->vctxt.error = RSSFeed_ErrorHandler;
    
    int opts = 0;
    opts |= XML_PARSE_RECOVER;
    
    const std::string& data = hc.get_response_data();
    xmlDoc* xmldoc = xmlCtxtReadMemory(cx,data.data(),data.length(),
				       m_url.get_string().c_str(),NULL,opts);

    bool success = false;
    if (xmldoc) {
      clear_items();
      m_mutex.lock();
      m_refresh_time = scx::Date::now();
      success = process(xmlDocGetRootElement(xmldoc));
      m_mutex.unlock();
    }

    std::ostringstream oss;
    if (success) {
      oss << "Refreshed feed, contains " << m_items.object()->size() 
	  << " items";
    } else if (xmldoc) {
      oss << "Unable to refresh feed - invalid feed data returned";
    } else {
      oss << "Unable to refresh feed - received no data";
    }
    LOG(oss.str());
    
    if (xmldoc) xmlFreeDoc(xmldoc);
    xmlFreeParserCtxt(cx);
  }
}

//=========================================================================
void Feed::set_refresh_period(const scx::Time& period)
{
  m_period = period;
}

//=========================================================================
RSSModule& Feed::get_module()
{
  return m_module;
}

//=========================================================================
const scx::Uri& Feed::get_url()
{
  return m_url;
}

//=========================================================================
std::string Feed::get_string() const
{
  return m_id;
}

//=========================================================================
scx::ScriptRef* Feed::script_op(const scx::ScriptAuth& auth,
				const scx::ScriptRef& ref,
				const scx::ScriptOp& op,
				const scx::ScriptRef* right)
{
  if (op.type() == scx::ScriptOp::Lookup) {
    const std::string name = right->object()->get_string();

    // Methods
    if ("set_refresh_period" == name ||
	"refresh" == name) {
      return new scx::ScriptMethodRef(ref,name);
    }

    // Properties
    if ("refresh_time" == name) 
      return new scx::ScriptRef(m_refresh_time.new_copy());
    if ("refresh_period" == name)
      return new scx::ScriptRef(m_period.new_copy());
    if ("url" == name) 
      return new scx::ScriptRef(m_url.new_copy());
    if ("image_url" == name) 
      return new scx::ScriptRef(m_image_url.new_copy());
    if ("title" == name) 
      return scx::ScriptString::new_ref(m_title);
    if ("description" == name) 
      return scx::ScriptString::new_ref(m_description);
    
    if ("items" == name) {
      scx::MutexLocker locker(m_mutex);
      return m_items.ref_copy(scx::ScriptRef::ConstRef);
    }
  }

  return scx::ScriptObject::script_op(auth,ref,op,right);
}

//=========================================================================
scx::ScriptRef* Feed::script_method(const scx::ScriptAuth& auth,
				    const scx::ScriptRef& ref,
				    const std::string& name,
				    const scx::ScriptRef* args)
{
  if ("set_refresh_period" == name) {
    if (!auth.admin()) return scx::ScriptError::new_ref("Not permitted");

    const scx::ScriptInt* a_period = 
      scx::get_method_arg<scx::ScriptInt>(args,0,"value");
    if (!a_period) 
      return scx::ScriptError::new_ref("Must specify value");
    int n_period = a_period->get_int();
    if (n_period < 0) 
      return scx::ScriptError::new_ref("Value must be >= 0");
    m_period = scx::Time(n_period);
    return 0;
  }

  if ("refresh" == name) {
    if (!auth.trusted()) return scx::ScriptError::new_ref("Not permitted");

    refresh(true);
    return 0;
  }

  return scx::ScriptObject::script_method(auth,ref,name,args);
}

//=========================================================================
bool Feed::process(xmlNode* root)
{
  RSSFeed_DEBUG_LOG("Processing feed");
  if (!root) return false;

  std::string root_name((char*)root->name);
  
  if (root_name == "rss") {
    process_rss(root);
    return true;
  
  } else if (root_name == "feed") {
    process_feed(root);
    return true;
  }

  RSSFeed_DEBUG_LOG("Unknown feed type, root element is '" << root_name << "'");
  return false;
}

//=========================================================================
void Feed::process_rss(xmlNode* root)
{
  RSSFeed_DEBUG_LOG("Processing rss");

  for (xmlNode* node = root->children; 
       node != 0; 
       node = node->next) {

    if (node->type == XML_ELEMENT_NODE) {
      std::string name = (char*)node->name;
      if (name == "channel") {
	process_rss_channel(node);
      }
    }

  }
}

//=========================================================================
void Feed::process_rss_channel(xmlNode* root)
{
  RSSFeed_DEBUG_LOG("Processing rss channel");

  for (xmlNode* node = root->children; 
       node != 0; 
       node = node->next) {

    if (node->type == XML_ELEMENT_NODE) {
      std::string name = (char*)node->name;
      if (name == "item") {
	process_rss_channel_item(node);

      } else if (name == "image") {
	process_rss_channel_image(node);

      } else if (name == "title") {
	m_title = (char*)node->children->content;

      } else if (name == "description") {
	m_description = (char*)node->children->content;
      }
    }
    
  }
}

//=========================================================================
void Feed::process_rss_channel_image(xmlNode* root)
{
  RSSFeed_DEBUG_LOG("Processing rss channel image");

  for (xmlNode* node = root->children; 
       node != 0; 
       node = node->next) {

    if (node->type == XML_ELEMENT_NODE) {
      std::string name = (char*)node->name;
      if (name == "url") {
	m_image_url = scx::Uri((char*)node->children->content);
      }
    }
    
  }
}

//=========================================================================
void Feed::process_rss_channel_item(xmlNode* root)
{
  RSSFeed_DEBUG_LOG("Processing rss channel item");

  scx::ScriptMap* item = new scx::ScriptMap();

  for (xmlNode* node = root->children;
       node != 0;
       node = node->next) {
    
    if (node->type == XML_ELEMENT_NODE) {
      std::string name = (char*)node->name;
      scx::ScriptObject* value = 0;
      if (node->children && node->children->content) {
	if (name == "link") {
	  value = new scx::Uri((char*)node->children->content);
	} else if (name == "pubDate") {
	  value = new scx::Date((char*)node->children->content);
	} else {
	  value = new scx::ScriptString((char*)node->children->content);
	}
      }
      if (value) item->give(name,new scx::ScriptRef(value));
    }
  }

  m_items.object()->give(new scx::ScriptRef(item));
}

//=========================================================================
void Feed::process_feed(xmlNode* root)
{
  RSSFeed_DEBUG_LOG("Processing feed");

  for (xmlNode* node = root->children; 
       node != 0; 
       node = node->next) {

    if (node->type == XML_ELEMENT_NODE) {
      std::string name = (char*)node->name;
      if (name == "entry") {
	process_feed_entry(node);

      } else if (name == "title") {
	m_title = (char*)node->children->content;

      } else if (name == "subtitle") {
	m_description = (char*)node->children->content;
      }
    }
    
  }
}

//=========================================================================
void Feed::process_feed_entry(xmlNode* root)
{
  RSSFeed_DEBUG_LOG("Processing feed entry");

  scx::ScriptMap* item = new scx::ScriptMap();

  for (xmlNode* node = root->children;
       node != 0;
       node = node->next) {
    
    if (node->type == XML_ELEMENT_NODE) {
      std::string name = (char*)node->name;
      scx::ScriptObject* value = 0;
      if (name == "link") {
	for (xmlAttr* attr = node->properties;
	     attr != 0; 
	     attr = attr->next) {
	  if (std::string((char*)attr->name) == "rel") {
	    name += std::string("_") + (char*)attr->children->content;
	  } else if (std::string((char*)attr->name) == "href") {
	    value = new scx::Uri((char*)attr->children->content);
	  }
	}
      } else if (name == "updated") {
	if (node->children && node->children->content) {
	  value = new scx::Date((char*)node->children->content);
	}
	  
      } else if (name == "author") {
	for (xmlNode* subnode = node->children;
	     subnode != 0;
	     subnode = subnode->next) {
	  if (std::string((char*)subnode->name) == "name") {
	    if (subnode->children && subnode->children->content) {
	      value = new scx::ScriptString((char*)subnode->children->content);
	    }
	    break;
	  }
	}
      } else {
	if (node->children && node->children->content) {
	  value = new scx::ScriptString((char*)node->children->content);
	}
      }
      if (value) item->give(name,new scx::ScriptRef(value));
    }
  }

  m_items.object()->give(new scx::ScriptRef(item));
}

//=========================================================================
void Feed::clear_items()
{
  m_mutex.lock();
  m_items.object()->clear();
  m_mutex.unlock();
}
