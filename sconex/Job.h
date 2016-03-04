/* SconeServer (http://www.sconemad.com)

Job

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

#ifndef scxJob_h
#define scxJob_h

#include <sconex/sconex.h>
#include <sconex/Thread.h>
#include <sconex/Time.h>
#include <sconex/Date.h>
namespace scx {

class Multiplexer;

typedef int JobID;

//=============================================================================
// A Job is a task to be run by the multiplexer.
class SCONEX_API Job {

public:

  Job(const std::string& type);
  virtual ~Job();

  // Prepare to schedule the job.
  // timeout: The expiry time that will be used when waiting for events.
  //   If the job needs to run before this timeout, then it should be
  //   adjusted accordingly. [in/out]
  // mask: Used to return the event mask, specifying which events to listen
  //   for [out]
  // return: true if the job should be considered for scheduling
  //   (i.e. is not already running).
  virtual bool prepare(Date& timeout, int& mask);

  // Get the file descriptor associated with this job, or -1 if the job has
  // no associated file descriptor.
  virtual int get_fd();

  // Get whether the job is ready to run with the specified events.
  virtual bool ready(int events);
  
  // Run the job (usually launched in JobThread)
  void run_job();

  // Return a description of the job
  virtual std::string describe() const;

  // Return the type of job specified on construction
  const std::string& type() const;

  JobID get_id() const;

  enum JobState { Wait, Run, Cycle, Purge };
  JobState get_state() const;
  
protected:
  std::string m_type;

  friend class Multiplexer;
  friend class JobThread;

  // Implement this to provide the actual task for this job.
  // return: true to remove the job.
  //         false to keep the job.
  virtual bool run() =0;
  
  void set_state(JobState state);
  
  JobState m_job_state;
  Date m_timeout;
  
  JobID m_jobid;
  static JobID s_next_jobid;

};

//=========================================================================
class SCONEX_API PeriodicJob : public Job {

public:

  PeriodicJob(const std::string& type, const Time& period);
  virtual ~PeriodicJob();

  virtual bool prepare(Date& timeout, int& mask);
  virtual bool ready(int events);
  virtual bool run() =0;
  virtual std::string describe() const;

private:
  scx::Time m_period;
};

//=============================================================================
class SCONEX_API JobThread : public Thread {

public:

  JobThread(Multiplexer& manager);
  virtual ~JobThread();

  // Thread entry point
  virtual void* run();

  void allocate_job(Job* job);

private:

  Multiplexer& m_manager;

  Job* m_job;

};

};
#endif
