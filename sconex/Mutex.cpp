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


};
