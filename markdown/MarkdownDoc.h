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

#ifndef markdownDoc_h
#define markdownDoc_h

#include <sconesite/Document.h>

#include <sconex/FilePath.h>
#include <sconex/Date.h>
#include <sconex/Mutex.h>

#include <cmark.h>

class Context;

//=========================================================================
// MarkdownDoc - A Markdown document implementation
//
class MarkdownDoc : public Document {
public:

  MarkdownDoc(const std::string& name,
	      const scx::FilePath& path,
	      const std::string& file);
  
  virtual ~MarkdownDoc();
  
  typedef scx::ScriptRefTo<MarkdownDoc> Ref;

  static void get_node_text(std::string& txt, cmark_node* node);
  
protected:

  virtual bool is_open() const;
  virtual bool handle_open();
  virtual bool handle_process(Context& context);
  virtual void handle_close();

  void process_node(Context& context, cmark_node* node);

  void scan_headings(cmark_node* node, int& index);
  
  cmark_node* m_doc;
  int indent;
  
};

#endif
