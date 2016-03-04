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
    m_timeout(0),
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
bool Job::prepare(Date& timeout, int& mask)
{
  switch (m_job_state) {
    case Run:
    case Purge:
      return false;
    case Cycle:
      m_job_state = Wait; // Recycle job into waiting state
    case Wait:
      break;
  }
  
  return true;
}
  
//=============================================================================
int Job::get_fd() { return -1; }

//=============================================================================
bool Job::ready(int events)
{
  return (m_job_state == Wait);
}

//=============================================================================
void Job::run_job()
{
  m_job_state = Job::Run;
  if (run()) {
    m_job_state = Job::Purge;
  } else {
    m_job_state = Job::Cycle;
  }
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
void Job::set_state(JobState state)
{
  m_job_state = state;
}

  
//=============================================================================
PeriodicJob::PeriodicJob(const std::string& type, const Time& period)
  : scx::Job(type),
    m_period(period)
{

}

//=============================================================================
PeriodicJob::~PeriodicJob()
{

}

//=============================================================================
bool PeriodicJob::prepare(Date& timeout, int& mask)
{
  if (!Job::prepare(timeout, mask)) return false;
 
  if (!m_timeout.future()) m_timeout = Date::now() + m_period;
  if (m_timeout < timeout) timeout = m_timeout;

  return true;
}

//=============================================================================
bool PeriodicJob::ready(int events)
{
  if (!Job::ready(events)) return false;
  return (m_timeout.valid() && !m_timeout.future());
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
  while (await_wakeup()) {

    if (!m_job) continue; // Woken up for no reason, hmm...
    
    try {
      // Run the job
      m_job->run_job();
      
    } catch (...) {
      DEBUG_LOG("EXCEPTION caught in job thread");
    }
    
    m_manager.finished_job(this,m_job);
    m_job = 0;

  }
  return 0;
}

//=============================================================================
void JobThread::allocate_job(Job* job)
{
  m_mutex.lock();
  DEBUG_ASSERT(m_job==0,"JobThread already has a job allocated");
  m_job = job;
  m_job->set_state(Job::Run);
  wakeup();
  m_mutex.unlock();
}

};
