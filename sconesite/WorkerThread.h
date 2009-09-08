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

#ifndef sconesiteWorkerThread_h
#define sconesiteWorkerThread_h

#include "Template.h"
#include "Context.h"
#include "sconex/Thread.h"

class ThreadManager;
  
//=============================================================================
class WorkerJob {

public:

  WorkerJob(const std::string& name);
  virtual ~WorkerJob();

  virtual void run() =0;

  const std::string& name() const;

private:
  std::string m_name;

};

//=============================================================================
class WorkerThread : public scx::Thread {

public:

  WorkerThread(ThreadManager& manager);
  virtual ~WorkerThread();

  virtual void* run();
  // Thread entry point

  bool allocate_job(WorkerJob* job);

protected:

  ThreadManager& m_manager;

  WorkerJob* m_job;

  scx::Mutex m_job_mutex;
  scx::ConditionEvent m_job_condition;
  
};

#endif
