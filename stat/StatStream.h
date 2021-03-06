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

#ifndef statStream_h
#define statStream_h

#include "StatModule.h"
#include "StatChannel.h"

#include <sconex/Stream.h>
#include <sconex/Module.h>

//=========================================================================
// StatStream - A transparent stream which collects connection and data 
// throughput statistics.
//
class StatStream : public scx::Stream {
public:

  StatStream(StatModule* module, 
	     StatChannel* channel);
  ~StatStream();
  
protected:

  virtual scx::Condition read(void* buffer,int n,int& na);
  virtual scx::Condition write(const void* buffer,int n,int& na);

  virtual std::string stream_status() const;
  
private:

  void inc_stat(Stats::Type type, long value);
  
  StatModule::Ref m_module;
  StatChannel::Ref m_channel;
  Stats::Ref m_stats;
};

#endif
