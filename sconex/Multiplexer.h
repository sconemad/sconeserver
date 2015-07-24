/* SconeServer (http://www.sconemad.com)

Job scheduler/multiplexer

Job states:

(New job)
   |
  \ /                   +--> Purge ------> (Delete job)
  Wait -----> Run ------|
  / \                   +--> Cycle ---+
   |                                  |
   +----------------------------------+

A pool of JobThreads is maintained, to which jobs can be allocated 
to run.

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

#ifndef scxMultiplexer_h
#define scxMultiplexer_h

#include <sconex/sconex.h>
#include <sconex/Mutex.h>
#include <sconex/Job.h>
namespace scx {

//=============================================================================
class SCONEX_API Multiplexer {

public:

  Multiplexer();
  virtual ~Multiplexer();

  JobID add_job(Job* job);
  // Add a job

  bool end_job(JobID jobid);
  // End a job

  int spin();
  // select() to determine waiting descriptors and dispatch events

  void close();
  // Shutdown the multiplexer

  std::string describe() const;
  // Get description of the current descriptors

  void set_num_threads(unsigned int n);
  unsigned int get_num_threads() const;
  // Set/get the number of threads used in the thread pool

  void set_latency(long latency);
  long get_latency() const;

  void enable_jobs(bool yesno);

protected:

  // Wakeup the main thread
  void wakeup();

  void update_stats(const Time& dispatch_time);
  
private:

  friend class JobThread;

  bool allocate_job(Job* job);
  bool finished_job(JobThread* thread, Job* job, bool purge);
  
  void check_thread_pool();

  typedef std::list<Job*> JobList;
  JobList m_jobs;
  JobList m_jobs_new;

  typedef std::list<JobThread*> ThreadList;
  ThreadList m_threads_pool;
  ThreadList m_threads_busy;
  unsigned int m_num_threads;
  
  mutable Mutex m_job_mutex;
  ConditionEvent m_job_condition;
  
  Mutex m_new_mutex;
  ConditionEvent m_end_condition;

  int m_wakeup[2];
  
  pthread_t m_main_thread;

  timeval m_latency;
  bool m_enable_jobs;
  
  // Statistics:
  long m_loops;
  long m_jobs_run;
  long m_job_waits;
  long m_job_waits_acc;
  long m_avail_threads;
  long m_busy_threads;
  long m_thread_usage;
};

};
#endif
