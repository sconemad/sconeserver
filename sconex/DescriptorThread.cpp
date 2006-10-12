/* SconeServer (http://www.sconemad.com)

Descriptor Thread  - A thread for dispatching events to descriptors

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


#include "sconex/DescriptorThread.h"
#include "sconex/Descriptor.h"
#include "sconex/Multiplexer.h"
namespace scx {

//=============================================================================
DescriptorThread::DescriptorThread(Multiplexer& multiplexer)
  : m_multiplexer(multiplexer),
    m_descriptor(0),
    m_events(0)
{
  DEBUG_COUNT_CONSTRUCTOR(DescriptorThread);
  start();
}
	
//=============================================================================
DescriptorThread::~DescriptorThread()
{
  stop();
  DEBUG_COUNT_DESTRUCTOR(DescriptorThread);
}

//=============================================================================
void* DescriptorThread::run()
{
  m_job_mutex.lock();
  
  while (true) {

    while (m_descriptor == 0) {
      // Wait for a job to arrive
      m_job_condition.wait(m_job_mutex);
    }
    
    int retval = m_descriptor->dispatch(m_events);
    
    m_multiplexer.finished_job(this,m_descriptor,retval);

    m_descriptor = 0;
    m_events = 0;
  }
  
  return 0;
}

//=============================================================================
bool DescriptorThread::allocate_job(Descriptor* descriptor, int events)
{
  m_job_mutex.lock();
  
  m_descriptor = descriptor;
  m_events = events;

  m_job_condition.signal();
  m_job_mutex.unlock();

  return true;
}

};
