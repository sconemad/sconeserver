/* SconeServer (http://www.sconemad.com)

Thread

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

#ifndef scxThread_h
#define scxThread_h

#include <sconex/sconex.h>
#include <sconex/Mutex.h>
namespace scx {

//=============================================================================
class SCONEX_API Thread {

public:

  Thread();
  virtual ~Thread();

  bool start();
  // Start the thread
  
  bool stop();
  // Stop the thread
  
  bool running() const;
  // Is this thread running?
  
  bool current() const;
  // Is this the currently running thread?

  virtual void* run() =0;
  // Thread entry point
  
protected:

  bool should_exit() const;

  Mutex m_mutex;
  ConditionEvent m_wakeup;
  
private:

  enum ThreadState { Stopped, Running, WaitingExit };
  ThreadState m_state;
  
  pthread_t m_thread;
  pthread_attr_t m_attr;
};

};
#endif
