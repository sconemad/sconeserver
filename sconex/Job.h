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

#include "sconex/sconex.h"
#include "sconex/Thread.h"
#include "sconex/Time.h"
#include "sconex/Date.h"
namespace scx {

class Multiplexer;

typedef int JobID;

//=============================================================================
class SCONEX_API Job {

public:

  Job(const std::string& type);
  virtual ~Job();

  virtual bool should_run() =0;
  // Return to indicate whether the job should run now

  virtual bool run() =0;
  // Run the job (usually launched in JobThread)

  virtual std::string describe() const;
  // Return a description of the job

  const std::string& type() const;
  // Return the type of job specified on construction

  JobID get_id() const;

  enum JobState { Wait, Run, Cycle, Purge };
  JobState get_state() const;
  
private:
  std::string m_type;

  friend class Multiplexer;
  friend class JobThread;

  JobState m_job_state;

  JobID m_jobid;
  static JobID s_next_jobid;

};

//=========================================================================
class SCONEX_API PeriodicJob : public Job {

public:

  PeriodicJob(const std::string& type, const Time& period);
  virtual ~PeriodicJob();

  void reset_timeout();

  virtual bool should_run();
  virtual bool run() =0;
  virtual std::string describe() const;

private:
  scx::Date m_timeout;
  scx::Time m_period;
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

};

};
#endif
