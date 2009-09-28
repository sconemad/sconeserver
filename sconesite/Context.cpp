/* SconeServer (http://www.sconemad.com)

Sconesite Context

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


#include "Context.h"

//=========================================================================
Context::Context()
{

}

//=========================================================================
Context::~Context()
{

}

//=========================================================================
bool Context::handle_doc_start(XMLDoc* doc)
{
  m_doc_stack.push(doc);
  return true;
}

//=========================================================================
bool Context::handle_doc_end(XMLDoc* doc)
{
  DEBUG_ASSERT(m_doc_stack.top() == doc,"Doc stack mismatch");
  m_doc_stack.pop();
  return false;
}

//=========================================================================
std::string Context::name() const
{
  return "context";
}

//=========================================================================
scx::Arg* Context::arg_resolve(const std::string& name)
{
  return SCXBASE ArgObjectInterface::arg_resolve(name);
}

//=========================================================================
scx::Arg* Context::arg_lookup(const std::string& name)
{
  // Methods
  if ("test" == name) {
    return new scx::ArgObjectFunction(new scx::ArgObject(this),name);
  }

  return SCXBASE ArgObjectInterface::arg_lookup(name);
}

//=========================================================================
scx::Arg* Context::arg_function(const scx::Auth& auth,const std::string& name,scx::Arg* args)
{
  scx::ArgList* l = dynamic_cast<scx::ArgList*>(args);

  return SCXBASE ArgObjectInterface::arg_function(auth,name,args);
}

//=========================================================================
XMLDoc* Context::get_current_doc()
{
  if (m_doc_stack.empty()) return 0;
  return m_doc_stack.top();
}
