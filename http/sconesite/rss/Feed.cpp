/* SconeServer (http://www.sconemad.com)

RSS Feed

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


#include "Feed.h"

#include "libxml/nanohttp.h"

//=========================================================================
void ErrorHandler(void* vcx,const char* str,...)
{
  va_list vl;
  va_start(vl,1);
  char* msg = va_arg(vl,char*);
  va_end(vl);

  //  xmlParserCtxt* cx = (xmlParserCtxt*)vcx;
  //  Feed* feed = (Feed*)cx->_private;
  DEBUG_LOG("Feed error: " << msg);
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
    m_period(period)
{

}

//=========================================================================
Feed::~Feed()
{

}

//=========================================================================
void Feed::refresh(bool force)
{
  if (force || scx::Date::now() >= (m_refresh_time + m_period)) {

    // Can set a HTTP proxy here:
    //xmlNanoHTTPScanProxy("http://proxy_host:8080");

    xmlParserCtxt* cx;
    cx = xmlNewParserCtxt();
    cx->_private = this;
    cx->sax->error = ErrorHandler;
    cx->vctxt.error = ErrorHandler;
    
    int opts = 0;
    opts |= XML_PARSE_RECOVER;
    
    xmlDoc* xmldoc = xmlCtxtReadFile(cx,m_url.get_string().c_str(),NULL,opts);
    
    if (xmldoc) {
      clear_items();
      m_mutex.lock();
      m_refresh_time = scx::Date::now();
      process(xmlDocGetRootElement(xmldoc));
      m_mutex.unlock();
      DEBUG_LOG("Refreshed feed: " << m_id);
    } else {
      DEBUG_LOG("Error refreshing feed: " << m_id);
    }
    
    xmlFreeDoc(xmldoc);
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
std::string Feed::name() const
{
  return m_id;
}

//=========================================================================
scx::Arg* Feed::arg_resolve(const std::string& name)
{
  return SCXBASE ArgObjectInterface::arg_resolve(name);
}

//=========================================================================
scx::Arg* Feed::arg_lookup(const std::string& name)
{
  // Methods
  if ("set_refresh_period" == name ||
      "refresh" == name) {
    return new_method(name);
  }

  // Properties
  if ("refresh_time" == name) return m_refresh_time.ref_copy(scx::Arg::ConstRef);
  if ("refresh_period" == name) return m_period.ref_copy(scx::Arg::ConstRef);
  if ("url" == name) return m_url.ref_copy(scx::Arg::ConstRef);
  if ("image_url" == name) return m_image_url.ref_copy(scx::Arg::ConstRef);
  if ("title" == name) return new scx::ArgString(m_title);
  if ("description" == name) return new scx::ArgString(m_description);
  
  if ("items" == name) {
    scx::MutexLocker locker(m_mutex);
    return m_items.ref_copy(scx::Arg::ConstRef);
  }

  return SCXBASE ArgObjectInterface::arg_lookup(name);
}

//=========================================================================
scx::Arg* Feed::arg_method(const scx::Auth& auth,const std::string& name,scx::Arg* args)
{
  scx::ArgList* l = dynamic_cast<scx::ArgList*>(args);

  if ("set_refresh_period" == name) {
    if (!auth.admin()) return new scx::ArgError("Not permitted");

    const scx::ArgInt* a_period = dynamic_cast<const scx::ArgInt*>(l->get(0));
    if (!a_period) return new scx::ArgError("set_refresh_period() Must specify value");
    int n_period = a_period->get_int();
    if (n_period < 0) return new scx::ArgError("set_refresh_period() Value must be >= 0");
    m_period = scx::Time(n_period);
    return 0;
  }

  if ("refresh" == name) {
    if (!auth.trusted()) return new scx::ArgError("Not permitted");

    refresh(true);
    return 0;
  }

  return SCXBASE ArgObjectInterface::arg_method(auth,name,args);
}

//=========================================================================
void Feed::process(xmlNode* root)
{
  std::string root_name((char*)root->name);
  
  if (root_name == "rss") {
    process_rss(root);
  
  } else if (root_name == "feed") {
    process_feed(root);

  } else {
    DEBUG_LOG("Unknown feed type: " << root_name);
  }
}

//=========================================================================
void Feed::process_rss(xmlNode* root)
{
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
  scx::ArgMap* item = new scx::ArgMap();

  for (xmlNode* node = root->children;
       node != 0;
       node = node->next) {
    
    if (node->type == XML_ELEMENT_NODE) {
      std::string name = (char*)node->name;
      scx::Arg* value = 0;
      if (node->children && node->children->content) {
	if (name == "link") {
	  value = new scx::Uri((char*)node->children->content);
	} else if (name == "pubDate") {
	  value = new scx::Date((char*)node->children->content);
	} else {
	  value = new scx::ArgString((char*)node->children->content);
	}
      }
      if (value) item->give(name,value);
    }
  }

  m_items.give(item);
}

//=========================================================================
void Feed::process_feed(xmlNode* root)
{
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
  scx::ArgMap* item = new scx::ArgMap();

  for (xmlNode* node = root->children;
       node != 0;
       node = node->next) {
    
    if (node->type == XML_ELEMENT_NODE) {
      std::string name = (char*)node->name;
      scx::Arg* value = 0;
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
	     subnode = node->next) {
	  if (std::string((char*)subnode->name) == "name") {
	    if (subnode->children && subnode->children->content) {
	      value = new scx::ArgString((char*)subnode->children->content);
	    }
	    break;
	  }
	}
      } else {
	if (node->children && node->children->content) {
	  value = new scx::ArgString((char*)node->children->content);
	}
      }
      if (value) item->give(name,value);
    }
  }

  m_items.give(item);
}

//=========================================================================
void Feed::clear_items()
{
  m_mutex.lock();
  while (scx::Arg* item = m_items.take(0)) { delete item; }
  m_mutex.unlock();
}
