/* SconeServer (http://www.sconemad.com)

Descriptor Thread - A thread for dispatching events to descriptors

Copyright (c) 2000-2006 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef scxDescriptorThread_h
#define scxDescriptorThread_h

#include "sconex/sconex.h"
#include "sconex/Thread.h"
namespace scx {

class Descriptor;
class Multiplexer;
  
//=============================================================================
class SCONEX_API DescriptorThread : public Thread {

public:

  DescriptorThread(Multiplexer& multiplexer);
  virtual ~DescriptorThread();

  virtual void* run();
  // Thread entry point

  bool allocate_job(Descriptor* descriptor, int events);
  
protected:

private:

  Multiplexer& m_multiplexer;
  
  Descriptor* m_descriptor;
  int m_events;

  Mutex m_job_mutex;
  ConditionEvent m_job_condition;
  
};

};
#endif
