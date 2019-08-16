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
#include <sconex/ScriptTypes.h>


static const char* statTypeNames[] =
  {"connections", "reads", "writes", "errors", "bytesRead", "bytesWritten"};

//=========================================================================
const char* Stats::stat_name(Type type)
{
  if (type >= StatTypeMax) return "";
  return statTypeNames[type];
}

//=========================================================================
Stats::Stats()
  : m_stats((int)StatTypeMax, 0)
{

}

//=========================================================================
void Stats::inc_stat(Type type, long value)
{
  if (type >= StatTypeMax) return;
  m_stats[type] += value;
}

//=============================================================================
long Stats::get_stat(Type type) const
{
  if (type >= StatTypeMax) return 0;
  return m_stats[type];
}

//=============================================================================
void Stats::clear()
{
  for (int i=0; i<StatTypeMax; ++i) m_stats[i] = 0;
}

//=============================================================================
std::string Stats::get_string() const
{
  return "stats";
}

//=============================================================================
scx::ScriptRef* Stats::script_op(const scx::ScriptAuth& auth,
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
    for (int t=0; t<StatTypeMax; ++t) {
      if (name == stat_name((Type)t)) {
	return scx::ScriptInt::new_ref(m_stats[t]);
      }
    }
  }
  
  return scx::ScriptObject::script_op(auth,ref,op,right);
}

//=============================================================================
scx::ScriptRef* Stats::script_method(const scx::ScriptAuth& auth,
				     const scx::ScriptRef& ref,
				     const std::string& name,
				     const scx::ScriptRef* args)
{
  if (!auth.admin()) return scx::ScriptError::new_ref("Not permitted");

  if ("clear" == name) {
    // Should check non-const ref
    clear();
    return 0;
  }
  
  return scx::ScriptObject::script_method(auth,ref,name,args);
}

// --- StatChannel ---


//=========================================================================
StatChannel::StatChannel(StatModule* module, const std::string& name)
  : m_module(module),
    m_name(name),
    m_stats(new Stats())
{

}

//=========================================================================
StatChannel::~StatChannel()
{

}

//=============================================================================
const Stats* StatChannel::get_stats() const
{
  return m_stats.object();
}

//=============================================================================
void StatChannel::inc_stat(Stats::Type type, long value)
{
  m_stats.object()->inc_stat(type, value);
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
    if ("totals" == name) {
      return m_stats.ref_copy(ref.reftype());
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
    m_stats.object()->clear();
    return 0;
  }
  
  return scx::ScriptObject::script_method(auth,ref,name,args);
}
