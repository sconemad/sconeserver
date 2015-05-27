/* SconeServer (http://www.sconemad.com)

I/O Multiplexer and event dispatcher

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

#include <sconex/Job.h>
#include <sconex/Multiplexer.h>
namespace scx {

JobID Job::s_next_jobid = 1;

//=============================================================================
Job::Job(const std::string& type)
  : m_type(type),
    m_job_state(Wait),
    m_jobid(s_next_jobid++)
{
  DEBUG_COUNT_CONSTRUCTOR(Job);
}

//=============================================================================
Job::~Job()
{
  DEBUG_COUNT_DESTRUCTOR(Job);
}

//=============================================================================
std::string Job::describe() const
{
  return "";
}

//=============================================================================
const std::string& Job::type() const
{
  return m_type;
}

//=============================================================================
JobID Job::get_id() const
{
  return m_jobid;
}

//=============================================================================
Job::JobState Job::get_state() const
{
  return m_job_state;
}


//=============================================================================
PeriodicJob::PeriodicJob(const std::string& type, const Time& period)
  : scx::Job(type),
    m_timeout(0),
    m_period(period)
{
  reset_timeout();
}

//=============================================================================
PeriodicJob::~PeriodicJob()
{

}

//=============================================================================
void PeriodicJob::reset_timeout()
{
  m_timeout = Date::now() + m_period;
}

//=============================================================================
bool PeriodicJob::should_run()
{
  return (Date::now() >= m_timeout);
}

//=============================================================================
std::string PeriodicJob::describe() const
{
  Time left = (m_timeout - Date::now());
  return "p:" + m_period.string() + " t:" + left.string();
}

//=============================================================================
JobThread::JobThread(Multiplexer& manager)
  : m_manager(manager),
    m_job(0)
{
  DEBUG_COUNT_CONSTRUCTOR(JobThread);
  start();
}
	
//=============================================================================
JobThread::~JobThread()
{
  stop();
  DEBUG_COUNT_DESTRUCTOR(JobThread);
}

//=============================================================================
void* JobThread::run()
{
  m_mutex.lock();
  
  while (true) {

    // Wait to be woken up
    m_wakeup.wait(m_mutex);

    if (should_exit()) {
      // Time for the thread to stop
      break;
    }

    if (m_job) {
      // There is a job to run

      bool purge = true;
      
      try {
	// Run the job
	purge = m_job->run();
	
      }
      catch (...) {
	DEBUG_LOG("EXCEPTION caught in job thread");
      }
      
      m_manager.finished_job(this,m_job,purge);
      m_job = 0;
    }
  }
  
  return 0;
}

//=============================================================================
void JobThread::allocate_job(Job* job)
{
  m_mutex.lock();
 
  DEBUG_ASSERT(m_job==0,"JobThread already has a job allocated");
  m_job = job;

  m_wakeup.signal();
  m_mutex.unlock();
}

};
