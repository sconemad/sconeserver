/* SconeServer (http://www.sconemad.com)

SconeX module reference

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

#include "sconex/ModuleRef.h"
#include "sconex/Module.h"

namespace scx {

//=============================================================================
ModuleRef::ModuleRef()
{
  m_module = 0;
}

//=============================================================================
ModuleRef::ModuleRef(Module* module)
  : m_module(module)
{
  if (m_module) {
    m_module->add_ref();
  }
}

//=============================================================================
ModuleRef::ModuleRef(const ModuleRef& c)
  : m_module(c.m_module)
{
  if (m_module) {
    m_module->add_ref();
  }
}

//=============================================================================
ModuleRef& ModuleRef::operator=(const ModuleRef& c)
{
  if (m_module) {
    m_module->remove_ref();
  }
  m_module = c.m_module;
  if (m_module) {
    m_module->add_ref();
  }
  return (*this);
}

//=============================================================================
ModuleRef::~ModuleRef()
{
  if (m_module) {
    m_module->remove_ref();
  }
}

//=============================================================================
Module* ModuleRef::module()
{
  return m_module;
}

//=============================================================================
const Module* ModuleRef::module() const
{
  return m_module;
}

//=============================================================================
bool ModuleRef::valid() const
{
  return (m_module!=0);
}

//=============================================================================
void ModuleRef::release()
{
  if (m_module) {
    m_module->remove_ref();
  }
  m_module=0;
}


};
