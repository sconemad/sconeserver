/* SconeServer (http://www.sconemad.com)

Worker Thread - A thread for generating sconesite pages

Copyright (c) 2000-2009 Andrew Wedgbury <wedge@sconemad.com>

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

#include "WorkerThread.h"
#include "ThreadManager.h"

//=============================================================================
WorkerJob::WorkerJob(const std::string& name)
  : m_name(name)
{
  
}

//=============================================================================
WorkerJob::~WorkerJob()
{

}

//=============================================================================
const std::string& WorkerJob::name() const
{
  return m_name;
}

//=============================================================================
WorkerThread::WorkerThread(ThreadManager& manager)
  : m_manager(manager),
    m_job(0)
{
  DEBUG_COUNT_CONSTRUCTOR(WorkerThread);
  start();
}
	
//=============================================================================
WorkerThread::~WorkerThread()
{
  stop();
  DEBUG_COUNT_DESTRUCTOR(WorkerThread);
}

//=============================================================================
void* WorkerThread::run()
{
  m_job_mutex.lock();
  
  while (true) {

    while (m_job == 0) {
      // Wait for a job to arrive
      m_job_condition.wait(m_job_mutex);
    }

    try {
      m_job->run();
    }
    catch (...) {
      DEBUG_LOG("EXCEPTION caught in worker thread");
    }
    
    m_manager.finished_job(this,m_job);

    m_job = 0;
  }
  
  return 0;
}

//=============================================================================
bool WorkerThread::allocate_job(WorkerJob* job)
{
  m_job_mutex.lock();
  
  m_job = job;

  m_job_condition.signal();
  m_job_mutex.unlock();

  return true;
}
