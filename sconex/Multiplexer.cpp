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

#include "sconex/Multiplexer.h"
#include "sconex/Descriptor.h"
#include "sconex/DescriptorThread.h"
#include "sconex/Stream.h"
namespace scx {

//=============================================================================
Job::Job(const std::string& type)
  : m_type(type)
{
  
}

//=============================================================================
Job::~Job()
{

}

//=============================================================================
std::string Job::describe() const
{
  return "\n";
}

//=============================================================================
const std::string& Job::type() const
{
  return m_type;
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
  m_job_mutex.lock();
  
  while (true) {

    while (m_job == 0) {
      // Wait for a job to arrive
      m_job_condition.wait(m_job_mutex);
    }

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
  
  return 0;
}

//=============================================================================
void JobThread::allocate_job(Job* job)
{
  m_job_mutex.lock();
 
  DEBUG_ASSERT(m_job==0,"JobThread already has a job allocated");
  m_job = job;

  m_job_condition.signal();
  m_job_mutex.unlock();
}


//=============================================================================
Multiplexer::Multiplexer()
  : m_num_threads(0)
{
  DEBUG_COUNT_CONSTRUCTOR(Multiplexer);
}

//=============================================================================
Multiplexer::~Multiplexer()
{
  DEBUG_COUNT_DESTRUCTOR(Multiplexer);

  // Delete jobs
  JobList::iterator it;
  for (it = m_jobs.begin(); it != m_jobs.end(); ++it) {
    delete (*it);
  }
  for (it = m_jobs_new.begin(); it != m_jobs_new.end(); ++it) {
    delete (*it);
  }
}

//=============================================================================
void Multiplexer::add(Job* job)
{
  m_new_mutex.lock();
  job->m_job_state = Job::Wait;
  m_jobs_new.push_back(job);
  m_new_mutex.unlock();
}

//=============================================================================
int Multiplexer::spin()
{
  fd_set fds_read; FD_ZERO(&fds_read);
  fd_set fds_write; FD_ZERO(&fds_write);
  fd_set fds_except; FD_ZERO(&fds_except);

  int num_added=0;
  int maxfd=0;

  check_thread_pool();
  
  m_job_mutex.lock();

  // Add any new jobs into the jobs list
  m_new_mutex.lock();
  while (!m_jobs_new.empty()) {
    m_jobs.push_back(m_jobs_new.front());
    m_jobs_new.pop_front();
  }
  m_new_mutex.unlock();
  
  int num = m_jobs.size();
  bool immediate = false;
 
  JobList::iterator it;
  for (it = m_jobs.begin(); it != m_jobs.end(); ++it) {
    Job* job = *it;
    if (job->m_job_state == Job::Wait || job->m_job_state == Job::Cycle) {
      // Job is waiting to be run
      job->m_job_state = Job::Wait;

      DescriptorJob* djob = dynamic_cast<DescriptorJob*>(job);
      if (djob) {
	Descriptor* d = djob->get_descriptor();
	
	int mask = d->get_event_mask();
	int fd = d->fd();
	if (fd >= 0) {
	  if (0 != (mask & (1<<Stream::Readable))) {
	    FD_SET(fd,&fds_read);
	    ++num_added;
	  }
	  if (0 != (mask & (1<<Stream::Writeable))) {
	    FD_SET(fd,&fds_write);
	    ++num_added;
	  }
	}
	
	if (0 != (mask & (1<<Stream::SendReadable)) ||
	    0 != (mask & (1<<Stream::SendWriteable))) {
	  // These events can be sent immediately, so put the
	  // select into immediate mode.
	  immediate = true;
	  // TODO: This may hog CPU if there is nothing after the
	  // sending stream waiting to read/write. 
	}
	
	maxfd = std::max(maxfd,fd);

      } else {
	// Other kind of job
	immediate = true;
	++num_added;
      }
    }
  }
  m_job_mutex.unlock();

  // Return and exit if there are no jobs
  if (num == 0) {
    return -1;
  }

  // Return if there are no runnable jobs
  if (num_added == 0) {
    return 0;
  }

  timeval time;
  if (immediate) {
    // Do a non-blocking select to immediately check if there are
    // any events to send
    time.tv_usec = 0; time.tv_sec = 0;
  } else {
    // Wait upto the specified timeout for an event
    //    time.tv_usec = 1; time.tv_sec = 0;
    time.tv_usec = 1000; time.tv_sec = 0;
  }

  // Select
  num = select(maxfd+1, &fds_read, &fds_write, &fds_except, &time);

  if (num < 0) {
    DEBUG_LOG("select failed, errno=" << errno);
    return 0;
  }

  // Decode events and allocate dispatch jobs
  for (it = m_jobs.begin(); it != m_jobs.end(); ++it) {
    Job* job = *it;
    
    m_job_mutex.lock();
    Job::JobState js = job->m_job_state;
    m_job_mutex.unlock();

    if (js == Job::Wait) {
      DescriptorJob* djob = dynamic_cast<DescriptorJob*>(job);
      if (djob) {
	Descriptor* d = djob->get_descriptor();
	int fd = d->fd();
	int events = 0;
	
	if (fd >= 0) {
	  events |= (FD_ISSET(fd,&fds_read) ? (1<<Stream::Readable) : 0);
	  events |= (FD_ISSET(fd,&fds_write) ? (1<<Stream::Writeable) : 0);
	}
	
	djob->set_events(events);
      }
      allocate_job(job);
    }
  }

  // Purge closed descriptors
  m_job_mutex.lock();
  for (it = m_jobs.begin(); it != m_jobs.end(); ) {
    Job* job = *it;
    if (job->m_job_state == Job::Purge) {
      it = m_jobs.erase(it);
      delete job;
    } else {
      it++;
    }
  }
  m_job_mutex.unlock();
  
  return num;
}

//=============================================================================
std::string Multiplexer::describe() const
{
  m_job_mutex.lock();

  std::ostringstream oss;

  oss << " " << m_jobs.size() << " jobs /" 
      << " " << (m_threads_busy.size() + m_threads_pool.size()) << " threads /"
      << " " << m_threads_busy.size() << " running"
      << "\n\n";

  for (JobList::const_iterator it = m_jobs.begin(); it != m_jobs.end(); ++it) {
    Job* job = *it;

    std::string state="?";
    switch (job->m_job_state) {
      case Job::Wait:  state = "z"; break;
      case Job::Run:   state = ">"; break;
      case Job::Cycle: state = "z"; break;
      case Job::Purge: state = "!"; break;
    }
    
    oss << " " << state << " " << job->type() << " " << job->describe();
  }

  m_job_mutex.unlock();
  return oss.str();
}

//=============================================================================
void Multiplexer::set_num_threads(unsigned int n)
{
  m_job_mutex.lock();
  m_num_threads = n;
  m_job_mutex.unlock();

  check_thread_pool();
}

//=============================================================================
unsigned int Multiplexer::get_num_threads() const
{
  m_job_mutex.lock();
  unsigned int n = m_num_threads;
  m_job_mutex.unlock();
  return n;
}

//=============================================================================
bool Multiplexer::allocate_job(Job* job)
{
  m_job_mutex.lock();

  if (m_num_threads == 0) {
    
    // Single threaded mode - run job in this thread
    job->m_job_state = Job::Run;
    if (job->run()) {
      job->m_job_state = Job::Purge;
    } else {
      job->m_job_state = Job::Cycle;
    }

  } else {
   
    // Multithreaded mode - wait for a thread to become available and 
    // allocate the job to it
    while (m_threads_pool.empty()) {
      m_job_condition.wait(m_job_mutex);
    }
    
    JobThread* thread = m_threads_pool.front();
    m_threads_pool.pop_front();
    m_threads_busy.push_back(thread);
    
    job->m_job_state = Job::Run;
    thread->allocate_job(job);
    
  }

  m_job_mutex.unlock();
  return true;
}

//=============================================================================
bool Multiplexer::finished_job(JobThread* thread, Job* job, bool purge)
{
  m_job_mutex.lock();

  // If the thread pool is empty, signal that a thread will now become 
  // available in case anyone is waiting
  if (m_threads_pool.empty()) {
    m_job_condition.signal();
  }

  // Place the thread back onto the available pool
  m_threads_busy.remove(thread);
  m_threads_pool.push_back(thread);

  if (purge) {
    job->m_job_state = Job::Purge;
  } else {
    job->m_job_state = Job::Cycle;
  }

  m_job_mutex.unlock();
  return true;
}

//=============================================================================
void Multiplexer::check_thread_pool()
{
  m_job_mutex.lock();

  unsigned int cur_threads = m_threads_pool.size() + m_threads_busy.size();
  if (cur_threads != m_num_threads) {
    // Thread pool needs resizing
    if (m_num_threads > cur_threads) {
      // Increase size of thread pool by creating more threads
      for (unsigned int i=cur_threads; i<m_num_threads; ++i) {
	m_threads_pool.push_back( new JobThread(*this) );
      }
    } else {
      // Decrease size of thread pool if possible, by eliminating idle threads
      unsigned int del = cur_threads - m_num_threads;
      del = std::min(del,(unsigned int)m_threads_pool.size());
      for (unsigned int i=0; i<del; ++i) {
	JobThread* thread = m_threads_pool.front();
	m_threads_pool.pop_front();
	delete thread;
      }
    }
  }

  m_job_mutex.unlock();
}


};
