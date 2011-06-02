/* SconeServer (http://www.sconemad.com)

Sconesite XML document

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


#include "XMLDoc.h"
#include "Article.h"
#include "Context.h"

#include <sconex/Stream.h>
#include <sconex/Date.h>
#include <sconex/FileStat.h>
#include <sconex/utils.h>
#include <sconex/ScriptTypes.h>

scx::Mutex* XMLDoc::m_clients_mutex = 0;

//=========================================================================
bool XMLAttr_bool(NodeAttrs& attrs, const std::string& value, bool def)
{
  NodeAttrs::const_iterator it = attrs.find(value);
  if (it != attrs.end()) {
    if (it->second == "1") {
      return true;
    }
    if (it->second == "0") {
      return false;
    }
    std::string ls = it->second;
    scx::strlow(ls);
    if (ls == "true" || ls == "yes" || ls == "on") {
      return true;
    }
    if (ls == "false" || ls == "no" || ls == "off") {
      return false;
    }
  }
  return def;
}

//=========================================================================
void ErrorHandler(void* vcx,const char* str,...)
{
  va_list vl;
  va_start(vl,1);
  char* msg = va_arg(vl,char*);
  va_end(vl);

  xmlParserCtxt* cx = (xmlParserCtxt*)vcx;
  XMLDoc* doc = (XMLDoc*)cx->_private;
  doc->parse_error(msg);
}

//=========================================================================
XMLDoc::XMLDoc(
  const std::string& name,
  const scx::FilePath& root,
  const std::string& file
) : m_name(name),
    m_root(root),
    m_file(file),
    m_xmldoc(0),
    m_clients(0),
    m_opening(false)
{
  if (!m_clients_mutex) {
    m_clients_mutex = new scx::Mutex();
  }
}

//=========================================================================
XMLDoc::~XMLDoc()
{
  close();
}

//=========================================================================
const std::string& XMLDoc::get_name() const
{
  return m_name;
}

//=========================================================================
const scx::FilePath& XMLDoc::get_root() const
{
  return m_root;
}

//=========================================================================
const std::string& XMLDoc::get_file() const
{
  return m_file;
}

//=========================================================================
scx::FilePath XMLDoc::get_filepath() const
{
  return m_root + m_file;
}

//=========================================================================
bool XMLDoc::process(Context& context)
{
  m_clients_mutex->lock();
  ++m_clients;
  m_clients_mutex->unlock();

  open();
  
  if (!m_xmldoc) {
    context.handle_error(m_errors);
    return false;
  }

  if (context.handle_doc_start(this)) {
    do {
      process_node(context,xmlDocGetRootElement(m_xmldoc));
    } while (context.handle_doc_end(this));
  }

  m_clients_mutex->lock();
  --m_clients;
  m_clients_mutex->unlock();
  
  return true;
}

//=========================================================================
const scx::Date& XMLDoc::get_modtime() const
{
  return m_modtime;
}

//=========================================================================
void XMLDoc::parse_error(const std::string& msg)
{
  m_errors += msg;
}

//=========================================================================
bool XMLDoc::purge(const scx::Date& purge_time)
{
  if (!m_xmldoc) return false;
  if (m_last_access > purge_time) return false;

  scx::MutexLocker locker(*m_clients_mutex);
  if (m_clients > 0) return false;

  DEBUG_LOG("Purging " + get_filepath().path());
  close();
  return true;
}

//=========================================================================
std::string XMLDoc::get_string() const
{
  return m_name;
}

//=========================================================================
scx::ScriptRef* XMLDoc::script_op(const scx::ScriptAuth& auth,
				  const scx::ScriptRef& ref,
				  const scx::ScriptOp& op,
				  const scx::ScriptRef* right)
{
  if (op.type() == scx::ScriptOp::Lookup) {
    const std::string name = right->object()->get_string();

    // Methods
    if ("test" == name) {
      return new scx::ScriptMethodRef(ref,name);
    }

    // Sub-objects
    if ("name" == name) {
      return scx::ScriptString::new_ref(m_name);
    }

    if ("modtime" == name) {
      return new scx::ScriptRef(m_modtime.new_copy());
    }
  }

  return scx::ScriptObject::script_op(auth,ref,op,right);
}

//=========================================================================
void XMLDoc::get_node_text(std::string& txt, xmlNode* node)
{
  while (node) {
    if (node->content) {
      txt += (char*)node->content;
    }
    if (node->children) {
      get_node_text(txt,node->children);
    }
    node = node->next;
  }
}

//=========================================================================
void XMLDoc::process_node(Context& context, xmlNode* start)
{
  int line = start->line;
  for (xmlNode* node = start;
       node != 0;
       node = node->next) {
    
    switch (node->type) {

      case XML_TEXT_NODE: {
	context.handle_text((char*)node->content);
      } break;
	
      case XML_PI_NODE: {
	std::string name((char*)node->name);
	context.handle_process(name,(char*)node->content,line);
      } break;
	
      case XML_ELEMENT_NODE: {
	std::string name((char*)node->name);
        NodeAttrs attrs;
        for (xmlAttr* attr = node->properties;
             attr != 0; 
             attr = attr->next) {
          attrs[(char*)attr->name] = (char*)attr->children->content;
        }
        bool empty = (node->content == 0 && node->children == 0);
        if (context.handle_start(name,attrs,empty,node->_private)) {
          do {
            process_node(context,node->children);
          } while (context.handle_end(name,attrs,node->_private));
        }
      } break;
	
      case XML_ENTITY_REF_NODE: {
	std::string name((char*)node->name);
        name = "&" + name + ";";
        context.handle_text(name.c_str());
      } break;

      case XML_COMMENT_NODE: {
	context.handle_comment((char*)node->content);
      } break;
	
      case XML_CDATA_SECTION_NODE: {
	DEBUG_LOG("CDATA: \n" << (char*)node->content);
      } break;

      default: {
        DEBUG_LOG("XMLDoc: Unknown node type " << node->type);
      } break;
    }

    line = node->line;
  }
}

//=========================================================================
void XMLDoc::handle_open()
{

}

//=========================================================================
void XMLDoc::handle_close()
{

}

//=========================================================================
bool XMLDoc::open()
{
  m_last_access = scx::Date::now();

  m_clients_mutex->lock();
  bool open_wait = m_opening;
  if (!m_opening) m_opening = true;
  m_clients_mutex->unlock();
  
  if (open_wait) {
    // Doc is being opened in another thread, wait for it to complete
    DEBUG_LOG("XMLDoc::Open() Waiting for another thread to complete");
    while (m_opening) { };
    DEBUG_LOG("XMLDoc::Open() Other thread finished open, continuing");
    return (m_xmldoc != 0);
  }
  
  scx::FilePath path = get_filepath();
  scx::FileStat stat(path);
  if (stat.is_file()) {
    if (m_xmldoc == 0 || m_modtime != stat.time()) {
      close();
      m_errors = "";

      xmlParserCtxt* cx;
      cx = xmlNewParserCtxt();
      cx->_private = this;
      cx->sax->error = ErrorHandler;
      cx->vctxt.error = ErrorHandler;

      int opts = 0;
      opts |= XML_PARSE_NONET;
      //opts |= XML_PARSE_RECOVER;
      
      m_xmldoc = xmlCtxtReadFile(cx,path.path().c_str(),NULL,opts);
      m_modtime = stat.time();

      xmlFreeParserCtxt(cx);

      handle_open();
    }
  } else {
    close();
  }

  m_clients_mutex->lock();
  m_opening = false;
  m_clients_mutex->unlock();

  return (m_xmldoc != 0);
}

//=========================================================================
void XMLDoc::close()
{
  if (m_xmldoc) {
    handle_close();
    xmlFreeDoc(m_xmldoc);
  }
  m_xmldoc = 0;
}
