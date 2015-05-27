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


#include <sconex/Thread.h>
namespace scx {

//=============================================================================
// Static thread startup proc for POSIX threads
static void* thread_run(void* data)
{
  return (void*) ((Thread*)data)->run();
}

//=============================================================================
Thread::Thread()
  : m_mutex(),
    m_wakeup(),
    m_state(Stopped),
    m_thread(),
    m_attr()
{
  DEBUG_COUNT_CONSTRUCTOR(Thread);
}
	
//=============================================================================
Thread::~Thread()
{
  DEBUG_COUNT_DESTRUCTOR(Thread);
}

//=============================================================================
bool Thread::start()
{
  MutexLocker locker(m_mutex);

  if (m_state != Stopped) {
    DEBUG_LOG("start() Thread already running");
    return false;
  }
  
  pthread_attr_init(&m_attr);
  //  pthread_attr_setdetachstate(&m_attr,PTHREAD_CREATE_DETACHED);
  if (pthread_create(
        &m_thread,
        &m_attr,
        thread_run,
        this)) {
    DEBUG_LOG("start() Unable to create thread");
    return false;
  }

  m_state = Running;
  
  return true;
}

//=============================================================================
bool Thread::stop()
{
  if (m_state != Running) {
    // Thread is not running
    return false;
  }

  if (current()) {
    // Called from within the thread, what to do?
    return false;
  }

  // Tell the thread that we're waiting for it to exit
  m_mutex.lock();
  m_state = WaitingExit;
  m_wakeup.signal();
  m_mutex.unlock();

  // Wait until the thread exits
  pthread_join(m_thread,0);
  m_state = Stopped;

  return true;
}

//=============================================================================
bool Thread::running() const
{
  return (m_state == Running);
}

//=============================================================================
bool Thread::current() const
{
  if (m_state != Running) {
    return false;
  }
  
  return pthread_equal(m_thread,pthread_self());
}

//=============================================================================
void Thread::set_priority(int prio)
{
  pthread_setschedprio(m_thread, prio);
}
  
//=============================================================================
bool Thread::should_exit() const
{
  return (m_state == WaitingExit);
}

};
