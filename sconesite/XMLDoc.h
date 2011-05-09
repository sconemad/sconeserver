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

#include "ArticleBody.h"

#include <sconex/FilePath.h>
#include <sconex/Date.h>
#include <sconex/Mutex.h>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/HTMLtree.h>

typedef std::map<std::string,std::string> XMLAttrs;

bool XMLAttr_bool(XMLAttrs& attrs, const std::string& value, bool def=false);

class Context;

//=========================================================================
// XMLDoc - An article body implementation for XML-based documents, using
// the libxml2 parser.
//
class XMLDoc : public ArticleBody {
public:

  XMLDoc(const std::string& name,
         const scx::FilePath& root,
         const std::string& file);
  
  virtual ~XMLDoc();
  
  virtual const std::string& get_name() const;
  virtual const scx::FilePath& get_root() const;
  virtual const std::string& get_file() const;
  virtual scx::FilePath get_filepath() const;

  virtual bool process(Context& context);

  const scx::Date& get_modtime() const;

  void parse_error(const std::string& msg);

  // Unload the article if it hasn't been accessed since purge_time
  virtual bool purge(const scx::Date& purge_time);
  
  // ScriptObject methods
  virtual std::string get_string() const;

  virtual scx::ScriptRef* script_op(const scx::ScriptAuth& auth,
				    const scx::ScriptRef& ref,
				    const scx::ScriptOp& op,
				    const scx::ScriptRef* right=0);

  typedef scx::ScriptRefTo<XMLDoc> Ref;

  static void get_node_text(std::string& txt, xmlNode* node);

protected:

  virtual void process_node(Context& context, xmlNode* node);
  
  virtual void handle_open();
  virtual void handle_close();
  
  bool open();
  void close();  

  std::string m_name;
  scx::FilePath m_root;
  std::string m_file;
  scx::Date m_modtime;

  xmlDoc* m_xmldoc;
  std::string m_errors;
  
  scx::Date m_last_access;
  int m_clients;
  volatile bool m_opening;

  static scx::Mutex* m_clients_mutex;
};

#endif
