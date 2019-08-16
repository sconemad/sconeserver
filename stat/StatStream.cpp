/* SconeServer (http://www.sconemad.com)

Statistics Stream

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


#include "StatStream.h"
#include "StatModule.h"

#include <sconex/ModuleInterface.h>
#include <sconex/Module.h>
#include <sconex/Stream.h>

//=========================================================================
StatStream::StatStream(StatModule* module,
		       StatChannel* channel)
  : scx::Stream("stat"),
    m_module(module),
    m_channel(channel),
    m_stats(new Stats())
{
  inc_stat(Stats::Connections, 1);
}

//=========================================================================
StatStream::~StatStream()
{

}

//=========================================================================
scx::Condition StatStream::read(void* buffer,int n,int& na)
{
  scx::Condition c = Stream::read(buffer,n,na);
  inc_stat(c == scx::Error ? Stats::Errors : Stats::Reads, 1);
  inc_stat(Stats::BytesRead, na);
  return c;
}

//=========================================================================
scx::Condition StatStream::write(const void* buffer,int n,int& na)
{
  scx::Condition c = Stream::write(buffer,n,na);
  inc_stat(c == scx::Error ? Stats::Errors : Stats::Writes, 1);
  inc_stat(Stats::BytesWritten, na);
  return c;
}

//=========================================================================
std::string StatStream::stream_status() const
{
  std::ostringstream oss;
  oss << m_channel.object()->get_string();
  oss << " r:" << m_stats.object()->get_stat(Stats::Reads) << "(" 
      << m_stats.object()->get_stat(Stats::BytesRead) << ")";
  oss << " w:" << m_stats.object()->get_stat(Stats::Writes) << "(" 
      << m_stats.object()->get_stat(Stats::BytesWritten) << ")";
  oss << " e:" << m_stats.object()->get_stat(Stats::Errors);
  return oss.str();
}

//=========================================================================
void StatStream::inc_stat(Stats::Type type, long value)
{
  if (type >= Stats::StatTypeMax) return;
  m_stats.object()->inc_stat(type, value);
  m_channel.object()->inc_stat(type,value);
}

