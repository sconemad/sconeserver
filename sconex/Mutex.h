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

#ifndef scxMutex_h
#define scxMutex_h

#include "sconex/sconex.h"
namespace scx {

class SCONEX_API ConditionEvent;
  
//=============================================================================
class SCONEX_API Mutex {

public:

  Mutex();
  virtual ~Mutex();

  bool lock();
  bool try_lock();
  
  bool unlock();

protected:

private:

  friend class ConditionEvent;
  
  pthread_mutex_t m_mutex;
  pthread_mutexattr_t m_attr;
};

//=============================================================================
class SCONEX_API MutexLocker {

public:

  MutexLocker(Mutex& m_mutex);
  
  ~MutexLocker();

private:

  Mutex& m_mutex;

};

//=============================================================================
class SCONEX_API ConditionEvent {

public:

  ConditionEvent();
  ~ConditionEvent();

  void wait(Mutex& mutex);

  void signal();
  void broadcast();
  
private:

  pthread_cond_t m_cond;
};
  
//=============================================================================
class SCONEX_API ReaderWriterLock {

public:
  
  ReaderWriterLock();
  ~ReaderWriterLock();

  void read_lock();
  void read_unlock();

  void write_lock();
  void write_unlock();

private:

  Mutex m_mutex;
  ConditionEvent m_condition;

  unsigned int m_readers;
  bool m_writing;
};
  
  
};
#endif
