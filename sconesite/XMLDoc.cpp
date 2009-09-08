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
XMLDoc::XMLDoc(
  const std::string& name,
  const scx::FilePath& path
) : m_name(name),
    m_path(path),
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
const scx::FilePath& XMLDoc::get_path() const
{
  return m_path;
}

//=========================================================================
bool XMLDoc::process(Context& context)
{
  open();

  if (!m_xmldoc) {
    context.handle_error();
    return false;

  }
  
  process_node(context,xmlDocGetRootElement(m_xmldoc));
  return true;
}

//=========================================================================
const scx::Date& XMLDoc::get_modtime() const
{
  return m_modtime;
}

//=========================================================================
std::string XMLDoc::name() const
{
  std::ostringstream oss;
  oss << "XMLDoc:" << m_name;
  return oss.str();
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
    return new scx::ArgObjectFunction(new scx::ArgObject(this),name);
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
scx::Arg* XMLDoc::arg_function(const std::string& name,scx::Arg* args)
{
  return 0;
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

      default: {
        std::cerr << "XMLDoc: Unknown node type " << node->type << "\n";
      } break;
    }
  }
}

//=========================================================================
bool XMLDoc::open()
{
  scx::FileStat stat(m_path);
  if (stat.is_file()) {
    if (m_xmldoc == 0 || m_modtime != stat.time()) {
      close();
      m_xmldoc = xmlReadFile(m_path.path().c_str(),NULL,0);
      m_modtime = stat.time();
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
