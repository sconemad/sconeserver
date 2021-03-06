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

#include <sconesite/Context.h>
namespace scs {

//=========================================================================
Context::Context()
{

}

//=========================================================================
Context::~Context()
{

}

//=========================================================================
bool Context::handle_doc_start(Document* doc)
{
  m_doc_stack.push(doc);
  return true;
}

//=========================================================================
bool Context::handle_doc_end(Document* doc)
{
  DEBUG_ASSERT(m_doc_stack.top() == doc,"Doc stack mismatch");
  m_doc_stack.pop();
  return false;
}

//=========================================================================
Document* Context::get_current_doc()
{
  if (m_doc_stack.empty()) return 0;
  return m_doc_stack.top();
}

};
