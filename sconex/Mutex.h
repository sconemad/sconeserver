/* SconeServer (http://www.sconemad.com)

Thread synchronisation classes

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

#ifndef scxMutex_h
#define scxMutex_h

#include <sconex/sconex.h>
namespace scx {

class SCONEX_API ConditionEvent;
  
//=============================================================================
// Mutex - A mutual exclusion
//
class SCONEX_API Mutex {
public:

  Mutex();
  ~Mutex();

  bool lock();
  bool try_lock();
  
  bool unlock();

private:

  Mutex(const Mutex& original);
  Mutex& operator=(const Mutex& rhs);
  // Prohibit copy

  friend class ConditionEvent;
  
  pthread_mutex_t m_mutex;
  pthread_mutexattr_t m_attr;
};

//=============================================================================
// MutexLocker - RAII class for Mutex
//
class SCONEX_API MutexLocker {
public:

  MutexLocker(Mutex& mutex, bool start_locked=true);
  ~MutexLocker();

  bool lock();
  bool unlock();

private:

  Mutex& m_mutex;
  bool m_locked;

};

//=============================================================================
// ConditionEvent - For signalling between threads
//
class SCONEX_API ConditionEvent {
public:

  ConditionEvent();
  ~ConditionEvent();

  void wait(Mutex& mutex);

  void signal();
  void broadcast();
  
private:

  ConditionEvent(const ConditionEvent& original);
  ConditionEvent& operator=(const ConditionEvent& rhs);
  // Prohibit copy

  pthread_cond_t m_cond;
};
  
//=============================================================================
// RWLock - A mutex which allows multiple readers but only one writer
//
class SCONEX_API RWLock {
public:

  enum Mode { Read, Write };
  
  RWLock();
  ~RWLock();

  void lock(Mode mode);
  void unlock(Mode mode);

  // Convert read lock to write lock or vice-versa
  void convert(Mode mode);

private:

  RWLock(const RWLock& original);
  RWLock& operator=(const RWLock& rhs);
  // Prohibit copy

  Mutex m_mutex;
  ConditionEvent m_condition;

  unsigned int m_readers;
  bool m_writing;

};
  
//=============================================================================
// RWLocker - RAII class for RWLock
//
class SCONEX_API RWLocker {
public:

  RWLocker(RWLock& rwlock,
	   bool start_locked=true,
	   RWLock::Mode mode=RWLock::Read);
  ~RWLocker();

  bool lock(RWLock::Mode mode=RWLock::Read);
  bool unlock();

private:

  RWLock& m_lock;
  RWLock::Mode m_mode;
  bool m_locked;

};
  
};
#endif
