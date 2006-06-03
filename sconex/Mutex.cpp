/* SconeServer (http://www.sconemad.com)

Mutual exclusion class for thread synchronisation

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

#include "sconex/Mutex.h"
namespace scx {

//=============================================================================
Mutex::Mutex()
{
  DEBUG_COUNT_CONSTRUCTOR(Mutex);

#ifdef WIN32
  InitializeCriticalSection(&m_mutex);
#else
  pthread_mutexattr_init(&m_attr);
  pthread_mutexattr_settype(&m_attr,PTHREAD_MUTEX_RECURSIVE);
  pthread_mutex_init(&m_mutex,&m_attr);
#endif
}
	
//=============================================================================
Mutex::~Mutex()
{
#ifdef WIN32
  DeleteCriticalSection(&m_mutex);
#else
  pthread_mutex_destroy(&m_mutex);
  pthread_mutexattr_destroy(&m_attr);
#endif

  DEBUG_COUNT_DESTRUCTOR(Mutex);
}

//=============================================================================
bool Mutex::lock()
{
#ifdef WIN32
  EnterCriticalSection(&m_mutex);
  return true;
#else
  return (0 == pthread_mutex_lock(&m_mutex));
#endif
}

//=============================================================================
bool Mutex::try_lock()
{
#ifdef WIN32
  return (0 != TryEnterCriticalSection(&m_mutex));
#else
  return (0 == pthread_mutex_trylock(&m_mutex));
#endif
}

//=============================================================================
bool Mutex::unlock()
{
#ifdef WIN32
  LeaveCriticalSection(&m_mutex);
  return true;
#else
  return (0 == pthread_mutex_unlock(&m_mutex));
#endif
}


//=============================================================================
MutexLocker::MutexLocker(
  Mutex& mutex
)
  : m_mutex(mutex)
{
  m_mutex.lock();
}

//=============================================================================
MutexLocker::~MutexLocker()
{
  m_mutex.unlock();
}


//=============================================================================
ConditionEvent::ConditionEvent()
{
  DEBUG_COUNT_CONSTRUCTOR(ConditionEvent);

#ifdef WIN32

#else
  pthread_cond_init(&m_cond,0);
#endif
}
	
//=============================================================================
ConditionEvent::~ConditionEvent()
{
#ifdef WIN32

#else
  pthread_cond_destroy(&m_cond);
#endif

  DEBUG_COUNT_DESTRUCTOR(ConditionEvent);
}

//=============================================================================
void ConditionEvent::wait(Mutex& mutex)
{
#ifdef WIN32

#else
  pthread_cond_wait(&m_cond,&mutex.m_mutex);
#endif
}

//=============================================================================
void ConditionEvent::signal()
{
#ifdef WIN32

#else
  pthread_cond_signal(&m_cond);
#endif
}

//=============================================================================
void ConditionEvent::broadcast()
{
#ifdef WIN32

#else
  pthread_cond_broadcast(&m_cond);
#endif
}


//=============================================================================
ReaderWriterLock::ReaderWriterLock()
  : m_readers(0),
    m_writing(false)
{
  DEBUG_COUNT_CONSTRUCTOR(ReaderWriterLock);
}
	
//=============================================================================
ReaderWriterLock::~ReaderWriterLock()
{
  DEBUG_COUNT_DESTRUCTOR(ReaderWriterLock);
}

//=============================================================================
void ReaderWriterLock::read_lock()
{
  m_mutex.lock();
  while (m_writing) {
    m_condition.wait(m_mutex);
  }
  ++m_readers;
  m_mutex.unlock();
}

//=============================================================================
void ReaderWriterLock::read_unlock()
{
  m_mutex.lock();
  if (m_readers > 0) {
    --m_readers;
    if (m_readers == 0) {
      m_condition.signal();
    }
  }
  m_mutex.unlock();
}

//=============================================================================
void ReaderWriterLock::write_lock()
{
  m_mutex.lock();
  while (m_writing || m_readers > 0) {
    m_condition.wait(m_mutex);
  }
  m_writing = true;
  m_mutex.unlock();
}

//=============================================================================
void ReaderWriterLock::write_unlock()
{
  m_mutex.lock();
  if (m_writing) {
    m_writing = false;
    m_condition.broadcast();
  }
  m_mutex.unlock();
}

};
