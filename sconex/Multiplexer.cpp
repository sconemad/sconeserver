/* SconeServer (http://www.sconemad.com)

I/O Multiplexer and event dispatcher

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

#include "sconex/Multiplexer.h"
#include "sconex/Descriptor.h"
#include "sconex/DescriptorThread.h"
#include "sconex/Stream.h"
namespace scx {

//=============================================================================
Multiplexer::Multiplexer()
  : m_num_threads(0)
{
  DEBUG_COUNT_CONSTRUCTOR(Multiplexer);
}

//=============================================================================
Multiplexer::~Multiplexer()
{
  DEBUG_COUNT_DESTRUCTOR(Multiplexer);
  
  std::list<Descriptor*>::iterator it = m_des.begin();
  while (it != m_des.end()) {
    delete (*it);
    it++;
  }
}

//=============================================================================
void Multiplexer::add(Descriptor* d)
{
  m_new_mutex.lock();
  d->m_runstate = Descriptor::Select;
  m_des_new.push_back(d);
  m_new_mutex.unlock();
}

//=============================================================================
int Multiplexer::spin()
{
  fd_set fds_read; FD_ZERO(&fds_read);
  fd_set fds_write; FD_ZERO(&fds_write);
  fd_set fds_except; FD_ZERO(&fds_except);
  std::list<Descriptor*>::iterator it;

  int num_added=0;
  int maxfd=0;

  check_thread_pool();
  
  m_job_mutex.lock();

  // Insert any new descriptors
  m_new_mutex.lock();
  while (!m_des_new.empty()) {
    m_des.push_back(m_des_new.front());
    m_des_new.pop_front();
  }
  m_new_mutex.unlock();
  
  int num = m_des.size();
  bool immediate = false;
 
  for (it = m_des.begin();
       it != m_des.end();
       it++) {
    Descriptor* d = *it;
    if (d->m_runstate == Descriptor::Select ||
        d->m_runstate == Descriptor::Cycle) {

      d->m_runstate = Descriptor::Select;
      int fd = d->fd();
      int mask = d->get_event_mask();

      if (0 != (mask & (1<<Stream::Readable))) {
        FD_SET(fd,&fds_read);
        ++num_added;
      }
      
      if (0 != (mask & (1<<Stream::Writeable))) {
        FD_SET(fd,&fds_write);
        ++num_added;
      }

      if (0 != (mask & (1<<Stream::SendReadable)) ||
	  0 != (mask & (1<<Stream::SendWriteable))) {
	// These events can be sent immediately, so put the
	// select into immediate mode.
	immediate = true;
      }

      maxfd = std::max(maxfd,fd);
    }
  }
  m_job_mutex.unlock();

  if (num == 0) {
    return -1;
  }
  
  if (num_added==0) {
    return 0;
  }

  timeval time;
  if (immediate) {
    // Do a non-blocking select to immediately check if there are
    // any events to send
    time.tv_usec = 0; time.tv_sec = 0;
  } else {
    // Wait upto the specified timeout for an event
    time.tv_usec = 1; time.tv_sec = 0;
  }

  // Select
  num = select(maxfd+1, &fds_read, &fds_write, &fds_except, &time);
  
  if (num < 0) {
    DEBUG_LOG("select failed, errno=" << errno);
    return 0;
  }

  // Dispatch socket events
  for (it = m_des.begin();
       it != m_des.end();
       it++) {
    Descriptor* d = *it;

    m_job_mutex.lock();
    Descriptor::RunState rs = d->m_runstate;
    m_job_mutex.unlock();
    
    if (rs == Descriptor::Select) {
      int fd = d->fd();
      int events = 0;
      events |= (FD_ISSET(fd,&fds_read) ? (1<<Stream::Readable) : 0);
      events |= (FD_ISSET(fd,&fds_write) ? (1<<Stream::Writeable) : 0);
      
      allocate_job(d,events);
    }
  }

  // Purge closed descriptors
  m_job_mutex.lock();
  for (it = m_des.begin();
       it != m_des.end();) {
    Descriptor* d = *it;
    if (d->m_runstate == Descriptor::Purge) {
      it = m_des.erase(it);
      // Descriptor should be closed
      d->close();
      delete d;
    } else {
      it++;
    }
  }
  m_job_mutex.unlock();
  
  return num;
}

//=============================================================================
std::string Multiplexer::describe() const
{
  m_job_mutex.lock();

  std::ostringstream oss;
  for (std::list<Descriptor*>::const_iterator it = m_des.begin();
       it != m_des.end();
       it++) {

    Descriptor* d = *it;

    char c = 'U';
    switch (d->m_runstate) {
      case Descriptor::Select: c = 'S'; break;
      case Descriptor::Run:    c = 'R'; break;
      case Descriptor::Cycle:  c = 'C'; break;
      case Descriptor::Purge:  c = 'X'; break;
    }
    
    oss << "[" << d->uid() << "] " << c << " " << d->describe() << "\n";
    
    std::list<Stream*>::const_iterator its = d->m_streams.begin();
    while (its != d->m_streams.end()) {
      oss << "      " << (*its)->stream_name()
          << " " << (*its)->stream_status() << "\n";
      its++;
    }
  }

  m_job_mutex.unlock();
  return oss.str();
}

//=============================================================================
void Multiplexer::set_num_threads(unsigned int n)
{
  m_job_mutex.lock();
  m_num_threads = n;
  m_job_mutex.unlock();

  check_thread_pool();
}

//=============================================================================
unsigned int Multiplexer::get_num_threads() const
{
  m_job_mutex.lock();
  unsigned int n = m_num_threads;
  m_job_mutex.unlock();
  return n;
}

//=============================================================================
bool Multiplexer::allocate_job(Descriptor* d, int events)
{
  m_job_mutex.lock();

  if (m_num_threads == 0) {
    
    // Single threaded mode - dispatch event in this thread
    d->m_runstate = Descriptor::Run;
    if (d->dispatch(events)) {
      d->m_runstate = Descriptor::Purge;
    } else {
      d->m_runstate = Descriptor::Cycle;
    }

  } else {
   
    // Multithreaded mode - wait for a thread to become available and 
    // allocate the job to it
    while (m_threads_pool.empty()) {
      m_job_condition.wait(m_job_mutex);
    }
    
    DescriptorThread* dt = m_threads_pool.front();
    m_threads_pool.pop_front();
    m_threads_busy.push_back(dt);
    
    d->m_runstate = Descriptor::Run;
    dt->allocate_job(d,events);
    
  }

  m_job_mutex.unlock();
  return true;
}

//=============================================================================
bool Multiplexer::finished_job(DescriptorThread* dt, Descriptor* d, int retval)
{
  m_job_mutex.lock();

  if (m_threads_pool.empty()) {
    m_job_condition.signal();
  }
  
  m_threads_busy.remove(dt);
  m_threads_pool.push_back(dt);

  if (retval) {
    d->m_runstate = Descriptor::Purge;
  } else {
    d->m_runstate = Descriptor::Cycle;
  }

  m_job_mutex.unlock();
  return true;
}

//=============================================================================
void Multiplexer::check_thread_pool()
{
  m_job_mutex.lock();

  unsigned int cur_threads = m_threads_pool.size() + m_threads_busy.size();
  if (cur_threads != m_num_threads) {
    // Thread pool needs resizing
    if (m_num_threads > cur_threads) {
      // Increase size of thread pool by creating more threads
      for (unsigned int i=cur_threads; i<m_num_threads; ++i) {
	m_threads_pool.push_back( new DescriptorThread(*this) );
      }
    } else {
      // Decrease size of thread pool if possible, by eliminating idle threads
      unsigned int del = cur_threads - m_num_threads;
      del = std::min(del,(unsigned int)m_threads_pool.size());
      for (unsigned int i=0; i<del; ++i) {
	DescriptorThread* dt = m_threads_pool.front();
	m_threads_pool.pop_front();
	delete dt;
      }
    }
  }

  m_job_mutex.unlock();
}


};
