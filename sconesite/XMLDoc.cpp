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

#include "sconex/Stream.h"
#include "sconex/Date.h"
#include "sconex/FileStat.h"

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
    m_xmldoc(0)
{

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
std::string XMLDoc::name() const
{
  return m_name;
}

//=========================================================================
scx::Arg* XMLDoc::arg_resolve(const std::string& name)
{
  return SCXBASE ArgObjectInterface::arg_resolve(name);
}

//=========================================================================
scx::Arg* XMLDoc::arg_lookup(const std::string& name)
{
  // Methods
  if ("test" == name) {
    return new_method(name);
  }

  // Sub-objects
  if ("name" == name) {
    return new scx::ArgString(m_name);
  }
  if ("modtime" == name) {
    return m_modtime.new_copy();
  }

  return SCXBASE ArgObjectInterface::arg_lookup(name);
}

//=========================================================================
scx::Arg* XMLDoc::arg_method(const scx::Auth& auth,const std::string& name,scx::Arg* args)
{
  return SCXBASE ArgObjectInterface::arg_method(auth,name,args);
}

//=========================================================================
void XMLDoc::process_node(Context& context, xmlNode* start)
{
  for (xmlNode* node = start;
       node != 0;
       node = node->next) {
    
    switch (node->type) {
      
      case XML_TEXT_NODE: {
	context.handle_text((char*)node->content);
      } break;
	
      case XML_PI_NODE: {
	std::string name((char*)node->name);
	context.handle_process(name,(char*)node->content);
      } break;
	
      case XML_ELEMENT_NODE: {
	std::string name((char*)node->name);
        XMLAttrs attrs;
        for (xmlAttr* attr = node->properties;
             attr != 0; 
             attr = attr->next) {
          attrs[(char*)attr->name] = (char*)attr->children->content;
        }
        bool empty = (node->content == 0 && node->children == 0);
        if (context.handle_start(name,attrs,empty)) {
          do {
            process_node(context,node->children);
          } while (context.handle_end(name,attrs));
        }
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
  }
}

//=========================================================================
bool XMLDoc::open()
{
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

      m_xmldoc = xmlCtxtReadFile(cx,path.path().c_str(),NULL,0);
      m_modtime = stat.time();

      xmlFreeParserCtxt(cx);
    }
  } else {
    close();
  }
  return (m_xmldoc != 0);
}

//=========================================================================
void XMLDoc::close()
{
  if (m_xmldoc) {
    xmlFreeDoc(m_xmldoc);
  }
  m_xmldoc = 0;
}
