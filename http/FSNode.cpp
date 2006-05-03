/* SconeServer (http://www.sconemad.com)

HTTP filesystem node

Copyright (c) 2000-2004 Andrew Wedgbury <wedge@sconemad.com>

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

#include "http/FSNode.h"
#include "http/FSDirectory.h"
#include "sconex/FilePath.h"
#include "sconex/FileStat.h"
namespace http {

//=============================================================================
FSNode::FSNode(
  Type nodetype, 
  const std::string& name
) : m_type(nodetype),
    m_name(name),
    m_parent(0)
{
  DEBUG_COUNT_CONSTRUCTOR(FSNode);
}
	
//=============================================================================
FSNode::~FSNode()
{
  DEBUG_COUNT_DESTRUCTOR(FSNode);
}

//=============================================================================
FSNode::Type FSNode::type() const
{
  return m_type;
}

//=============================================================================
std::string FSNode::name() const
{
  return m_name;
}

//=============================================================================
std::string FSNode::url() const
{
  const FSDirectory* c = parent();
  if (c) {
    std::string a = c->url();
    if (a=="/") {
      return a + m_name;
    }
    return a + "/" + m_name;
  } 
  return "/";
}

//=============================================================================
scx::FilePath FSNode::path() const
{
  const FSDirectory* c = parent();
  if (c) {
    return c->path() + m_name;
  } 
  return scx::FilePath();
}

//=============================================================================
const FSDirectory* FSNode::parent() const
{
  return m_parent;
}

//=============================================================================
FSDirectory* FSNode::parent()
{
  return m_parent;
}

//=============================================================================
bool FSNode::modified() const
{
  scx::FileStat stat(path());
  return (m_modtime != stat.time());
}

//=============================================================================
void FSNode::set_unmodified()
{
  scx::FileStat stat(path());
  m_modtime = stat.time();
}

//=============================================================================
scx::Arg* FSNode::arg_lookup(const std::string& name)
{
  // Methods
  if ("add" == name) {
    return new scx::ArgObjectFunction(new scx::ArgObject(this),name);
  }

  return SCXBASE ArgObjectInterface::arg_lookup(name);
}

//=============================================================================
scx::Arg* FSNode::arg_resolve(const std::string& name)
{
  scx::Arg* a = SCXBASE ArgObjectInterface::arg_resolve(name);
  if ( m_parent && (a==0 || (dynamic_cast<scx::ArgError*>(a)!=0)) ) {
    delete a;
    return m_parent->arg_resolve(name);
  }
  return a;
}

//=============================================================================
scx::Arg* FSNode::arg_function(
  const std::string& name,
  scx::Arg* args
)
{
  return 0;
}

};
