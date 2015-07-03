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

#ifndef sconesiteXMLDoc_h
#define sconesiteXMLDoc_h

#include "Document.h"

#include <sconex/ScriptStatement.h>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/HTMLtree.h>

bool XMLAttr_bool(NodeAttrs& attrs, const std::string& value, bool def=false);

//=========================================================================
// XMLDoc - An article body implementation for XML-based documents, using
// the libxml2 parser.
//
class XMLDoc : public Document {
public:

  XMLDoc(const std::string& name,
         const scx::FilePath& root,
         const std::string& file);
  
  virtual ~XMLDoc();
  
  typedef scx::ScriptRefTo<XMLDoc> Ref;

  static void get_node_text(std::string& txt, xmlNode* node);

  void report_xml_error(const std::string& error);
  
protected:

  virtual bool is_open() const;
  virtual bool handle_open();
  virtual bool handle_process(Context& context);
  virtual void handle_close();

  void process_node(Context& context, xmlNode* node);

  void scan_scripts(xmlNode* start);
  scx::ScriptStatement::Ref* parse_script(char* data, int line);

  void scan_headings(xmlNode* start, int& index);

  xmlDoc* m_xmldoc;

  typedef std::vector<scx::ScriptStatement::Ref*> Scripts;
  Scripts m_scripts;

};

#endif
