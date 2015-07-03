/* SconeServer (http://www.sconemad.com)

Sconesite XML document

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


#include "XMLDoc.h"
#include "Context.h"

#include <sconex/MemFile.h>
#include <sconex/ScriptEngine.h>
#include <sconex/ScriptTypes.h>
#include <sconex/Stream.h>
#include <sconex/utils.h>
#include <sconex/Log.h>

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
XMLDoc::XMLDoc(const std::string& name,
	       const scx::FilePath& root,
	       const std::string& file)
  : Document(name, root, file),
    m_xmldoc(0)
{
}

//=========================================================================
XMLDoc::~XMLDoc()
{
  handle_close();
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
void XMLDoc::report_xml_error(const std::string& error)
{
  report_error(error);
}

//=========================================================================
static void xml_error_handler(void* vcx,const char* str,...)
{
  va_list vl;
  va_start(vl,str);
  const int MAX_MSG = 256;
  char msg[MAX_MSG];
  vsnprintf(msg,MAX_MSG,str,vl);
  va_end(vl);

  xmlParserCtxt* cx = (xmlParserCtxt*)vcx;
  XMLDoc* doc = (XMLDoc*)cx->_private;

  // Strip trailing newline, which libxml seems to include
  int len = strlen(msg);
  if (len > 0 && msg[len-1] == '\n') msg[len-1] = '\0';
  
  doc->report_xml_error(msg);
}

//=========================================================================
void XMLDoc::process_node(Context& context, xmlNode* start)
{
  int line = start ? start->line : 0;
  for (xmlNode* node = start;
       node != 0;
       node = node->next) {
    
    switch (node->type) {

      case XML_TEXT_NODE: {
	context.handle_text((char*)node->content);
      } break;
	
      case XML_PI_NODE: {
	std::string name((char*)node->name);
	context.handle_process(name,(char*)node->content,line,
                               node->_private);
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
bool XMLDoc::is_open() const
{
  return (m_xmldoc != 0);
}

//=========================================================================
bool XMLDoc::handle_open()
{
  xmlParserCtxt* cx;
  cx = xmlNewParserCtxt();
  cx->_private = this;
  cx->sax->error = xml_error_handler;
  cx->vctxt.error = xml_error_handler;
  
  int opts = 0;
  opts |= XML_PARSE_NONET;
  //opts |= XML_PARSE_RECOVER;
  m_xmldoc = xmlCtxtReadFile(cx,get_filepath().path().c_str(),NULL,opts);
  xmlFreeParserCtxt(cx);

  if (!m_xmldoc) {
    return false;
  }
  
  scan_scripts(xmlDocGetRootElement(m_xmldoc));
  
  int index = 0;
  scan_headings(xmlDocGetRootElement(m_xmldoc),index);

  return true;
}

//=========================================================================
bool XMLDoc::handle_process(Context& context)
{
  if (context.handle_doc_start(this)) {
    do {
      process_node(context,xmlDocGetRootElement(m_xmldoc));
    } while (context.handle_doc_end(this));
  }
  return true;
}

//=========================================================================
void XMLDoc::handle_close()
{
  for (Scripts::iterator it = m_scripts.begin();
       it != m_scripts.end(); ++it) {
    delete (*it);
  }
  m_scripts.clear();

  if (m_xmldoc) {
    xmlFreeDoc(m_xmldoc);
  }
  m_xmldoc = 0;
}

//=========================================================================
void XMLDoc::scan_scripts(xmlNode* start)
{
  for (xmlNode* node = start;
       node != 0;
       node = node->next) {
    
    if (node->type == XML_ELEMENT_NODE) {
      scan_scripts(node->children);

    } else if (node->type == XML_PI_NODE) {
      std::string name((char*)node->name);
      if (name == "scx") {
        scx::ScriptStatement::Ref* script =
          parse_script((char*)node->content, node->line);
        m_scripts.push_back(script);
        node->_private = (void*)script;
      }
    }
  }
}

//=========================================================================
scx::ScriptStatement::Ref* XMLDoc::parse_script(char* data, int line)
{
  int len = strlen(data);
  scx::MemFileBuffer fbuf(len);
  fbuf.get_buffer()->push_from(data,len);
  scx::MemFile mfile(&fbuf);

  scx::ScriptStatement::Ref* root =
    new scx::ScriptStatement::Ref(new scx::ScriptStatementGroup(0));

  // Create a script parser
  scx::ScriptEngine* script = new scx::ScriptEngine(root);
  mfile.add_stream(script);
  
  // Parse statements
  if (script->parse() != scx::End) {
    std::ostringstream oss;
    oss << get_filepath().path() << ":" 
        << (line + script->get_error_line()) << ": Script ";
    switch (script->get_error_type()) {
      case scx::ScriptEngine::Tokenization: oss << "tokenization"; break;
      case scx::ScriptEngine::Syntax: oss << "syntax"; break;
      case scx::ScriptEngine::Underflow: oss << "underflow"; break;
      default: oss << "unknown"; break;
    }
    oss << " error";
    scx::Log("sconesite").submit(oss.str());
  }
  
  return root;
}

//=========================================================================
void XMLDoc::scan_headings(xmlNode* start,int& index)
{
  for (xmlNode* node = start;
       node != 0;
       node = node->next) {
    
    if (node->type == XML_ELEMENT_NODE) {
      std::string name((char*)node->name);
      if (name == "h1" || name == "h2" || name == "h3" ||
          name == "h4" || name == "h5" || name == "h6") {
        if (node->children) {
          std::string text;
          get_node_text(text,node->children);
          int level = atoi(name.substr(1).c_str());
          ++index;
          m_headings.add(level,text,index);
          const Heading* h = m_headings.lookup_index(index);
          node->_private = (void*)h;
        }
      } else {
        scan_headings(node->children,index);
      }
    }
  }
}
