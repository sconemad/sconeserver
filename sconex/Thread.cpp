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


#include "sconex/Thread.h"
namespace scx {

#ifdef WIN32
  
//=============================================================================
// Static thread startup proc for Win32 threads
static DWORD WINAPI thread_run(void* data)
{
  return (DWORD) ((Thread*)data)->run();
}
  
#else
  
//=============================================================================
// Static thread startup proc for POSIX threads
static void* thread_run(void* data)
{
  return (void*) ((Thread*)data)->run();
}

#endif

//=============================================================================
Thread::Thread()
  : m_running(false)
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

  DEBUG_ASSERT(!m_running,"start() Thread already running");
  
#ifdef WIN32
  m_thread = CreateThread( 
    NULL,           // no security attributes 
    0,              // use default stack size  
    thread_run,     // thread function 
    this,           // argument to thread function 
    0,              // use default creation flags 
    &m_thread_id);  // returns the thread identifier
  
  if (m_thread < 0) {
    return false;
  }
#else
  pthread_attr_init(&m_attr);
  pthread_attr_setdetachstate(&m_attr,PTHREAD_CREATE_DETACHED);
  if (pthread_create(
        &m_thread,
        &m_attr,
        thread_run,
        this)) {
    return false;
  }
#endif

  m_running = true;
  
  return true;
}

//=============================================================================
bool Thread::stop()
{
  MutexLocker locker(m_mutex);

  if (!m_running) {
    return false;
  }

#ifdef WIN32
  CloseHandle(m_thread);
#else
  if (current()) {
    pthread_exit(0);
  } else {
    //    pthread_kill(m_thread,SIGKILL);
    pthread_cancel(m_thread);
  }
#endif

  m_running = false;
  
  return true;
}

//=============================================================================
bool Thread::running() const
{
  MutexLocker locker(m_mutex);

  return m_running;
}

//=============================================================================
bool Thread::current() const
{
  MutexLocker locker(m_mutex);

  if (!m_running) {
    return false;
  }
  
#ifdef WIN32
  return (m_thread_id == GetCurrentThreadId());
#else
  return pthread_equal(m_thread,pthread_self());
#endif
}


};
