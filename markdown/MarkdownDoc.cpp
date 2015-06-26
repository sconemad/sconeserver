/* SconeServer (http://www.sconemad.com)

Markdown Document

Copyright (c) 2000-2015 Andrew Wedgbury <wedge@sconemad.com>

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


#include "MarkdownDoc.h"

#include <sconesite/Context.h>
#include <sconex/utils.h>
#include <sconex/File.h>

//=========================================================================
MarkdownDoc::MarkdownDoc(const std::string& name,
			 const scx::FilePath& path,
			 const std::string& file)
  : Document(name, path, file),
    m_doc(0)
{
}

//=========================================================================
MarkdownDoc::~MarkdownDoc()
{
  handle_close();
}

//=========================================================================
void MarkdownDoc::get_node_text(std::string& txt, cmark_node* node)
{
  const char* content = cmark_node_get_literal(node);
  if (content) txt += content;
  for (cmark_node* child = cmark_node_first_child(node);
       child != 0; child = cmark_node_next(child)) {
    get_node_text(txt, child);
  }
}

//=========================================================================
bool MarkdownDoc::is_open() const
{
  return (m_doc != 0);
}

//=========================================================================
bool MarkdownDoc::handle_open()
{
  m_errors = "";
  m_doc = 0;

  scx::File file;
  if (scx::Ok != file.open(get_filepath(),scx::File::Read)) {
    m_errors = "Cannot open file";
    return false;
  }
  
  cmark_parser* parser = cmark_parser_new(CMARK_OPT_DEFAULT);
  char buffer[1024];
  int na = 0;
  while (scx::Ok == file.read(buffer, 1024, na)) {
    cmark_parser_feed(parser, buffer, na);
  }
  m_doc = cmark_parser_finish(parser);
  cmark_parser_free(parser);

  m_headings.clear();
  int index = 0;
  scan_headings(m_doc, index);
  
  return (m_doc != 0);
}

//=========================================================================
bool MarkdownDoc::handle_process(Context& context)
{
  if (context.handle_doc_start(this)) {
    do {
      indent = 0;
      process_node(context, m_doc);
    } while (context.handle_doc_end(this));
  }
  return true;
}

//=========================================================================
void MarkdownDoc::handle_close()
{
  if (m_doc) {
    cmark_node_free(m_doc);
    m_doc = 0;
  }
}

//=========================================================================
void MarkdownDoc::process_node(Context& context, cmark_node* node)
{
  std::string html;
  NodeAttrs attrs;
  const char* text = cmark_node_get_literal(node);
  cmark_node* child = cmark_node_first_child(node);
  void* data = cmark_node_get_user_data(node);
  bool empty = !child && !text;

  int type = cmark_node_get_type(node);
  switch (type) {
    case CMARK_NODE_DOCUMENT: {
      break;
    }

    case CMARK_NODE_BLOCK_QUOTE: {
      html = "blockquote";
      break;
    }
      
    case CMARK_NODE_LIST: {
      int ltype = cmark_node_get_list_type(node);
      switch (ltype) {
        case CMARK_BULLET_LIST: {
          html = "ul";
          break;
        }
        case CMARK_ORDERED_LIST: {
          int start = cmark_node_get_list_start(node);
          std::ostringstream oss; oss << start;
          if (start > 0) attrs["start"] = oss.str();
          html = "ol";
          break;
        }
        default: {
          DEBUG_LOG("Unknown list type: " << ltype);
          break;
        }
      }
      break;
    }
      
    case CMARK_NODE_ITEM: {
      html = "li";
      break;
    }
      
    case CMARK_NODE_CODE_BLOCK: {
      attrs["class"] = cmark_node_get_fence_info(node);
      html = "pre";
      break;
    }
      
    case CMARK_NODE_HTML: {
      break;
    }
      
    case CMARK_NODE_PARAGRAPH: {
      html = "p";
      break;
    }
      
    case CMARK_NODE_HEADER: {
      switch (cmark_node_get_header_level(node)) {
        case 1: html = "h1"; break;
        case 2: html = "h2"; break;
        case 3: html = "h3"; break;
        case 4: html = "h4"; break;
        case 5: html = "h5"; break;
        case 6: html = "h6"; break;
      }
      break;
    }
      
    case CMARK_NODE_HRULE: {
      html = "hr";
      break;
    }

    case CMARK_NODE_TEXT: {
      break;
    }
      
    case CMARK_NODE_SOFTBREAK: {
      text = " ";
      break;
    }
      
    case CMARK_NODE_LINEBREAK: {
      html = "br";
      break;
    }
      
    case CMARK_NODE_CODE: {
      html = "code";
      break;
    }
      
    case CMARK_NODE_INLINE_HTML: {
      break;
    }
      
    case CMARK_NODE_EMPH: {
      html = "em";
      break;
    }
      
    case CMARK_NODE_STRONG: {
      html = "strong";
      break;
    }
      
    case CMARK_NODE_LINK: {
      html = "a";
      attrs["href"] = cmark_node_get_url(node);
      break;
    }
      
    case CMARK_NODE_IMAGE: {
      html = "img";
      attrs["src"] = cmark_node_get_url(node);
      // Extract alt text, if present
      if (child) {
        std::string alt; get_node_text(alt, child);
        attrs["alt"] = alt;
        child = 0; text = 0; empty = true;
      }
      break;
    }

    default: {
      DEBUG_LOG("Unknown node type: " << type);
      break;
    }
  }

  // Start tag
  if (!html.empty()) {
    context.handle_start(html, attrs, empty, data);
  }

  // Debug output node tree
  //  const char* stype = cmark_node_get_type_string(node);
  //  for (int i=0; i<indent; ++i) fprintf(stderr, "  ");
  //  fprintf(stderr, "%s: %s\n", stype, text);

  if (!empty) {

    // Text
    if (text) {
      context.handle_text(text);
    }
    
    // Recurse to child nodes
    ++indent;
    for ( ; child != 0; child = cmark_node_next(child)) {
      process_node(context, child);
    }
    --indent;

    // End tag
    if (!html.empty()) {
      context.handle_end(html, attrs, data);
    }
  }
}

//=========================================================================
void MarkdownDoc::scan_headings(cmark_node* start, int& index)
{
  for (cmark_node* node = cmark_node_first_child(start);
       node != 0; node = cmark_node_next(node)) {

    if (cmark_node_get_type(node) == CMARK_NODE_HEADER) {
      int level = cmark_node_get_header_level(node);
      std::string text; get_node_text(text, node);
      ++index;
      m_headings.add(level, text, index);
      const Heading* h = m_headings.lookup_index(index);
      cmark_node_set_user_data(node, (void*)h);
      
    } else {
      scan_headings(node, index);
    }
  }
}
