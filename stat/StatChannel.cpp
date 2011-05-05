/* SconeServer (http://www.sconemad.com)

Statistics Channel

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


#include "StatChannel.h"
#include "StatModule.h"
#include "sconex/ScriptTypes.h"

//=========================================================================
StatChannel::StatChannel(const std::string& name)
  : m_name(name)
{

}

//=========================================================================
StatChannel::~StatChannel()
{

}

//=========================================================================
void StatChannel::inc_stat(const std::string& type, long value)
{
  if (m_stats.count(type) == 0) {
    m_stats[type] = value;
  } else {
    m_stats[type] += value;
  }
}

//=============================================================================
long StatChannel::get_stat(const std::string& type) const
{
  StatMap::const_iterator it = m_stats.find(type);
  if (it != m_stats.end()) {
    return it->second;
  }
  return 0;
}

//=============================================================================
void StatChannel::clear()
{
  m_stats.clear();
}

//=============================================================================
std::string StatChannel::get_string() const
{
  return m_name;
}

//=============================================================================
scx::ScriptRef* StatChannel::script_op(const scx::ScriptAuth& auth,
				       const scx::ScriptRef& ref,
				       const scx::ScriptOp& op,
				       const scx::ScriptRef* right)
{
  if (op.type() == scx::ScriptOp::Lookup) {
    const std::string name = right->object()->get_string();

    // Methods
    if ("clear" == name) {
      return new scx::ScriptMethodRef(ref,name);
    }

    // Properties
    if ("list" == name) {
      scx::ScriptMap* list = new scx::ScriptMap();
      for (StatMap::const_iterator it = m_stats.begin();
	   it != m_stats.end();
	   ++it) {
	list->give(it->first,scx::ScriptInt::new_ref(it->second));
      }
      return new scx::ScriptRef(list);
    }

    StatMap::const_iterator it = m_stats.find(name);
    if (it != m_stats.end()) {
      return scx::ScriptInt::new_ref(it->second);
    }
  }

  return scx::ScriptObject::script_op(auth,ref,op,right);
}

//=============================================================================
scx::ScriptRef* StatChannel::script_method(const scx::ScriptAuth& auth,
					   const scx::ScriptRef& ref,
					   const std::string& name,
					   const scx::ScriptRef* args)
{
  if (!auth.admin()) return scx::ScriptError::new_ref("Not permitted");

  if ("clear" == name) {
    clear();
    return 0;
  }
  
  return scx::ScriptObject::script_method(auth,ref,name,args);
}
