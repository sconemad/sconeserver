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

#include "ThreadManager.h"
#include "SconesiteModule.h"
#include "WorkerThread.h"

#include "sconex/Descriptor.h"
#include "sconex/Stream.h"

//=============================================================================
ThreadManager::ThreadManager(SconesiteModule& module)
  : m_module(module),
    m_num_threads(0)
{

}

//=============================================================================
ThreadManager::~ThreadManager()
{
  std::list<WorkerJob*>::iterator it = m_jobs.begin();
  while (it != m_jobs.end()) {
    delete (*it);
    it++;
  }
}

//=============================================================================
void ThreadManager::add(WorkerJob* job)
{
  m_job_mutex.lock();
  m_jobs.push_back(job);
  m_job_mutex.unlock();

  //-----
  spin();
  //-----
}

//=============================================================================
int ThreadManager::spin()
{
  check_thread_pool();

  m_job_mutex.lock();
  int num = m_jobs.size();
  WorkerJob* job = 0;
  if (num > 0) {
    std::list<WorkerJob*>::iterator it = m_jobs.begin();
    job = *it;
    m_jobs.erase(it);
  }
  m_job_mutex.unlock();

  if (job == 0) {
    return 0;
  }
  
  allocate_job(job);
  return 1;
}

//=============================================================================
std::string ThreadManager::describe() const
{
  m_job_mutex.lock();

  std::ostringstream oss;
  for (std::list<WorkerJob*>::const_iterator it = m_jobs.begin();
       it != m_jobs.end();
       it++) {
    
    WorkerJob* job = *it;
    oss << job->name() << "\n";
  }

  m_job_mutex.unlock();
  return oss.str();
}

//=============================================================================
void ThreadManager::set_num_threads(unsigned int n)
{
  m_job_mutex.lock();
  m_num_threads = n;
  m_job_mutex.unlock();

  check_thread_pool();
}

//=============================================================================
unsigned int ThreadManager::get_num_threads() const
{
  m_job_mutex.lock();
  unsigned int n = m_num_threads;
  m_job_mutex.unlock();
  return n;
}

//=============================================================================
bool ThreadManager::allocate_job(WorkerJob* job)
{
  m_job_mutex.lock();

  if (m_num_threads == 0) {
    
    // Single threaded mode - run job this thread
    m_module.log("Running '" + job->name() + "' (multiplexed)");
    job->run();
    m_module.log("Finished '" + job->name() + "' (multiplexed)");
    delete job;

  } else {
   
    // Multithreaded mode - wait for a thread to become available and 
    // allocate the job to it
    while (m_threads_pool.empty()) {
      m_job_condition.wait(m_job_mutex);
    }
    
    WorkerThread* wt = m_threads_pool.front();
    m_threads_pool.pop_front();
    m_threads_busy.push_back(wt);

    m_module.log("Running '" + job->name() + "'");
    wt->allocate_job(job);
  }

  m_job_mutex.unlock();
  return true;
}

//=============================================================================
bool ThreadManager::finished_job(WorkerThread* wt, WorkerJob* job)
{
  m_module.log("Finished '" + job->name() + "'");
    
  m_job_mutex.lock();

  int num = m_jobs.size();
  std::ostringstream oss;
  oss << "Now " << num << " jobs in queue";
  m_module.log(oss.str());

  if (m_threads_pool.empty()) {
    m_job_condition.signal();
  }
  
  m_threads_busy.remove(wt);
  m_threads_pool.push_back(wt);

  delete job;

  m_job_mutex.unlock();
  return true;
}

//=============================================================================
void ThreadManager::check_thread_pool()
{
  m_job_mutex.lock();

  unsigned int cur_threads = m_threads_pool.size() + m_threads_busy.size();
  if (cur_threads != m_num_threads) {
    // Thread pool needs resizing
    if (m_num_threads > cur_threads) {
      // Increase size of thread pool by creating more threads
      for (unsigned int i=cur_threads; i<m_num_threads; ++i) {
	m_threads_pool.push_back( new WorkerThread(*this) );
      }
    } else {
      // Decrease size of thread pool if possible, by eliminating idle threads
      unsigned int del = cur_threads - m_num_threads;
      del = std::min(del,(unsigned int)m_threads_pool.size());
      for (unsigned int i=0; i<del; ++i) {
	WorkerThread* dt = m_threads_pool.front();
	m_threads_pool.pop_front();
	delete dt;
      }
    }
  }

  m_job_mutex.unlock();
}
