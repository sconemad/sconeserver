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

#include <sconex/Multiplexer.h>
#include <sconex/Descriptor.h>
#include <sconex/Stream.h>

namespace scx {

//=============================================================================
Multiplexer::Multiplexer()
  : m_num_threads(0),
    m_main_thread(pthread_self()),
    m_enable_jobs(true),
    m_loops(0),
    m_jobs_run(0),
    m_job_waits(0),
    m_job_waits_acc(0)
{
  // Create a socketpair for waking up the main thread.
  socketpair(PF_UNIX,SOCK_STREAM,0,m_wakeup);
  fcntl(m_wakeup[0],F_SETFL,fcntl(m_wakeup[0],F_GETFL) | O_NONBLOCK);
  fcntl(m_wakeup[1],F_SETFL,fcntl(m_wakeup[1],F_GETFL) | O_NONBLOCK);
    
  set_latency(1000000);
}

//=============================================================================
Multiplexer::~Multiplexer()
{
  ::close(m_wakeup[0]);
  ::close(m_wakeup[1]);
}

//=============================================================================
JobID Multiplexer::add_job(Job* job)
{
  DEBUG_ASSERT(job!=0,"NULL Job added to kernel");
  JobID id = job->get_id();
  job->m_job_state = Job::Wait;

  m_new_mutex.lock();
  m_jobs_new.push_back(job);
  m_new_mutex.unlock();

  wakeup();
  return id;
}

//=============================================================================
bool Multiplexer::end_job(JobID jobid)
{
  bool found = false;
  Job* job = 0;
  JobList::iterator it;
  m_job_mutex.lock();

  for (it = m_jobs.begin(); it != m_jobs.end(); ++it) {
    job = *it;
    if (job->get_id() == jobid) {
      found = true;
      break;
    }
  }
  
  if (found) {
    while (job->m_job_state == Job::Run) {
      // Job is running, wait for it to finish
      DEBUG_LOG("end_job: Waiting for job to finish");
      m_job_condition.wait(m_job_mutex);
    }
    job->m_job_state = Job::Purge;
  }

  wakeup();

  // Wait for the job to be purged
  while (found) {
    m_end_condition.wait(m_job_mutex);
    found = false;
    for (it = m_jobs.begin(); it != m_jobs.end(); ++it) {
      job = *it;
      if (job->get_id() == jobid) {
	found = true;
	break;
      }
    }
  }

  m_job_mutex.unlock();
  return !found;
}

//=============================================================================
int Multiplexer::spin()
{
  fd_set fds_read; FD_ZERO(&fds_read);
  fd_set fds_write; FD_ZERO(&fds_write);
  fd_set fds_except; FD_ZERO(&fds_except);
  bool immediate = false;

  check_thread_pool();
  
  m_job_mutex.lock();

  // Add any new jobs into the jobs list
  m_new_mutex.lock();
  while (!m_jobs_new.empty()) {
    m_jobs.push_back(m_jobs_new.front());
    m_jobs_new.pop_front();
    immediate = true;
  }
  m_new_mutex.unlock();
  
  // Return and exit if there are no jobs
  int num = m_jobs.size();
  if (num == 0) {
    m_job_mutex.unlock();
    return -1;
  }

  // Find jobs to run
  int num_added=0;
  int maxfd=m_wakeup[0];
  FD_SET(m_wakeup[0],&fds_read); // Add wakeup socket
  JobList::iterator it;
  for (it = m_jobs.begin(); it != m_jobs.end(); ++it) {
    Job* job = *it;
    if (job->m_job_state != Job::Purge) {
      if (job->m_job_state == Job::Cycle) {
	// Recycle job into waiting state
	job->m_job_state = Job::Wait;
      }
      DescriptorJob* djob = dynamic_cast<DescriptorJob*>(job);
      if (djob) {
	Descriptor* d = djob->get_descriptor();
	int mask = djob->get_event_mask();
	int fd = d->fd();
	if (fd >= 0) {
          ++num_added;
          if (job->m_job_state == Job::Wait) {
            // Don't select readable/writable for running jobs
            if (0 != (mask & (1<<Stream::Readable))) FD_SET(fd,&fds_read);
            if (0 != (mask & (1<<Stream::Writeable))) FD_SET(fd,&fds_write);
          }
          FD_SET(fd,&fds_except);
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
	++num_added;
      }
    }
  }

  m_job_mutex.unlock();

  if (num_added == 0) {
    return 0;
  }

  // Determine timeval to use depending on mode
  timeval time;
  if (immediate) {
    // * Immediate mode select *
    // Perform a non-blocking select, this checks for descriptor events and 
    // returns straight away.
    time.tv_usec = 0; time.tv_sec = 0;

  } else {
    // * Normal mode select *
    // Perform a blocking select, returning only when an event occurs, or the 
    // specified timeout is reached, unless we are interrupted by a signal 
    // sent from another thread.
    time.tv_usec = m_latency.tv_usec; time.tv_sec = m_latency.tv_sec;
  }
  
  // Make the select call
  if (select(maxfd+1, &fds_read, &fds_write, &fds_except, &time) < 0) {
    // Select returned an error or it was interrupted
    if (errno != EINTR) {
      DEBUG_LOG_ERRNO("select failed");
    }

  } else {

    Date start = Date::now();
    
    // Select succeeded, decode events and allocate jobs
    for (it = m_jobs.begin(); it != m_jobs.end(); ++it) {
      Job* job = *it;
      Job::JobState js = job->m_job_state;
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
	} else if (!m_enable_jobs) {
	  // Ignore all non-descriptor jobs
	  continue;
	} 
	if (job->should_run()) {
	  // Run the job
	  allocate_job(job);
          ++m_jobs_run;
	}
      }
    }

    update_stats(Date::now() - start);
    
    // Read from the wakeup socket to clear
    if (FD_ISSET(m_wakeup[0],&fds_read)) {
      char buffer[16];
      recv(m_wakeup[0],buffer,16,0);
    }
  }

  // Purge jobs
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

  m_end_condition.signal();
  m_job_mutex.unlock();
    
  return 0;
}

//=============================================================================
void Multiplexer::close()
{
  // Wait for any outstanding jobs to finish, no more will be allocated as the
  // spin() loop has terminated.
  m_job_mutex.lock();
  while (!m_threads_busy.empty()) { 
    DEBUG_LOG("Waiting for " << m_threads_busy.size() << " job(s) to finish");
    m_job_condition.wait(m_job_mutex);
  }
  m_job_mutex.unlock();

  // Stop and remove all threads
  set_num_threads(0);
  DEBUG_ASSERT(m_threads_pool.empty(),"Thread pool is not empty");

  // Delete jobs
  JobList::iterator it;
  for (it = m_jobs.begin(); it != m_jobs.end(); ++it) {
    delete (*it);
  }
  m_jobs.clear();

  // Delete any new jobs
  for (it = m_jobs_new.begin(); it != m_jobs_new.end(); ++it) {
    delete (*it);
  }
  m_jobs_new.clear();
}

//=============================================================================
std::string Multiplexer::describe() const
{
  m_job_mutex.lock();

  std::ostringstream oss;

  oss << " " << m_jobs.size() << " jobs,";
  if (m_num_threads == 0) {
    oss << " multiplexing";
  } else {
    oss << " " << (m_threads_busy.size()+m_threads_pool.size()) << " threads,"
        << " " << m_threads_busy.size() << " running";
  }
  oss << "\n";
  
  oss << " mean wait time: " << m_job_waits << "us\n";
  if (m_num_threads) {
    oss << " mean thread usage: " << m_thread_usage << "%\n";
  }

  oss << "\n";
  
  for (JobList::const_iterator it = m_jobs.begin(); it != m_jobs.end(); ++it) {
    Job* job = *it;

    std::string state="?";
    switch (job->m_job_state) {
      case Job::Wait:  state = "Z"; break;
      case Job::Run:   state = "R"; break;
      case Job::Cycle: state = "C"; break;
      case Job::Purge: state = "X"; break;
    }
    if (!m_enable_jobs && 0 == dynamic_cast<DescriptorJob*>(job)) {
      state = "D";
    }
    
    oss << " {" << job->get_id() << "} " << state 
	<< " " << job->type() 
	<< " " << job->describe() 
	<< "\n";
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
void Multiplexer::set_latency(long latency)
{
  m_latency.tv_usec = latency % 1000000;
  m_latency.tv_sec = latency / 1000000;
}

//=============================================================================
long Multiplexer::get_latency() const
{
  return (m_latency.tv_sec * 1000000) + m_latency.tv_usec;
}

//=============================================================================
void Multiplexer::enable_jobs(bool yesno)
{
  m_enable_jobs = yesno;
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
      if (!m_job_condition.wait_timeout(m_job_mutex,Time(m_latency))) {
        DEBUG_LOG("Timeout allocating new job, adding another thread to the pool");
	m_threads_pool.push_back( new JobThread(*this) );
        ++m_num_threads;
        break;
      }
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

  // Signal that a thread will become available in case anyone is waiting
  m_job_condition.signal();

  // Place the thread back onto the available pool
  m_threads_busy.remove(thread);
  m_threads_pool.push_back(thread);

  if (purge) {
    job->m_job_state = Job::Purge;
  } else {
    job->m_job_state = Job::Cycle;
  }

  m_job_mutex.unlock();

  wakeup();

  return true;
}

//=============================================================================
void Multiplexer::check_thread_pool()
{
  m_job_mutex.lock();

  unsigned int avail_threads = m_threads_pool.size();
  unsigned int busy_threads = m_threads_busy.size();
  m_avail_threads += avail_threads;
  m_busy_threads += busy_threads;
  
  unsigned int cur_threads = avail_threads + busy_threads;
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

//=============================================================================
void Multiplexer::wakeup()
{
  // Wake up the main thread by sending a single byte through the wakeup
  // socketpair - this will wakeup any ongoing select call or cause it to
  // return immediately if it is just about to start.
  char buffer[] = "!";
  send(m_wakeup[1],buffer,1,0);
}

//=============================================================================
void Multiplexer::update_stats(const Time& dispatch_time)
{
  m_job_waits_acc += dispatch_time.to_microseconds();
    
  if (++m_loops >= 100) {
    m_job_waits = m_job_waits_acc / m_jobs_run;
    m_job_waits_acc = 0;
    m_jobs_run = 0;
    
    long total_threads = m_busy_threads + m_avail_threads;
    if (total_threads > 0) {
      m_thread_usage = 100.0 * (double)m_busy_threads / (double)total_threads;
    } else {
      m_thread_usage = 0;
    }
    m_busy_threads = 0;
    m_avail_threads = 0;

    m_loops = 0;
  }
}
  
};
