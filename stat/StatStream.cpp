/* SconeServer (http://www.sconemad.com)

Statistics Stream

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


#include "StatStream.h"
#include "StatModule.h"

#include "sconex/ModuleInterface.h"
#include "sconex/Module.h"
#include "sconex/Stream.h"

//=========================================================================
StatStream::StatStream(
  StatModule& mod,
  const std::string& channel
)
  : scx::Stream("stat"),
    m_mod(mod),
    m_channel(channel)
{
  add_stats(StatChannel::Connections,1);
}

//=========================================================================
StatStream::~StatStream()
{

}

//=========================================================================
scx::Condition StatStream::read(void* buffer,int n,int& na)
{
  scx::Condition c = Stream::read(buffer,n,na);
  add_stats(StatChannel::BytesInput,na);
  return c;
}

//=========================================================================
scx::Condition StatStream::write(const void* buffer,int n,int& na)
{
  scx::Condition c = Stream::write(buffer,n,na);
  add_stats(StatChannel::BytesOutput,na);
  return c;
}

//=========================================================================
std::string StatStream::stream_status() const
{
  std::ostringstream oss;
  oss << m_channel;

  StatStream* unc = const_cast<StatStream*>(this);
  StatChannel* channel = unc->m_mod.find_channel(m_channel);
  if (channel) {
    scx::Arg* a = channel->arg_lookup("connections");
    oss << " con:" << (a ? a->get_string() : "?");
    delete a;

    a = channel->arg_lookup("input");
    oss << " r:" << (a ? a->get_string() : "?");
    delete a;

    a = channel->arg_lookup("output");
    oss << " w:" << (a ? a->get_string() : "?");
    delete a;
  }
  
  return oss.str();
}

//=========================================================================
void StatStream::add_stats(StatChannel::StatType type,long count)
{
  m_mod.add_stats(m_channel,type,count);
}

