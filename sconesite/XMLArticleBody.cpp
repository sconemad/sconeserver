/* SconeServer (http://www.sconemad.com)

Sconesite article body in XML format

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

#include "XMLArticleBody.h"

//=========================================================================
XMLArticleBody::XMLArticleBody(const std::string& name,
                               const scx::FilePath& path
) : XMLDoc(name,path,"article.xml")
{


}

//=========================================================================
XMLArticleBody::~XMLArticleBody()
{

}

//=========================================================================
void XMLArticleBody::handle_open()
{
  m_headings.clear();
  int index = 0;
  scan_headings(xmlDocGetRootElement(m_xmldoc),index);
}

//=========================================================================
void XMLArticleBody::handle_close()
{

}

//=========================================================================
void XMLArticleBody::scan_headings(xmlNode* start,int& index)
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
          const ArticleHeading* h = m_headings.lookup_index(index);
          node->_private = (void*)h;
        }
      } else {
        scan_headings(node->children,index);
      }
    }
  }
}
