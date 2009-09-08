/* SconeServer (http://www.sconemad.com)

Sconesite thread manager

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

#ifndef sconesiteThreadManager_h
#define sconesiteThreadManager_h

#include "sconex/sconex.h"
#include "sconex/Mutex.h"

class WorkerJob;
class WorkerThread;
class SconesiteModule;

//=============================================================================
class ThreadManager {

public:

  ThreadManager(SconesiteModule& module);
  virtual ~ThreadManager();

  void add(WorkerJob* job);

  int spin();

  std::string describe() const;
  // Get description of the current descriptors

  void set_num_threads(unsigned int n);
  unsigned int get_num_threads() const;
  // Set/get the number of threads used in the thread pool
  
protected:

private:

  friend class WorkerThread;

  bool allocate_job(WorkerJob* job);
  bool finished_job(WorkerThread* wt, WorkerJob* job);
  
  void check_thread_pool();

  
  SconesiteModule& m_module;

  std::list<WorkerJob*> m_jobs;

  std::list<WorkerThread*> m_threads_pool;
  std::list<WorkerThread*> m_threads_busy;
  unsigned int m_num_threads;
  
  mutable scx::Mutex m_job_mutex;
  scx::ConditionEvent m_job_condition;

};

#endif
