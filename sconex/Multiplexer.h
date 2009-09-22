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

#ifndef scxMultiplexer_h
#define scxMultiplexer_h

#include "sconex/sconex.h"
#include "sconex/Thread.h"
namespace scx {

class Multiplexer;

//=============================================================================
class SCONEX_API Job {

public:

  Job(const std::string& type);
  virtual ~Job();

  virtual bool run() =0;
  virtual std::string describe() const;

  const std::string& type() const;

private:
  std::string m_type;

  friend class Multiplexer;
  friend class JobThread;

  enum JobState { Wait, Run, Cycle, Purge };
  JobState m_job_state;

};

//=============================================================================
class SCONEX_API JobThread : public Thread {

public:

  JobThread(Multiplexer& manager);
  virtual ~JobThread();

  virtual void* run();
  // Thread entry point

  void allocate_job(Job* job);

private:

  Multiplexer& m_manager;

  Job* m_job;

  Mutex m_job_mutex;
  ConditionEvent m_job_condition;
  
};

//=============================================================================
class SCONEX_API Multiplexer {

public:

  Multiplexer();
  virtual ~Multiplexer();

  void add(Job* job);
  // Add a job

  int spin();
  // select() to determine waiting descriptors and dispatch events

  std::string describe() const;
  // Get description of the current descriptors

  void set_num_threads(unsigned int n);
  unsigned int get_num_threads() const;
  // Set/get the number of threads used in the thread pool
  
protected:

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

};

};
#endif
