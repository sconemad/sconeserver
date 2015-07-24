/* SconeServer (http://www.sconemad.com)

Mutual exclusion class for thread synchronisation

Copyright (c) 2000-2011 Andrew Wedgbury <wedge@sconemad.com>

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

#include <sconex/Mutex.h>
#include <sconex/Time.h>
namespace scx {

//=============================================================================
Mutex::Mutex()
{
  // NOTE: Cannot use DEBUG_COUNT here as the counter itself uses a mutex!
  // DEBUG_COUNT_CONSTRUCTOR(Mutex);

  pthread_mutexattr_init(&m_attr);
  pthread_mutexattr_settype(&m_attr,PTHREAD_MUTEX_RECURSIVE);
  pthread_mutex_init(&m_mutex,&m_attr);
}
	
//=============================================================================
Mutex::~Mutex()
{
  pthread_mutex_destroy(&m_mutex);
  pthread_mutexattr_destroy(&m_attr);

  // DEBUG_COUNT_DESTRUCTOR(Mutex);
}

//=============================================================================
bool Mutex::lock()
{
  return (0 == pthread_mutex_lock(&m_mutex));
}

//=============================================================================
bool Mutex::try_lock()
{
  return (0 == pthread_mutex_trylock(&m_mutex));
}

//=============================================================================
bool Mutex::unlock()
{
  return (0 == pthread_mutex_unlock(&m_mutex));
}


//=============================================================================
MutexLocker::MutexLocker(Mutex& mutex, bool start_locked)
  : m_mutex(mutex),
    m_locked(false)
{
  if (start_locked) lock();
}

//=============================================================================
MutexLocker::~MutexLocker()
{
  if (m_locked) unlock();
}

//=============================================================================
bool MutexLocker::lock()
{
  if (m_mutex.lock()) {
    m_locked = true;
    return true;
  }
  return false;
}

//=============================================================================
bool MutexLocker::unlock()
{
  if (m_mutex.unlock()) {
    m_locked = false;
    return true;
  }
  return false;
}

//=============================================================================
ConditionEvent::ConditionEvent()
{
  DEBUG_COUNT_CONSTRUCTOR(ConditionEvent);

  pthread_cond_init(&m_cond,0);
}
	
//=============================================================================
ConditionEvent::~ConditionEvent()
{
  pthread_cond_destroy(&m_cond);

  DEBUG_COUNT_DESTRUCTOR(ConditionEvent);
}

//=============================================================================
void ConditionEvent::wait(Mutex& mutex)
{
  pthread_cond_wait(&m_cond,&mutex.m_mutex);
}

//=============================================================================
bool ConditionEvent::wait_timeout(Mutex& mutex, const Time& time)
{
  timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  ts.tv_sec += time.seconds();
  ts.tv_nsec += time.microseconds() * 1000;
  while (ts.tv_nsec > 1e9) { ts.tv_nsec -= 1e9; ++ts.tv_sec; }
  return (0 == pthread_cond_timedwait(&m_cond,&mutex.m_mutex,&ts));
}
  
//=============================================================================
void ConditionEvent::signal()
{
  pthread_cond_signal(&m_cond);
}

//=============================================================================
void ConditionEvent::broadcast()
{
  pthread_cond_broadcast(&m_cond);
}


//=============================================================================
RWLock::RWLock()
  : m_readers(0),
    m_writing(false)
{
  DEBUG_COUNT_CONSTRUCTOR(RWLock);
}
	
//=============================================================================
RWLock::~RWLock()
{
  DEBUG_COUNT_DESTRUCTOR(RWLock);
}

//=============================================================================
void RWLock::lock(Mode mode)
{
  m_mutex.lock();

  if (mode == Read) { // Lock reader
    while (m_writing) {
      m_condition.wait(m_mutex);
    }
    ++m_readers;

  } else { // Lock writer
    while (m_writing || m_readers > 0) {
      m_condition.wait(m_mutex);
    }
    m_writing = true;
  }
  m_mutex.unlock();
}

//=============================================================================
void RWLock::unlock(Mode mode)
{
  m_mutex.lock();
  if (mode == Read) { // Unlock reader
    if (m_readers > 0) {
      --m_readers;
      if (m_readers == 0) {
	m_condition.signal();
      }
    }

  } else { // Unlock writer
    if (m_writing) {
      m_writing = false;
      m_condition.broadcast();
    }
  }
  m_mutex.unlock();
}

//=============================================================================
void RWLock::convert(Mode mode)
{
  m_mutex.lock();
  if (mode == Read) {
    if (m_writing) {
      m_writing = false;
      m_condition.broadcast();
    }
    ++m_readers;

  } else {
    if (m_readers > 0) {
      --m_readers;
      if (m_readers == 0) {
	m_condition.signal();
      }
    }
    while (m_writing || m_readers > 0) {
      m_condition.wait(m_mutex);
    }
    m_writing = true;
  }
  m_mutex.unlock();
}

//=============================================================================
RWLocker::RWLocker(RWLock& rrlock,
		   bool start_locked,
		   RWLock::Mode mode)
  : m_lock(rrlock),
    m_mode(mode),
    m_locked(false)
{
  if (start_locked) lock(mode);
}

//=============================================================================
RWLocker::~RWLocker()
{
  if (m_locked) unlock();
}

//=============================================================================
bool RWLocker::lock(RWLock::Mode mode)
{
  if (m_locked) {
    if (m_mode == mode) 
      return false; // Already locked in this mode

    m_lock.convert(mode);

  } else {
    m_lock.lock(mode);
  }

  m_locked = true;
  m_mode = mode;
  return true;
}

//=============================================================================
bool RWLocker::unlock()
{
  if (!m_locked) return false;

  m_lock.unlock(m_mode);
  m_locked = false;
  return true;
}

};
