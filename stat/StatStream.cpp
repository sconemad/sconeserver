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

#include "sconex/ModuleInterface.h"
#include "sconex/Module.h"
#include "sconex/Stream.h"

//=========================================================================
StatStream::StatStream(StatModule& mod,
		       StatChannel* channel)
  : scx::Stream("stat"),
    m_mod(mod),
    m_channel(channel)
{
  inc_stat("conn",1);
}

//=========================================================================
StatStream::~StatStream()
{

}

//=========================================================================
scx::Condition StatStream::read(void* buffer,int n,int& na)
{
  scx::Condition c = Stream::read(buffer,n,na);
  inc_stat("in",na);
  return c;
}

//=========================================================================
scx::Condition StatStream::write(const void* buffer,int n,int& na)
{
  scx::Condition c = Stream::write(buffer,n,na);
  inc_stat("out",na);
  return c;
}

//=========================================================================
std::string StatStream::stream_status() const
{
  std::ostringstream oss;
  oss << m_channel.object()->get_string();
  oss << " conn:" << m_channel.object()->get_stat("conn");
  oss << " in:" << m_channel.object()->get_stat("in");
  oss << " out:" << m_channel.object()->get_stat("out");
  return oss.str();
}

//=========================================================================
void StatStream::inc_stat(const std::string& type, long value)
{
  m_channel.object()->inc_stat(type,value);
}

