/* SconeServer (http://www.sconemad.com)

Sconesite document processing context

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

#ifndef sconesiteContext_h
#define sconesiteContext_h

#include "XMLDoc.h"
#include "sconex/ScriptBase.h"

//=========================================================================
// Context - A document processing context
//
class Context : public scx::ScriptObject {
public:

  Context();
  ~Context();

  virtual bool handle_doc_start(XMLDoc* doc);

  virtual bool handle_doc_end(XMLDoc* doc);

  virtual bool handle_start(const std::string& name, 
			    XMLAttrs& attrs, 
			    bool empty, 
			    void* data) =0;

  virtual bool handle_end(const std::string& name, 
			  XMLAttrs& attr, 
			  void* data) =0;

  virtual void handle_process(const std::string& name, 
			      const char* data) =0;

  virtual void handle_text(const char* text) =0;

  virtual void handle_comment(const char* text) =0;

  virtual void handle_error(const std::string& msg) =0;

protected:

  XMLDoc* get_current_doc();
  
  typedef std::stack<XMLDoc*> XMLDocStack;
  XMLDocStack m_doc_stack;
  
};

#endif
