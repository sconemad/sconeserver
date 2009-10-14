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

//=========================================================================
StatChannel::StatChannel(
  const std::string& name
)
  : m_name(name)
{
  reset();
}

//=========================================================================
StatChannel::~StatChannel()
{

}

//=========================================================================
void StatChannel::add_stats(
  StatType type,
  long count
)
{
  DEBUG_ASSERT(type >= 0 && type < Max,"add_stats() Invalid stat type");

  m_stats[type] += count;
}

//=============================================================================
void StatChannel::reset()
{
  for (int i=0; i<Max; ++i) {
    m_stats[i]=0;
  }
}

//=============================================================================
std::string StatChannel::name() const
{
  return m_name;
}

//=============================================================================
scx::Arg* StatChannel::arg_lookup(
  const std::string& name
)
{
  // Methods

  if ("reset" == name) {
    return new_method(name);
  }

  // Properties
  
  if ("connections" == name) {
    return new scx::ArgInt(m_stats[Connections]);
  }
  if ("input" == name) {
    return new scx::ArgInt(m_stats[BytesInput]);
  }
  if ("output" == name) {
    return new scx::ArgInt(m_stats[BytesOutput]);
  }
  
  return SCXBASE ArgObjectInterface::arg_lookup(name);
}

//=============================================================================
scx::Arg* StatChannel::arg_method(
  const scx::Auth& auth,
  const std::string& name,
  scx::Arg* args
)
{
  if (!auth.admin()) return new scx::ArgError("Not permitted");

  if ("reset" == name) {
    reset();
  }
  
  return SCXBASE ArgObjectInterface::arg_method(auth,name,args);
}
